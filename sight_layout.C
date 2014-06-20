// Licence information included in file LICENCE
#include "sight_layout_internal.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include "binreloc.h"
#include <errno.h>
#include "sight_common.h"
#include "getAllHostnames.h"
#include "process.h"
#include "process.C"

//#define VERBOSE

using namespace std;
using namespace sight::common;

namespace sight {
namespace layout {

// Maps the names of various tags that may be read by the layout reader to the functions
// that instantiate the objects they encode.
// An entry handler is called when the object's entry tag is encountered in the debug log. It 
//   takes as input the properties of the object to be laid out and returns a pointer to the 
//   created object. NULL is a valid return value.
// An exit handler is called when the object's exit tag is encountered and takes as input a pointer
//   to the object.
// It is assumed that all objects are hierarchically scoped, in that objects are exted in the 
//   reverse order of their entry. The layout engine keeps track of the entry/exit stacks and 
//   ensures that the appropriate object pointers are passed to exit handlers.
std::map<std::string, layoutEnterHandler>* layoutHandlerInstantiator::layoutEnterHandlers;
std::map<std::string, layoutExitHandler>* layoutHandlerInstantiator::layoutExitHandlers;

layoutHandlerInstantiator::layoutHandlerInstantiator() : 
  sight::common::LoadTimeRegistry("layoutHandlerInstantiator", 
                                   layoutHandlerInstantiator::init) 
{}

void layoutHandlerInstantiator::init() {
  layoutEnterHandlers = new std::map<std::string, layoutEnterHandler>();
  layoutExitHandlers  = new std::map<std::string, layoutExitHandler>();
}

// Default entry/exit handlers to use when no special handling is needed
void* defaultEntryHandler(properties::iterator props) { return NULL; }
void  defaultExitHandler(void* obj) { }

void* indentEnterHandler(properties::iterator props) { return new indent(props); }
void  indentExitHandler(void* obj) { indent* i = static_cast<indent*>(obj); delete i; }

sightLayoutHandlerInstantiator::sightLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["sight"]  = &SightInit;
  (*layoutExitHandlers )["sight"]  = &defaultExitHandler;
  (*layoutEnterHandlers)["indent"] = &indentEnterHandler;
  (*layoutExitHandlers )["indent"] = &indentExitHandler;
  (*layoutEnterHandlers)["link"]   = &anchor::link;
  (*layoutExitHandlers )["link"]   = &defaultExitHandler;
}
sightLayoutHandlerInstantiator sightLayoutHandlerInstantance;

// Call the entry handler of the most recently-entered object with name objName
// and push the object it returns onto the stack dedicated to objects of this type.
void invokeEnterHandler(map<string, list<void*> >& stack, string objName, properties::iterator iter) {
  #ifdef VERBOSE
  cout << "<<<"<<stack[objName].size()<<": "<<objName<<endl;
  cout << "    "<<iter.str()<<endl;
  #endif
  if(layoutHandlerInstantiator::layoutEnterHandlers->find(objName) == layoutHandlerInstantiator::layoutEnterHandlers->end()) { cerr << "ERROR: no entry handler for \""<<objName<<"\" tags!" << endl; }
  assert(layoutHandlerInstantiator::layoutEnterHandlers->find(objName) != layoutHandlerInstantiator::layoutEnterHandlers->end());
  stack[objName].push_back((*layoutHandlerInstantiator::layoutEnterHandlers)[objName](iter));
}

// Call the exit handler of the most recently-entered object with name objName
// and pop the object off its stack
void invokeExitHandler(map<string, list<void*> >& stack, string objName) {
  #ifdef VERBOSE
  cout << ">>>"<<stack[objName].size()<<": "<<objName<<endl;
  #endif
  assert(stack[objName].size()>0);
  if(layoutHandlerInstantiator::layoutEnterHandlers->find(objName) == layoutHandlerInstantiator::layoutEnterHandlers->end()) { cerr << "ERROR: no exit handler for \""<<objName<<"\" tags!" << endl; }
  assert(layoutHandlerInstantiator::layoutExitHandlers->find(objName) != layoutHandlerInstantiator::layoutExitHandlers->end());
  (*layoutHandlerInstantiator::layoutExitHandlers)[objName](stack[objName].back());
  stack[objName].pop_back();
}

// Given a parser that reads the structure of a given log file, lays it out and prints it to the output Sight stream
void layoutStructure(structureParser& parser) {
  #ifdef VERBOSE
  cout << "layoutHandlers:\n";
  for(map<std::string, layoutEnterHandler>::iterator i=layoutHandlerInstantiator::layoutEnterHandlers->begin(); i!=layoutHandlerInstantiator::layoutEnterHandlers->end(); i++)
    cout << i->first << endl;
  #endif

  // The stack of all the objects of each type that have been entered but not yet exited
  map<string, list<void*> > stack;
  
  pair<properties::tagType, const properties*> props = parser.next();
  while(props.second->size()>0) {
    if(props.first == properties::enterTag) {
      // If this is just text between tags, print it out 
      if(props.second->name() == "text")
        //fprintf(f, "%s", properties::get(props.second->begin(), "text").c_str());
        dbg << properties::get(props.second->begin(), "text");
      
      // Else, if this is the entry into a new tag, process it
      else {
        // Call the entry handler of the most recently-entered object with this tag name
        // and push the object it returns onto the stack dedicated to objects of this type.
        invokeEnterHandler(stack, props.second->name(), props.second->begin());
                  
        // If this tag denotes one or more variants of the log
        if(props.second->name() == "variants") {
          // Iterate through the structure files of all the variants, adding their layout to the log
          int numVariants = props.second->begin().getNumKeys();
          for(int i=0; i<numVariants; i++) {
            string variantDir = properties::get(props.second->begin(), txt()<<"var_"<<i);
            //cout << "variantDir="<<variantDir<<"\n";
            FILEStructureParser parser(variantDir+"/structure", 10000);
            layoutStructure(parser);
            if(i!=numVariants-1) invokeEnterHandler(stack, "inter_variants", props.second->begin());
          }
        }
      }
    } else if(props.first == properties::exitTag) {
      // Call the exit handler of the most recently-entered object with this tag name
      // and pop the object off its stack
      invokeExitHandler(stack, props.second->name());
    }
    props = parser.next();
  }
}

bool initializedDebug=false;
// Returns whether log generation has been enabled or explicitly disabled
bool isEnabled() {
  static bool checked=false;
  static bool enabledDebug; // Records whether log generation has been enabled or explicitly disabled
  if(!checked) {
    checked = true;
    enabledDebug = (getenv("DISABLE_DBGLOG") == NULL);
  }
  return enabledDebug;
}

// Records the information needed to call the application
bool saved_appExecInfo=false; // Indicates whether the application execution info has been saved
int argc = 0;
string* argv = NULL;
string execFile;

// All the aliases of the name of the host that this application is currently executing on
list<string> hostnames;

// The current user's user name 
string username;

void* SightInit(properties::iterator props) {
  if(!isEnabled()) return NULL;
  // Note: we allow users to specify argc and argv multiple times to support use-cases where
  // sight needs to be used and therefore initialized before main() starts and argc and argv 
  // are not available until after the initialization was first performed.
  // If a different title and workDir are provided in subsequent SightInit() calls, they
  // are ignored.

  /*char fname[1000];
  sprintf(fname, 1000, "%s/.dbginit", workDir); 
  FILE* dbginit = fopen(fname, "w");
  if(dbginit == NULL) { fprintf(stderr, "ERROR opening file \"%s\" for writing! %s\n", fname, strerror(errno)); exit(-1); }
  
  fclose(dbginit);*/
  
  loadSightConfig(configFileEnvVars("SIGHT_LAYOUT_CONFIG", "SIGHT_CONFIG"));
  
  if(properties::getInt(props, "commandLineKnown")) {
    saved_appExecInfo = true;
    
    argc = properties::getInt(props, "argc");
    argv = new string[argc];
    for(int i=0; i<argc; i++)
      argv[i] = properties::get(props, txt()<<"argv_"<<i);

    execFile = properties::get(props, "execFile");

  #if REMOTE_ENABLED
  if(!isPortUsed(GDB_PORT)) {
    ostringstream cmd; cmd << ROOT_PATH << "/widgets/mongoose/mongoose -document_root "<<ROOT_PATH<<" -listening_ports "<<GDB_PORT<<"&";
    system(cmd.str().c_str());
  }
  #endif
  
    // Get all the aliases of the current host's name
    long numHostnames = properties::getInt(props, "numHostnames");
    for(long i=0; i<numHostnames; i++)
      hostnames.push_back(properties::get(props, txt()<<"hostname_"<<i));

    username = properties::get(props, "username");
  } else
    saved_appExecInfo = false;
    
  string workDir = properties::get(props, "workDir");
  
  // Main output directory
  createDir(workDir, "");

  // Directory where the html files go
  createDir(workDir, "html");
  
  // Directory where the files from the individual widgets go
  createDir(workDir, "html/widgets");
  
  // Directory where client-generated images will go
  string imgDir = createDir(workDir, "html/dbg_imgs");
  
  // Copy the default images directory to the work directory
  copyDir(workDir+"/html", "img");

  // Copy the default scripts directory to the work directory
  copyDir(workDir+"/html", "script");
  
  // Create an index.html file that refers to the root html file
  copyDir(workDir, "html/index.html");
  
  // Directory that widgets can use as temporary scratch space
  string tmpDir = createDir(workDir, "html/tmp");
  
  initializedDebug = true;
  
  dbg.init(properties::get(props, "title"), workDir, imgDir, tmpDir);
    
  return NULL;
}


/***************
 ***** dbg *****
 ***************/

dbgStream dbg;


/******************
 ***** anchor *****
 ******************/
int anchor::minAnchorID=0;
anchor anchor::noAnchor(/*false,*/ -1);

// Associates each anchor with its location (if known). Useful for connecting anchor objects with started out
// unlocated (e.g. forward links) and then were located when we reached their target. Since there may be multiple
// copies of the original unlocated anchor running around, these copies won't be automatically updated. However,
// this map will always keep the latest information.
// Note: an alternative design would use smart pointers, since it would remove the need for keeping copies of anchors
//       around since no anchor would ever go out of scope. However, a dependence on boost or C++11 seems too much to add.
map<int, location> anchor::anchorLocs;

// Associates each anchor with a unique anchor ID. Useful for connecting multiple anchors that were created
// independently but then ended up referring to the same location. We'll record the ID of the first one to reach
// this location on locAnchorIDs and the others will be able to adjust themselves by adopting this ID.
map<location, int> anchor::locAnchorIDs;
  
/*anchor::anchor(bool located) {
  assert(initializedDebug);
  
  anchorID = --minAnchorID;
  //dbg << "anchor="<<anchorID<<endl;
  // Initialize this->located to false so that reachedLocation() doesn't think we're calling it for multiple locations
  this->located = false;
  if(located) reachedLocation();
}*/

anchor::anchor(const anchor& that) {
  assert(initializedDebug);
  
  anchorID = that.anchorID;
  loc      = that.loc;
  located  = that.located;
  update();
  //cout << "anchor::anchor() >>> this="<<str()<<" that="<<that.str()<<endl;
}

anchor::anchor(/*dbgStream& myDbg, bool located,*/ int anchorID) : 
  located(false), anchorID(anchorID)
{
//  assert(initializedDebug);
  update();
}

/*void anchor::init(bool located) {
  anchorID = --minAnchorID;
  // Initialize this->located to false so that reachedLocation() doesn't think we're calling it for multiple locations
  this->located = false;
  if(located) reachedLocation();
}*/

// If this anchor is unlocated, checks anchorLocs to see if a location has been found and updates this
// object accordingly;
void anchor::update() {
  /*cout << "  anchor::update() located="<<located<<" anchorID="<<anchorID<<endl;
  cout << "        anchorLocs="<<endl;
  for(map<int, location>::iterator i=anchorLocs.begin(); i!=anchorLocs.end(); i++)
    cout << "            "<<i->first<<" => "<<dbg.blockGlobalStr(i->second)<<endl;*/
  
  // If this copy of the anchor object is not yet located, check if another copy of this object has reached
  // a location and if so, copy it over here.
  if(!located && (anchorLocs.find(anchorID) != anchorLocs.end())) {
    located = true;
    loc = anchorLocs[anchorID];
  }
  
  //cout << "  anchor::update() mid located="<<located<<endl;
  
  // If this is the first anchor at this location, associate this location with this anchor ID 
  if(located) {
    if(locAnchorIDs.find(loc) == locAnchorIDs.end())
      locAnchorIDs[loc] = anchorID;
    // If this is not the first anchor here, update this anchor object's ID to be the same as all
    // the other anchors at this location
    else
      anchorID = locAnchorIDs[loc];
  }
  //cout << "  anchor::update() final located="<<located<<", anchorID="<<anchorID<<endl;
}

void anchor::operator=(const anchor& that) {
  anchorID = that.anchorID;
  loc      = that.loc;
  located  = that.located;
  update();
}

bool anchor::operator==(const anchor& that) const {
  // anchorID of -1 indicates that this is the noAnchor object and any copies of it are equivalent
  assert(anchorID>=-1); assert(that.anchorID>=-1);
  if(anchorID==-1 && that.anchorID==-1) return true;

  const_cast<anchor*>(this)->update();
  const_cast<anchor &>(that).update();
  
/*  // If we've identified the locations of both anchors
  if(located && that.located)
    // They're equal if they refer to the same location
    return loc == that.loc;
  // If we've identified the location of neither object 
  else if(!located && !that.located)
    // They're equal if they have the same ID
    return anchorID == that.anchorID;
  // If one has been located while the other has not, they're definitely not equal
  else
    return false;*/
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
  
  const_cast<anchor*>(this)->update();
  const_cast<anchor &>(that).update();
  
/*  // If we'ver identified the locations of both anchors
  if(located && that.located)
    // Compare their locations
    return loc < that.loc;
  // If we've identified the location of neither object 
  else if(!located && !that.located)
    // Compare their IDs
    return anchorID < that.anchorID;
  // If one has been located while the other has not, compare their located-ness statuses
  else
    return located < that.located;*/

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

const location& anchor::getLocation() const
{ 
  const_cast<anchor*>(this)->update();
  return loc;
}

// Called when the file location of this anchor has been reached
void anchor::reachedLocation() {
  //cout << "    reachedLocation() located="<<located<<", anchorID="<<anchorID<<" dbg.getLocation()="<<dbg.blockGlobalStr(dbg.getLocation())<<"<BR>"<<endl;
  // If this anchor has already been set to point to its target location, emit a warning
  if(located && loc != dbg.getLocation())
    cerr << "Warning: anchor "<<anchorID<<" is being set to multiple target locations! current location="<<dbg.blockGlobalStr(loc)<<", new location="<<dbg.blockGlobalStr(dbg.getLocation())<< endl;
  else {
    located = true; // We've now reached this anchor's location in the output
    loc     = dbg.getLocation();
    //cout << "        loc="<<dbg.blockGlobalStr(loc)<<endl;
    
    dbg.writeToAnchorScript(anchorID, loc);
    
    // Record the connection between this anchor ID and the location in the global table.
    // If there are other copies of this anchor object with the same anchor ID they'll notice
    // this new location when they call update and use this object's location. 
    anchorLocs[anchorID] = loc;
    
    // Update this anchor object based on the currently known about its location. 
    // Since this object is known to be located, all that we'll do here is check if this is not
    // the first anchor at this location and if so, update its anchorID to be the same as all
    // the anchors at this location.
    update();
  }
  //cout << "        this="<<str()<<endl;
}

// Emits an <a href> tag that denotes a link to an anchor.
void* anchor::link(properties::iterator props) {
  anchor a(/*false,*/ properties::getInt(props, "anchorID"));
  
  if(properties::getInt(props, "img"))
    a.linkImg(properties::get(props, "text"));
  else
    a.link(properties::get(props, "text"));
  
  return NULL;
}

void anchor::link(string text) const {
  const_cast<anchor*>(this)->update();
  
  dbg << "<a href=\"javascript:" << getLinkJS() << "\">"<<text<<"</a>";
}

// Returns an <a href> tag that denotes a link to this anchor, using the default link image, which is followed by the given text.
void anchor::linkImg(string text) const {
  const_cast<anchor*>(this)->update();
  
  dbg << "<a href=\"javascript:" << getLinkJS() << "\">"<<
         "<img src=\"img/link"<<(isLocated()?"Up":"Down")<<".gif\">"<<
          text<<
         "</a>";
}

// Returns the JavaScript code that links to this anchor, which can be embedded on other javascript code.
std::string anchor::getLinkJS() const {
  const_cast<anchor*>(this)->update();
  
  //cout << "anchor::getLinkJS() a="<<str()<<", anchorID="<<anchorID<<endl;
  
  ostringstream oss;
  // If we've already reached this link's location (this is a backward link)
  if(located) {
    oss << "goToAnchor([], "<<dbg.fileLevelJSIntArray(loc)<<",  ";
    oss << "function() { focusLinkDetail('"<<dbg.blockGlobalStr(loc)<<"'); focusLinkSummary('"<<dbg.blockGlobalStr(loc)<<"');});";
  // If we have not yet reached this anchor's location in the output (it is a forward link)
  } else {
    oss << "loadAnchorScriptsFile("<<(anchorID/dbg.getAnchorsPerScriptFile())<<", "<<
             "function() { "<<
               "if(anchors.hasItem("<<anchorID<<")) { "<<
                 "goToAnchor([], anchors.getItem("<<anchorID<<").fileID, "<<
                   "function() { focusLinkDetail(anchors.getItem("<<anchorID<<").blockID); focusLinkSummary(anchors.getItem("<<anchorID<<").blockID); }); "<<
               "} else { "<<
                 "focusLinkDetail(anchors.getItem("<<anchorID<<").blockID); focusLinkSummary(anchors.getItem("<<anchorID<<").blockID); "<<
               "} "<<
             "}"<<
           ");";
  }
  //cout << oss.str()<<endl;
  return oss.str();
}

std::string anchor::str(std::string indent) const {
  const_cast<anchor*>(this)->update();
  
  ostringstream oss;
  if(anchorID==-1)
    oss << "[noAnchor]";
  else {
    oss << "[anchor: ID="<<anchorID<<", loc=&lt;"<<dbg.blockGlobalStr(loc)<<"&gt;, located=\""<<located<<"\"]";
  }
  return oss.str();
}


/*****************
 ***** block *****
 *****************/

int block::blockCount=0;

// Initializes this block with the given properties
block::block(properties::iterator props) : startA(/*false,*/ -1) /*=noAnchor, except that noAnchor may not yet be initialized)*/ {
  assert(initializedDebug);
  label = properties::get(props, "label");
  // Record the ID assigned to this block in the structure layer
  blockIDFromStructure = properties::getInt(props, "ID");
  long numAnchors = properties::getInt(props, "numAnchors");
  for(long i=0; i<numAnchors; i++) {
    pointsToAnchors.insert(anchor(/*false,*/ properties::getInt(props, txt()<<"anchor_"<<i)));
  }
  
  startA.setID(properties::getInt(props, "anchorID"));
    
  scriptFile       = dbg.getCurScriptFile();      // assert(scriptFile);
  scriptPrologFile = dbg.getCurScriptPrologFile();// assert(scriptPrologFile);
  scriptEpilogFile = dbg.getCurScriptEpilogFile();// assert(scriptEpilogFile);

  blockCount++;
}

// Initializes this block with the given label, used for creating additional blocks that were not listed in the structure file
block::block(string label) : label(label), startA(/*false,*/ -1) /*=noAnchor, except that noAnchor may not yet be initialized)*/ {
  scriptFile       = dbg.getCurScriptFile();      // assert(scriptFile);
  scriptPrologFile = dbg.getCurScriptPrologFile();// assert(scriptPrologFile);
  scriptEpilogFile = dbg.getCurScriptEpilogFile();// assert(scriptEpilogFile); 

  blockCount++;
}

block::~block() {
  //cout << "deleting block "<<this<<", label="<<label<<endl;
}

// Attaches a given un-located anchor at this block
void block::attachAnchor(anchor& a) {
  // If this block has not yet been located, add the anchor to pointsToAnchors so that it can be
  // located in the call to setLocation())
  if(loc.size()==0)
    pointsToAnchors.insert(a);
  // Otherwise, set its location directly
  else
    a.reachedLocation();
}

void block::setLocation(const location& loc) { 
  this->loc = loc;
  blockID = dbg.blockGlobalStr(loc);
  fileID = dbg.fileLevelStr(loc);
  
  //cout << "block::setLocation() blockID="<<blockID<<", anchorID="<<startA.getID()<<endl;
  
  // We don't need to initialize startA since it as either initialized in the constructor based
  // on the properties from the structure file or should remain equal to noAnchor.
  //startA.init(true);
  // If this block's anchor was specified in the structure file, update it's location. 
  // Otherwise, ignore it since this anchor will not be used.
  if(startA.getID()!=-1) startA.reachedLocation();
  
  //cout << "block("<<getLabel()<<")::setLocation() <<< #pointsToAnchors="<<pointsToAnchors.size()<<"\n";
  for(set<anchor>::iterator a=pointsToAnchors.begin(); a!=pointsToAnchors.end(); a++) {
    anchor a2 = *a;
    a2.reachedLocation();
  }
  //cout << "block::setLocation() >>> \n";
}

anchor& block::getAnchorRef()
{ return startA; }

anchor block::getAnchor() const
{ return startA; }

// Adds the given JavaScript command text to the script that will be loaded with the current file.
// This command is guaranteed to run after the body of the file is loaded but before the anchors
// referents are loaded.
void block::widgetScriptCommand(std::string command) {
  *scriptFile << command << endl;
}

// Adds the given JavaScript command text to be executed before the commands added with widgetScriptCommand()
void block::widgetScriptPrologCommand(std::string command) {
  *scriptPrologFile << command << endl;
}

// Adds the given JavaScript command text to be executed after the commands added with widgetScriptCommand()
void block::widgetScriptEpilogCommand(std::string command) {
  *scriptEpilogFile << command << endl;
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
  //justSynched = false;
  needIndent = false;
  indents.push_back(std::list<std::string>());
}

// This dbgBuf has no buffer. So every character "overflows"
// and can be put directly into the teed buffers.
int dbgBuf::overflow(int c)
{
  //cout << "overflow\n";
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

// Get the current indentation depth within the current div
int dbgBuf::getIndentDepth()
{
  return indents.rbegin()->size();
}

// Get the current indentation within the current div
std::string dbgBuf::getIndent()
{
  string out = "";
  if(indents.size()>0) {
    for(std::list<std::string>::iterator it=indents.rbegin()->begin(); it!=indents.rbegin()->end(); it++)
      out += *it;
    //cout << "getIndent("<<out<<")"<<endl;
  }
  return out;
}

// Add more indentation within the current div
void dbgBuf::addIndent(std::string indent)
{
  assert(indents.size()>0);
  indents.rbegin()->push_back(indent);
  //cout << "addIndent() #indents="<<indents.size()<<" = "<<getIndent()<<endl;
  
  /*if(justSynched) {
    int ret = printString(indent); if(ret != 0) return;
  }*/
  needIndent = true;
}

// Remove the most recently added indent within the current div
void dbgBuf::remIndent()
{
  //cout << "remIndent() #indents="<<indents.size()<<", indents.rbegin()->size()="<<indents.rbegin()->size()<<endl;
  assert(indents.size()>0);
  assert(indents.rbegin()->size()>0);
  indents.rbegin()->pop_back();
}

streamsize dbgBuf::xsputn(const char * s, streamsize n)
{
  /*cout << "dbgBuf::xsputn"<<endl;
  cout << "  blocks.size()="<<blocks.size()<<", needIndent="<<needIndent<<endl;
  cout.flush();*/
  
  //cout << "xputn() ownerAccess="<<ownerAccess<<" n="<<n<<" s=\""<<string(s)<<"\"\n";
  
  if(needIndent) {
    int ret = printString(escape(getIndent())); if(ret != 0) return 0;
    //cout << "getIndent()=\""<<getIndent()<<"\" escaped=\""<<escape(getIndent())<<"\""<<endl;
    needIndent = false;
  }
  // If the owner is printing, output their text exactly
  if(ownerAccess) {
    return baseBuf->sputn(s, n);
  // Otherwise, replace all line-breaks with <BR>'s
  } else {
    int ret;
    int i=0;
    char br[]="<BR>\n";
    char space[]=" ";
    char spaceHTML[]="&nbsp;";
    char tab[]="\t";
    char tabHTML[]="&#09;";
    //char lt[]="&lt;";
    //char gt[]="&gt;";
    while(i<n) {
      int j;
      // Scan through next sequence of non-whitespace/non-break characters, counting any 
      // openings and closings of HTML tags
      for(j=i; j<n && s[j]!='\n' && s[j]!=' ' && s[j]!='\t'/* && s[j]!='<' && s[j]!='>'*/; j++) {
        if(s[j]=='<') numOpenAngles++;
        else if(s[j]=='>') numOpenAngles--;
      }
      //cout << "char=\""<<s[j]<<"\" numOpenAngles="<<numOpenAngles<<"\n";
      
      // Send out all the bytes from the start of the string or the 
      // last line-break until this line-break
      if(j-i>0) {
        if(needIndent) {
          int ret = printString(escape(getIndent())); if(ret != 0) return 0;
          needIndent = false;
        }

        ret = baseBuf->sputn(&(s[i]), j-i);
        if(ret != (j-i)) return 0;
        //cout << "   printing char "<<i<<" - "<<j<<"\n";
      }

      // Consider the next whitespace/line break character
      if(j<n) {
        // If we're at at line-break Send out the line-break
        if(s[j]=='\n') {
          ret = baseBuf->sputn(br, sizeof(br)-1);
          if(ret != sizeof(br)-1) return 0;
          //string indent = getIndent();
          //cout << "New Line indent=\""<<getIndent()<<"\""<<endl;
          //baseBuf->sputn(indent.c_str(), indent.size());
          needIndent = true;
        } else if(s[j]==' ') {
          // If we're at a space and not inside an HTML tag, replace it with an HTML space escape code
          /*if(numOpenAngles==0) {
            ret = baseBuf->sputn(spaceHTML, sizeof(spaceHTML)-1);
            if(ret != sizeof(spaceHTML)-1) return 0;
          // If we're inside an HTML tag, emit a regular space character
          } else {*/
            ret = baseBuf->sputn(space, sizeof(space)-1);
            if(ret != sizeof(space)-1) return 0;
          //}
        } else if(s[j]=='\t') {
          // If we're at a tab and not inside an HTML tag, replace it with an HTML tab escape code
          if(numOpenAngles==0) {
            ret = baseBuf->sputn(tabHTML, sizeof(tabHTML)-1);
            if(ret != sizeof(tabHTML)-1) return 0;
          // If we're inside an HTML tag, emit a regular tab character
          } else {
            ret = baseBuf->sputn(tab, sizeof(tab)-1);
            if(ret != sizeof(tab)-1) return 0;
          }
        }/* else if(s[j]=='<') {
          ret = baseBuf->sputn(lt, sizeof(lt)-1);
          if(ret != sizeof(lt)-1) return 0;
        } else if(s[j]=='>') {
          ret = baseBuf->sputn(gt, sizeof(gt)-1);
          if(ret != sizeof(gt)-1) return 0;
        }*/
        //cout << "   printing <BR>\n";
      }

      // Point i to immediately after the line-break
      i = j+1;
    }

    return n;
  }
}

// Sync buffer.
int dbgBuf::sync()
{
  int r = baseBuf->pubsync();
  if(r!=0) return -1;

  if(synched && !ownerAccess) {
    int ret=0;
// ??? Do we ever need this line break?
    ret = printString("<br>\n");    if(ret != 0) return 0;
    /*ret = printString(getIndent()); if(ret != 0) return 0;
    justSynched=true;*/
    needIndent = true;
    synched = false;
  }
  synched = true;

  return 0;
}

// Switch between the owner class and user code writing text
void dbgBuf::userAccessing() { ownerAccess = false; synched = true; }
void dbgBuf::ownerAccessing() { ownerAccess = true; synched = true; }

// Called when a block is entered.
// b - The block that was just entered
// isAttrSubBlock - Records whether b is a sub-block that spans between creation points of attribute values and 
//   terminal points of blocks. For such blocks we record that they exist to make sure they get unique names but
//   don't add any additional indentation.
void dbgBuf::enterBlock(block* b, bool isAttrSubBlock)
{
  //cout << "enterBlock("<<b.getLabel()<<") numOpenAngles="<<numOpenAngles<<endl;
  blocks.push_back(b);
  
  if(!isAttrSubBlock)
    indents.push_back(std::list<std::string>());
}
// Called when a block is exited. Returns the block that was exited.
// isAttrSubBlock - Records whether b is a sub-block that spans between creation points of attribute values and 
//   terminal points of blocks. For such blocks we record that they exist to make sure they get unique names but
//   don't add any additional indentation.
block* dbgBuf::exitBlock(bool isAttrSubBlock)
{
  //cout << "exitBlock("<<funcName<<") numOpenAngles="<<numOpenAngles<<endl;
  /*if(funcName != blocks.back()) { 
    cout << "dbgStream::exitBlock() ERROR: exiting from block "<<b.getLabel()<<" which is not the most recent function entered!\n";
    cout << "blocks=\n";
    for(list<string>::iterator f=blocks.begin(); f!=blocks.end(); f++)
      cout << "    "<<*f<<"\n";
    cout.flush();
    baseBuf->pubsync();
    exit(-1);
  }*/
  
  assert(blocks.size()>0);
  block* lastB = blocks.back();
  blocks.pop_back();
  if(!isAttrSubBlock) {
     indents.pop_back();
    needIndent=false;
  }
  return lastB;
}

// Returns the depth of enterBlock calls that have not yet been matched by exitBlock calls
int dbgBuf::blockDepth()
{ return blocks.size(); }

/*********************
 ***** dbgStream *****
 *********************/

dbgStream::dbgStream() : common::dbgStream(&defaultFileBuf), initialized(false)
{
}

dbgStream::dbgStream(string title, string workDir, string imgDir, std::string tmpDir)
  : common::dbgStream(&defaultFileBuf)
{
  init(title, workDir, imgDir, tmpDir);
}

void dbgStream::init(string title, string workDir, string imgDir, std::string tmpDir)
{
  // Initialize fileLevel with a 0 to make it possible to count top-level files
  loc.push_back(make_pair(0, list<int>(1, 0)));
  
  this->title   = title;
  char absWorkDir[32000]; // WARNING: there is no real upper bound on the number of characters realpath might write to absWorkDir! http://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
  realpath(workDir.c_str(), absWorkDir);
  this->workDir = absWorkDir;
  this->imgDir  = imgDir;
  this->tmpDir  = tmpDir;

  numImages++;
  
  anchorsPerScriptFile = 1000;
    
  stringstream scriptIncludesFName; scriptIncludesFName << workDir << "/html/script/script_includes";
  try {
    scriptIncludesFile.open(scriptIncludesFName.str().c_str());
  } catch (ofstream::failure e)
  { cout << "dbgStream::init() ERROR opening file \""<<scriptIncludesFName.str()<<"\" for writing!"; exit(-1); }
  
  // Including jQuery, which is required by the trace widget and must be loaded before prototype.js, which is part of Canviz
  // and is used by the graph and module widgets
  dbg.includeWidgetScript("jquery-1.9.1.min.js", "text/javascript"); dbg.includeFile("jquery-1.9.1.min.js");
  
  // This should be recorded in the structure log
  enterFileLevel(new block(title), true);
  
  initialized = true;
}

dbgStream::~dbgStream()
{
  if (!initialized)
    return;

  // This should be recorded in the structure log
  block* topB = exitFileLevel(true);
  delete topB;
  
  scriptIncludesFile.close();
  
  { ostringstream cmd;
    cmd << "rm -rf " << tmpDir;
    system(cmd.str().c_str());
  }
}

// Switch between the owner class and user code writing text into this stream
void dbgStream::userAccessing() { 
  assert(fileBufs.size()>0);
  fileBufs.back()->userAccessing();
}

void dbgStream::ownerAccessing()  { 
  assert(fileBufs.size()>0);
  fileBufs.back()->ownerAccessing();
}

// Returns the file stream to the file that contains the commands to be executed when the current sub-file is loaded
std::ofstream* dbgStream::getCurScriptFile() const {
  if(scriptFiles.size()==0) return NULL;
  else                      return scriptFiles.back();
}

// Returns the file stream to the file that contains the commands to be executed before/after all the 
// commands in the script file are executed
std::ofstream* dbgStream::getCurScriptPrologFile() const {
  if(scriptPrologFiles.size()==0) return NULL;
  else                      return scriptPrologFiles.back();
}

std::ofstream* dbgStream::getCurScriptEpilogFile() const {
  if(scriptEpilogFiles.size()==0) return NULL;
  else                      return scriptEpilogFiles.back();
}

// Add an include of the given script in the generated HTML output. There are generic scripts that will be
// included in every output document that is loaded. The path of the script must be absolute
void dbgStream::includeScript(std::string scriptPath, std::string scriptType) {
  // If we have not yet included this script, do so now
  if(includedScripts.find(scriptPath) == includedScripts.end()) {
    includedScripts[scriptPath] = scriptType;
    // Write out command to include the given script file.
    // !!! Note that this is highly sensitive to where it is placed in the generated HTML. In the typical use-case
    // !!! this function will be called at the start of a block, where the inserted <script> tag will be processed normally.
    // !!! However, if we insert this text in the middle of another tag, we'll cause an error. Checking for this condition 
    // !!! part of future work.
    scriptIncludesFile << scriptPath << " " << scriptType << "\n";
    scriptIncludesFile.flush();
  }
}

// Add an include of the given script in the generated HTML output. The path of the script must be relative
// to the output HTML directory
void dbgStream::includeWidgetScript(std::string scriptPath, std::string scriptType) {
  includeScript(string("widgets/") + scriptPath, scriptType);
}

// Adds the given JavaScript command text to the script that will be loaded with the current file.
// This command is guaranteed to run after the body of the file is loaded but before the anchors
// referents are loaded.
void dbgStream::widgetScriptCommand(std::string command) {
  (*scriptFiles.back()) << command << endl;
}

// Adds the given JavaScript command text to be executed before the commands added with widgetScriptCommand()
void dbgStream::widgetScriptPrologCommand(std::string command) {
  (*scriptPrologFiles.back()) << command << endl;
}

// Adds the given JavaScript command text to be executed after the commands added with widgetScriptCommand()
void dbgStream::widgetScriptEpilogCommand(std::string command) {
  (*scriptEpilogFiles.back()) << command << endl;
}

// Includes the given file or directory into the generated HTML output by copying it from its relative
// path within the widgets code directory into the widgets subdirectory of the generated output directory
void dbgStream::includeFile(std::string path) {
  // If we have not yet included this file, do so now
  if(includedFiles.find(path) == includedFiles.end()) {
    ostringstream cmd; cmd << "cp -r "<<ROOT_PATH<<"/widgets/"<<path<<" "<<workDir<<"/html/widgets/"<<path;
    system(cmd.str().c_str());
    includedFiles.insert(path);
  }
}

// Returns the unique ID string of the current file within the hierarchy of files
string dbgStream::fileLevelStr(const location& myLoc) const
{
  if(myLoc.size()<=1) return "";

  ostringstream oss;
  list<pair<int, list<int> > >::const_iterator i=myLoc.begin(); 
  int lastFileID = i->first;
  i++;
  while(i!=myLoc.end()) { 
    oss << i->first;
    lastFileID = i->first;
    i++;
    if(i!=myLoc.end()) oss << "-";
  }
  return oss.str();
}

// Returns a string that encodes a Javascript array of integers that together form the unique ID string of 
// the current file within the hierarchy of files
string dbgStream::fileLevelJSIntArray(const location& myLoc) const
{
  ostringstream oss;
  oss << "[";
  list<pair<int, list<int> > >::const_iterator l=myLoc.begin(); 
  int lastFileID = l->first;
  l++;
  while(l!=myLoc.end()) { 
    oss << lastFileID;
    lastFileID = l->first;
    l++;
    if(l!=myLoc.end()) oss << ", ";
  }
  oss << "]";
  return oss.str();
}

const location& dbgStream::getLocation() const
{ return loc; }

// Returns the unique ID string of the current region, including all the files that it may be contained in
string dbgStream::blockGlobalStr(const location& myLoc) {
  ostringstream blockStr;
  
  for(list<pair<int, list<int> > >::const_iterator l=myLoc.begin(); l!=myLoc.end(); ) {
    assert(l->second.size()>0);
    list<int>::const_iterator b=l->second.begin();
    blockStr << l->first << "-"; // Note: Yahoo UI widgets don't seem to work if we use non-trivial characters as separators in block names
    int lastBlock=*b;
    b++;
    while(b!=l->second.end()) {
      blockStr << lastBlock;
      lastBlock=*b;
      b++;
      if(b!=l->second.end()) blockStr << "_";
    }
    //blockStr << ">";
    l++;
    //if(l!=myLoc.end() && l->second.size()>1) blockStr << ":";
  }
  
  return blockStr.str();
}

// Given two location objects, returns the longest prefix location that is common to both of them
location dbgStream::commonSubLocation(const location& a, const location& b) {
  list<pair<int, list<int> > >::const_iterator itA = a.begin(),
                                               itB = b.begin();
  list<pair<int, list<int> > > common;
  
  for(; itA != a.end() && itB != b.end(); itA++, itB++) {
    // If we've reached the point where the two locations no longer align, return the 
    // common prefix identified so far
    if(itA->first != itB->first) return common;
    int commonFileID = itA->first;
    
    // If a and b are still aligned at the file level, look at the blocks inside the file
    list<int>::const_iterator it2A = itA->second.begin(),
                              it2B = itB->second.begin();
    for(; it2A != itA->second.end() && it2B != itB->second.end(); it2A++, it2B++) {
      // If we've reached the point where the two locations no longer align, return the 
      // common prefix identified so far
      if(*it2A != *it2A) return common;
      int commonBlockID = *it2A;
      
      // If a and b are still aligned at the block level within the current file, add the current 
      // block to common
      
      // If this is the first block within the current file, first add a new entry for the file 
      // (until now we didn't know if there'd be any common blocks to add within this file)
      if(it2A == itA->second.begin())
        common.push_back(make_pair(commonFileID, list<int>()));
      
      // Now add the common block ID
      assert(common.size()>0);
      common.back().second.push_back(commonBlockID);
    }
    
    // If we've reached the end of the blocks in this file for one location but not the other,
    // we've found the point of disagreement, so return the common prefix identified so far
    if(it2A != itA->second.end() || it2B != itB->second.end())
      return common;
  }
  
  // We've reached the end of the file list of either location or both locations. There are no more
  // files or blocks that are common, so return what we've found so far
  return common;
}

// Called to inform the blocks that contain the given block that it has been entered
void dbgStream::subBlockEnterNotify(block* subBlock)
{
  //cout << "Entering "<<subBlock->getLabel()<<endl;
  // Walk backwards through the current location stack, informing each block about the new arrival until the block's
  // subBlockEnterNotify() function returns false to indicate that the notification should not be propagated further.
  for(list<pair<block*, list<block*> > >::const_reverse_iterator fb=blocks.rbegin(); fb!=blocks.rend(); fb++) {
    // Iterate through the sub-blocks of this file
    for(list<block*>::const_reverse_iterator sb=fb->second.rbegin(); sb!=fb->second.rend(); sb++) {
      //cout << "  Informing"<<(*sb)->getLabel()<<endl;
      if(!(*sb)->subBlockEnterNotify(subBlock)) return;
    }
    
    // Now call subBlockEnterNotify on the block that contains the current file within its parent file
    if(!fb->first->subBlockEnterNotify(subBlock)) return;
  }  
}

// Called to inform the blocks that contain the given block that it has been exited
void dbgStream::subBlockExitNotify(block* subBlock)
{
  // Walk backwards through the current location stack, informing each block about the new arrival until the block's
  // subBlockExitNotify() function returns false to indicate that the notification should not be propagated further.
  for(list<pair<block*, list<block*> > >::const_reverse_iterator fb=blocks.rbegin(); fb!=blocks.rend(); fb++) {
    // Iterate through the sub-blocks of this file
    for(list<block*>::const_reverse_iterator sb=fb->second.rbegin(); sb!=fb->second.rend(); sb++)
      if(!(*sb)->subBlockExitNotify(subBlock)) return;
    
    // Now call subBlockExitNotify on the block that contains the current file within its parent file
    if(!fb->first->subBlockExitNotify(subBlock)) return;
  }
}

// Returns the depth of enterBlock calls that have not yet been matched by exitBlock calls within the current file
int dbgStream::blockDepth() const {
  assert(loc.size()>0);
  return loc.back().second.size();
}

// Index of the current block in the current file
int dbgStream::blockIndex() const {
  if(loc.size()==0) return 0;
  else {
    assert(loc.back().second.size()>0);
    return loc.back().second.back();
  }
}

// Index of the current block's parent block in the current file
int dbgStream::parentBlockIndex() const {
  if(loc.size()<=1) return 0;
  else {
    assert(loc.back().second.size()>0);
    list<int>::const_reverse_iterator it = loc.back().second.rbegin();
    it++;
    return *it;
  }
}

// Enter a new file level. Return a string that contains the JavaScript command to open this file in the current view.
string dbgStream::enterFileLevel(block* b, bool topLevel)
{
  assert(loc.size()>0);
  // Increment the index of thi  s file unit within the current nesting level
  (loc.back().first)++;
  // Add a fresh file level to the location
  loc.push_back(make_pair(0, list<int>(1, 0)));
  
  //if(!topLevel) (*this)<< "dbgStream::enterFileLevel("<<b->getLabel()<<") <<<<<\n";
  
  // Each file unit consists of three files: 
  //    - the detail file that contains all the text output and regions inside the file,
  //    - the summary file that lists these regions
  //    - the script file that contains all the code that needs to be run as the regions in the detail file 
  //      are loaded (substitute for having an explicit onload callback for each region's div)
  //    - the index file that contains both of these
  // The detail and summary files can be viewed on their own via the index file or can be loaded into higher-level 
  // detail and summary files, where they appear like regular regions.
  // These are the absolute and relative names of these files.
  string fileID = fileLevelStr(loc);
  string blockID = blockGlobalStr(loc);
  //if(!topLevel) (*this)<< "fileID="<<fileID<<" blockID="<<blockID<<endl;
  ostringstream indexAbsFName;  indexAbsFName  << workDir << "/html/index." << fileID << ".html";
  ostringstream detailAbsFName; detailAbsFName << workDir << "/html/detail."  << fileID;
  ostringstream detailRelFName; detailRelFName << "detail."  << fileID;
  ostringstream sumAbsFName;    sumAbsFName    << workDir << "/html/summary." << fileID;
  ostringstream sumRelFName;    sumRelFName    << "summary."  << fileID;
  ostringstream scriptAbsFName; scriptAbsFName << workDir << "/html/script/script." << fileID;
  ostringstream scriptRelFName; scriptRelFName << "script/script."  << fileID;
  
  //cout << "enterFileLevel("<<b->getLabel()<<") topLevel="<<topLevel<<" #fileBlocks="<<fileBlocks.size()<<" #location="<<loc.size()<<endl;
  // Add a new function level within the parent file unit that will refer to the child file unit
  string loadCmd="";
  blocks.push_back(make_pair(b, list<block*>()));
  if(!topLevel) {
//!!!    dbg << "enterFileLevel() !topLevel, fileBufs.size()="<<fileBufs.size()<<" && blocks.size()="<<blocks.size()<<" blocks.back()->second.size()="<<blocks.back().second.size()<<endl;
    loadCmd = enterBlock(b, true, true);
  }
  /*else
    enterAttrSubBlock();*/
  fileBlocks.push_back(b);
  
  //if(!topLevel) (*this)<< "dbgStream::enterFileLevel("<<b->getLabel()<<") >>>>>\n";
  
  // Create the index file, which is a frameset that refers to the detail and summary files
  ofstream &indexFile = createFile(indexAbsFName.str());
  indexFiles.push_back(&indexFile);
  
  indexFile << "<frameset cols=\"20%,80%\">\n";
  indexFile << "\t<frame src=\""<<sumRelFName.str()<<".html\" name=\"summary\" id=\"summary\"/>\n";
  indexFile << "\t<frame src=\""<<detailRelFName.str() <<".html\" name=\"detail\" id=\"detail\"/>\n";
  indexFile << "</frameset>\n";
  indexFile.close();
  
  // Create the detail file. It is empty initially and will be filled with text by the user because its dbgBuf
  // object will be set to be the primary buffer of this stream, meaning that all the text written to this
  // stream will flow into the detail file.
  ofstream &dbgFile = createFile(detailAbsFName.str()+".body");
  dbgFiles.push_back(&dbgFile);
  detailFileRelFNames.push_back(detailRelFName.str()+".body");
  
  dbgBuf *nextBuf = new dbgBuf(dbgFile.rdbuf());
  fileBufs.push_back(nextBuf);
  // Call the parent class initialization function to connect it dbgBuf of the child file
  ostream::init(nextBuf);
  
  // Create the html file container for the detail html text
  printDetailFileContainerHTML(detailAbsFName.str(), /*b->getLabel()*/"");
  
  // Create the summary file. It is initially set to be an empty table and is filled with entries each time
  // a region is opened inside the detail file.
  {
    ofstream &summaryFile = createFile(sumAbsFName.str()+".body");
    summaryFiles.push_back(&summaryFile);
    
    // Start the table in the current summary file
    summaryFile << "\t\t<table width=\"100%\">\n";
    summaryFile << "\t\t\t<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
    
    // Create the html file container for the summary html text
    printSummaryFileContainerHTML(sumAbsFName.str(), sumRelFName.str(), b->getLabel());
  }
  
  // Create the main script file. It is initially set to be an empty <html> tag and filled with entries each time
  // a region is opened inside the detail file.
  {
    ofstream &scriptFile = createFile(scriptAbsFName.str());
    scriptFiles.push_back(&scriptFile);
    
    ofstream &scriptPrologFile = createFile(scriptAbsFName.str()+".prolog");
    scriptPrologFiles.push_back(&scriptPrologFile);
    
    ofstream &scriptEpilogFile = createFile(scriptAbsFName.str()+".epilog");
    scriptEpilogFiles.push_back(&scriptEpilogFile);
    
    // Next, record that the file was loaded
    scriptPrologFile << "\trecordFile("<<fileLevelJSIntArray(loc)<<", 'loaded', 1);\n";
  }
  
  return loadCmd;
}

// Exit a current file level
block* dbgStream::exitFileLevel(bool topLevel)
{
  //cout << "exitFileLevel("<<b->getLabel()<<") topLevel="<<topLevel<<" #fileBlocks="<<fileBlocks.size()<<" #location="<<loc.size()<<endl;
  assert(loc.size()>1);
  
  dbgFiles.back()->close();
  
  // Complete the table in the current summary file
  (*summaryFiles.back()) << "\t\t\t</td></tr>\n";
  (*summaryFiles.back()) << "\t\t</table>\n";
  summaryFiles.back()->close();
  
  // Complete the current script file
  scriptFiles.back()->close();
  scriptPrologFiles.back()->close();
  scriptEpilogFiles.back()->close();

  indexFiles.pop_back();
  dbgFiles.pop_back();
  detailFileRelFNames.pop_back();
  summaryFiles.pop_back();
  scriptFiles.pop_back();
  scriptPrologFiles.pop_back();
  scriptEpilogFiles.pop_back();
  fileBufs.pop_back();
  // Call the ostream class initialization function to connect it dbgBuf of the parent detail file
  ostream::init(fileBufs.back());
  
  // Exit the function level within the parent file
  //assert(b->getLabel() == fileBlocks.back());
  if(!topLevel) {
    //block* topBlock = exitBlock();
    exitBlock();
    //delete topBlock;
  }/* else
    exitAttrSubBlock();*/
  
  loc.pop_back();
  
  block* lastB = fileBlocks.back();
  fileBlocks.pop_back();
  blocks.pop_back();
  return lastB;
}

// Record the mapping from the given anchor ID to the given string in the global script file
void dbgStream::writeToAnchorScript(int anchorID, const location& myLoc) {
  int anchorFileIdx=anchorID/anchorsPerScriptFile;
  // Open the file that holds the anchorID's within this ID's block
  ostringstream anchorScriptFName; anchorScriptFName << workDir << "/html/script/anchor_script."<<anchorFileIdx;
  try {
    // If we haven't already created this file, open it for output, otherwise for appending
    bool alreadyCreated = createdAnchorFiles.find(anchorFileIdx) != createdAnchorFiles.end();
    ofstream anchorScriptFile(anchorScriptFName.str().c_str(), 
                              alreadyCreated? std::fstream::app: 
                                              std::fstream::out);
    if(!alreadyCreated) createdAnchorFiles.insert(anchorFileIdx);
    
    // Write the anchor's record into this file
    anchorScriptFile << "anchors.setItem("<<anchorID<<", new anchor("<<fileLevelJSIntArray(myLoc)<<", \""<<blockGlobalStr(myLoc)<<"\"));"<<endl;

    anchorScriptFile.close();

  } catch (ofstream::failure e)
  { cout << "dbgStream::init() ERROR opening file \""<<anchorScriptFName.str()<<"\" for writing!"; exit(-1); }
}

void dbgStream::printSummaryFileContainerHTML(string absoluteFileName, string relativeFileName, string title)
{
  ofstream sum;
  ostringstream fullFName; fullFName << absoluteFileName << ".html";
  try { sum.open(fullFName.str().c_str()); }
  catch (ofstream::failure e)
  { cout << "dbgStream::init() ERROR opening file \""<<fullFName.str()<<"\" for writing!"; exit(-1); }
  
  sum << "<html>\n";
  sum << "\t<head>\n";
  sum << "\t<title>"<<title<<"</title>\n";
  sum << "\t<script src=\"script/hashtable.js\"></script>\n";
  sum << "\t<script src=\"script/placement.js\"></script>\n";
  sum << "\t<script src=\"script/attributes.js\"></script>\n";
  sum << "\t<script src=\"script/core.js\"></script>\n";
  sum << "\t<script type=\"text/javascript\">\n";
  sum << "\tfunction loadURLIntoDiv(doc, url, divName) {\n";
  sum << "\t\tvar xhr= new XMLHttpRequest();\n";
  sum << "\t\txhr.open('GET', url, true);\n";
  sum << "\t\txhr.onreadystatechange= function() {\n";
  sum << "\t\t\t//Wait until the data is fully loaded\n";
  sum << "\t\t\tif (this.readyState!==4) return;\n";
  sum << "\t\t\tdoc.getElementById(divName).innerHTML= this.responseText;\n";
  sum << "\t\t};\n";
  sum << "\t\txhr.send();\n";
  sum << "\t}\n";
  sum << "\n";
  sum << "// Set this page's initial contents\n";
  sum << "window.onload=function () { loadURLIntoDiv(document, '"<<relativeFileName<<".body', 'detailContents'); }\n";
  sum << "\t</script>\n";
  sum << "\t</head>\n";
  sum << "\t<body>\n";
  sum << "\t<h1>Summary</h1>\n";
  sum << "\t<div id='detailContents'></div>\n";
  sum << "\t</body>\n";
  sum << "</html>\n\n";
}

void dbgStream::printDetailFileContainerHTML(string absoluteFileName, string title)
{
  ofstream det;
  ostringstream fullFName; fullFName << absoluteFileName << ".html";
  try { det.open(fullFName.str().c_str()); }
  catch (ofstream::failure e)
  { cout << "dbgStream::init() ERROR opening file \""<<fullFName.str()<<"\" for writing!"; exit(-1); }
  
  det << "<html>\n";
  det << "\t<head>\n";
  det << "\t<title>"<<title<<"</title>\n";
  //det << "\t<script type='text/javascript' src='https://www.google.com/jsapi?autoload={\"modules\":[{\"name\":\"visualization\",\"version\":\"1\",\"packages\":[\"orgchart\"]}]}'></script>\n";
  det << "\t<script src=\"script/hashtable.js\"></script>\n";
  det << "\t<script src=\"script/taffydb/taffy.js\"></script>\n";
  det << "\t<script src=\"script/placement.js\"></script>\n";
  det << "\t<script src=\"script/attributes.js\"></script>\n";
  det << "\t<script src=\"script/core.js\"></script>\n";
  det << "\t<STYLE TYPE=\"text/css\">\n";
  det << "\tBODY\n";
  det << "\t\t{\n";
  det << "\t\tfont-family:courier;\n";
  det << "\t\t}\n";
  det << "\t.hidden { display: none; }\n";
  det << "\t.unhidden { display: block; }\n";
  det << "\t</style>\n";
  det << "\t<script type=\"text/javascript\">\n";
  det << "\t\twindow.onload=function () { \n";
  string fileID = fileLevelStr(loc);
  det << "\t\t\tloadScriptsInFile(document, 'script/script_includes', \n";
  det << "\t\t\t\tfunction() { loadURLIntoDiv(document, 'detail."<<fileID<<".body', 'detailContents', \n";
  det << "\t\t\t\t\tfunction() { loadjscssfile('script/script."<<fileID<<".prolog', 'text/javascript',\n";
  det << "\t\t\t\t\t\tfunction() { loadjscssfile('script/script."<<fileID<<"', 'text/javascript',\n";//, \n";
  det << "\t\t\t\t\t\tfunction() { loadjscssfile('script/script."<<fileID<<".epilog', 'text/javascript'\n";//, \n";
  //det << "\t\t\t\t\t\tfunction() { loadjscssfile('script/anchor_script', 'text/javascript'); } \n";
  det << "\t\t\t\t\t\t\t); }\n";
  det << "\t\t\t\t\t\t); }\n";
  det << "\t\t\t\t\t); }\n";
  det << "\t\t\t\t); }\n";
  det << "\t\t\t);\n";
  det << "\t\t}\n";
  det << "\t</script>\n";
  det << "\t</head>\n";
  det << "\t<body>\n";
  /*det << "<div id=\"attrTable\"></div>\n";
  det << "<a href=\"javascript:writeKeyValTable(document.getElementById('attrTable'));\">Attributes</a><br>\n";
  det << "<div id=\"AttrControlPanel\">\n";
  det << "<a href=\"\">Add</a><br>\n";
  det << "<a href=\"\">Remove</a><br>\n";
  det << "<a href=\"\">Only</a><br>\n";
  det << "</div>\n";
  det << "<script type=\"text/javascript\">AddDivPlacementEvents(function () { PlaceFixedDiv(\"AttrControlPanel\"); })</script>\n";*/
  det << "<div id=\"attrTable\" style=\"background-color:#ffffff\"></div>\n";
  det << "<div id=\"AttrControlPanel\" style=\"background-color:#ffffff;width:50\">\n";
  det << "<a href=\"javascript:writeKeyValTable('attrTable', 'add');\"><img src=\"img/attrAdd.gif\" width=50 height=46 alt=\"Add by Attribute\"></a><br>\n";
  det << "<a href=\"javascript:writeKeyValTable('attrTable', 'remove');\"><img src=\"img/attrRemove.gif\" width=50 height=46 alt=\"Remove by Attribute\"></a><br>\n";
  det << "<a href=\"javascript:writeKeyValTable('attrTable', 'only');\"><img src=\"img/attrOnly.gif\" width=50 height=46 alt=\"Filter Only Attribute\"></a><br>\n";
  det << "</div>\n";
  det << "<script type=\"text/javascript\">AddDivPlacementEvents(function () { PlaceFixedDiv(\"AttrControlPanel\", false); });</script>\n";
  
  if(title != "")
    det << "\t<h1>"<<title<<"</h1>\n";

  det << "\t\t<table width=\"100%\">\n";
  det << "\t\t\t<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  det << "\t\t\t<div id='detailContents'></div>\n";
  det << "\t\t\t</td></tr>\n";
  det << "\t\t</table>\n";
  #if REMOTE_ENABLED
  for(list<string>::iterator h=hostnames.begin(); h!=hostnames.end(); h++) {
    det << "<img src=\"http://"<<*h<<":"<<GDB_PORT<<"/img/divDL.gif\" onload=\"javascript:hostnameReachable('"<<*h<<"')\" width=1 height=1>\n";
  }
  #endif
  det << "\t</body>\n";
  det << "</html>\n\n";
  
  det.close();
}

// Called when a block is entered.
// b: The block that is being entered
// newFileEntered: records whether by entering this block we're also entering a new file
// addSummaryEntry: records whether we need to add an entry to the summary frame for this block
// recursiveEnterBlock: records whether we're calling enterBlock() recursively to place a block between
//    the start of this major block and the next setting of an attribute.
string dbgStream::enterBlock(block* b, bool newFileEntered, bool addSummaryEntry, bool recursiveEnterBlock)
{
  //cout << "<<<enter(newFileEntered="<<newFileEntered<<", addSummaryEntry="<<addSummaryEntry<<", recursiveEnterBlock="<<recursiveEnterBlock<<") b="<<b<<"="<<(b? b->getLabel(): "NULL")<<endl;
//!!!  dbg << "<<<enter() fileBufs.size()="<<fileBufs.size()<<" && #blocks="<<blocks.size()<<", #blocks.back().second="<<blocks.back().second.size()<<", #fileBufs.back()->blocks="<<(fileBufs.size()==0? -1: fileBufs.back()->blocks.size())<<endl;
/*  if(!recursiveEnterBlock)
    exitAttrSubBlock();*/
  
  //cout << "<<<enterBlock: b="<<b<<", newFileEntered="<<newFileEntered<<", addSummaryEntry="<<addSummaryEntry<<", recursiveEnterBlock="<<recursiveEnterBlock<<endl;
  // if recursiveEnterBlock, newFileEntered and addSummaryEntry may not be
  assert(!addSummaryEntry || !(recursiveEnterBlock && newFileEntered));
  //(*this) << "dbgStream::enterBlock("<<(b? b->getLabel(): "NULL")<<")"<<endl;
  
  fileBufs.back()->ownerAccessing();
  
  fileBufs.back()->enterBlock(b, recursiveEnterBlock);
  
  assert(loc.size()>0);
  // Increment the index of this block unit within the current nesting level of this file
  loc.back().second.back()++;
  // Add a new level to the block list, starting the index at 0
  loc.back().second.push_back(0);
  
  // Initialize this block's location (must be done before the call to 
  // subBlockEnterNotify() to make sure b's containers know there it is located)
  string blockID = blockGlobalStr(loc);
  b->setLocation(loc);
  
  // Inform this block's container blocks that we have entered it
  if(!recursiveEnterBlock) subBlockEnterNotify(b);
  
  // Add this block to the record of the block nesting structure
  // (after the call to subBlockEnterNotify to keep the block from being informed about itself)
  assert(blocks.size()>0);
  blocks.back().second.push_back(b);
  
  ostringstream loadCmd; // The command to open this file in the current view
  
  if(newFileEntered) {
    string fileID = fileLevelStr(b->getLocation());
    loadCmd << "loadSubFile(top.detail.document, "<<fileLevelJSIntArray(loc)<<", 'detail."<<fileID<<".body', 'div"<<blockID<<"', "<<
               "top.summary.document, 'summary."<<fileID<<".body', 'sumdiv"<<blockID<<"', "<<
               "'script/script."<<fileID<<"'";
  }
  
  if(!recursiveEnterBlock) b->printEntry(loadCmd.str());
  dbg.ownerAccessing();  
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<div id=\"div"<<b->getBlockID()<<"\" class=\"unhidden\">\n"; dbg.flush();
  dbg.userAccessing();
  
  if(!recursiveEnterBlock) {
//    enterAttrSubBlock();
  
    if(addSummaryEntry) {
      *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth())<<"</td></tr>\n";
      *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
      *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<table width=\"100%\" style=\"border:0px\">\n";
      *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<tr width=\"100%\"><td width=50></td><td id=\"link"<<blockID<<"\" width=\"100%\">";
      *summaryFiles.back() <<     "<a name=\"anchor"<<blockID<<"\" href=\"javascript:focusLinkDetail('"<<blockID<<"')\">"<<b->getLabel()<<"</a></td></tr>\n";
      *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
      *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<div id=\"sumdiv"<<blockID<<"\">\n";
      summaryFiles.back()->flush();
    }
    
    if(newFileEntered) {
      (*scriptFiles.back()) << "\trecordFile("<<fileLevelJSIntArray(loc)<<", 'loadFunc', function(continuationFunc) {"<<loadCmd.str()<<", continuationFunc)});\n";
      scriptFiles.back()->flush();
    }
  }
  
  (*scriptFiles.back()) << "\trecordAttr("<<attributes.strJS()<<", '"<<blockID<<"');\n";
  scriptFiles.back()->flush();
  
  fileBufs.back()->userAccessing();

  //cout << ":enterBlock>>>\n";
//!!!  dbg << ">>>enter("<<recursiveEnterBlock<<") #blocks="<<blocks.size()<<", #blocks.back().second="<<blocks.back().second.size()<<", #fileBufs.back()->blocks="<<(fileBufs.size()==0? -1: fileBufs.back()->blocks.size())<<endl;
  
  return loadCmd.str();
}

// Called to enter a mini-block between the start/end of a block and the definition of an attribute and between
// adjacent attribute definitions
string dbgStream::enterAttrSubBlock() {
  //cout << "enterAttrSubBlock()"<<endl;
  // Only enter an attribute sub-block if we've already begun a block
  return enterBlock(new block(""), false, false, true);
}

// Called when a block is exited. Returns the block that was exited.
// recursiveEnterBlock: records whether we're calling exitBlock() recursively to place a block between
//    the most recent setting of an attribute and the end of this block
block* dbgStream::exitBlock(bool recursiveExitBlock)
{
  //cout << "<<<exitBlock(recursiveExitBlock="<<recursiveExitBlock<<") fileBufs.size()="<<fileBufs.size()<<" && #blocks="<<blocks.size()<<", #blocks.back().second="<<blocks.back().second.size()<<", #fileBufs.back()->blocks="<<(fileBufs.size()==0? -1: fileBufs.back()->blocks.size())<<endl;
/*  if(!recursiveExitBlock)
    exitAttrSubBlock();*/
  
  fileBufs.back()->ownerAccessing();
  assert(fileBufs.size()>0);
  block* lastB = fileBufs.back()->exitBlock(recursiveExitBlock);
  
  assert(loc.size()>0);
  assert(loc.back().second.size()>0);
  loc.back().second.pop_back();
  
  // Remove this block from the record of the block nesting structure
  assert(blocks.size()>0);
  assert(blocks.back().second.size()>0);
  block* topB = blocks.back().second.back();
  blocks.back().second.pop_back();
  fileBufs.back()->userAccessing();
  
  //cout << "<<<exitBlock: lastB="<<lastB<<", topB="<<topB<<" recursiveExitBlock="<<recursiveExitBlock<<"\n";
  
  // Inform this block's container blocks that we have exited it
  // (after the removal of this block from blocks to keep the block from being informed about itself)
  subBlockExitNotify(topB);
  
  dbg.ownerAccessing();
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</div>\n"; dbg.flush();
  dbg.userAccessing();
  
  lastB->printExit();
  // We enfore the invariant that the number of open and close angle brackets must be balanced
  // between the entry into and exit from a block
  while(fileBufs.back()->getNumOpenAngles() > 0)
    (*this) << ">";
  //this->flush();
  
  if(!recursiveExitBlock) {
    *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"</div></td></tr>\n";
    *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"</table>\n";
    *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth())<<"</td></tr>\n";
    *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
    summaryFiles.back()->flush();
  }
  
  
  if(recursiveExitBlock) {
    delete lastB;
    lastB = NULL;
  }
  
/*  if(!recursiveExitBlock)
    enterAttrSubBlock();*/
  
  //cout << ":exitBlock>>>\n";
//!!!  dbg << ">>>exitBlock("<<recursiveExitBlock<<")\n";
  
  return lastB;
}

// Called to exit a mini-block between the start/end of a block and the definition of an attribute and between
// adjacent attribute definitions
block* dbgStream::exitAttrSubBlock() {
//!!!  dbg << "exitAttrSubBlock() fileBufs.size()="<<fileBufs.size()<<" && #blocks="<<blocks.size()<<", #blocks.back().second="<<blocks.back().second.size()<<", #fileBufs.back()->blocks="<<(fileBufs.size()==0? -1: fileBufs.back()->blocks.size())<<endl;
  // Only exit an attribute sub-block if we've already begun a block
  if(fileBufs.size()>0 && blocks.size()>0 && blocks.back().second.size()>0 && fileBufs.back()->blocks.size()>0)
    return exitBlock(true);
  else
    return NULL;
}

// Adds an image to the output with the given extension and returns the path of this image
// so that the caller can write to it.
string dbgStream::addImage(string ext)
{
  assert(initializedDebug);
  
  assert(fileBufs.size()>0);
  ostringstream imgFName; imgFName << imgDir << "/image_" << numImages << "." << ext;
  fileBufs.back()->ownerAccessing();
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<img src=\"dbg_imgs/image_" << numImages << "." << ext <<"\">\n";
  fileBufs.back()->userAccessing();
  return imgFName.str();
}

// Add a given amount of indent space to all subsequent text within the current function
void dbgStream::addIndent(string indent)
{
  assert(fileBufs.size()>0);
  fileBufs.back()->addIndent(indent);
}

// Remove the most recently added indent
void dbgStream::remIndent()
{
  assert(fileBufs.size()>0);
  fileBufs.back()->remIndent();
}

/******************
 ***** indent *****
 ******************/

indent::indent(properties::iterator props)
{
  int repeatCnt = properties::getInt(props, "repeatCnt");
  string prefix = properties::get(props, "prefix");
  
  string fullPrefix=prefix;
  // Concatenate repeatCnt-1 copies of prefix onto its end
  for(int i=1; i<repeatCnt; i++)
    fullPrefix += prefix;
  //cout << "indent::init("<<prefix<<", "<<repeatCnt<<") fullPrefix=\""<<fullPrefix<<"\""<<endl;
  dbg.addIndent(fullPrefix);
}

indent::~indent() {
  //cout << "Exiting indent"<<std::endl;
  dbg.remIndent();
}

// Given a string, returns a version of the string with all the control characters that may appear in the 
// string escaped to that the string can be written out to Dbg::dbg with no formatting issues.
// This function can be called on text that has already been escaped with no harm.
std::string escape(std::string s)
{
  string out;
  for(unsigned int i=0; i<s.length(); i++) {
    // Manage HTML tags
         if(s[i] == '<') out += "&lt;";
    else if(s[i] == '>') out += "&gt;";
    // Manage hashes, since they confuse the C PreProcessor CPP
    else if(s[i] == '#') out += "&#35;";
    else if(s[i] == ' ') out += "&nbsp;";
    else                 out += s[i];
  }
  return out;
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

}; // namespace layout
}; // namespace sight
