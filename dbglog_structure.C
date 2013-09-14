// Licence information included in file LICENCE
#include "dbglog_structure_internal.h"
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
#include "binreloc.h"
#include <errno.h>
#include "getAllHostnames.h" 
#include "utils.h"
using namespace std;
using namespace dbglog;

namespace dbglog {
namespace structure{

/***************
 ***** dbg *****
 ***************/

dbgStream dbg;

bool initializedDebug=false;

void initializeDebug_internal(int argc, char** argv, string title, string workDir);

// Records the information needed to call the application
bool saved_appExecInfo=false; // Indicates whether the application execution info has been saved
int saved_argc = 0;
char** saved_argv = NULL;
char* saved_execFile = NULL;

void initializeDebug(int argc, char** argv, string title, string workDir)
{
  if(initializedDebug) return;
  initializeDebug_internal(argc, argv, title, workDir);
}

// Initializes the debug sub-system
void initializeDebug(string title, string workDir)
{
  if(initializedDebug) return; 
  initializeDebug_internal(0, NULL, title, workDir);
}

void initializeDebug_internal(int argc, char** argv, string title, string workDir)
{
  map<string, string> properties;

  properties["title"] = title;
  properties["workDir"] = workDir;
  // Records whether we know the application's command line, which would enable us to call it
  properties["commandLineKnown"] = (argv!=NULL);
  // If the command line is known, record it in properties
  if(argv!=NULL) {
    properties["argc"] = txt()<<argc;
    for(int i=0; i<argc; i++)
      properties[txt()<<"argv["<<i<<"]"] = string(argv[i]);

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
    properties["execFile"] = execFile;
    
//!!!    properties["appPWD"] = `pwd`;
  }
  
  // Get the name of the current host
  char hostname[10000]; // The name of the host that this application is currently executing on
  int ret = gethostname(hostname, 10000);
  properties["hostname"] = string(hostname);

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
  properties["username"] = string(username);
  
  // Create the directory structure for the structural information
  // Main output directory
  createDir(workDir, "");

  // Directory where client-generated images will go
  string imgDir = createDir(workDir, "html/dbg_imgs");
  
  // Directory that widgets can use as temporary scratch space
  string tmpDir = createDir(workDir, "html/tmp");
  
  initializedDebug = true;
  
  dbg.init(properties, workDir, imgDir, tmpDir);
}

// Returns a string that contains n tabs
string tabs(int n)
{
  string s;
  for(int i=0; i<n; i++)
    s+="\t";
  return s;
}

/******************
 ***** anchor *****
 ******************/
int anchor::maxAnchorID=0;
anchor anchor::noAnchor(-1);
  
anchor::anchor()                   : anchorID(maxAnchorID++) {}
anchor::anchor(const anchor& that) : anchorID(that.anchorID) {} 
anchor::anchor(int anchorID)       : anchorID(anchorID)      {}

void anchor::operator=(const anchor& that) {
  anchorID = that.anchorID;
}

bool anchor::operator==(const anchor& that) const {
  // anchorID of -1 indicates that this is the noAnchor object and any copies of it are equivalent
  assert(anchorID>=-1); assert(that.anchorID>=-1);
  if(anchorID==-1 && that.anchorID==-1) return true;

  // They're equal if they have the same ID
  return anchorID == that.anchorID;
}

bool anchor::operator!=(const anchor& that) const
{ return !(*this == that); }

bool anchor::operator<(const anchor& that) const
{
  // anchorID of -1 indicates that this is the noAnchor object and any copies of it are equivalent
  assert(anchorID>=-1); assert(that.anchorID>=-1);
  if(anchorID==-1 && that.anchorID==-1) return false;
  
  // Compare their IDs
  return anchorID < that.anchorID;
}

// Emits to the output an html tag that denotes a link to this anchor. Embeds the given text in the link.
void anchor::link(string text) const {
  map<string, string> properties;
  properties["anchorID"] = txt()<<anchorID;
  properties["text"] = text;
  properties["img"] = "false";
  dbg.tag("link", properties);
}

// Emits to the output an html tag that denotes a link to this anchor, using the default link image, which is followed by the given text.
void anchor::linkImg(string text) const {
  map<string, string> properties;
  properties["anchorID"] = txt()<<anchorID;
  properties["text"] = text;
  properties["img"] = "true";
  dbg.tag("link", properties);
}

std::string anchor::str(std::string indent) const {
  if(anchorID==-1) return "[noAnchor_structure]";
  else             return txt()<<"[anchor_structure: ID="<<anchorID<<"\"]";
}

/*****************
 ***** block *****
 *****************/

// The unique ID of this block as well as the static global counter of the maximum ID assigned to any block.
// Unlike the rendering module, these blockIDs are integers since all we need from them is uniqueness and not
// any structural information.
int block::maxBlockID;

// Initializes this block with the given label
block::block(string label) : label(label) {
  advanceBlockID();
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
    
  map<string, string> properties;
  properties["label"] = label;
  properties["ID"] = txt()<<blockID;
  
  dbg.enter("block", properties);
}

// Initializes this block with the given label.
// Includes one or more incoming anchors thas should now be connected to this block.
block::block(string label, const anchor& pointsTo) : label(label) {
  advanceBlockID();
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  
  map<string, string> properties;
  properties["label"] = label;
  properties["ID"] = txt()<<blockID;
  properties["numAnchors"] = txt()<<1;
  properties["anchor_0"] = txt()<<pointsTo.getID();
  
  dbg.enter("block", properties);
}

// Initializes this block with the given label.
// Includes one or more incoming anchors thas should now be connected to this block.
block::block(string label, const set<anchor>& pointsTo) : label(label)
{
  advanceBlockID();
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");

  map<string, string> properties;
  properties["label"] = label;
  properties["ID"] = txt()<<blockID;
  properties["numAnchors"] = txt()<<pointsTo.size();
  
  int i=0;
  for(set<anchor>::const_iterator a=pointsTo.begin(); a!=pointsTo.end(); a++, i++)
    properties[txt()<<"anchor_"<<i] = txt()<<a->getID();
  
  dbg.enter("block", properties);
}

block::~block() {
  dbg.exit("block");
}

// Increments blockD. This function serves as the one location that we can use to target conditional
// breakpoints that aim to stop when the block count is a specific number
int block::advanceBlockID() {
  maxBlockID++;
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
  cerr << "overflow\n";
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
  cerr << "xputn() << ownerAccess="<<ownerAccess<<" n="<<n<<" s=\""<<string(s)<<"\" query="<<attributes.query()<<"\n";
  
  // Only emit text if the current query on attributes evaluates to true
  if(!attributes.query()) return n;
  
  // If the owner is printing, output their text exactly
  if(ownerAccess) {
    int ret = baseBuf->sputn(s, n);
    cerr << "xputn() >>>\n";
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

    cerr << "xputn() >>>\n";
    return n;
  }
}

// Sync buffer.
int dbgBuf::sync()
{
  // Only emit text if the current query on attributes evaluates to true
  //  if(!attributes.query()) return 0;
  cerr << "dbgBuf::sync()\n";
  
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

dbgStream::dbgStream() : std::ostream(&defaultFileBuf), initialized(false)
{
  dbgFile = NULL;
}

dbgStream::dbgStream(std::map<std::string, std::string>& properties, string workDir, string imgDir, std::string tmpDir)
  : std::ostream(&defaultFileBuf)
{
  init(properties, workDir, imgDir, tmpDir);
}

void dbgStream::init(std::map<std::string, std::string>& properties, string workDir, string imgDir, std::string tmpDir)
{
  this->workDir = workDir;
  this->imgDir  = imgDir;
  this->tmpDir  = tmpDir;

  numImages++;
  
  // Create the output file to which the debug log's structure will be written
  dbgFile = &(createFile(txt()<<workDir<<"/structure"));
  // Call the parent class initialization function to connect it dbgBuf of the output file
  buf=new dbgBuf(dbgFile->rdbuf());
  ostream::init(buf);
  
  enter("dbglog", properties);
  
  initialized = true;
}

dbgStream::~dbgStream()
{
  if (!initialized)
    return;
  
  assert(dbgFile);
  dbgFile->close();
  
  { ostringstream cmd;
    cmd << "rm -rf " << tmpDir;
    system(cmd.str().c_str());
  }
  
  exit("dbglog");
}

// Called when a block is entered.
// b: The block that is being entered
void dbgStream::enterBlock(block* b) {
  blocks.push_back(b);
}

// Called when a block is exited. Returns the block that was exited.
block* dbgStream::exitBlock() {
  assert(blocks.size()>0);
  
  block* lastB = blocks.back();
  blocks.pop_back();
  return lastB;
}

// Called to inform the blocks that contain the given block that it has been entered
void dbgStream::subBlockEnterNotify(block* subBlock)
{
  // Walk up the block stack, informing each block about the new arrival until the block's
  // subBlockEnterNotify() function returns false to indicate that the notification should not be propagated further.
  for(list<block*>::const_reverse_iterator b=blocks.rbegin(); b!=blocks.rend(); b++)
    if(!(*b)->subBlockEnterNotify(subBlock)) return;
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
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  
  ostringstream imgFName; imgFName << imgDir << "/image_" << numImages << "." << ext;
  map<string, string> properties;
  properties["path"] = imgFName.str();
  tag("image", properties);
  return imgFName.str();
}

// Emit the entry into a tag to the structured output file. The tag is set to the given property key/value pairs
void dbgStream::enter(std::string name, const std::map<std::string, std::string>& properties) {
  ownerAccessing();
  *this << enterStr(name, properties);
  userAccessing();
}

// Returns the text that should be emitted to the structured output file that denotes the the entry into a tag. 
// The tag is set to the given property key/value pairs
string dbgStream::enterStr(std::string name, const std::map<std::string, std::string>& properties) {
  ostringstream oss;
  oss << "["<<name<<" ";
  oss << "numProperties=\""<<properties.size()<<"\"";
  
  int i=0;
  for(std::map<std::string, std::string>::const_iterator p=properties.begin(); p!=properties.end(); p++, i++)
    oss << " name"<<i<<"=\""<<p->first<<"\" val"<<i<<"=\""<<p->second<<"\"";
  
  oss << "]\n";
  
  return oss.str();
}

// Emit the exit from a given tag to the structured output file
void dbgStream::exit(std::string name) {
  ownerAccessing();
  *this << exitStr(name);
  userAccessing();
}

// Returns the text that should be emitted to the the structured output file to that denotes exit from a given tag
std::string dbgStream::exitStr(std::string name) {
  ostringstream oss;
  oss <<"[/"<<name<<"]\n";
  return oss.str();
}

// Emit an entry an an immediate exit  
void dbgStream::tag(std::string name, const std::map<std::string, std::string>& properties)
{
  enter(name, properties);
  exit(name);
}

// Returns the text that should be emitted to the the structured output file to that denotes a full tag an an the structured output file
std::string dbgStream::tagStr(std::string name, const std::map<std::string, std::string>& properties) {
  return enterStr(name, properties) + exitStr(name);
}

/******************
 ***** indent *****
 ******************/

indent::indent(std::string prefix)
{ init(prefix, 1, NULL); }
indent::indent(std::string prefix, int repeatCnt, const attrOp& onoffOp)
{ init(prefix, repeatCnt, &onoffOp); }
indent::indent(std::string prefix, int repeatCnt)
{ init(prefix, repeatCnt, NULL); }
indent::indent(                    int repeatCnt)
{ init("    ", repeatCnt, NULL); }
indent::indent(std::string prefix,                const attrOp& onoffOp)
{ init(prefix, 1, &onoffOp); }
indent::indent(                    int repeatCnt, const attrOp& onoffOp)
{ init("    ", repeatCnt, &onoffOp); }
indent::indent(                                   const attrOp& onoffOp)
{ init("    ", 1, &onoffOp); }
indent::indent()
{ init("    ", 1, NULL); }

void indent::init(std::string prefix, int repeatCnt, const attrOp* onoffOp) {
///  if(repeatCnt>0 && attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    active = true;
    map<string, string> properties;
    properties["prefix"] = prefix;
    properties["repeatCnt"] = txt()<<repeatCnt;
    dbg.enter("indent", properties);
/*  } else
    active = false;*/
}

indent::~indent() {
  if(active) {
    dbg.exit("indent");
  }
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

}; // namespace structure
}; // namespace dbglog
