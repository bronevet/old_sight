// Licence information included in file LICENCE
#include "sight_common_internal.h"
#include "sight_structure_internal.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include "binreloc.h"
#include <errno.h>
#include "getAllHostnames.h" 
#include "utils.h"
#include "fdstream.h"
#include "process.h"
#include "process.C"
using namespace std;
using namespace sight::common;

// Support for text diffs
#include <dtl/dtl.hpp>
using namespace std;
using namespace dtl;
using dtl::Diff;

namespace sight {
namespace structure{

// Merges the two strings into one
string mergeText(const string& a, const string& b) {
  typedef char   elem;
  typedef string sequence;
  
  Diff< elem, sequence > d(a, b);
  d.compose();
  
  vector< pair< char, elemInfo > > ses_v = d.getSes().getSequence();
  string merged;
  for(int i=0; i<ses_v.size(); i++)
    merged += ses_v[i].first;
  
  return merged;
}

/***************
 ***** dbg *****
 ***************/

dbgStream dbg;

bool initializedDebug=false;

void SightInit_internal(int argc, char** argv, string title, string workDir);

// Records the information needed to call the application
bool saved_appExecInfo=false; // Indicates whether the application execution info has been saved
int saved_argc = 0;
char** saved_argv = NULL;
char* saved_execFile = NULL;

// The unique ID of this process' output stream
int outputStreamID=0;

// Provides the output directory and run title as well as the arguments that are required to rerun the application
void SightInit(int argc, char** argv, string title, string workDir)
{
  if(initializedDebug) return;
  SightInit_internal(argc, argv, title, workDir);
}

// Provides the output directory and run title as well but does not provide enough info to rerun the application
void SightInit(string title, string workDir)
{
  if(initializedDebug) return; 
  SightInit_internal(0, NULL, title, workDir);
}

// Low-level initialization of Sight that sets up some basics but does not allow users to use the dbg
// output stream to emit their debug output, meaning that other widgets cannot be used directly.
// Users that using this type of initialization must create their own dbgStreams and emit enter/exit
// tags to these streams directly without using any of the high-level APIs.
void SightInit_LowLevel()
{
  loadSightConfig(configFileEnvVars("SIGHT_STRUCTURE_CONFIG", "SIGHT_CONFIG"));
  
  initializedDebug = true;
}

void SightInit_internal(int argc, char** argv, string title, string workDir)
{
  map<string, string> newProps;
  
  loadSightConfig(configFileEnvVars("SIGHT_STRUCTURE_CONFIG", "SIGHT_CONFIG"));
  
  newProps["title"] = title;
  newProps["workDir"] = workDir;
  // Records whether we know the application's command line, which would enable us to call it
  newProps["commandLineKnown"] = (argv!=NULL? "1": "0");
  // If the command line is known, record it in newProps
  if(argv!=NULL) {
    newProps["argc"] = txt()<<argc;
    for(int i=0; i<argc; i++)
      newProps[txt()<<"argv_"<<i] = string(argv[i]);

    BrInitError error;
    int ret = br_init(&error);
    if(!ret) { 
      cerr << "ERROR reading application's executable name! "<<
                   (error==BR_INIT_ERROR_NOMEM?     "Cannot allocate memory." :
                   (error==BR_INIT_ERROR_OPEN_MAPS? "Cannot open /proc/self/maps "+string(strerror(errno)) :
                   (error==BR_INIT_ERROR_READ_MAPS? "The file format of /proc/self/maps is invalid; kernel bug?" :
                   (error==BR_INIT_ERROR_DISABLED?  "BinReloc is disabled (the ENABLE_BINRELOC macro is not defined)" :
                    "???"
              ))))<<endl;
      assert(0);
    }
    char* execFile = br_find_exe(NULL);
    if(execFile==NULL) { cerr << "ERROR reading application's executable name after successful initialization!"<<endl; assert(0); }
    newProps["execFile"] = execFile;
    
    char cwd[FILENAME_MAX];
    getcwd(cwd, FILENAME_MAX);
    newProps["cwd"] = cwd;
   
    // Record the names and values of all the environment variables
    char** e = environ;
    int numEnvVars=0;
    while(*e) {
      string var = *e;
      int splitPoint = var.find("=");
      //cout << *e << ": " << var.substr(0, splitPoint) << " => "<<var.substr(splitPoint+1)<<endl;
      newProps[txt()<<"envName_"<<numEnvVars] = var.substr(0, splitPoint);
      newProps[txt()<<"envVal_"<<numEnvVars] = var.substr(splitPoint+1);
      numEnvVars++; 
      e++;
    }
    newProps["numEnvVars"] = txt()<<numEnvVars;
  }
  
  // Get all the aliases of the current host's name
  //char hostname[10000]; // The name of the host that this application is currently executing on
  //int ret = gethostname(hostname, 10000);
  list<string> hostnames = getAllHostnames();
  newProps["numHostnames"] = txt()<<hostnames.size();
  { int i=0;
    for(list<string>::iterator h=hostnames.begin(); h!=hostnames.end(); h++, i++)
    newProps[txt()<<"hostname_"<<i] = *h;
  }

  // Get the current user's username, using the environment if possible
  char username[10000]; // The current user's user name 
  if(getenv("USER")) 
    strncpy(username, getenv("USER"), sizeof(username));
  else if(getenv("USERNAME"))
    strncpy(username, getenv("USERNAME"), sizeof(username));
  else {
    // If the username is not available from the environment, use the id command
    ostringstream cmd;
    cmd << "id --user --name";
    FILE* fp = popen(cmd.str().c_str(), "r");
    if(fp == NULL) { cerr << "Failed to run command \""<<cmd.str()<<"\"!"<<endl; assert(0); }
    
    if(fgets(username, sizeof(username), fp) == NULL) { cerr << "Failed to read output of \""<<cmd.str()<<"\"!"<<endl; assert(0); }
    pclose(fp);
  }
  newProps["username"] = string(username);

  // Set the unique ID of this process' output stream
  outputStreamID = getpid();
  newProps["outputStreamID"] = txt()<<outputStreamID;
  
  properties* props = new properties();
  props->add("sight", newProps);
  
  // Create the directory structure for the structural information
  // Main output directory
  createDir(workDir, "");

  // Directory where client-generated images will go
  string imgDir = createDir(workDir, "html/dbg_imgs");
  
  // Directory that widgets can use as temporary scratch space
  string tmpDir = createDir(workDir, "html/tmp");
  
  initializedDebug = true;
  
  dbg.init(props, title, workDir, imgDir, tmpDir);
}

void SightInit_internal(properties* props, bool storeProps)
{
  properties::iterator sightIt = props->find("sight");
  assert(!sightIt.isEnd());
  
  // Create the directory structure for the structural information
  // Main output directory
  createDir(properties::get(sightIt, "workDir"), "");

  // Directory where client-generated images will go
  string imgDir = createDir(properties::get(sightIt, "workDir"), "html/dbg_imgs");
  
  // Directory that widgets can use as temporary scratch space
  string tmpDir = createDir(properties::get(sightIt, "workDir"), "html/tmp");
  
  initializedDebug = true;
  
  dbg.init(storeProps? props: NULL, properties::get(sightIt, "title"), properties::get(sightIt, "workDir"), imgDir, tmpDir);
}

// Creates a new dbgStream based on the given properties of a "sight" tag and returns a pointer to it.
// storeProps: if true, the given properties object is emitted into this dbgStream's output file
structure::dbgStream* createDbgStream(properties* props, bool storeProps) {
  properties::iterator sightIt = props->find("sight");
  assert(!sightIt.isEnd());
  
  // Create the directory structure for the structural information
  // Main output directory
  createDir(properties::get(sightIt, "workDir"), "");

  // Directory where client-generated images will go
  string imgDir = createDir(properties::get(sightIt, "workDir"), "html/dbg_imgs");
  
  // Directory that widgets can use as temporary scratch space
  string tmpDir = createDir(properties::get(sightIt, "workDir"), "html/tmp");
  
  return new dbgStream(storeProps? props: NULL, properties::get(sightIt, "title"), properties::get(sightIt, "workDir"), imgDir, tmpDir);
}

// Empty implementation of Sight initialization, to be used when Sight is disabled via #define DISABLE_SIGHT
void NullSightInit(std::string title, std::string workDir) {}
void NullSightInit(int argc, char** argv, std::string title, std::string workDir) {}

/**********************
 ***** Call Paths *****
 **********************/

CallpathRuntime CPRuntime;
string cp2str(const Callpath& cp) {
  ostringstream s;
  //const_cast<Callpath&>(cp).write_out(s);
  //Callpath cp2 = cp;
  //cp2.write_out(s);
  s << cp;
  //cout << "callpath: "<<s.str()<<endl<<cp<<endl;
  return s.str();
}

/*Callpath str2cp(string str) {
  istringstream s(str);
  return Callpath::read_in(s);
}*/

/********************
 ***** location *****
 ********************/

location::location()
{
  // Initialize fileLevel with a 0 to make it possible to count top-level files
  l.push_back(make_pair(0, list<int>(1, 0)));
}

location::~location()
{
  assert(l.size()==1);
  l.pop_back();
}

void location::enterFileBlock() {
  assert(l.size()>0);
  
  // Increment the index of this file unit within the current nesting level
  (l.back().first)++;
  // Add a fresh file level to the location
  l.push_back(make_pair(0, list<int>(1, 0)));
  l.push_back(make_pair(0, list<int>()));
}

void location::exitFileBlock() {
  assert(l.size()>0);
  l.pop_back();
}

void location::enterBlock() {
  assert(l.size()>0);
  assert(l.back().second.size()>0);

  // Increment the index of this block unit within the current nesting level of this file
  l.back().second.back()++;
  // Add a new level to the block list, starting the index at 0
  l.back().second.push_back(0);
}
void location::exitBlock() {
  assert(l.size()>0);
  assert(l.back().second.size()>0);
  l.back().second.pop_back();
}

void location::operator=(const location& that)
{ l = that.l; }

bool location::operator==(const location& that) const
{ return l==that.l; }

bool location::operator!=(const location& that) const
{ return l!=that.l; }

bool location::operator<(const location& that) const
{ return l<that.l; }

//void location::print(std::ofstream& ofs) const {
std::string location::str(std::string indent) const {
  ostringstream ofs;
  ofs << "[location: "<<endl;
  for(std::list<std::pair<int, std::list<int> > >::const_iterator i=l.begin(); i!=l.end(); i++) {
    ofs << "    "<<i->first<<" :";
    for(std::list<int>::const_iterator j=i->second.begin(); j!=i->second.end(); j++) {
      ofs << " "<<*j;
    }
    ofs << endl;
  }
  ofs << "]";
  return ofs.str();
}

/********************
 ***** sightObj *****
 ********************/
// The stack of sightObjs that are currently in scope. We declare it as a static inside this function
// to make it possible to ensure that the global soStack is constructed before it is used and therefore
// destructed after all of its user objects are destructed.
// There is a separate stack for each outgoing stream.
std::map<dbgStream*, std::list<sightObj*> > staticSoStack;
std::list<sightObj*>& soStack(dbgStream* outStream) {
  return staticSoStack[outStream];
}

std::map<dbgStream*, std::list<sightObj*> >& soStackAllStreams() {
  return staticSoStack;
}

// Records whether we've begun the process of destroying all objects. This is to differentiate`
// the case of Sight doing the destruction and of the runtime system destroying all objects
// at process termination
bool sightObj::SightDestruction=false;

// Records whether we've started processing static destructors. In this case
// the sight object stack may not be valid anymore and thus should not be accessed.
bool sightObj::stackMayBeInvalidFlag=false;

// The of clocks currently being used, mapping the name of each clock class to the set of active 
// clock objects of this class.
std::map<std::string, std::set<sightClock*> > sightObj::clocks;

sightObj::sightObj(dbgStream* outStream) : 
    props(NULL), emitExitTag(false), destroyed(false), outStream(outStream?outStream:&structure::dbg) {
  // Push this sightObj onto the stack
  //if(initializedDebug) soStack.push_back(this);
  // Don't push this sightObj onto the stack because emitExitTag is initialized to false
  //soStack(this->outStream);
}

// isTag - if true, we emit a single enter/exit tag combo in the constructor and nothing in the destructor
sightObj::sightObj(properties* props, bool isTag, dbgStream* outStream) : 
      props(props), destroyed(false), outStream(outStream?outStream:&structure::dbg) {
  init(props, isTag);
}

void sightObj::init(properties* props, bool isTag) {
  assert(outStream);
//  cout << "sightObj::sightObj isTag="<<isTag<<" props="<<(props? props->str(): "NULL")<<endl;
  if(props && props->active && props->emitTag) {
    // Add the properties of any clocks associated with this sightObj
    for(map<string, set<sightClock*> >::iterator i=clocks.begin(); i!=clocks.end(); i++) {
      for(set<sightClock*>::iterator j=i->second.begin(); j!=i->second.end(); j++)
        // If the value of the current clock was modified since the last time we observed it
        //if((*j)->modified())
        (*j)->setProperties(props);
    }
    
    if(isTag) {
      outStream->tag(this);
      emitExitTag = false;
    } else {
      outStream->enter(this);
      emitExitTag = true;
    }
  } else
    emitExitTag = false;
  
  // Push this sightObj onto the stack
  if(initializedDebug && emitExitTag) {
//    cout << "[[[ "<<(props? props->str(): "NULL")<<endl;
    soStack(outStream).push_back(this);
  }
  
  // If Sight was not initialized at the time this object was created, record that
  // this object does not emit an exit tag since this object should be left invisible to Sight.
  if(!initializedDebug) emitExitTag = false;
  
  // Initially removeFromStack is identical to emitExitTag but if we call emitTag()
  // before the object is destructed, it will become true while emitExitTag is set to false.
  removeFromStack = emitExitTag;
}

// Emits this sightObj's exit tag to the outgoing stream
// If popStack==true, remove this sightObj from the stack of objects on its dbgStream.
// It will be set to false inside the dbgStream constructor since 1. its redundant and 
// 2. if the dbgStream is the static one, the stack may have been destructed before the dbgStream's destructor
void sightObj::exitTag(bool popStack) {
  assert(outStream);
  if(!emitExitTag) return;
  
  outStream->exit(this);
  
  // Pop this sightObj off the top of the stack
  // The stack is empty when we're trying to destroy global/static sightObjs created before Sight was initialized
  if(popStack)
    if(soStack(this->outStream).size()>0) {
      // Remove this object from the stack
      assert(soStack(this->outStream).back() == this);
      soStack(this->outStream).pop_back();
    }
  
  emitExitTag = false;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void sightObj::destroy() {
  this->~sightObj();
}

sightObj::~sightObj() {
  //assert(!destroyed);
  if(destroyed) return;
  assert(outStream);
  
/*  if(soStack(outStream).size()>0 && emitExitTag) {
    cout << "]]](#"<<soStack(outStream).size()<<") props="<<(props? props->str(): "NULL")<<endl;
    cout << "soStack(outStream).back()="<<(soStack(outStream).back()->props? soStack(outStream).back()->props->str(): "NULL")<<endl;
  }*/
  
  // If the application calls to exit(), this will call the destructors of all the static objects
  // but not those on the stack or heap. As such, it is possible to reach the destructor of an object
  // in the middle of the soStack without calling the destructors for objects in the middle.
  // As such, we call their destructors directly right now.
/*  while(emitExitTag && soStack(outStream).back() != this) 
    soStack(outStream).back()->destroy();*/
  
  //assert(props);
  if(props) {
    //cout << "sightObj::~sightObj(), emitExitTag="<<emitExitTag<<" props="<<props->str()<<endl;
    if(props->active && props->emitTag && emitExitTag)
      outStream->exit(this);
    delete props;
    props = NULL;
  }

  // Pop this sightObj off the top of the stack
  // The stack is empty when we're trying to destroy global/static sightObjs created before Sight was initialized
  if(!stackMayBeInvalidFlag && removeFromStack && soStack(outStream).size()>0) {
    // Remove this object from the stack
    assert(soStack(outStream).back() == this);
    soStack(outStream).pop_back();
  }
  
  // We've finished destroying this object and it should not be destroyed again,
  // even if the destructor is explicitly called.
  destroyed = true;

  // Invoke the callbacks registered by classes that derive from sightObj to notify them that the destruction of this
  // sightObj has completed.
  for(list<destructNotifier>::iterator n=sightObjDestructNotifiers.begin(); n!=sightObjDestructNotifiers.end(); n++)
    (*n)(this);
  sightObjDestructNotifiers.clear();
}

// Destroy all the currently live sightObjs on the stack
void sightObj::destroyAll() {
  // Record that we've begun the process of destroying all sight objects
  SightDestruction = true;
  
  // Call the destroy method of each object on the soStack
  map<dbgStream*, list<sightObj*> >& stack = soStackAllStreams();
  for(map<dbgStream*, list<sightObj*> >::iterator s=stack.begin(); s!=stack.end(); s++) {
    while(s->second.size()>0) {
      list<sightObj*>::reverse_iterator o=s->second.rbegin();
  //    if((*o)->emitExitTag) {
        //cout << ">!> "<<(*o)->props->str()<<endl;
        assert(!stackMayBeInvalidFlag);
        (*o)->destroy();
  //    }
      // The call to destroy will remove the last element from soStack(outStream)
    }
  }
  stack.clear();
}

// Returns whether this object is active or not
bool sightObj::isActive() const {
  assert(props);
  return props->active;
}

// Registers a new clock with sightObj
void sightObj::addClock(std::string clockName, sightClock* c) { 
  // This clockName/clock object combination does not currently exist in clocks
  assert(clocks.find(clockName) == clocks.end() ||
         (clocks.find(clockName) != clocks.end() && clocks[clockName].find(c) == clocks[clockName].end()));
  clocks[clockName].insert(c);
}

// Updates the registration of the given clock to refer to the given sightClock object
void sightObj::updClock(std::string clockName, sightClock* c) { 
  // This clockName/clock object combination must currently exist in clocks
  assert(clocks.find(clockName) != clocks.end());
  assert(clocks[clockName].find(c) != clocks[clockName].end());
  clocks[clockName].insert(c);
}

// Unregisters the clock with the given name
void sightObj::remClock(std::string clockName) { 
  // This clockName/clock object combination must currently exist in clocks
  assert(clocks.find(clockName) != clocks.end());
  clocks.erase(clockName);
}

// Returns whether the given clock object is currently registered
bool sightObj::isActiveClock(std::string clockName, sightClock* c) {
  return clocks.find(clockName) != clocks.end() &&
         clocks[clockName].find(c) != clocks[clockName].end();
}

// Registers a given callback function to be called when the destruction of this object completes.
void sightObj::registerDestructNotifier(destructNotifier notifier) {
  sightObjDestructNotifiers.push_back(notifier);
}

/************************************
 ***** MergeHandlerInstantiator *****
 ************************************/

std::map<std::string, MergeHandler>*    MergeHandlerInstantiator::MergeHandlers;
std::map<std::string, MergeKeyHandler>* MergeHandlerInstantiator::MergeKeyHandlers;
std::set<GetMergeStreamRecord>*         MergeHandlerInstantiator::MergeGetStreamRecords;

MergeHandlerInstantiator::MergeHandlerInstantiator() :
  sight::common::LoadTimeRegistry("MergeHandlerInstantiator", 
                                  MergeHandlerInstantiator::init)
{ }

void MergeHandlerInstantiator::init() {
  MergeHandlers          = new std::map<std::string, MergeHandler>();
  MergeKeyHandlers       = new std::map<std::string, MergeKeyHandler>();
  MergeGetStreamRecords  = new std::set<GetMergeStreamRecord>();
}

/*MergeHandlerInstantiator::MergeHandlerInstantiator() {
  // Initialize the handlers mappings, using environment variables to make sure that
  // only the first instance of this MergeHandlerInstantiator creates these objects.
  if(!getenv("SIGHT_MERGE_HANDLERS_INSTANTIATED")) {
    MergeHandlers          = new std::map<std::string, MergeHandler>();
    MergeKeyHandlers       = new std::map<std::string, MergeKeyHandler>();
    MergeGetStreamRecords  = new std::set<GetMergeStreamRecord>();
    setenv("SIGHT_MERGE_HANDLERS_INSTANTIATED", "1", 1);
  }
}*/

// Returns a mapping from the names of objects for which records are kept within this MergeHandlerInstantiator
// object to the freshly-allocated streamRecord objects that keep their records. The records are specialized 
// with the given stream ID.
std::map<std::string, streamRecord*> MergeHandlerInstantiator::GetAllMergeStreamRecords(int streamID) {
  std::map<std::string, streamRecord*> mergeMap;
  for(std::set<GetMergeStreamRecord>::iterator i=MergeGetStreamRecords->begin(); i!=MergeGetStreamRecords->end(); i++) {
    std::map<std::string, streamRecord*> subMergeMap = (*i)(streamID);
    // Copy over the mappings from subMergeMap to mergeMap;
    for(std::map<std::string, streamRecord*>::iterator j=subMergeMap.begin(); j!=subMergeMap.end(); j++) {
      // No two mergeMaps may contain overlapping keys
      assert(mergeMap.find(j->first) == mergeMap.end());
      
      mergeMap[j->first] = j->second;
    }
  }
  
  return mergeMap;
}

std::string MergeHandlerInstantiator::str() {
  std::ostringstream s;
  s << "[MergeHandlerInstantiator:"<<endl;
  s << "    MergeHandlers=(#"<<MergeHandlers->size()<<"): ";
  for(std::map<std::string, MergeHandler>::const_iterator i=MergeHandlers->begin(); i!=MergeHandlers->end(); i++) {
    if(i!=MergeHandlers->begin()) s << ", ";
    s << i->first;
  }
  s << endl;
  s << "    MergeKeyHandlers=(#"<<MergeKeyHandlers->size()<<"): ";
  for(std::map<std::string, MergeKeyHandler>::const_iterator i=MergeKeyHandlers->begin(); i!=MergeKeyHandlers->end(); i++) {
    if(i!=MergeKeyHandlers->begin()) s << ", ";
    s << i->first;
  }
  s << "]"<<endl;
  s << "    MergeGetStreamRecords(#"<<MergeGetStreamRecords->size()<<")]"<<endl;
  return s.str();
}

/*****************************************
 ***** SightMergeHandlerInstantiator *****
 *****************************************/

SightMergeHandlerInstantiator::SightMergeHandlerInstantiator() { 
  (*MergeHandlers   )["sight"]  = dbgStreamMerger::create;
  (*MergeKeyHandlers)["sight"]  = dbgStreamMerger::mergeKey;
  (*MergeHandlers   )["indent"] = IndentMerger::create;
  (*MergeKeyHandlers)["indent"] = IndentMerger::mergeKey;
  (*MergeHandlers   )["text"]   = TextMerger::create;
  (*MergeKeyHandlers)["text"]   = TextMerger::mergeKey;
  (*MergeHandlers   )["link"]   = LinkMerger::create;
  (*MergeKeyHandlers)["link"]   = LinkMerger::mergeKey;
    
  MergeGetStreamRecords->insert(&SightGetMergeStreamRecord);
}
SightMergeHandlerInstantiator sightMergeHandlerInstantance;

std::map<std::string, streamRecord*> SightGetMergeStreamRecord(int streamID) {
  std::map<std::string, streamRecord*> mergeMap;
  mergeMap["sight"] = new dbgStreamStreamRecord(streamID);
  mergeMap["block"] = new BlockStreamRecord(streamID);
  return mergeMap;
}

/************************
 ***** streamRecord *****
 ************************/

// Return the tagType (enter or exit) that is common to all incoming streams in tags, or 
// properties::unknownTag if they're not consistent
properties::tagType streamRecord::getTagType(const vector<pair<properties::tagType, properties::iterator> >& tags) {
  properties::tagType tag;
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin(); t!=tags.end(); t++) {
    if(t==tags.begin()) tag = t->first;
    else if(tag != t->first) return properties::properties::unknownTag;
  }
  return tag;
}

// Merge the IDs of the next ID field (named IDName, e.g. "anchorID" or "blockID") of the current tag maintained 
// within object nameed objName (e.g. "anchor" or "block") of each the incoming stream into a single ID in the 
// outgoing stream, updating each incoming stream's mappings from its IDs to the outgoing stream's 
// IDs. If a given incoming stream anchorID has already been assigned to a different outgoing stream anchorID, yell.
// For example, to merge the anchorIDs of anchors, blocks or any other tags that maintain anchorIDs, we need to set 
//    objName="anchor" and IDName="anchorID", while to merge the blockIDs of Blocks we set objName="block", 
//    IDName="blockID". Thus, anchor objects maintain the anchorID mappings for blocks as well as others, 
//    while blocks maintain blockID mappings for themselves. This means that each object type (e.g. anchor, block, 
//    trace) can only maintain one type of ID in its own streamRecord.
// The merged ID on the outgoing stream can be explicitly specified via the mergedID argument. If this argument
//    is not provided, a fresh ID is generated automatically.
// Returns the merged ID in the outgoing stream.
int streamRecord::mergeIDs(std::string objName, std::string IDName, 
                           std::map<std::string, std::string>& pMap, 
                           const vector<pair<properties::tagType, properties::iterator> >& tags,
                           std::map<std::string, streamRecord*>& outStreamRecords,
                           std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                           int mergedID) {
  //cout << "streamRecord::mergeIDs()\n";
  
  // Find the anchorID in the outgoing stream that the anchors in the incoming streams will be mapped to.
  // First, see if the anchors on the incoming streams have already been assigned an anchorID on the outgoing stream
  std::pair<bool, streamID> ret = streamRecord::sameID_ex(objName, IDName, pMap, tags, outStreamRecords, inStreamRecords);
  // Records whether we've found the outSID that the anchor on at least one incoming stream has been mapped to
  bool outSIDKnown = ret.first; 
  // The anchor's ID within the outgoing stream
  streamID outSID = ret.second;

  streamRecord* outS = outStreamRecords[objName];
  
  // If none of the anchors on the incoming stream have been mapped to an anchor in the outgoing stream,
  // create a fresh anchorID in the outgoing stream, advancing the maximum anchor ID in the process
  if(!outSIDKnown) {
    // If mergedID is not specified, set it to a fresh ID
    if(mergedID<0) mergedID = ++outS->maxID;
    // If it is specified, update maxID to ensure that the next auto-generated ID doesn't conflict with this one
    else
      outS->maxID = (outS->maxID <= mergedID? outS->maxID = mergedID+1: outS->maxID);
    
    outSID = streamID(mergedID, outS->getVariantID());
    //cout << "streamRecord::mergeIDs(), assigned new ID on outgoing stream outSID="<<outSID.str()<<endl;
  }
  
  // Update inStreamRecords to map the anchor's ID within each incoming stream to the assigned ID in the outgoing stream
    for(int i=0; i<tags.size(); i++) {
      streamRecord* s = (streamRecord*)inStreamRecords[i][objName];
      
      // The anchor's ID within the current incoming stream
      streamID inSID(properties::getInt(tags[i].second, IDName), s->getVariantID());
      
      /*cout << "|   "<<i<<": inSID="<<inSID.str()<<", in2outIDs=(#"<<s->in2outIDs.size()<<")"<<endl;
      for(map<streamID, streamID>::iterator j=s->in2outIDs.begin(); j!=s->in2outIDs.end(); j++)
        cout << "|       "<<j->first.str()<<" => "<<j->second.str()<<endl;*/
      
      // Yell if we're changing an existing mapping
      if((s->in2outIDs.find(inSID) != s->in2outIDs.end()) && s->in2outIDs[inSID] != outSID)
      { cerr << "ERROR: merging ID "<<inSID.str()<<" for object "<<objName<<" from incoming stream "<<i<<" multiple times. Old mapping: "<<s->in2outIDs[inSID].str()<<". New mapping: "<<outSID.str()<<"."<<endl; assert(0); }
      //cout << "|    outSID="<<outSID.str()<<endl;
      
      s->in2outIDs[inSID] = outSID;
    }
    
  // Assign to the merged block the next ID for this output stream
  pMap[IDName] = txt()<<outSID.ID;
  //pMap["vID"] = outS->getVariantID().serialize();
  
  return outSID.ID;
}

// Variant of mergeIDs where it is assumed that all the IDs in the incoming stream map to the same ID in the outgoing
// stream and an error is thrown if this is not the case
int streamRecord::sameID(std::string objName, std::string IDName, 
                         std::map<std::string, std::string>& pMap, 
                         const vector<pair<properties::tagType, properties::iterator> >& tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  std::pair<bool, streamID> ret = streamRecord::sameID_ex(objName, IDName, pMap, tags, outStreamRecords, inStreamRecords);
  if(!ret.first) { cerr << "ERROR: expecting IDs of object "<<objName<<" to be known but they currently are not!"; assert(0); }
  
  return ret.second.ID;
}

// Variant of mergeIDs where it is assumed that all the IDs in the incoming stream map to the same ID in the outgoing
// stream and an error is thrown if this is not the case. Returns a pair containing a flag that indicates whether
// an ID on the outgoing stream is known and if so, the ID itself
std::pair<bool, streamID> streamRecord::sameID_ex(
                            std::string objName, std::string IDName, 
                            std::map<std::string, std::string>& pMap, 
                            const vector<pair<properties::tagType, properties::iterator> >& tags,
                            std::map<std::string, streamRecord*>& outStreamRecords,
                            std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  // Identify the ID on the outgoing stream that the IDs on the incoming streams have been assigned to, 
  // ensuring that they've all been assigned to the same one.
    
  // The ID within the outgoing stream
  streamID outSID;
  // Records whether we've found the outSID that the anchor on at least one incoming stream has been mapped to
  bool outSIDKnown=false; 
  
  for(int i=0; i<tags.size(); i++) {
    streamRecord* s = inStreamRecords[i][objName];
    
    // The anchor's ID within the current incoming stream
    streamID inSID(properties::getInt(tags[i].second, IDName), 
                   inStreamRecords[i][objName]->getVariantID());
    
    if(s->in2outIDs.find(inSID) != s->in2outIDs.end()) {
      // If we've already found an outSID, make this it is the same one
      if(outSIDKnown) {
        if(outSID != s->in2outIDs[inSID]) { cerr << "ERROR: Attempting to merge IDs of object "<<objName<<" of multiple incoming streams but they are mapped to different anchorIDs in the outgoing stream!"<<endl; assert(0); }
      } else {
        outSID = s->in2outIDs[inSID];
        outSIDKnown = true;
      }
    }
  }
  
  return make_pair(outSIDKnown, outSID);
}

// Given an anchor ID on the current incoming stream return its ID in the outgoing stream, yelling if it is missing.
streamID streamRecord::in2outID(streamID inSID) const {
  map<streamID, streamID>::const_iterator it = in2outIDs.find(inSID);
  if(it==in2outIDs.end()) { cerr << "ERROR: ID "<<inSID.str()<<" could not be converted from incoming to outgoing because it was not found!"<<endl<<str("")<<endl; assert(0); }
   return it->second;
}

// Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void streamRecord::resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams) {
  // Compute the maximum maxID among all the streams
  maxID = -1;
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++)
    maxID = ((*s)[objName]->maxID > maxID? (*s)[objName]->maxID: maxID);
  
  // Set in2outIDs to be the union of its counterparts in streams
  in2outIDs.clear();
  
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    for(map<streamID, streamID>::const_iterator i=(*s)[objName]->in2outIDs.begin(); i!=(*s)[objName]->in2outIDs.end(); i++)
      in2outIDs.insert(*i);
  }
}

std::string streamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[streamRecord: maxID="<<maxID<<endl;
  
  s << indent << "in2outIDs(#"<<in2outIDs.size()<<")="<<endl;
  for(map<streamID, streamID>::const_iterator i=in2outIDs.begin(); i!=in2outIDs.end(); i++)
    s << indent << "    "<<i->first.str()<<" =&gt; "<<i->second.str()<<endl;
  s << indent << "]";
  
  return s.str();
}

/******************
 ***** Merger *****
 ******************/

Merger::Merger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
               std::map<std::string, streamRecord*>& outStreamRecords,
               std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
               properties* props): props(props) {
  emitTagFlag = true;
  
  if(props==NULL) props = new properties();
  
  // Iterate through the properties of any clocks associated with this object
  while(!isIterEnd(tags)) {
    properties::tagType type = streamRecord::getTagType(tags);
    if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging!"<<endl; exit(-1); }
    if(type==properties::enterTag) {
      vector<string> names = getNames(tags); assert(allSame<string>(names));
      //assert(*names.begin() == "text");
      
      // Create a Merger object for the current clock, using it to update props with the clock's merged properties
      Merger* m = (*MergeHandlerInstantiator::MergeHandlers)[*names.begin()](tags, outStreamRecords, inStreamRecords, props);
      // Reset this Merger's properties pointer to NULL so that props doesn't get deallocated when we deallocate m
      m->resetProps();
      // Deallocate m since we no longer need it
      delete m;
    } else { }
    
    tags = advance(tags);
  }
}

Merger::~Merger() {
  if(props) delete props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void Merger::mergeKey(properties::tagType type, properties::iterator tag, 
                      std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  // Iterate through the properties of any clocks associated with this object
  while(!tag.isEnd()) {
    if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging keys!"<<endl; exit(-1); }
    if(type==properties::enterTag) {
      // Call the current clock's mergeKey method
      (*MergeHandlerInstantiator::MergeKeyHandlers)[tag.name()](type, tag, inStreamRecords, info);
    } else { }
    
    tag++;
  }
}

// Given a vector of tag properties, returns whether the given key exists in all tags
bool Merger::keyExists(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags, std::string key) {
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin(); t!=tags.end(); t++) {
    if(!t->second.exists(key)) return false;
  }
  return true;
}

// Given a vector of tag properties, returns the set of values assigned to the given key within the given tag
std::vector<std::string> Merger::getValues(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags, 
                                           std::string key) {
  vector<string> vals;
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin(); t!=tags.end(); t++) {
    vals.push_back(properties::get(t->second, key));
  }
  return vals;
}

// Given a vector of tag properties, merges their values and returns the merged string
std::string Merger::getMergedValue(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags, 
                                  std::string key) {
	string merged;
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin(); t!=tags.end(); t++) {
    if(t==tags.begin()) merged = properties::get(t->second, key);
    else                merged = mergeText(properties::get(t->second, key), merged);
  }
  return merged;
}

// Given a vector of tag properties that must be the same, returns their common value
std::string Merger::getSameValue(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags, 
                                 std::string key) {
  string ret;
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin(); t!=tags.end(); t++) {
    if(t==tags.begin()) ret = t->second.get(key);
    else assert(ret == t->second.get(key));
  }
  return ret;
}

// Returns whether all the elements in the given set are equal to each others
/*template<class T>
bool Merger::allSame(const std::vector<T>& s) {
  for(typename vector<T>::const_iterator i=s.begin(); i!=s.end(); i++)
    if(i!=s.begin() && *i!=*s.begin()) return false;
  return true;
}*/

// Given a vector of tag property iterators, returns the set of names of all the object types they refer to
std::vector<std::string> Merger::getNames(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags) {
  vector<string> names;
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin(); t!=tags.end(); t++) {
    names.push_back(t->second.name());
  }
  return names;
}

// Given a vector of tag property iterators that must be the same, returns their common name
std::string Merger::getSameName(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags) {
  string ret;
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin(); t!=tags.end(); t++) {
    if(t==tags.begin()) ret = t->second.name();
    else assert(ret == t->second.name());
  }
  return ret;
}

// Converts the given set of strings to the corresponding set of integral numbers
std::vector<long> Merger::str2int(const std::vector<std::string>& strSet) {
  vector<long> intSet;
  for(vector<string>::const_iterator s=strSet.begin(); s!=strSet.end(); s++)
    intSet.push_back(strtol(s->c_str(), NULL, 10));
  return intSet;
}
  
long Merger::vMax(const std::vector<long>& intSet) {
  long m=LONG_MIN;
  for(vector<long>::const_iterator i=intSet.begin(); i!=intSet.end(); i++)
    m = (*i>m? *i: m);
  return m;
}

long Merger::vMin(const std::vector<long>& intSet)  {
  long m=LONG_MAX;
  for(vector<long>::const_iterator i=intSet.begin(); i!=intSet.end(); i++)
    m = (*i<m? *i: m);
  return m;
}

long Merger::vSum(const std::vector<long>& intSet)  {
  long sum=0;
  for(vector<long>::const_iterator i=intSet.begin(); i!=intSet.end(); i++)
    sum += *i;
  return sum;
}

long Merger::vAvg(const std::vector<long>& intSet)  {
  return vSum(intSet)/intSet.size();
}

// Converts the given set of strings to the corresponding set of floating point numbers
std::vector<double> Merger::str2float(const std::vector<std::string>& strSet) {
  vector<double> floatSet;
  for(vector<string>::const_iterator s=strSet.begin(); s!=strSet.end(); s++)
    floatSet.push_back(strtod(s->c_str(), NULL));
  return floatSet;
}

double Merger::vMax(const std::vector<double>& floatSet) {
  double m=-1e100;//DBL_MIN;
  for(vector<double>::const_iterator i=floatSet.begin(); i!=floatSet.end(); i++)
    m = (*i>m? *i: m);
  return m;
}

double Merger::vMin(const std::vector<double>& floatSet)  {
  double m=1e100;//DBL_MAX;
  for(vector<double>::const_iterator i=floatSet.begin(); i!=floatSet.end(); i++)
    m = (*i<m? *i: m);
  return m;
}

double Merger::vSum(const std::vector<double>& floatSet)  {
  double sum=0;
  for(vector<double>::const_iterator i=floatSet.begin(); i!=floatSet.end(); i++)
    sum += *i;
  return sum;
}

double Merger::vAvg(const std::vector<double>& floatSet)  {
  return vSum(floatSet)/floatSet.size();
}

// Advance the iterators in the given tags vector, returning the resulting vector
std::vector<std::pair<properties::tagType, properties::iterator> >
              Merger::advance(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags) {
  vector<pair<properties::tagType, properties::iterator> > advancedTags;
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin();
      t!=tags.end(); t++) {
    properties::iterator newT = t->second;
    newT++;
    advancedTags.push_back(make_pair(t->first, newT));
  }
  
  return advancedTags;
}

// Returns true if any of the iterators in tags have reached their end. This must be the same for all iterators.
// Either all have reached their end or none have.
bool Merger::isIterEnd(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags) {
  bool ended=false;
  for(vector<pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin();
      t!=tags.end(); t++) {
    if(t==tags.begin()) ended = t->second.isEnd();
    else                assert(ended == t->second.isEnd());
  }
  
  return ended;
}


/**********************
 ***** TextMerger *****
 **********************/

TextMerger::TextMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                       properties* props) :
                              Merger(advance(tags), outStreamRecords, inStreamRecords, props) {
  assert(tags.size()>0);
  
  if(props==NULL) props = new properties();
  this->props = props;

  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "text");
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Text!"<<endl; assert(0); }
  if(type==properties::enterTag) {
    
    pMap["text"] = getMergedValue(tags, "text");
  }
  
  props->add("text", pMap);
}

/******************
 ***** anchor *****
 ******************/
int anchor::maxAnchorID=0;
anchor anchor::noAnchor(-1);

// Maps all anchor IDs to their locations, if known. When we establish forward links we create
// anchors that are initially not connected to a location (the target location has not yet been reached). Thus,
// when we reach a given location we may have multiple anchors with multiple IDs all for a single location.
// This map maintains the canonical anchor ID for each location. Other anchors are resynched to used this ID
// whenever they are copied. This means that data structures that index based on anchors may need to be
// reconstructed after we're sure that their targets have been reached to force all anchors to use their canonical IDs.
map<int, location> anchor::anchorLocs;

// Associates each anchor with a unique anchor ID. Useful for connecting multiple anchors that were created
// independently but then ended up referring to the same location. We'll record the ID of the first one to reach
// this location on locAnchorIDs and the others will be able to adjust themselves by adopting this ID.
std::map<location, int> anchor::locAnchorIDs;

anchor::anchor()                   : anchorID(maxAnchorID++), located(false) {
}
anchor::anchor(const anchor& that) : anchorID(that.anchorID), located(false) {
  // If we know that is located then we just copy its location information since it will not change
  if(that.located) {
    located = true;
    loc     = that.loc;
  // If that's locatedness is unknown, call update to check with the anchorLocs map
  } else
    update();
} 
anchor::anchor(int anchorID)       : anchorID(anchorID), located(false)      {
  // Check to see if this anchorID is already located
  update();
}

anchor::~anchor() {
}

// Records that this anchor's location is the current spot in the output
void anchor::reachedLocation() {
  // If this anchor has already been set to point to its target location, emit a warning
  if(located && loc != dbg.getLocation()) {
    cerr << "Warning: anchor "<<anchorID<<" is being set to multiple target locations! current location="<<loc.str()<<", new location="<<dbg.getLocation().str()<< endl;
    cerr << "noAnchor="<<noAnchor.str()<<endl;
    if(anchorLocs.find(anchorID) != anchorLocs.end())
      cerr << "anchorLocs[anchorID]="<<anchorLocs[anchorID].str()<<endl;
    for(map<int, location>::iterator i=anchorLocs.begin(); i!=anchorLocs.end(); i++)
      cerr << "    "<<i->first<<" => "<<i->second.str()<<endl;
  } else {
    located = true;
    loc = dbg.getLocation();
    anchorLocs[anchorID] = loc;

    update();
  }
}

// Updates this anchor to use the canonical ID of its location, if one has been established
void anchor::update() {
  if(anchorID==-1) {
    located = false;
  } else if(anchorLocs.find(anchorID) != anchorLocs.end()) {
    located = true;
    loc = anchorLocs[anchorID];
  }

  // If this is the first anchor at this location, associate this location with this anchor ID
  if(located) {
    if(locAnchorIDs.find(loc) == locAnchorIDs.end())
      locAnchorIDs[loc] = anchorID;
    // If this is not the first anchor here, update this anchor object's ID to be the same as all
    // the other anchors at this location
    else
      anchorID = locAnchorIDs[loc];
  }  
}


void anchor::operator=(const anchor& that) {
  anchorID = that.anchorID;
  // If we know that is located then we just copy its location information since it will not change
  if(that.located) {
    located = that.located;
    loc     = that.loc;
  // If that's locatedness is unknown, call update to check with the anchorLocs map
  } else
    update();
}

bool anchor::operator==(const anchor& that) const {
  // anchorID of -1 indicates that this is the noAnchor object and any copies of it are equivalent
  assert(anchorID>=-1); assert(that.anchorID>=-1);
  if(anchorID==-1 && that.anchorID==-1) return true;

  // They're equal if they're located and they have the same location OR
  return (located && that.located && loc == that.loc) ||
         // either one is not located and have the same ID.
         (anchorID == that.anchorID);
  // NOTE: we do not call update on either anchor to make sure that once an anchor is included
  //       in a data structure, its relations to other anchors do not change. Update is only
  //       called when anchors are copied, meaning that we cannot simply copy data structures
  //       that use anchors as keys and must instead re-create them.
}

bool anchor::operator!=(const anchor& that) const
{ return !(*this == that); }

bool anchor::operator<(const anchor& that) const
{
  // anchorID of -1 indicates that this is the noAnchor object and any copies of it are equivalent
  assert(anchorID>=-1); assert(that.anchorID>=-1);
  if(anchorID==-1 && that.anchorID==-1) return false;
  
  // They're LT if one is located while the other is not (located anchors are ordered before unlocated ones), OR
  return (located && !that.located) ||
          // they're both located and their locations are LT OR
         (located && that.located && loc < that.loc) ||
         // neither one is located and their IDs are LT.
         (!located && !that.located && anchorID < that.anchorID);
  // NOTE: we do not call update on either anchor to make sure that once an anchor is included
  //       in a data structure, its relations to other anchors do not change. Update is only
  //       called when anchors are copied, meaning that we cannot simply copy data structures
  //       that use anchors as keys and must instead re-create them.
}

// Emits to the output an html tag that denotes a link to this anchor. Embeds the given text in the link.
void anchor::link(string text) const {
  properties p;
  map<string, string> newProps;
  newProps["anchorID"] = txt()<<anchorID;
  newProps["text"] = text;
  newProps["img"] = "0";
  newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
  p.add("link", newProps);
  
  dbg.tag(p);
}

// Emits to the output an html tag that denotes a link to this anchor, using the default link image, which is followed by the given text.
void anchor::linkImg(string text) const {
  properties p;
  map<string, string> newProps;
  newProps["anchorID"] = txt()<<anchorID;
  newProps["text"] = text;
  newProps["img"] = "1";
  newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
  p.add("link", newProps);
  
  dbg.tag(p);
}

std::string anchor::str(std::string indent) const {
  if(anchorID==-1) return "[noAnchor_structure]";
  else             return txt()<<"[anchor_structure: ID="<<anchorID<<", "<<(located?"":"not")<<" located"<<(located? ", ": "")<<(located? loc.str(): "")<<"]";
}

// vSuffixID: ID that identifies this variant within the next level of variants in the heirarchy
AnchorStreamRecord::AnchorStreamRecord(const AnchorStreamRecord& that, int vSuffixID) :
  streamRecord((const streamRecord&)that, vSuffixID), 
  /*maxAnchorID(that.maxAnchorID), in2outAnchorIDs(that.in2outAnchorIDs), */anchorLocs(that.anchorLocs), locAnchorIDs(that.locAnchorIDs) 
{ }

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* AnchorStreamRecord::copy(int vSuffixID) {
  return new AnchorStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void AnchorStreamRecord::resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams) {
  streamRecord::resumeFrom(streams);
  
  // Set anchorLocs and locAnchorIDs to be the union of its counterparts in streams
  anchorLocs.clear();
  locAnchorIDs.clear();
  
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    AnchorStreamRecord* as = (AnchorStreamRecord*)(*s)["anchor"];
      
    for(map<streamID, streamLocation>::const_iterator i=as->anchorLocs.begin(); i!=as->anchorLocs.end(); i++)
      anchorLocs.insert(*i);
    
    for(map<streamLocation, streamID>::const_iterator i=as->locAnchorIDs.begin(); i!=as->locAnchorIDs.end(); i++)
      locAnchorIDs.insert(*i);
  }
}

std::string AnchorStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[AnchorStreamRecord: ";
  s << streamRecord::str(indent+"    ") << endl;
  
  s << indent << "anchorLocs(#"<<anchorLocs.size()<<")="<<endl;
  for(map<streamID, streamLocation>::const_iterator i=anchorLocs.begin(); i!=anchorLocs.end(); i++)
    s << indent << "    "<<i->first.str()<<" =&gt; "<<i->second.str()<<endl;
  
  s << indent << "locAnchorIDs(#"<<locAnchorIDs.size()<<")="<<endl;
  for(map<streamLocation, streamID>::const_iterator i=locAnchorIDs.begin(); i!=locAnchorIDs.end(); i++)
    s << indent << "    "<<i->first.str()<<" =&gt; "<<i->second.str()<<endl;
  
  s << indent << "]";
  
  return s.str();
}

/**********************
 ***** LinkMerger *****
 **********************/

LinkMerger::LinkMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                       properties* props) :
                              Merger(advance(tags), outStreamRecords, inStreamRecords, props) {
  assert(tags.size()>0);
  
  if(props==NULL) props = new properties();
  this->props = props;

  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "link");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Link!"<<endl; assert(0); }
  if(type==properties::enterTag) {
    // Merge the IDs of the anchors this link targets
    streamRecord::mergeIDs("anchor", "anchorID", pMap, tags, outStreamRecords, inStreamRecords);
      
    pMap["text"] = getMergedValue(tags, "text");
    
    //cout << "LinkMerger::LinkMerger anchorID="<<pMap["anchorID"]<<", text="<<pMap["text"]<<endl;
    
    // If the link in any of the variants has an image, they all do
    vector<long> imgFlags = str2int(getValues(tags, "img"));
    if(imgFlags.size()==1) pMap["img"] = txt()<<*imgFlags.begin();
    else                   pMap["img"] = "1";
    
    vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();
  }
  
  props->add("link", pMap);
}

/************************
 ***** streamAnchor *****
 ************************/

streamAnchor::streamAnchor(std::map<std::string, streamRecord*>& myStream) : located(false) {
  dbgStreamR = (dbgStreamStreamRecord*)myStream["sight"];  assert(dbgStreamR);
  anchorR    = (AnchorStreamRecord*)   myStream["anchor"]; assert(anchorR);
  
  ID = streamID(anchorR->maxID++, anchorR->getVariantID());
}

streamAnchor::streamAnchor(const streamAnchor& that) : ID(that.ID), located(false), dbgStreamR(that.dbgStreamR), anchorR(that.anchorR) {
  // If we know that is located then we just copy its location information since it will not change
  if(that.located) {
    located = true;
    loc     = that.loc;
  // If that's locatedness is unknown, call update to check with the anchorLocs map
  } else
    update();
}

streamAnchor::streamAnchor(int anchorID, std::map<std::string, streamRecord*>& myStream) : located(false) {
  dbgStreamR = (dbgStreamStreamRecord*)myStream["sight"];  assert(dbgStreamR);
  anchorR    = (AnchorStreamRecord*)   myStream["anchor"]; assert(anchorR);
  this->ID = streamID(anchorID, anchorR->getVariantID());
  
  // Check to see if this anchorID is already located
  update();
}

streamAnchor::streamAnchor(streamID ID, std::map<std::string, streamRecord*>& myStream) : ID(ID), located(false) {
  dbgStreamR = (dbgStreamStreamRecord*)myStream["sight"];  assert(dbgStreamR);
  anchorR    = (AnchorStreamRecord*)   myStream["anchor"]; assert(anchorR);
  
  // Check to see if this anchorID is already located
  update();
}


streamAnchor::~streamAnchor() {
}

// Records that this anchor's location is the current spot in the output
void streamAnchor::reachedLocation() {
  // If this anchor has already been set to point to its target location, emit a warning
  if(located && loc != dbgStreamR->getLocation())
    cerr << "Warning: anchor "<<ID.str()<<" is being set to multiple target locations! current location="<<loc.str()<<", new location="<<dbgStreamR->getLocation().str()<< endl;
  else {
    located = true;
    loc = dbgStreamR->getLocation();
    anchorR->anchorLocs[ID] = loc;

    update();
  }
}

// Updates this anchor to use the canonical ID of its location, if one has been established
void streamAnchor::update() {
  if(anchorR->anchorLocs.find(ID) != anchorR->anchorLocs.end()) {
    located = true;
    loc = anchorR->anchorLocs[ID];
  }
  
  // If this is the first anchor at this location, associate this location with this anchor ID
  if(located) {
    if(anchorR->locAnchorIDs.find(loc) == anchorR->locAnchorIDs.end())
      anchorR->locAnchorIDs[loc] = ID;
    // If this is not the first anchor here, update this anchor object's ID to be the same as all
    // the other anchors at this location
    else
      ID = anchorR->locAnchorIDs[loc];
  }  
}

void streamAnchor::operator=(const streamAnchor& that) {
  ID = that.ID;
  // If we know that is located then we just copy its location information since it will not change
  if(that.located) {
    located = that.located;
    loc     = that.loc;
  // If that's locatedness is unknown, call update to check with the anchorLocs map
  } else
    update();
}

bool streamAnchor::operator==(const streamAnchor& that) const {
  // anchorID of -1 indicates that this is the noAnchor object and any copies of it are equivalent
  assert(ID.ID>=-1); assert(that.ID.ID>=-1);
  if(ID.ID==-1 && that.ID.ID==-1) return true;

  // They're equal if they're located and they have the same location OR
  return (located && that.located && loc == that.loc) ||
         // either one is not located and have the same ID.
         (ID == that.ID);
  // NOTE: we do not call update on either anchor to make sure that once an anchor is included
  //       in a data structure, its relations to other anchors do not change. Update is only
  //       called when anchors are copied, meaning that we cannot simply copy data structures
  //       that use anchors as keys and must instead re-create them.
}

bool streamAnchor::operator!=(const streamAnchor& that) const
{ return !(*this == that); }

bool streamAnchor::operator<(const streamAnchor& that) const
{
  // anchorID of -1 indicates that this is the noAnchor object and any copies of it are equivalent
  assert(ID.ID>=-1); assert(that.ID.ID>=-1);
  if(ID.ID==-1 && that.ID.ID==-1) return false;
  
  // They're LT if one is located while the other is not (located anchors are ordered before unlocated ones), OR
  return (located && !that.located) ||
          // they're both located and their locations are LT OR
         (located && that.located && loc < that.loc) ||
         // neither one is located and their IDs are LT.
         (!located && !that.located && ID < that.ID);
  // NOTE: we do not call update on either anchor to make sure that once an anchor is included
  //       in a data structure, its relations to other anchors do not change. Update is only
  //       called when anchors are copied, meaning that we cannot simply copy data structures
  //       that use anchors as keys and must instead re-create them.
}


std::string streamAnchor::str(std::string indent) const {
  ostringstream s;
  s << "[streamAnchor: ID="<<ID.str()<<(located? " loc="+loc.str():"")<<"]";
  return s.str();
}

/*****************
 ***** block *****
 *****************/

// The unique ID of this block as well as the static global counter of the maximum ID assigned to any block.
// Unlike the rendering module, these blockIDs are integers since all we need from them is uniqueness and not
// any structural information.
int block::maxBlockID;

// Initializes this block with the given label
block::block(string label, properties* props) : label(label), sightObj(setProperties(label, props)) {
  advanceBlockID();
  if(this->props->active && this->props->emitTag) {
    // Connect startA and pointsTo anchors to the current location (pointsTo is not modified);
    startA.reachedLocation();
    
    dbg.enterBlock(this);
  }
}

// Sets the properties of this object
properties* block::setProperties(string label, properties* props) {
  if(!initializedDebug) SightInit("Debug Output", "dbg");
    
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    // Connect startA to the current location (pointsTo is not modified). We do this for 
    // local variable because we cannot reference the anchorA field of a particular
    // block since the block that called setProperties() has not yet been constructed.
    anchor startA; startA.reachedLocation();
    
    map<string, string> newProps;
    newProps["label"] = label;
    newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
    newProps["ID"] = txt()<<(maxBlockID+1);
    newProps["anchorID"] = txt()<<startA.getID();
    newProps["numAnchors"] = "0";
    props->add("block", newProps);
  }
  return props;
}

// Initializes this block with the given label.
// Includes one or more incoming anchors thas should now be connected to this block.
block::block(string label, anchor& pointsTo, properties* props) : label(label), sightObj(setProperties(label, pointsTo, props))  {
  advanceBlockID();
  
  if(this->props->active && this->props->emitTag) {
    // Connect startA and pointsTo anchors to the current location (pointsTo is not modified);
    startA.reachedLocation();
    
    // Record that all the anchors in pointsTo have reached their target location, making sure
    // not to modify the original pointsTo anchor.
    anchor pointsToCopy(pointsTo);
    if(pointsToCopy!=anchor::noAnchor) pointsToCopy.reachedLocation();
    
    dbg.enterBlock(this);
  }
}

// Sets the properties of this object
properties* block::setProperties(string label, anchor& pointsTo, properties* props) {
  if(!initializedDebug) SightInit("Debug Output", "dbg");
  
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    // Connect startA to the current location (pointsTo is not modified). We do this for 
    // local variable because we cannot reference the anchorA field of a particular
    // block since the block that called setProperties() has not yet been constructed.
    anchor startA; startA.reachedLocation();
    
    map<string, string> newProps;
    newProps["label"] = label;
    newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
    newProps["ID"] = txt()<<(maxBlockID+1);
    newProps["anchorID"] = txt()<<startA.getID();
    if(pointsTo != anchor::noAnchor) {
      newProps["numAnchors"] = "1";
      newProps["anchor_0"] = txt()<<pointsTo.getID();
    } else
      newProps["numAnchors"] = "0";
    props->add("block", newProps);
  }
  
  return props;
}

// Initializes this block with the given label.
// Includes one or more incoming anchors thas should now be connected to this block.
block::block(string label, set<anchor>& pointsTo, properties* props) : label(label), sightObj(setProperties(label, pointsTo, props)) {
  advanceBlockID();
    
  if(this->props->active && this->props->emitTag) {    
    // Connect startA and pointsTo anchors to the current location (pointsTo is not modified)
    startA.reachedLocation();
    
    // Record that all the anchors in pointsTo have reached their target location, making sure
    // not to modify the original anchors since the pointsTo set may be used by the caller for other
    // purposes and we cannot invalidate its order.
    for(set<anchor>::iterator a=pointsTo.begin(); a!=pointsTo.end(); a++) {
      if(*a!=anchor::noAnchor) {
        anchor aCopy(*a);
        aCopy.reachedLocation();
      }
    }
    
    dbg.enterBlock(this);
  }
}
  
// Sets the properties of this object
properties* block::setProperties(string label, set<anchor>& pointsTo, properties* props) {
  if(!initializedDebug) SightInit("Debug Output", "dbg");

  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    // Connect startA to the current location (pointsTo is not modified). We do this for 
    // local variable because we cannot reference the anchorA field of a particular
    // block since the block that called setProperties() has not yet been constructed.
    anchor startA; startA.reachedLocation();
    
    map<string, string> newProps;
    newProps["label"] = label;
    newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
    newProps["ID"] = txt()<<(maxBlockID+1);
    newProps["anchorID"] = txt()<<startA.getID();
    
    int i=0;
    for(set<anchor>::const_iterator a=pointsTo.begin(); a!=pointsTo.end(); a++) {
      if(*a != anchor::noAnchor) {
        newProps[txt()<<"anchor_"<<i] = txt()<<a->getID();
        i++;
      }
    }
    newProps["numAnchors"] = txt()<<i;

    props->add("block", newProps);
  }
  
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void block::destroy() {
  this->~block();
}


block::~block() {
  assert(!destroyed);
  
  assert(props);
  if(props->active && props->emitTag)
    dbg.exitBlock();
}

// Increments blockD. This function serves as the one location that we can use to target conditional
// breakpoints that aim to stop when the block count is a specific number
int block::advanceBlockID() {
  maxBlockID++;
  blockID = maxBlockID;
  // THIS COMMENT MARKS THE SPOT IN THE CODE AT WHICH GDB SHOULD BREAK
  return maxBlockID;
}

anchor& block::getAnchorRef()
{ return startA; }

anchor block::getAnchor() const
{ return startA; }

BlockMerger::BlockMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
                                     Merger(advance(tags), outStreamRecords, inStreamRecords, props)
{
  assert(tags.size()>0);
  assert(inStreamRecords.size() == tags.size());
 
  if(props==NULL) props = new properties();

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags);
  //cout << "type="<<(type==properties::enterTag? "enterTag": (type==properties::exitTag? "exitTag": "unknownTag"))<<endl;
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Block!"<<endl; assert(0); }
  if(type==properties::enterTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "block");
  
    /*cout << "BlockMerger::BlockMerger(), nextTag("<<tags.size()<<")"<<endl;
    for(vector<pair<properties::tagType, properties::iterator> >::iterator t=tags.begin(); t!=tags.end(); t++)
      cout << "    "<<(t->first==properties::enterTag? "enterTag": (t->first==properties::exitTag? "exitTag": "unknownTag"))<<", "<<properties::str(t->second)<<endl;*/
    
    pMap["label"] = getMergedValue(tags, "label");
    
    // Initialize the block and anchor entries in outStreamRecords and inStreamRecords if needed
    //initStreamRecords<BlockStreamRecord> ("block",  outStreamRecords, inStreamRecords);
    //initStreamRecords<AnchorStreamRecord>("anchor", outStreamRecords, inStreamRecords);
    
    // Merge the block IDs along all the streams
    streamRecord::mergeIDs("block", "ID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // Merge the IDs of the anchors that target the block
    streamRecord::mergeIDs("anchor", "anchorID", pMap, tags, outStreamRecords, inStreamRecords);
    
    //cout << "    anchor="<<pMap["anchorID"]<<endl;
    
    // Update the current location in the incoming and outgoing streams to account for entry into the block
    dbgStreamStreamRecord::enterBlock(inStreamRecords);
    //cout << "outLocation="<<((dbgStreamStreamRecord*)outStreamRecords["sight"])->getLocation().str()<<endl;
    dbgStreamStreamRecord::enterBlock(outStreamRecords);
    //cout << "outLocation="<<((dbgStreamStreamRecord*)outStreamRecords["sight"])->getLocation().str()<<endl;
    
    set<streamAnchor> outAnchors; // Set of anchorIDs, within the anchor ID space of the outgoing stream, that terminate at this block
    // Iterate over all the anchors that terminate at this block within all the incoming streams and add their corresponding 
    // IDs within the outgoing stream to outAnchors
    for(int i=0; i<tags.size(); i++) {
      AnchorStreamRecord* as = (AnchorStreamRecord*)inStreamRecords[i]["anchor"];
      
      // Iterate over all the anchors within incoming stream i that terminate at this block
      int inNumAnchors = properties::getInt(tags[i].second, "numAnchors");
      for(int a=0; a<inNumAnchors; a++) {
        streamAnchor curInAnchor(properties::getInt(tags[i].second, txt()<<"anchor_"<<a), inStreamRecords[i]);
        
        if(as->in2outIDs.find(curInAnchor.getID()) == as->in2outIDs.end())
          cerr << "ERROR: Do not have a mapping for anchor "<<curInAnchor.str()<<" on incoming stream "<<i<<" to its anchorID in the outgoing stream!";
        assert(as->in2outIDs.find(curInAnchor.getID()) != as->in2outIDs.end());
        
        // Record, within the records of both the incoming and outgoing streams, that this anchor has reached its target
        streamAnchor curOutAnchor(as->in2outIDs[curInAnchor.getID()], outStreamRecords);
        //cout << "        "<<a<<": curInAnchor="<<curInAnchor.str()<<" => "<<curOutAnchor.str()<<endl;
        curInAnchor.reachedLocation(); 
        curOutAnchor.reachedLocation();
        //cout << "        "<<a<<": curInAnchor="<<curInAnchor.str()<<" => "<<curOutAnchor.str()<<endl;
        
        outAnchors.insert(curOutAnchor);
      }
    }
    
    // Add all the IDs within outAnchors to the properties of the merged block
    pMap["numAnchors"] = txt()<<outAnchors.size();
    int aIdx=0;
    //cout << "    outAnchors="<<endl;
    for(set<streamAnchor>::iterator a=outAnchors.begin(); a!=outAnchors.end(); a++, aIdx++) {
      //cout << "        "<<a->str()<<endl;
      pMap[txt()<<"anchor_"<<aIdx] = txt()<<a->getID().ID;
    }
    
    vector<string> cpValues = getValues(tags, "callPath");
    if(allSame<string>(cpValues)) pMap["callPath"] = *cpValues.begin();
    else                          pMap["callPath"] = "";
    
    props->add("block", pMap);
  } else {
    props->add("block", pMap);
    // Update the current location in the incoming and outgoing streams to account for exit into the block
    dbgStreamStreamRecord::exitBlock(inStreamRecords);
    dbgStreamStreamRecord::exitBlock(outStreamRecords);
  }
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void BlockMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; assert(0); }
  if(type==properties::enterTag) {
    info.add(properties::get(tag, "callPath"));
  }
}

// vSuffixID: ID that identifies this variant within the next level of variants in the heirarchy
BlockStreamRecord::BlockStreamRecord(const BlockStreamRecord& that, int vSuffixID) : 
  streamRecord((const streamRecord&)that, vSuffixID)//, maxBlockID(that.maxBlockID)//, in2outBlockIDs(that.in2outBlockIDs)
{ }

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* BlockStreamRecord::copy(int vSuffixID) {
  return new BlockStreamRecord(*this, vSuffixID);
}

std::string BlockStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[BlockStreamRecord: ";
  s << streamRecord::str(indent+"    ") << "]";
  
  return s.str();
}

/******************
 ***** dbgBuf *****
 ******************/
  
dbgBuf::dbgBuf()
{
  init(NULL);
}

dbgBuf::dbgBuf(std::streambuf* baseBuf)
{
  init(baseBuf);
}

void dbgBuf::init(std::streambuf* baseBuf)
{
  this->baseBuf = baseBuf;
  synched = true;
  ownerAccess = false;
  numOpenAngles = 0;
}

// This dbgBuf has no buffer. So every character "overflows"
// and can be put directly into the teed buffers.
int dbgBuf::overflow(int c)
{
  // Only emit text if the current query on attributes evaluates to true
  if(!attributes.query()) return c;
  
  //cerr << "overflow\n";
  if (c == EOF)
  {
     return !EOF;
  }
  else
  {
    int const r1 = baseBuf->sputc(c);
    return r1 == EOF ? EOF : c;
  }
}

// Prints the given string to the stream buffer
int dbgBuf::printString(string s)
{
  int r = baseBuf->sputn(s.c_str(), s.length());
  if(r!=(int)s.length()) return -1;
  return 0;
}

streamsize dbgBuf::xsputn(const char * s, streamsize n)
{
  //cerr << "xputn() << ownerAccess="<<ownerAccess<<" n="<<n<<" s=\""<<string(s)<<"\" query="<<attributes.query()<<"\n";
  
  // Only emit text if the current query on attributes evaluates to true
  if(!attributes.query()) return n;
  
  // If the owner is printing, output their text exactly
  if(ownerAccess) {
    int ret = baseBuf->sputn(s, n);
    //cerr << "xputn() >>>\n";
    return ret;
  } else {
    // Otherwise, replace all special characters with their HTML encodings
    int ret;
    int i=0;
    char open[]="&#91;";
    char close[]="&#93;";
    while(i<n) {
      if(s[i]=='[') {
        ret = baseBuf->sputn(open, sizeof(open)-1);
        if(ret != sizeof(open)-1) return 0;
      } else if(s[i]==']') {
        ret = baseBuf->sputn(close, sizeof(close)-1);
        if(ret != sizeof(close)-1) return 0;
      } else {
        ret = baseBuf->sputn(&(s[i]), 1);
        if(ret != 1) return 0;
      }
      i++;
    }

//    cerr << "xputn() >>>\n";
    return n;
  }
}

// Sync buffer.
int dbgBuf::sync()
{
  // Only emit text if the current query on attributes evaluates to true
  //  if(!attributes.query()) return 0;
  //cerr << "dbgBuf::sync() attributes.query()="<<attributes.query()<<"\n";
  
  // Only emit text if the current query on attributes evaluates to true
  if(!attributes.query()) return 0;
  
  int r = baseBuf->pubsync();
  if(r!=0) return -1;

  if(synched && !ownerAccess) {
    int ret;
    //ret = printString("<br>\n");    if(ret != 0) return 0;
    synched = false;
  }
  synched = true;

  return 0;
}

// Switch between the owner class and user code writing text
void dbgBuf::userAccessing() { ownerAccess = false; synched = true; }
void dbgBuf::ownerAccessing() { ownerAccess = true; synched = true; }

/*********************
 ***** dbgStream *****
 *********************/

dbgStream::dbgStream() : common::dbgStream(&defaultFileBuf), sightObj(this), initialized(false)
{
  dbgFile = NULL;
  //buf = new dbgBuf(cout.rdbuf());
  buf = new dbgBuf(preInitStream.rdbuf());
  ostream::init(buf);
}

dbgStream::dbgStream(properties* props, string title, string workDir, string imgDir, std::string tmpDir)
  : common::dbgStream(&defaultFileBuf), sightObj(this)
{
  init(props, title, workDir, imgDir, tmpDir);
}

void dbgStream::init(properties* props, string title, string workDir, string imgDir, std::string tmpDir)
{
  this->title   = title;
  this->workDir = workDir;
  this->imgDir  = imgDir;
  this->tmpDir  = tmpDir;

  numImages++;
  
  // Version 1: write output to a file 
  // Create the output file to which the debug log's structure will be written
  if(getenv("SIGHT_FILE_OUT")) {
    dbgFile = &(createFile(txt()<<workDir<<"/structure"));
    // Call the parent class initialization function to connect it dbgBuf of the output file
    buf=new dbgBuf(dbgFile->rdbuf());
  // Version 2 (default): write output to a pipe for a caller-specified layout executable to use immediately
  } else if(getenv("SIGHT_LAYOUT_EXEC")) {
//cout << "getenv(\"SIGHT_LAYOUT_EXEC\")="<<getenv("SIGHT_LAYOUT_EXEC")<<endl;
    dbgFile = NULL;
    // Unset the mutex environment variables from LoadTimeRegistry to make sure that they don't leak to the layout process
    LoadTimeRegistry::liftMutexes();
    
    // Execute the layout process
    FILE *out = popen(getenv("SIGHT_LAYOUT_EXEC"), "w");
    if(out == NULL) { cerr << "Failed to run command \""<<getenv("SIGHT_LAYOUT_EXEC")<<"\"!"<<endl; assert(0); }
    
    // Restore the LoadTimeRegistry mutexes
    LoadTimeRegistry::restoreMutexes();
    
    int outFD = fileno(out);
    buf = new dbgBuf(new fdoutbuf(outFD));
  // Version 3 (default): write output to a pipe for the default slayout to use immediately
  } else {
    dbgFile = NULL;
    // Unset the mutex environment variables from LoadTimeRegistry to make sure that they don't leak to the layout process
    LoadTimeRegistry::liftMutexes();
    
    // Execute the layout process
    FILE *out = popen((txt()<<ROOT_PATH<<"/slayout").c_str(), "w");
    if(out == NULL) { cerr << "Failed to run command \""<<ROOT_PATH<<"/slayout\"!"<<endl; assert(0); }
    
    // Restore the LoadTimeRegistry mutexes
    LoadTimeRegistry::restoreMutexes();
    
    int outFD = fileno(out);
    buf = new dbgBuf(new fdoutbuf(outFD));
  }
  ostream::init(buf);
  
  this->props = props; 
  //if(props) enter(this);
  sightObj::init(props, false);

  // The application may have written text to this dbgStream before it was fully initialized.
  // This text was stored in preInitStream. Print it out now.
  ownerAccessing();
  *this << preInitStream.str();
  userAccessing();
  
  initialized = true;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void dbgStream::destroy() {
  this->~dbgStream();
}

dbgStream::~dbgStream() {
  assert(!destroyed);
  
  if (!initialized)
    return;
  
  // Emit the exit tag for this dbgStream
  sightObj::exitTag(false);
  
  // If we're processing the destructor of the static dbgStream object and we're not doing this as
  // part of a Sight-driven destruction process (i.e. the process is being shut down), record that 
  // the stack may no longer be valid so that the sighObj destructor knows not to access it
  if(this == &dbg && !SightDestruction) 
    sightObj::stackMayBeInvalid();
  
//  assert(dbgFile);
  if(dbgFile) dbgFile->close();
  
  { ostringstream cmd;
    cmd << "rm -rf " << tmpDir;
    system(cmd.str().c_str());
  }

  /*// If this is the static dbgStream object, det the destroyed flag to true to keep the sightObj destructor 
  // from removing this object from the object stack, which may have already been destroyed.
  if(this == &dbg)
    destroyed = true;*/
}

// Called when a block is entered.
// b: The block that is being entered
void dbgStream::enterBlock(block* b) {
  subBlockEnterNotify(b);

  blocks.push_back(b);
  loc.enterBlock();
}

// Called when a block is exited. Returns the block that was exited.
block* dbgStream::exitBlock() {
  assert(blocks.size()>0);
 
  loc.exitBlock();
 
  block* lastB = blocks.back();
  blocks.pop_back();

  subBlockExitNotify(lastB);

  return lastB;
}

// Called to inform the blocks that contain the given block that it has been entered
void dbgStream::subBlockEnterNotify(block* subBlock)
{
  // Walk up the block stack, informing each block about the new arrival until the block's
  // subBlockEnterNotify() function returns false to indicate that the notification should not be propagated further.
  for(list<block*>::const_reverse_iterator b=blocks.rbegin(); b!=blocks.rend(); b++)
  {
    if(!(*b)->subBlockEnterNotify(subBlock)) return;
  }
}

// Called to inform the blocks that contain the given block that it has been exited
void dbgStream::subBlockExitNotify(block* subBlock)
{
  // Walk up the block stack, informing each block about the new exit until the block's
  // subBlockExitNotify() function returns false to indicate that the notification should not be propagated further.
  for(list<block*>::const_reverse_iterator b=blocks.rbegin(); b!=blocks.rend(); b++)
    if(!(*b)->subBlockExitNotify(subBlock)) return;
}

// Switch between the owner class and user code writing text into this stream
void dbgStream::userAccessing() { 
  buf->userAccessing();
}

void dbgStream::ownerAccessing()  { 
  buf->ownerAccessing();
}

// Adds an image to the output with the given extension and returns the path of this image
// so that the caller can write to it.
string dbgStream::addImage(string ext)
{
  if(!initializedDebug) SightInit("Debug Output", "dbg");
  
  ostringstream imgFName; imgFName << imgDir << "/image_" << numImages << "." << ext;
  
  properties p;
  map<string, string> newProps;
  newProps["path"] = imgFName.str();
  newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
  p.add("image", newProps);
  
  tag(p);
  return imgFName.str();
}

// Emit the entry into a tag to the structured output file. The tag is set to the given property key/value pairs
//void dbgStream::enter(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom) {
void dbgStream::enter(sightObj* obj) {
  ownerAccessing();
  *this << enterStr(*(obj->props));
  userAccessing();
}

void dbgStream::enter(const properties& props) {
  ownerAccessing();
  *this << enterStr(props);
  userAccessing();
}

// Returns the text that should be emitted to the structured output file that denotes the the entry into a tag. 
// The tag is set to the given property key/value pairs
//string dbgStream::enterStr(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom) {
string dbgStream::enterStr(const properties& props) {
  ostringstream oss;
  
  //for(list<pair<string, map<string, string> > >::const_iterator i=props.begin(); i!=props.end(); i++) {
  for(properties::iterator i(props); !i.isEnd(); i++) {
    properties::iterator iNext=i; iNext++;
    oss << "["<<(!iNext.isEnd()? "|": "")<<i.name()<<" ";
    oss << "numProperties=\""<<i.getNumKeys()<<"\"";
    
    int j=0;
    for(std::map<std::string, std::string>::const_iterator p=i.getMap().begin(); p!=i.getMap().end(); p++, j++) {
      oss << " name"<<j<<"=\""<<escape(p->first)<<"\" val"<<j<<"=\""<<escape(p->second)<<"\"";
    }
    
    oss << "]";
  }
  
  return oss.str();
}

// Emit the exit from a given tag to the structured output file
//void dbgStream::exit(std::string name) {
void dbgStream::exit(sightObj* obj) {
  ownerAccessing();
/*cout << "props="<<obj->props->str()<<endl;
cout << exitStr(*(obj->props)) << endl;*/
  *this << exitStr(*(obj->props));
  userAccessing();
}

void dbgStream::exit(const properties& props) {
  ownerAccessing();
  *this << exitStr(props);
  userAccessing();
}

// Returns the text that should be emitted to the the structured output file to that denotes exit from a given tag
//std::string dbgStream::exitStr(std::string name) {
std::string dbgStream::exitStr(const properties& props) {
  ostringstream oss;
  oss <<"[/"<<props.name()<<"]";
  return oss.str();
}

// Emit an entry an an immediate exit  
//void dbgStream::tag(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom)
void dbgStream::tag(sightObj* obj)
{
  enter(obj);
  exit(obj);
}

void dbgStream::tag(const properties& props) {
  enter(props);
  exit(props);
}

// Returns the text that should be emitted to the the structured output file to that denotes a full tag an an the structured output file
//std::string dbgStream::tagStr(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom) {
std::string dbgStream::tagStr(const properties& props) {
  return enterStr(props) + exitStr(props);
}


/***************************
 ***** dbgStreamMerger *****
 ***************************/

// The directory into which the merged will be written. This directory must be explicitly set before
// an instance of the dbgStreamMerger class is created
std::string dbgStreamMerger::workDir;

dbgStreamMerger::dbgStreamMerger(//std::string workDir, 
                                 std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                 map<string, streamRecord*>& outStreamRecords,
                                 vector<map<string, streamRecord*> >& inStreamRecords,
                                 properties* props) : 
                                         Merger(advance(tags), outStreamRecords, inStreamRecords, props) {
  assert(tags.size()>0);
  map<string, string> pMap;
  
  if(props==NULL) props = new properties();
  this->props = props;

  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "sight");
  
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging dbgStream!"<<endl; assert(0); }
  if(type==properties::enterTag) {
    pMap["workDir"] = workDir;
    
    pMap["title"] = getMergedValue(tags, "title");
    
    // Merge the command lines, environments, machine and user details (info required to 
    // re-execute the application) across all the runs. Treat this as unknown if there are
    // any inconsistencies.
    if(keyExists(tags, "commandLineKnown")) {
    vector<string> commandLineKnownValues = getValues(tags, "commandLineKnown");
    if(allSame<string>(commandLineKnownValues) && *commandLineKnownValues.begin()=="1") {
      map<string, string> execPMap;
      long argc;
      { vector<string> argcValues = getValues(tags, "argc");
      //assert(allSame<string>(argcValues));
      execPMap["argc"] = *argcValues.begin();
      // If the command lines are inconsistent, act as if the re-execution info is unknown
      if(!allSame<string>(argcValues)) goto LABEL_INCONSISTENT_EXEC;
      argc = strtol((*argcValues.begin()).c_str(), NULL, 10); }
      
      for(long i=0; i<argc; i++) {
        vector<string> argvValues = getValues(tags, txt()<<"argv_"<<i);
        // If the command lines are inconsistent, act as if the re-execution info is unknown
        if(!allSame<string>(argvValues)) goto LABEL_INCONSISTENT_EXEC;
        
        //assert(allSame<string>(argvValues));
        execPMap[txt()<<"argv_"<<i] = *argvValues.begin();
      }
      
      { vector<string> execFileValues = getValues(tags, "execFile");
      // If the command lines are inconsistent, act as if the re-execution info is unknown
      if(!allSame<string>(execFileValues)) goto LABEL_INCONSISTENT_EXEC;
      execPMap["execFile"] = *execFileValues.begin(); }
      
      long numEnvVars;
      { vector<string> numEnvVarsValues = getValues(tags, "numEnvVars");
      // If the environments are inconsistent, act as if the re-execution info is unknown
      if(!allSame<string>(numEnvVarsValues)) goto LABEL_INCONSISTENT_EXEC;
      execPMap["numEnvVars"] = *numEnvVarsValues.begin();
      numEnvVars = strtol((*numEnvVarsValues.begin()).c_str(), NULL, 10); }
      
      for(long i=0; i<numEnvVars; i++) {
        { vector<string> envNameValues = getValues(tags, txt()<<"envName_"<<i);
        // If the environments are inconsistent, act as if the re-execution info is unknown
        if(!allSame<string>(envNameValues)) goto LABEL_INCONSISTENT_EXEC;
        execPMap[txt()<<"envName_"<<i] = *envNameValues.begin(); }
        
        { vector<string> envValValues = getValues(tags, txt()<<"envVal_"<<i);
        // If the environments are inconsistent, act as if the re-execution info is unknown
        if(!allSame<string>(envValValues)) goto LABEL_INCONSISTENT_EXEC;
        execPMap[txt()<<"envVal_"<<i] = *envValValues.begin(); }
      }
      
      long numHostnames;
      { vector<string> numHostnamesValues = getValues(tags, "numHostnames");
      // If the machine details are inconsistent, act as if the re-execution info is unknown
      if(!allSame<string>(numHostnamesValues)) goto LABEL_INCONSISTENT_EXEC;
      execPMap["numHostnames"] = *numHostnamesValues.begin(); 
      numHostnames = strtol((*numHostnamesValues.begin()).c_str(), NULL, 10); }
      
      for(long i=0; i<numHostnames; i++) {
        vector<string> hostnameValues = getValues(tags, txt()<<"hostname_"<<i);
        // If the machine details are inconsistent, act as if the re-execution info is unknown
        if(!allSame<string>(hostnameValues)) goto LABEL_INCONSISTENT_EXEC;
        execPMap[txt()<<"hostname_"<<i] = *hostnameValues.begin();
      }
      
      { vector<string> usernameValues = getValues(tags, "username");
      // If the user details are inconsistent, act as if the re-execution info is unknown
      if(!allSame<string>(usernameValues)) goto LABEL_INCONSISTENT_EXEC;
      execPMap["username"] = *usernameValues.begin(); }
      
      // All the execution info is consistent, so add it to pMap
      pMap.insert(execPMap.begin(), execPMap.end());
      pMap["commandLineKnown"] = "1";
      goto LABEL_EXEC_END;
    } }

    // The label we'll jump to if we discover that the command line is not the same
    // across all runs
    LABEL_INCONSISTENT_EXEC:
    pMap["commandLineKnown"] = "0";

    // The label we'll jump to when we've finished processing the command line
    LABEL_EXEC_END: ;   
    
    props->add("sight", pMap);
    //SightInit_internal(props, false);
  } else
    props->add("sight", pMap);
}

// vSuffixID: ID that identifies this variant within the next level of variants in the heirarchy
dbgStreamStreamRecord::dbgStreamStreamRecord(const dbgStreamStreamRecord& that, int vSuffixID) : 
  streamRecord((const streamRecord&)that, vSuffixID), loc(that.loc), emitFlags(that.emitFlags)
{ }

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* dbgStreamStreamRecord::copy(int vSuffixID) {
  return new dbgStreamStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void dbgStreamStreamRecord::resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams) {
  streamRecord::resumeFrom(streams);
  
  // Set emitFlags to be the union of its counterparts in streams
  emitFlags.clear();
  
  // Due to the fact that hierarchical merging only recurses from identical tag stacks, the emitFlag lists
  // must be identical in all the streams
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    dbgStreamStreamRecord* ds = (dbgStreamStreamRecord*)(*s)["sight"];
    
    if(s==streams.begin()) emitFlags = ds->emitFlags;
    else assert(emitFlags == ds->emitFlags);
  }
}

// Called when a block is entered.
void dbgStreamStreamRecord::enterBlock(vector<map<std::string, streamRecord*> >& streamRecords) {
  //cout << "dbgStreamStreamRecord::enterBlock() In\n";
  for(vector<map<std::string, streamRecord*> >::iterator r=streamRecords.begin(); r!=streamRecords.end(); r++) {
    assert(r->find("sight") != r->end());
    ((dbgStreamStreamRecord*)(*r)["sight"])->loc.enterBlock();
  }
}

void dbgStreamStreamRecord::enterBlock(map<std::string, streamRecord*>& streamRecord) {
  //cout << "dbgStreamStreamRecord::enterBlock() Out\n";
  assert(streamRecord.find("sight") != streamRecord.end());
  ((dbgStreamStreamRecord*)streamRecord["sight"])->loc.enterBlock();
}

// Called when a block is exited.
void dbgStreamStreamRecord::exitBlock(std::vector<std::map<std::string, streamRecord*> >& streamRecords) {
  //cout << "dbgStreamStreamRecord::exitBlock() In\n";
  for(vector<map<std::string, streamRecord*> >::iterator r=streamRecords.begin(); r!=streamRecords.end(); r++) {
    assert(r->find("sight") != r->end());
    ((dbgStreamStreamRecord*)(*r)["sight"])->loc.exitBlock();
  }
}

void dbgStreamStreamRecord::exitBlock(map<std::string, streamRecord*>& streamRecord) {
  //cout << "dbgStreamStreamRecord::exitBlock() Out\n";
  assert(streamRecord.find("sight") != streamRecord.end());
  ((dbgStreamStreamRecord*)streamRecord["sight"])->loc.exitBlock();
}

std::string dbgStreamStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[dbgStreamStreamRecord: s="<<s.str()<<"]";
  return s.str();
}

/******************
 ***** indent *****
 ******************/

indent::indent(std::string prefix,                                       properties* props) : sightObj(setProperties(prefix, 1,         NULL,     props)) {}
indent::indent(std::string prefix, int repeatCnt, const attrOp& onoffOp, properties* props) : sightObj(setProperties(prefix, repeatCnt, &onoffOp, props)) {}
indent::indent(std::string prefix, int repeatCnt,                        properties* props) : sightObj(setProperties(prefix, repeatCnt, NULL,     props)) {}
indent::indent(                    int repeatCnt,                        properties* props) : sightObj(setProperties("    ", repeatCnt, NULL,     props)) {}
indent::indent(std::string prefix,                const attrOp& onoffOp, properties* props) : sightObj(setProperties(prefix, 1,         &onoffOp, props)) {}
indent::indent(                    int repeatCnt, const attrOp& onoffOp, properties* props) : sightObj(setProperties("    ", repeatCnt, &onoffOp, props)) {}
indent::indent(                                   const attrOp& onoffOp, properties* props) : sightObj(setProperties("    ", 1,         &onoffOp, props)) {}
indent::indent(                                                          properties* props) : sightObj(setProperties("    ", 1,         NULL,     props)) {}


properties* indent::setProperties(std::string prefix, int repeatCnt, const attrOp* onoffOp, properties* props) {
  if(props==NULL) props = new properties();
    
  if(repeatCnt>0 && attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    newProps["prefix"] = prefix;
    newProps["repeatCnt"] = txt()<<repeatCnt;
    //newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
    props->add("indent", newProps);
  } else
    props->active = false;
    
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void indent::destroy() {
  this->~indent();
}

indent::~indent() {
  assert(!destroyed);
}

IndentMerger::IndentMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                           map<string, streamRecord*>& outStreamRecords,
                           vector<map<string, streamRecord*> >& inStreamRecords,
                           properties* props) : 
                                      Merger(advance(tags), outStreamRecords, inStreamRecords, props) {
  assert(tags.size()>0);
  
  if(props==NULL) props = new properties();
  this->props = props;
  
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "indent");
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Indent!"<<endl; assert(0); }
  if(type==properties::enterTag) {
    pMap["prefix"] = getMergedValue(tags, "prefix");
    pMap["repeatCnt"] = txt()<<vAvg(str2int(getValues(tags, "repeatCnt")));
    
    /*vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();*/
  }
  
  props->add("indent", pMap);
}


char printbuf[100000];
int dbgprintf(const char * format, ... )    
{
  va_list args;
  va_start(args, format);
  vsnprintf(printbuf, 100000, format, args);
  va_end(args);
  
  dbg << printbuf;
  
  return 0;// Before return you can redefine it back if you want...
}

}; // namespace structure
}; // namespace sight
