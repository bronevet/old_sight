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
#include "binreloc.h"
#include <errno.h>
#include "getAllHostnames.h" 
#include "utils.h"
#include "fdstream.h"
using namespace std;
using namespace sight::common;

namespace sight {
namespace structure{

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

void SightInit(int argc, char** argv, string title, string workDir)
{
  if(initializedDebug) return;
  SightInit_internal(argc, argv, title, workDir);
}

// Initializes the debug sub-system
void SightInit(string title, string workDir)
{
  if(initializedDebug) return; 
  SightInit_internal(0, NULL, title, workDir);
}

void SightInit_internal(int argc, char** argv, string title, string workDir)
{
  map<string, string> newProps;

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
      exit(-1);
    }
    char* execFile = br_find_exe(NULL);
    if(execFile==NULL) { cerr << "ERROR reading application's executable name after successful initialization!"<<endl; exit(-1); }
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
    if(fp == NULL) { cerr << "Failed to run command \""<<cmd.str()<<"\"!"<<endl; exit(-1); }
    
    if(fgets(username, sizeof(username), fp) == NULL) { cerr << "Failed to read output of \""<<cmd.str()<<"\"!"<<endl; exit(-1); }
  }
  newProps["username"] = string(username);
  
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

void SightInit_internal(properties* props)
{
  properties::iterator sightIt = props->find("sight");
  assert(sightIt != props->end());
  
  // Create the directory structure for the structural information
  // Main output directory
  createDir(properties::get(sightIt, "workDir"), "");

  // Directory where client-generated images will go
  string imgDir = createDir(properties::get(sightIt, "workDir"), "html/dbg_imgs");
  
  // Directory that widgets can use as temporary scratch space
  string tmpDir = createDir(properties::get(sightIt, "workDir"), "html/tmp");
  
  initializedDebug = true;
  
  dbg.init(props, properties::get(sightIt, "title"), properties::get(sightIt, "workDir"), imgDir, tmpDir);
}

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
  if(located && loc != dbg.getLocation())
    cerr << "Warning: anchor "<<anchorID<<" is being set to multiple target locations! current location="<<loc.str()<<", new location="<<dbg.getLocation().str()<< endl;
  else {
    located = true;
    loc = dbg.getLocation();
    anchorLocs[anchorID] = loc;

    update();
  }
}

// Updates this anchor to use the canonical ID of its location, if one has been established
void anchor::update() {
  if(anchorLocs.find(anchorID) != anchorLocs.end()) {
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
  sightObj *obj = new sightObj(new properties());
  
  map<string, string> newProps;
  newProps["anchorID"] = txt()<<anchorID;
  newProps["text"] = text;
  newProps["img"] = "0";
  obj->props->add("link", newProps);
  
  dbg.tag(obj);
  
  delete(obj);
}

// Emits to the output an html tag that denotes a link to this anchor, using the default link image, which is followed by the given text.
void anchor::linkImg(string text) const {
  sightObj *obj = new sightObj(new properties());
  
  map<string, string> newProps;
  newProps["anchorID"] = txt()<<anchorID;
  newProps["text"] = text;
  newProps["img"] = "1";
  obj->props->add("link", newProps);
  
  dbg.tag(obj);
  
  delete(obj);
}

std::string anchor::str(std::string indent) const {
  if(anchorID==-1) return "[noAnchor_structure]";
  else             return txt()<<"[anchor_structure: ID="<<anchorID<<", "<<(located?"":"not")<<" located"<<(located? ", ": "")<<(located? loc.str(): "")<<"]";
}

/*****************
 ***** block *****
 *****************/

// The unique ID of this block as well as the static global counter of the maximum ID assigned to any block.
// Unlike the rendering module, these blockIDs are integers since all we need from them is uniqueness and not
// any structural information.
int block::maxBlockID;

// Initializes this block with the given label
block::block(string label, properties* props) : label(label) {
  advanceBlockID();
  if(!initializedDebug) SightInit("Debug Output", "dbg");
    
  if(props==NULL) this->props = new properties();
  else            this->props = props;
  
  if(this->props->active) {
    // Connect startA to the current location 
    startA.reachedLocation();

    map<string, string> newProps;
    newProps["label"] = label;
    newProps["ID"] = txt()<<blockID;
    newProps["anchorID"] = txt()<<startA.getID();
    newProps["numAnchors"] = "0";
    this->props->add("block", newProps);
    
    dbg.enter(this);

    dbg.enterBlock(this);
  }
}

// Initializes this block with the given label.
// Includes one or more incoming anchors thas should now be connected to this block.
block::block(string label, anchor& pointsTo, properties* props) : label(label) {
  advanceBlockID();
  if(!initializedDebug) SightInit("Debug Output", "dbg");
  
  if(props==NULL) this->props = new properties();
  else            this->props = props;
  
  if(this->props->active) {
    // Connect startA and pointsTo anchors to the current location (pointsTo is not modified);
    startA.reachedLocation();
    anchor pointsToCopy(pointsTo);
    pointsToCopy.reachedLocation();

    map<string, string> newProps;
    newProps["label"] = label;
    newProps["ID"] = txt()<<blockID;
    newProps["anchorID"] = txt()<<startA.getID();
    if(pointsTo != anchor::noAnchor) {
      newProps["numAnchors"] = "1";
      newProps["anchor_0"] = txt()<<pointsTo.getID();
    } else
      newProps["numAnchors"] = "0";
    this->props->add("block", newProps);
    
    dbg.enter(this);

    dbg.enterBlock(this);
  }
}

// Initializes this block with the given label.
// Includes one or more incoming anchors thas should now be connected to this block.
block::block(string label, set<anchor>& pointsTo, properties* props) : label(label)
{
  advanceBlockID();
  if(!initializedDebug) SightInit("Debug Output", "dbg");

  if(props==NULL) this->props = new properties();
  else            this->props = props;
  
  if(this->props->active) {
    // Connect startA and pointsTo anchors to the current location (pointsTo is not modified)
    startA.reachedLocation();
    for(set<anchor>::iterator a=pointsTo.begin(); a!=pointsTo.end(); a++) {
      anchor aCopy(*a);
      aCopy.reachedLocation();
    }

    map<string, string> newProps;
    newProps["label"] = label;
    newProps["ID"] = txt()<<blockID;
    newProps["anchorID"] = txt()<<startA.getID();
    
    int i=0;
    for(set<anchor>::const_iterator a=pointsTo.begin(); a!=pointsTo.end(); a++) {
      if(*a != anchor::noAnchor) {
        newProps[txt()<<"anchor_"<<i] = txt()<<a->getID();
        i++;
      }
    }
    newProps["numAnchors"] = txt()<<i;

    this->props->add("block", newProps);
    
    dbg.enter(this);

    dbg.enterBlock(this);
  }
}

block::~block() {
  assert(props);
  if(props->active) {
    dbg.exitBlock();
    dbg.exit(this);
  }
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
    char close[]="&#91;";
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
  //cerr << "dbgBuf::sync()\n";
  
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

dbgStream::dbgStream() : common::dbgStream(&defaultFileBuf), initialized(false)
{
  dbgFile = NULL;
  //buf = new dbgBuf(cout.rdbuf());
  buf = new dbgBuf(preInitStream.rdbuf());
  ostream::init(buf);
}

dbgStream::dbgStream(properties* props, string title, string workDir, string imgDir, std::string tmpDir)
  : common::dbgStream(&defaultFileBuf)
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
    FILE *out = popen(getenv("SIGHT_LAYOUT_EXEC"), "w");
    int outFD = fileno(out);
    buf = new dbgBuf(new fdoutbuf(outFD));
  // Version 3 (default): write output to a pipe for the default slayout to use immediately
  } else {
//cout << "slayout"<<endl;
    dbgFile = NULL;
    FILE *out = popen((txt()<<ROOT_PATH<<"/slayout").c_str(), "w");
    int outFD = fileno(out);
    buf = new dbgBuf(new fdoutbuf(outFD));
  }
  ostream::init(buf);

  this->props = props; 
  enter(this);

  // The application may have written text to this dbgStream before it was fully initialized.
  // This text was stored in preInitStream. Print it out now.
  ownerAccessing();
  dbg << preInitStream.str();
  userAccessing();
  
  initialized = true;
}

dbgStream::~dbgStream()
{
  if (!initialized)
    return;
  
//  assert(dbgFile);
  if(dbgFile) dbgFile->close();
  
  { ostringstream cmd;
    cmd << "rm -rf " << tmpDir;
    system(cmd.str().c_str());
  }
  
  exit(this);
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
  //cout << "subBlockEnterNotify("<<subBlock->getLabel()<<")"<<endl;
  // Walk up the block stack, informing each block about the new arrival until the block's
  // subBlockEnterNotify() function returns false to indicate that the notification should not be propagated further.
  for(list<block*>::const_reverse_iterator b=blocks.rbegin(); b!=blocks.rend(); b++)
  {
    //cout << "    "<<(*b)->getLabel()<<endl;
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
  
  sightObj *obj = new sightObj(new properties());
  
  map<string, string> newProps;
  newProps["path"] = imgFName.str();
  obj->props->add("image", newProps);
  
  tag(obj);
  
  delete(obj);
  
  return imgFName.str();
}

// Emit the entry into a tag to the structured output file. The tag is set to the given property key/value pairs
//void dbgStream::enter(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom) {
void dbgStream::enter(sightObj* obj) {
  if(obj->props) {
    ownerAccessing();
    *this << enterStr(*(obj->props));
    userAccessing();
  }
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
  
  for(list<pair<string, map<string, string> > >::const_iterator i=props.begin(); i!=props.end(); i++) {
    list<pair<string, map<string, string> > >::const_iterator iNext=i; iNext++;
    oss << "["<<(iNext!=props.end()? "|": "")<<i->first<<" ";
    oss << "numProperties=\""<<i->second.size()<<"\"";
    
    int j=0;
    for(std::map<std::string, std::string>::const_iterator p=i->second.begin(); p!=i->second.end(); p++, j++)
      oss << " name"<<j<<"=\""<<escape(p->first)<<"\" val"<<j<<"=\""<<escape(p->second)<<"\"";
    
    oss << "]";
  }
  
  return oss.str();
}

// Emit the exit from a given tag to the structured output file
//void dbgStream::exit(std::string name) {
void dbgStream::exit(sightObj* obj) {
  if(obj->props) {
    ownerAccessing();
    *this << exitStr(*(obj->props));
    userAccessing();
  }
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

// Returns the text that should be emitted to the the structured output file to that denotes a full tag an an the structured output file
//std::string dbgStream::tagStr(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom) {
std::string dbgStream::tagStr(sightObj* obj) {
  return enterStr(*(obj->props)) + exitStr(*(obj->props));
}

/******************
 ***** indent *****
 ******************/

indent::indent(std::string prefix,                                       properties* props)
{ init(prefix, 1, NULL, props); }
indent::indent(std::string prefix, int repeatCnt, const attrOp& onoffOp, properties* props)
{ init(prefix, repeatCnt, &onoffOp, props); }
indent::indent(std::string prefix, int repeatCnt,                        properties* props)
{ init(prefix, repeatCnt, NULL, props); }
indent::indent(                    int repeatCnt,                        properties* props)
{ init("    ", repeatCnt, NULL, props); }
indent::indent(std::string prefix,                const attrOp& onoffOp, properties* props)
{ init(prefix, 1, &onoffOp, props); }
indent::indent(                    int repeatCnt, const attrOp& onoffOp, properties* props)
{ init("    ", repeatCnt, &onoffOp, props); }
indent::indent(                                   const attrOp& onoffOp, properties* props)
{ init("    ", 1, &onoffOp, props); }
indent::indent(                                                          properties* props)
{ init("    ", 1, NULL, props); }

void indent::init(std::string prefix, int repeatCnt, const attrOp* onoffOp, properties* props) {
  if(props==NULL) this->props = new properties();
  else            this->props = props;
  
  if(repeatCnt>0 && attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    this->props->active = true;
    map<string, string> newProps;
    newProps["prefix"] = prefix;
    newProps["repeatCnt"] = txt()<<repeatCnt;
    this->props->add("indent", newProps);
    
    dbg.enter(this);
  } else
    this->props->active = false;
}

indent::~indent() {
  assert(props);
  if(props->active) {
    dbg.exit(this);
  }
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
