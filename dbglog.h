#pragma once

#include <list>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

namespace dbglog {

class printable
{
  public:
  virtual ~printable() {}
  virtual std::string str(std::string indent="")=0;
};

class dottable
{
  public:
  virtual ~dottable() {}
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  virtual std::string toDOT(std::string graphName)=0;
};

class anchor;

class dbgStream;
typedef std::list<std::pair<int, std::list<int> > > location;

// A block out debug output, which may be filled by various visual elements
class block
{
  std::string label;
  location    loc;
  std::string fileID;
  std::string blockID;
  
  public:
  block(std::string label);
  
  std::string getLabel() const { return label; }
  const location& getLocation() const { return loc; }
  void setLocation(const location& loc);
  std::string getFileID() const { return fileID; }
  std::string getBlockID() const { return blockID; }
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  virtual bool subBlockStartNotify(const block& subBlock) { return true; }
  virtual bool subBlockEndNotify  (const block& subBlock) { return true; }
  
  // Called to enable the block to print its entry and exit text
  virtual void printEntry(std::string loadCmd) {}
  virtual void printExit() {}
};

// Adopted from http://wordaligned.org/articles/cpp-streambufs
// A extension of stream that corresponds to a single file produced by dbglog
class dbgBuf: public std::streambuf
{
  friend class dbgStream;
  // True immediately after a new line
  bool synched;
  // True if the owner dbgStream is writing text and false if the user is
  bool ownerAccess;
  std::streambuf* baseBuf;
  std::list<block*> blocks;

  // The number of observed '<' characters that have not yet been balanced out by '>' characters.
  //      numOpenAngles = 1 means that we're inside an HTML tag
  //      numOpenAngles > 1 implies an error or text inside a comment
  int numOpenAngles;
  
  public:
  int getNumOpenAngles() const { return numOpenAngles; }
  
  protected:

  // The number of divs that have been inserted into the output
  std::list<int> parentDivs;
  // For each div a list of the strings that need to be concatenated to form the indent for each line
  std::list<std::list<std::string> > indents;

  // Flag that indicates that a new line has begun and thus before the next printed character
  // we need to print the indent
  bool needIndent;
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

  // Prints the indent to the stream buffer, returns 0 on success non-0 on failure
  //int printIndent();

  // Prints the given string to the stream buffer
  int printString(std::string s);

  //virtual int sputc(char c);

  // Get the current indentation depth within the current div
  int getIndentDepth();
  // Get the current indentation within the current div
  std::string getIndent();
  // Add more indentation within the current div
  void addIndent(std::string indent);
  // Remove the most recently added indent within the current div
  void remIndent();

  virtual std::streamsize xsputn(const char * s, std::streamsize n);

  // Sync buffer.
  virtual int sync();
        
  // Switch between the owner class and user code writing text
protected:
  void userAccessing();
  void ownerAccessing();

  // Called when a block is entered.
  void enterBlock(block* b);
  // Called when a block is exited. Returns the block that was exited.
  block* exitBlock();
  
  // Returns the depth of enterBlock calls that have not yet been matched by exitBlock calls
  int blockDepth();
};


// Stream that uses dbgBuf
class dbgStream : public std::ostream
{
  std::list<std::ofstream*> indexFiles;
  std::list<std::ofstream*> dbgFiles;
  std::list<std::string>    detailFileRelFNames; // Relative names of all the dbg files on the stack
  std::list<std::ofstream*> summaryFiles;
  std::list<std::ofstream*> scriptFiles;
  // Global script file that stores the mapping between anchor IDs and their referents
  std::ofstream             anchorScriptFile;
  std::list<int>            fileLevel;
  location                  loc;
  std::list<dbgBuf*>        fileBufs;
  // The scopes inside which sub-files are loaded
  std::list<block*>         fileBlocks;
  dbgBuf defaultFileBuf;
  
  // The title of the file
  std::string title;
  // The root working directory
  std::string workDir;
  // The directory where all images will be stored
  std::string imgPath;
  // The total number of images in the output file
  int numImages;
  
  bool initialized;
  
public:
  // Construct an ostream which tees output to the supplied
  // ostreams.
  dbgStream();
  dbgStream(std::string title, std::string workDir, std::string imgPath);
  void init(std::string title, std::string workDir, std::string imgPath);
  ~dbgStream();

  // Switch between the owner class and user code writing text into this stream
  void userAccessing();
  void ownerAccessing();

  // Returns the string representation of the current file level
  std::string fileLevelStr(const location& myLoc) const;
  
  // Returns a string that encodes a Javascript array of integers that together form the unique ID string of 
  // the current file within the hierarchy of files
  std::string fileLevelJSIntArray(const location& myLoc) const;
  
  ///const std::list<int>& getFileLevel() const;
  const location& getLocation() const;
    
  // Returns the unique ID string of the current region, including all the files that it may be contained in
  std::string blockGlobalStr(const location& myLoc) const;

  // Returns the depth of enterBlock calls that have not yet been matched by exitBlock calls within the current file
  int blockDepth() const;
  // Index of the current block in the current fil
  int blockIndex() const;

  // Enter a new file level. Return a string that contains the JavaScript command to open this file in the current view.
  std::string enterFileLevel(block* b, bool topLevel=false);
  
  // Exit a current file level
  block* exitFileLevel(bool topLevel=false);
    
  // Record the mapping from the given anchor ID to the given string in the global script file
  void writeToAnchorScript(int anchorID, const location& myLoc, std::string blockID);

  void printSummaryFileContainerHTML(std::string absoluteFileName, std::string relativeFileName, std::string title);
  void printDetailFileContainerHTML(std::string absoluteFileName, std::string title);
  /*void printDetailFileHeader(std::string title);
  void printDetailFileTrailer();*/
  
  // Called when a block is entered.
  std::string enterBlock(block* b, bool newFileEntered);//, std::string fileIDJSArray="");//std::string detailContentURL="", std::string summaryContentURL="");
    // Called when a block is exited. Returns the block that was exited.
  block* exitBlock();

  // Adds an image to the output with the given extension and returns the path of this image
  // so that the caller can write to it.
  std::string addImage(std::string ext=".gif");

  // Given a reference to an object that can be represented as a dot graph, create an image from it and add it to the output.
  // Return the path of the image.
  std::string addDOT(dottable& obj);
  // Given a reference to an object that can be represented as a dot graph, create an image of it and return the string
  // that must be added to the output to include this image.
  std::string addDOTStr(dottable& obj);
  // Given a representation of a graph in dot format, create an image from it and add it to the output.
  // Return the path of the image.
  std::string addDOT(std::string dot);
  // The common work code for all the addDOT methods
  void addDOT(std::string imgFName, std::string graphName, std::string dot, std::ostream& ret);

  // Add a given amount of indent space to all subsequent text within the current function
  void addIndent(std::string indent);
  // Remove the most recently added indent
  void remIndent();
  
  // Creates a link to a given scope, which may be in another file
  //std::string linkTo(const anchor& id, std::string text) const;
}; // dbgStream

extern bool initializedDebug;
extern dbgStream dbg;

class indent {
public:
  bool active;
  indent(std::string space="&nbsp;&nbsp;&nbsp;&nbsp;", int curDebugLevel=0, int targetDebugLevel=0);
  //indent(std::string space="&nbsp;&nbsp;&nbsp;&nbsp;");
  ~indent();
};

// Class that makes it possible to generate string labels by using the << syntax.
// Examples: Label() << "a=" << (5+2) << "!"
//           Label("a=") << (5+2) << "!"
struct txt : std::string {
  txt() {}
  txt(const std::string& initTxt) {
  	 _stream << initTxt;
  	 assign(_stream.str());
  }
  
  template <typename T>
  txt& operator<<(T const& t) {
    _stream << t;
    assign(_stream.str());
    return *this;
  }

  std::string str() const { return _stream.str(); }
  std::ostringstream _stream;
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
  public:
  static int maxAnchorID;
  int anchorID;
  
  // The debug stream this anchor is associated with
  //dbgStream& myDbg;
  
  // Itentifies this anchor's location in the file and region hierarchies
  location loc;
  std::string blockID;
  
  // Flag that indicates whether we've already reached this anchor's location in the output
  bool reached;
  
  public:
  anchor(/*dbgStream& myDbg, */bool reached=false);
  anchor(const anchor& that);
  anchor(/*dbgStream& myDbg, */bool reached, int anchorID);
  
  void init(bool reached);
  
  static anchor noAnchor;
  
  void operator=(const anchor& that);
  
  bool operator==(const anchor& that) const;
  bool operator!=(const anchor& that) const;
    
  // Called when the file location of this anchor has been reached
  void reachedAnchor();
  // Emits to the output dbgStream the string that denotes a link to this anchor.
  // Embed the given text in the link.
  void link(std::string text) const;
  std::string str() const;
};

class scope: public block
{
  static std::vector<std::string> colors;
  static int colorIdx; // The current index into the list of colors 
  
  public:
  bool active;
  typedef enum {high, medium, low} scopeLevel;
  scopeLevel level;
  
  // The anchor that denotes the starting point of this scope
  anchor startA;
  
  scope(std::string label, scopeLevel level=medium, int curDebugLevel=0, int targetDebugLevel=0);
  scope(std::string label, anchor& pointsTo, scopeLevel level=medium, int curDebugLevel=0, int targetDebugLevel=0);
  
  private:
  // Common initialization code
  void init(scopeLevel level, int curDebugLevel, int targetDebugLevel);
  
  public:
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockStartNotify(const block& subBlock) { return true; }
  bool subBlockEndNotify  (const block& subBlock) { return true; }
  
  // Called to enable the block to print its entry and exit text
  void printEntry(std::string loadCmd);
  void printExit();

  anchor getAnchor() const;
  
  /*std::ostringstream labelStream;
  /*std::ostringstream& operator<<(std::string s);
  std::ostringstream& operator<<(std::ostringstream& oss);*/
  
  ~scope();
}; // scope

class graph: public block
{
  std::set<block*> nodes;
  std::set<std::pair<block*, block*> > edges;
};

// Initializes the debug sub-system
void initializeDebug(std::string title, std::string workDir);

// Indicates that the application has entered or exited a file
/*void enterFile(std::string funcName);
void exitFile(std::string funcName);*/
        
/* // Called when a block is entered.
void enterBlock(block* b, bool advanceColor=true);
// Called when a block is exited. Returns the block that was exited.
block* exitBlock(bool advanceColor=true);*/

// Adds an image to the output with the given extension and returns the path of this image
// so that the caller can write to it.
std::string addImage(std::string ext=".gif");

// Given a reference to an object that can be represented as a dot graph,  create an image from it and add it to the output.
// Return the path of the image.
std::string addDOT(dottable& obj);

// Given a reference to an object that can be represented as a dot graph, create an image of it and return the string
// that must be added to the output to include this image.
std::string addDOTStr(dottable& obj);

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
std::string addDOT(std::string dot);

// Given a string, returns a version of the string with all the control characters that may appear in the 
// string escaped to that the string can be written out to Dbg::dbg with no formatting issues.
// This function can be called on text that has already been escaped with no harm.
std::string escape(std::string s);
} // namespace dbglog

