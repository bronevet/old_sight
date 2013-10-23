#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <stdarg.h>
#include <assert.h>
#include "dbglog_common.h"
#include "attributes_structure.h"
#include "utils.h"

namespace dbglog {
namespace structure{

// Records the information needed to call the application
extern bool saved_appExecInfo; // Indicates whether the application execution info has been saved
extern int saved_argc;
extern char** saved_argv;
extern char* saved_execFile;

// The name of the host that this application is currently executing on
extern char hostname[];

// The current user's user name
extern char username[10000];

// Initializes the debug sub-system
void initializeDebug(int argc, char** argv, std::string title="Debug Output", std::string workDir="dbg");
void initializeDebug(std::string title, std::string workDir);

class dbgStream;

// Represents a unique location in the dbglog output
class location : printable {
  std::list<std::pair<int, std::list<int> > > l;
  
  public:
  location();
  ~location();
  
  void enterFileBlock();
  void exitFileBlock();

  void enterBlock();
  void exitBlock();

  void operator=(const location& that);
  bool operator==(const location& that);
  bool operator<(const location& that);

  void print(std::ofstream& ofs) const;
};

// Base class of all dbglog objects that provides some common functionality
class dbglogObj {
  public:
  properties* props;
  dbglogObj() : props(NULL) {}
  dbglogObj(properties* props) : props(props) {}
    
  ~dbglogObj() {
    assert(props);
    delete(props);
  }
};

// Uniquely identifies a location with the debug information, including the file and region hierarchy
// Anchors can be created in two ways:
// - When their host output locations are reached. In this case the anchor's full file and region location is available
// - Before their host locations are reached (when forward links are created). In this case the anchor's location info
//   is not available at anchor creation time and must be initialized subsequently. We'll need to generate output for
//   links to anchors either before (forward links) or after (backward links) the anchor's output location has been 
//   reached. For backward links we can generate code easily since the anchor's target is known at output generation time.
//   To support forward links we do the following:
//   - When the output location is reached, we emit a piece of code to the script file that register's the location
//     in a data sctructure. There is one anchor script file for the entire output (not one per sub-file) to make it
//     easier to deal with anchors that cross files.
//   - When forward links are created we generate code that looks the link's destination in the data structure. If
//     the destination has already been loaded in the data structure, we go directly there. If the destination has not
//     been loaded, this means that the application generating the debug log has crashed and thus, we shouldn't follow
//     the link.
class anchor
{
  protected:
  static int maxAnchorID;
  int anchorID;

  // Itentifies this anchor's location in the file and region hierarchies
//  location loc;

  // Flag that indicates whether we've already reached this anchor's location in the output
  //bool located;
  
  public:
  anchor();
  anchor(const anchor& that);
  anchor(int anchorID);
  
  int getID() const { return anchorID; }

 // bool isLocated() const { return located; }
  
  public:
  static anchor noAnchor;
  
  void operator=(const anchor& that);
  
  bool operator==(const anchor& that) const;
  bool operator!=(const anchor& that) const;
  
  bool operator<(const anchor& that) const;
  
  // Emits to the output an html tag that denotes a link to this anchor. Embeds the given text in the link.
  void link(std::string text) const;
    
  // Emits to the output an html tag that denotes a link to this anchor, using the default link image, which is followed by the given text.
  void linkImg(std::string text="") const;
  
  std::string str(std::string indent="") const;
};

// A block out debug output, which may be filled by various visual elements
class block : public dbglogObj
{
  std::string label;
  // The unique ID of this block as well as the static global counter of the maximum ID assigned to any block.
  // Unlike the rendering module, these blockIDs are integers since all we need from them is uniqueness and not
  // any structural information.
  int blockID;
  // maxBlockID also counts the number of times that the block constructor was called, which makes it possible
  // to set conditional breakpoints to run a debugger to the entry into a specific block in the debug output.
  static int maxBlockID;
  
  // The anchor that denotes the starting point of this scope
  anchor startA;

  protected:
  
  public:
  // Initializes this block with the given label
  block(std::string label="", properties* props=NULL);
    
  // Initializes this block with the given label.
  // Includes one or more incoming anchors thas should now be connected to this block.
  block(std::string label, const anchor& pointsTo, properties* props=NULL);
  block(std::string label, const std::set<anchor>& pointsTo, properties* props=NULL);
 
  ~block();
  
  // Increments blockID. This function serves as the one location that we can use to target conditional
  // breakpoints that aim to stop when the block count is a specific number
  int advanceBlockID();
  
  std::string getLabel() const { return label; }
  int getBlockID() const { return blockID; }
  
  protected:
  anchor& getAnchorRef();
  
  public:
  anchor getAnchor() const;
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  virtual bool subBlockEnterNotify(block* subBlock) { return true; }
  virtual bool subBlockExitNotify (block* subBlock) { return true; }

};

// Adapted from http://wordaligned.org/articles/cpp-streambufs
// A extension of stream that corresponds to a single file produced by dbglog
class dbgBuf: public std::streambuf
{
  friend class dbgStream;
  // True immediately after a new line
  bool synched;
  // True if the owner dbgStream is writing text and false if the user is
  bool ownerAccess;
  std::streambuf* baseBuf;

  // The number of observed '<' characters that have not yet been balanced out by '>' characters.
  //      numOpenAngles = 1 means that we're inside an HTML tag
  //      numOpenAngles > 1 implies an error or text inside a comment
  int numOpenAngles;
  
  public:
  int getNumOpenAngles() const { return numOpenAngles; }

  public:
        
  virtual ~dbgBuf() {};
  // Construct a streambuf which tees output to both input
  // streambufs.
  dbgBuf();
  dbgBuf(std::streambuf* baseBuf);
  void init(std::streambuf* baseBuf);
  
private:
  // This dbgBuf has no buffer. So every character "overflows"
  // and can be put directly into the teed buffers.
  virtual int overflow(int c);
  
  // Prints the given string to the stream buffer
  int printString(std::string s);

  virtual std::streamsize xsputn(const char * s, std::streamsize n);

  // Sync buffer.
  virtual int sync();
        
  // Switch between the owner class and user code writing text
protected:
  void userAccessing();
  void ownerAccessing();

  // b - The block that was just entered
  // isAttrSubBlock - Records whether b is a sub-block that spans between creation points of attribute values and 
  //   terminal points of blocks. For such blocks we record that they exist to make sure they get unique names but
  //   don't add any additional indentation.
  void enterBlock(block* b, bool isAttrSubBlock);
  
  // Called when a block is exited. Returns the block that was exited.
  // isAttrSubBlock - Records whether b is a sub-block that spans between creation points of attribute values and 
  //   terminal points of blocks. For such blocks we record that they exist to make sure they get unique names but
  //   don't add any additional indentation.
  block* exitBlock(bool isAttrSubBlock);
  
  // Returns the depth of enterBlock calls that have not yet been matched by exitBlock calls
  int blockDepth();
}; // class dbgBuf


// Stream that uses dbgBuf
class dbgStream : public std::ostream, public dbglogObj
{
  dbgBuf defaultFileBuf;
  // Stream to the file where the structure will be written
  std::ofstream *dbgFile;
  // Buffer for the above stream
  dbgBuf* buf;
  
  // The root working directory
  std::string workDir;
  // The directory where all images will be stored
  std::string imgDir;
  // The directory that widgets can use as temporary scratch space
  std::string tmpDir;
  // The total number of images in the output file
  int numImages;
 
  // The current location within the debug output
  location loc;
 
  // Stack of blocks currently in scope
  std::list<block*> blocks;
  
  bool initialized;
  
public:
  // Construct an ostream which tees output to the supplied
  // ostreams.
  dbgStream();
  dbgStream(properties* props, std::string workDir, std::string imgDir, std::string tmpDir);
  void init(properties* props, std::string workDir, std::string imgDir, std::string tmpDir);
  ~dbgStream();
  
  // Switch between the owner class and user code writing text into this stream
  void userAccessing();
  void ownerAccessing();
  
  // Return the root working directory
  const std::string& getWorkDir() const { return workDir; }
  // Return the directory where all images will be stored
  const std::string& getImgDir() const { return imgDir; }
  // Return the directory that widgets can use as temporary scratch space
  const std::string& getTmpDir() const { return tmpDir; }
  
  // Called when a block is entered.
  // b: The block that is being entered
  void enterBlock(block* b);
  
  // Called when a block is exited. Returns the block that was exited.
  block* exitBlock();
    
  // Called to inform the blocks that contain the given block that it has been entered
  void subBlockEnterNotify(block* subBlock);
  
  // Called to inform the blocks that contain the given block that it has been exited
  void subBlockExitNotify(block* subBlock);
    
  // Adds an image to the output with the given extension and returns the path of this image
  // so that the caller can write to it.
  std::string addImage(std::string ext=".gif");
    
  // ----- Output of tags ----
  // Emit the entry into a tag to the structured output file. The tag is set to the given property key/value pairs
  //void enter(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom);
  void enter(dbglogObj* obj);
    
  // Returns the text that should be emitted to the structured output file that denotes the the entry into a tag. 
  // The tag is set to the given property key/value pairs
  //std::string enterStr(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom);
  std::string enterStr(dbglogObj* obj);
    
  // Emit the exit from a given tag to the structured output file
  //void exit(std::string name);
  void exit(dbglogObj* obj);
    
  // Returns the text that should be emitted to the the structured output file to that denotes exit from a given tag
  //std::string exitStr(std::string name);
  std::string exitStr(dbglogObj* obj);
  
  // Emit a full tag an an the structured output file
  //void tag(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom);
  void tag(dbglogObj* obj);
  
  // Returns the text that should be emitted to the the structured output file to that denotes a full tag an an the structured output file
  //std::string tagStr(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom);
  std::string tagStr(dbglogObj* obj);
}; // dbgStream

extern bool initializedDebug;
extern dbgStream dbg;

class indent : public dbglogObj
{
  public:
  //bool active;
  // prefix - the string that will be prepended to all subsequent lines. (default is "    ")
  // repeatCnt - the number of repetitions of this string. If repeatCnt=0 we will not do any indentation. (default is 1).
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  indent(std::string prefix,                                       properties* props=NULL);
  indent(std::string prefix, int repeatCnt, const attrOp& onoffOp, properties* props=NULL);
  indent(std::string prefix, int repeatCnt,                        properties* props=NULL);
  indent(                    int repeatCnt,                        properties* props=NULL);
  indent(std::string prefix,                const attrOp& onoffOp, properties* props=NULL);
  indent(                    int repeatCnt, const attrOp& onoffOp, properties* props=NULL);
  indent(                                   const attrOp& onoffOp, properties* props=NULL);
  indent(                                                          properties* props=NULL);
  void init(std::string prefix, int repeatCnt, const attrOp* onoffOp, properties* props);
    
  ~indent();
};

// Given a string, returns a version of the string with all the control characters that may appear in the 
// string escaped to that the string can be written out to Dbg::dbg with no formatting issues.
// This function can be called on text that has already been escaped with no harm.
std::string escape(std::string s);
  
int dbgprintf(const char * format, ... );

}; // namespace structure
}; // namespace dbglog
