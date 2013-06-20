#include "dbglog.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

//#define ROOT_PATH "/cygdrive/c/work/code/dbglog"

namespace dbglog {

bool initializedDebug=false;

// Create the directory workDir/dirName and return the string that contains the absolute
// path of the created directory
string createDir(string workDir, string dirName) {
  ostringstream fullDirName; fullDirName<<workDir<<"/"<<dirName;
  ostringstream cmd; cmd << "mkdir -p "<<fullDirName.str();
  int ret = system(cmd.str().c_str());
  if(ret == -1) { cout << "ERROR creating directory \""<<workDir<<"/"<<dirName<<"\"!"; exit(-1); }
  return fullDirName.str();
}

// Copy dirName from the ROOT_PATH to the workDir
string copyDir(string workDir, string dirName) {
  ostringstream fullDirName; fullDirName << workDir << "/" << dirName;
  
  ostringstream cmd; cmd << "cp -fr "<<ROOT_PATH<<"/"<<dirName<<" "<<workDir;
  //cout << "Command \""<<cmd.str()<<"\"\n";
  int ret = system(cmd.str().c_str());
  if(ret == -1) { cout << "ERROR copying files from directory \""<<ROOT_PATH<<"/"<<dirName<<"\" directory \""<<fullDirName.str()<<"\"!"; exit(-1); }
    
  return fullDirName.str();
}

// Initializes the debug sub-system
void initializeDebug(string title, string workDir)
{
  if(initializedDebug) return;
  
  // Main output directory
  createDir(workDir, "");
  
  // Directory where the html files go
  createDir(workDir, "html");
  
  // Directory where the detail files go
  //createDir(workDir, "html/detail");
  
  // Directory where the summary files go
  //createDir(workDir, "html/summary");
  
  // Directory where client-generated images will go
  string imgPath = createDir(workDir, "html/dbg_imgs");
  
  // Copy the default images directory to the work directory
  copyDir(workDir+"/html", "img");

  // Copy the default scripts directory to the work directory
  copyDir(workDir+"/html", "script");
  
  // Create an index.html file that refers to the root html file
  copyDir(workDir, "html/index.html");
  /*{
    ostringstream cmd; cmd << "ln -s "<<workDir<<"/html/index.0.html "<<workDir<<"/index.html";
    int ret = system(cmd.str().c_str());
    if(ret == -1) { cout << "Dbg::init() ERROR creating link \""<<workDir<<"/index.html\"!"; exit(-1); }
  }*/
  
  initializedDebug = true;
  dbg.init(title, workDir, imgPath);
}

/***************
 ***** dbg *****
 ***************/

dbgStream dbg;

/*****************
 ***** block *****
 *****************/

block::block(std::string label) : label(label) {
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
}

void block::setLocation(const location& loc) { 
  this->loc = loc;
  blockID = dbg.blockGlobalStr(loc);
  fileID = dbg.fileLevelStr(loc);
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
  
  // Reset justSynched since we're now adding new characters after the last line break, meaning that no 
  // additional indent will be needed for this line
  //justSynched = false;
  
  if(needIndent) {
    int ret = printString(getIndent()); if(ret != 0) return 0;
    needIndent = false;
  }
  
  //cout << "xputn() ownerAccess="<<ownerAccess<<" n="<<n<<" s=\""<<string(s)<<"\"\n";
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
          int ret = printString(getIndent()); if(ret != 0) return 0;
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
          if(numOpenAngles==0) {
            ret = baseBuf->sputn(spaceHTML, sizeof(spaceHTML)-1);
            if(ret != sizeof(spaceHTML)-1) return 0;
          // If we're inside an HTML tag, emit a regular space character
          } else {
            ret = baseBuf->sputn(space, sizeof(space)-1);
            if(ret != sizeof(space)-1) return 0;
          }
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
    int ret;
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
void dbgBuf::enterBlock(block* b)
{
  //cout << "enterBlock("<<b.getLabel()<<") numOpenAngles="<<numOpenAngles<<endl;
  blocks.push_back(b);
  
  indents.push_back(std::list<std::string>());
}

// Called when a block is exited. Returns the block that was exited.
block* dbgBuf::exitBlock()
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
  indents.pop_back();
  needIndent=false;
  return lastB;
}

// Returns the depth of enterBlock calls that have not yet been matched by exitBlock calls
int dbgBuf::blockDepth()
{ return blocks.size(); }

/*********************
 ***** dbgStream *****
 *********************/

// Returns a string that contains n tabs
string tabs(int n)
{
  string s;
  for(int i=0; i<n; i++)
    s+="\t";
  return s;
}

dbgStream::dbgStream() : std::ostream(&defaultFileBuf), initialized(false)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
}

dbgStream::dbgStream(string title, string workDir, string imgPath)
  : std::ostream(&defaultFileBuf)
{
  init(title, workDir, imgPath);
}

void dbgStream::init(string title, string workDir, string imgPath)
{
  // Initialize fileLevel with a 0 to make it possible to count top-level files
  loc.push_back(make_pair(0, list<int>(1, 0)));
  
  this->title       = title;
  this->workDir     = workDir;
  this->imgPath     = imgPath;

  numImages++;
  
  ostringstream anchorScriptFName; anchorScriptFName << workDir << "/html/script/anchor_script";
  try {
    anchorScriptFile.open(anchorScriptFName.str().c_str());
  } catch (ofstream::failure e)
  { cout << "dbgStream::init() ERROR opening file \""<<anchorScriptFName.str()<<"\" for writing!"; exit(-1); }
  
  enterFileLevel(new block(title), true);
  
  initialized = true;
}

dbgStream::~dbgStream()
{
  if (!initialized)
    return;

  block* topB = exitFileLevel(true);
  delete topB;
  
  anchorScriptFile.close();
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
string dbgStream::blockGlobalStr(const location& myLoc) const {
  ostringstream blockStr;
  
  for(list<pair<int, list<int> > >::const_iterator l=myLoc.begin(); l!=myLoc.end(); ) {
    assert(l->second.size()>0);
    list<int>::const_iterator b=l->second.begin();
    blockStr << l->first << "<";
    int lastBlock=*b;
    b++;
    while(b!=l->second.end()) {
      blockStr << lastBlock;
      lastBlock=*b;
      b++;
      if(b!=l->second.end()) blockStr << "_";
    }
    blockStr << ">";
    l++;
    //if(l!=myLoc.end() && l->second.size()>1) blockStr << ":";
  }
  
  return blockStr.str();
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

// Enter a new file level. Return a string that contains the JavaScript command to open this file in the current view.
string dbgStream::enterFileLevel(block* b, bool topLevel)
{
  assert(loc.size()>0);
  // Increment the index of this file unit within the current nesting level
  (loc.back().first)++;
  // Add a fresh file level to the location
  loc.push_back(make_pair(0, list<int>(1, 0)));
  
  if(!topLevel) (*this)<< "dbgStream::enterFileLevel("<<b->getLabel()<<") <<<<<\n";
  
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
  if(!topLevel) (*this)<< "fileID="<<fileID<<" blockID="<<blockID<<endl;
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
  if(!topLevel)
    loadCmd = enterBlock(b, true);//, fileLevelJSIntArray(location)); // detailRelFName.str(), sumRelFName.str());*/
  fileBlocks.push_back(b);
  
  if(!topLevel) (*this)<< "dbgStream::enterFileLevel("<<b->getLabel()<<") >>>>>\n";
  
  // Create the index file, which is a frameset that refers to the detail and summary files
  ofstream indexFile;
  try {
    indexFile.open(indexAbsFName.str().c_str());
  } catch (ofstream::failure e)
  { cout << "dbgStream::init() ERROR opening file \""<<indexAbsFName.str()<<"\" for writing!"; exit(-1); }
  indexFiles.push_back(&indexFile);
  
  indexFile << "<frameset cols=\"20%,80%\">\n";
  indexFile << "\t<frame src=\""<<sumRelFName.str()<<".html\" name=\"summary\"/>\n";
  indexFile << "\t<frame src=\""<<detailRelFName.str() <<".html\" name=\"detail\"/>\n";
  indexFile << "</frameset>\n";
  indexFile.close();
  
  // Create the detail file. It is empty initially and will be filled with text by the user because its dbgBuf
  // object will be set to be the primary buffer of this stream, meaning that all the text written to this
  // stream will flow into the detail file.
  ofstream *dbgFile = new ofstream();
  try {
    dbgFile->open((detailAbsFName.str()+".body").c_str());
  } catch (ofstream::failure e)
  { cout << "dbgStream::init() ERROR opening file \""<<detailAbsFName.str()<<"\" for writing!"; exit(-1); }
  dbgFiles.push_back(dbgFile);
  detailFileRelFNames.push_back(detailRelFName.str()+".body");
  
  dbgBuf *nextBuf = new dbgBuf(dbgFile->rdbuf());
  fileBufs.push_back(nextBuf);
  // Call the parent class initialization function to connect it dbgBuf of the child file
  ostream::init(nextBuf);
  
  // Create the html file container for the detail html text
  printDetailFileContainerHTML(detailAbsFName.str(), b->getLabel());
  
  // Create the summary file. It is initially set to be an empty table and is filled with entries each time
  // a region is opened inside the detail file.
  {
    ofstream *summaryFile = new ofstream();
    try {
      //cout << "    sumAbsFName="<<fullFileName.str()<<endl;
      summaryFile->open((sumAbsFName.str()+".body").c_str());
    } catch (ofstream::failure e)
    { cout << "dbgStream::init() ERROR opening file \""<<sumAbsFName.str()<<"\" for writing!"; exit(-1); }
    summaryFiles.push_back(summaryFile);
    
    // Start the table in the current summary file
    (*summaryFiles.back()) << "\t\t<table width=\"100%\">\n";
    (*summaryFiles.back()) << "\t\t\t<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
    
    // Create the html file container for the summary html text
    printSummaryFileContainerHTML(sumAbsFName.str(), sumRelFName.str(), b->getLabel());
  }
  
  // Create the script file. It is initially set to be an empty <html> tag and filled with entries each time
  // a region is opened inside the detail file.
  {
    ofstream *scriptFile = new ofstream();
    try {
      //cout << "    sumAbsFName="<<fullFileName.str()<<endl;
      scriptFile->open((scriptAbsFName.str()).c_str());
    } catch (ofstream::failure e)
    { cout << "dbgStream::init() ERROR opening file \""<<sumAbsFName.str()<<"\" for writing!"; exit(-1); }
    scriptFiles.push_back(scriptFile);
    
    // The script file starts out with a command to record that the file was loaded
    (*scriptFiles.back()) << "\trecordFile("<<fileLevelJSIntArray(loc)<<", 'loaded', 1);\n";
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

  indexFiles.pop_back();
  dbgFiles.pop_back();
  detailFileRelFNames.pop_back();
  summaryFiles.pop_back();
  scriptFiles.pop_back();
  fileBufs.pop_back();
  // Call the ostream class initialization function to connect it dbgBuf of the parent detail file
  ostream::init(fileBufs.back());
  
  // Exit the function level within the parent file
  //assert(b->getLabel() == fileBlocks.back());
  if(!topLevel) {
    //block* topBlock = exitBlock();
    exitBlock();
    //delete topBlock;
  }
  
  loc.pop_back();
  
  block* lastB = fileBlocks.back();
  fileBlocks.pop_back();
  return lastB;
}

// Record the mapping from the given anchor ID to the given string in the global script file
void dbgStream::writeToAnchorScript(int anchorID, const location& myLoc, string blockID) {
  anchorScriptFile << "anchors.setItem("<<anchorID<<", new anchor("<<fileLevelJSIntArray(myLoc)<<", \""<<blockID<<"\"));"<<endl;
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
  det << "\t<script src=\"script/hashtable.js\"></script>\n";
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
  det << "\t\t\tloadURLIntoDiv(document, 'detail."<<fileID<<".body', 'detailContents', \n";
  det << "\t\t\t\tfunction() { loadjscssfile('script/script."<<fileID<<"', 'js', \n";
  det << "\t\t\t\t\tfunction() { loadjscssfile('script/anchor_script', 'js'); }); });\n";
  det << "\t\t}\n";
  det << "\t</script>\n";
  det << "\t</head>\n";
  det << "\t<body>\n";
  det << "\t<h1>"<<title<<"</h1>\n";
  det << "\t\t<table width=\"100%\">\n";
  det << "\t\t\t<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  det << "\t\t\t<div id='detailContents'></div>\n";
  det << "\t\t\t</td></tr>\n";
  det << "\t\t</table>\n";
  det << "\t</body>\n";
  det << "</html>\n\n";
  
  det.close();
}

string dbgStream::enterBlock(block* b, bool newFileEntered)
{
  (*this) << "dbgStream::enterBlock("<<(b? b->getLabel(): "NULL")<<")"<<endl;
  
  fileBufs.back()->ownerAccessing();
  
  fileBufs.back()->enterBlock(b);
  
  assert(loc.size()>0);
  // Increment the index of this block unit within the current nesting level of this file
  loc.back().second.back()++;
  // Add a new level to the block list, starting the index at 0
  loc.back().second.push_back(0);
  
  ostringstream loadCmd; // The command to open this file in the current view
  string blockID = blockGlobalStr(loc);
  b->setLocation(loc);
  string fileID = fileLevelStr(b->getLocation());
  if(newFileEntered)
    loadCmd << "loadSubFile(top.detail.document, 'detail."<<fileID<<".body', 'div"<<blockID<<"', "<<
               "top.summary.document, 'summary."<<fileID<<".body', 'sumdiv"<<blockID<<"', "<<
               "'script/script."<<fileID<<"'";
  
  (*this) << "blockID="<<blockID<<" loadCmd="<<loadCmd<<endl;
  b->printEntry(loadCmd.str());  

  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth())<<"</td></tr>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<table width=\"100%\" style=\"border:0px\">\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<tr width=\"100%\"><td width=50></td><td id=\"link"<<blockID<<"\" width=\"100%\">";
  *summaryFiles.back() <<     "<a name=\"anchor"<<blockID<<"\" href=\"javascript:focusLinkDetail('"<<blockID<<"')\">"<<b->getLabel()<<"</a> ("<<blockID<<")</td></tr>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  /*// If the corresponding div in the detail page may be filled in with additional content, we create another summary
  // div to fill in with the corresponding summary content
  if(detailContentURL != "") */*summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<div id=\"sumdiv"<<blockID<<"\">\n";
  summaryFiles.back()->flush();
  
  if(newFileEntered) {
    (*scriptFiles.back()) << "\trecordFile("<<fileLevelJSIntArray(loc)<<", 'loadFunc', function(continuationFunc) {"<<loadCmd.str()<<", continuationFunc)});\n";
    scriptFiles.back()->flush();
  }
  
  fileBufs.back()->userAccessing();
  
  return loadCmd.str();
}

block* dbgStream::exitBlock()
{
  //{ cout << "exitBlock("<<b->getLabel()<<", "<<contentURL<<")"<<endl; }
  fileBufs.back()->ownerAccessing();
  block* lastB = fileBufs.back()->exitBlock();
  
  assert(loc.size()>0);
  assert(loc.back().second.size()>0);
  loc.back().second.pop_back();
  
  lastB->printExit();
    
  // We enfore the invariant that the number of open and close angle brackets must be balanced
  // between the entry into and exit from a block
  while(fileBufs.back()->getNumOpenAngles() > 0)
    (*this) << ">";
  
  this->flush();
  fileBufs.back()->userAccessing();

  //summaryFiles.back() << "</ul>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"</div></td></tr>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"</table>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth())<<"</td></tr>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->blockDepth())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  summaryFiles.back()->flush();
  
  return lastB;
}

// Adds an image to the output with the given extension and returns the path of this image
// so that the caller can write to it.
string dbgStream::addImage(string ext)
{
  assert(fileBufs.size()>0);
  ostringstream imgFName; imgFName << imgPath << "/image_" << numImages << "." << ext;
  fileBufs.back()->ownerAccessing();
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->blockDepth()+1)<<"<img src="<<imgFName.str()<<">\n";
  fileBufs.back()->userAccessing();
  return imgFName.str();
}

// Given a reference to an object that can be represented as a dot graph, create an image from it and add it to the output.
// Return the path of the image.
string dbgStream::addDOT(dottable& obj)
{
  ostringstream imgFName; imgFName << "dbg_imgs/image_" << numImages << ".svg";
  ostringstream graphName; graphName << "graph_"<<numImages;
  addDOT(imgFName.str(), graphName.str(), obj.toDOT(graphName.str()), *this);
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

// Given a reference to an object that can be represented as a dot graph, create an image of it and return the string
// that must be added to the output to include this image.
std::string dbgStream::addDOTStr(dottable& obj)
{
  ostringstream imgFName; imgFName << "dbg_imgs/image_" << numImages << ".svg";
  ostringstream graphName; graphName << "graph_"<<numImages;
  ostringstream ret;
  addDOT(imgFName.str(), graphName.str(), obj.toDOT(graphName.str()), ret);

  return ret.str();
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
std::string dbgStream::addDOT(string dot)
{
  ostringstream imgFName; imgFName << "dbg_imgs/image_" << numImages << ".svg";
  ostringstream graphName; graphName << "graph_"<<numImages;

  addDOT(imgFName.str(), graphName.str(), dot, *this);

  return imgFName.str();
}

// The common work code for all the addDOT methods
void dbgStream::addDOT(string imgFName, string graphName, string dot, ostream& ret)
{
  #ifdef DOT_PATH
  ostringstream dotFName; dotFName << imgPath << "/image_" << numImages << ".dot";
  ostringstream mapFName; mapFName << imgPath<<"/image_" << numImages << ".map";

  ofstream dotFile;
  dotFile.open(dotFName.str().c_str());
  dotFile << dot;
  dotFile.close();

  // Create the SVG file's picture of the dot file
  ostringstream cmd; cmd << DOT_PATH << "dot -Tsvg -o"<<imgPath<<"/image_" << numImages << ".svg "<<dotFName.str() << "&"; 
  //cout << "Command \""<<cmd.str()<<"\"\n";
  system(cmd.str().c_str());

  ret << "<img src=\""<<imgFName<<"\"><br>\n";
  
  numImages++;
  #else
  scope dotScope("Warning:", scope::medium);
  *this << "graphviz not available" << endl;
  #endif  
}

/******************
 ***** indent *****
 ******************/

indent::indent(std::string space, int curDebugLevel, int targetDebugLevel) {
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  
  if(curDebugLevel >= targetDebugLevel) {
    active = true;
    //cout << "Entering indent space=\""<<space<<"\""<<endl;
    dbg.addIndent(space);
  }
  else
    active = false;
}
/*indent::indent() {
  std::string space = "&nbsp;&nbsp;&nbsp;&nbsp;";
  active = true;
  //dbg << "Entering indent space=\""<<space<<"\""<<endl;
  dbg.addIndent(space);
}* /
indent::indent(std::string space) {
  active = true;
  //dbg << "Entering indent space=\""<<space<<"\""<<endl;
  dbg.addIndent(space);
}*/
indent::~indent() {
  if(active) {
    //cout << "Exiting indent"<<std::endl;
    dbg.remIndent();
  }
}

/******************
 ***** anchor *****
 ******************/
int anchor::maxAnchorID=0;
anchor anchor::noAnchor(false, -1);

anchor::anchor(/*dbgStream& myDbg, */bool reached) /*: myDbg(myDbg)*/ {
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  
  anchorID = maxAnchorID++;
  //dbg << "anchor="<<anchorID<<endl;
  // Initialize this->reached to false so that reachedAnchor() doesn't think we're calling it for multiple locations
  this->reached = false;
  if(reached) reachedAnchor();
}

anchor::anchor(const anchor& that) {
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  
  anchorID = that.anchorID;
  //myDbg    = that.myDbg;
  loc      = that.loc;
  blockID  = that.blockID;
  reached  = that.reached;
}

anchor::anchor(/*dbgStream& myDbg, */bool reached, int anchorID) : 
  reached(reached), anchorID(anchorID)
{ 
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");  
}

void anchor::init(bool reached) {
  anchorID = maxAnchorID++;
  // Initialize this->reached to false so that reachedAnchor() doesn't think we're calling it for multiple locations
  this->reached = false;
  if(reached) reachedAnchor();
}

void anchor::operator=(const anchor& that) {
  anchorID = that.anchorID;
  //myDbg    = that.myDbg;
  loc      = that.loc;
  blockID  = that.blockID;
  reached  = that.reached;
}

bool anchor::operator==(const anchor& that) const {
  // anchorID of -1 indicates that this is the noAnchor object and any copies of it are equivalent
  assert(anchorID>=-1); assert(that.anchorID>=-1);
  if(anchorID==-1 && that.anchorID==-1) return true;
   
  return anchorID == that.anchorID &&
         loc      == that.loc &&
         blockID  == that.blockID &&
         reached  == that.reached;
}
bool anchor::operator!=(const anchor& that) const
{ return !(*this == that); }

// Called when the file location of this anchor has been reached
void anchor::reachedAnchor() {
  // If this anchor has already been set to point to its target location, emit a warning
  if(reached) 
    cerr << "Warning: anchor "<<anchorID<<" is being set to multiple target locations" << endl;
  else {
    reached = true; // We've now reached this anchor's location in the output
    loc   = dbg.getLocation();
    blockID = dbg.blockGlobalStr(loc);
    
    //dbg << "anchor="<<anchorID<<" reached. loc="<<dbg.fileLevelJSIntArray(loc)<<", blockID="<<blockID<<endl;
    dbg.writeToAnchorScript(anchorID, loc, blockID);
  }
}

// Emits to the output dbgStream the string that denotes a link to this anchor
// Embed the given text in the link.
void anchor::link(string text) const {
  dbg << "<a href=\"javascript:";
  // If we've already reached this link's location (this is a backward link)
  if(reached) {
    dbg << "goToAnchor([], "<<dbg.fileLevelJSIntArray(loc)<<",  ";
    dbg << "function() { focusLinkDetail('"<<blockID<<"'); focusLinkSummary('"<<blockID<<"');});";
  // If we have not yet reached this anchor's location in the output (it is a forward link)
  } else {
    dbg << "if(anchors.hasItem("<<anchorID<<")) { goToAnchor([], anchors.getItem("<<anchorID<<").fileID, "<<
                    "function() { focusLinkDetail(anchors.getItem("<<anchorID<<").blockID); focusLinkSummary(anchors.getItem("<<anchorID<<").blockID); }); } ";
    dbg << "else { focusLinkDetail(anchors.getItem("<<anchorID<<").blockID); focusLinkSummary(anchors.getItem("<<anchorID<<").blockID); } ";
  }
  dbg << "\">"<<text<<"</a>";
}

std::string anchor::str() const {
  ostringstream oss;
  if(anchorID==-1)
    oss << "[noAnchor]";
  else {
    oss << "[anchor: ID="<<anchorID<<", loc=&lt;"<<dbg.blockGlobalStr(loc)<<"&gt;, reached=\""<<reached<<"\"]";
  }
  return oss.str();
}

/*****************
 ***** scope *****
 *****************/
scope::scope(std::string label, scopeLevel level, int curDebugLevel, int targetDebugLevel) : 
  block(label),
  // startA is initialized before init is called to ensure that the anchor's blockID is the same as 
  // the scope's blockID, rather than a blockID inside the scope.
  startA(true) 
{
  init(level, curDebugLevel, targetDebugLevel);
}

scope::scope(std::string label, anchor& pointsTo, scopeLevel level, int curDebugLevel, int targetDebugLevel): 
  block(label),
  startA(anchor::noAnchor) // initialize startA to be noAnchor
{
  init(level, curDebugLevel, targetDebugLevel);
  // If we're given an anchor from a forward link to this region,
  // inform the anchor that it is pointing to the location of this scope's start.
  // This is done before the initialization to ensure that the anchor's blockID is 
  // the same as the scope's blockID, rather than a blockID inside the scope.
  if(pointsTo != anchor::noAnchor) {
    startA = pointsTo;
    pointsTo.reachedAnchor();
  // Else, if there are no forward links to this node, initialize startA to be
  // a fresh anchor that refers to the start of this scope
  } else {
    startA.init(true);
  }
    
  
  //dbg << "scope: pointsTo="<<pointsTo.str()<<endl;
}

std::vector<std::string> scope::colors;
int scope::colorIdx=0; // The current index into the list of colors 

// Common initialization code
void scope::init(scopeLevel level, int curDebugLevel, int targetDebugLevel)
{
  // If the colors list has not yet been initialized, do so now
  if(colors.size() == 0) {
    // Initialize colors with a list of light pastel colors 
    colors.push_back("FF97E8");
    colors.push_back("75D6FF");
    colors.push_back("72FE95");
    colors.push_back("8C8CFF");
    colors.push_back("57BCD9");
    colors.push_back("99FD77");
    colors.push_back("EDEF85");
    colors.push_back("B4D1B6");
    colors.push_back("FF86FF");
    colors.push_back("4985D6");
    colors.push_back("D0BCFE");
    colors.push_back("FFA8A8");
    colors.push_back("A4F0B7");
    colors.push_back("F9FDFF");
    colors.push_back("FFFFC8");
    colors.push_back("5757FF");
    colors.push_back("6FFF44");
  }
  
  if(curDebugLevel >= targetDebugLevel) {
    active = true;
    this->level = level;
    // If this block corresponds to a new file, this string will be set to the Javascript command to 
    // load this file into the current view
    string loadCmd="";
    if(level == high) {
      colorIdx++; // Advance to a new color for this func
      loadCmd = dbg.enterFileLevel(this);
    } else if(level == medium) {
      colorIdx++; // Advance to a new color for this func
      dbg.enterBlock(this, false);
    }
    else if(level == low)
      dbg.enterBlock(this, false);
  }
  else
    active = false;  
}

anchor scope::getAnchor() const
{ return startA; }

scope::~scope()
{ 
  if(active) {
    if(level == high) {
      dbg.exitFileLevel();
      colorIdx--; // Return to the last color for this func's parent
    }
    else if(level == medium) {
      dbg.exitBlock();
      colorIdx--; // Return to the last color for this func's parent
    } else if(level == low)
      dbg.exitBlock();
    assert(colorIdx>=0);
  }
}

// Called to enable the block to print its entry and exit text
void scope::printEntry(string loadCmd) {
  dbg.ownerAccessing();
  dbg << "blockID="<<getBlockID()<<endl;
  if(dbg.blockIndex()==0) dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"</td></tr>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<table bgcolor=\"#"<<colors[(colorIdx-1)%colors.size()]<<"\" width=\"100%\" id=\"table"<<getBlockID()<<"\" style=\"border:1px solid white\" onmouseover=\"this.style.border='1px solid black'; highlightLink('"<<getBlockID()<<"', '#F4FBAA');\" onmouseout=\"this.style.border='1px solid white'; highlightLink('"<<getBlockID()<<"', '#FFFFFF');\" onclick=\"focusLinkSummary('"<<getBlockID()<<"', event);\">\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\"><h2>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<a name=\"anchor"<<getBlockID()<<"\" href=\"javascript:unhide('"<<getBlockID()<<"');\">";
  dbg.userAccessing();
  dbg << getLabel();
  dbg.ownerAccessing();
  dbg << "</a>\n";
  
  if(loadCmd != "") {
    dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1);
    dbg << "<a href=\"javascript:"<<loadCmd<<")\">";
    //dbg << "<a href=\"javascript:loadURLIntoDiv(top.detail.document, '"<<detailContentURL<<".body', 'div"<<getBlockID()<<"'); loadURLIntoDiv(top.summary.document, '"<<summaryContentURL<<".body', 'sumdiv"<<getBlockID()<<"')\">";
    dbg << "<img src=\"img/divDL.gif\" width=25 height=35></a>\n";
    dbg << "\t\t\t<a target=\"_top\" href=\"index."<<getFileID()<<".html\">";
    dbg << "<img src=\"img/divGO.gif\" width=35 height=25></a>\n";
  }
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</h2></td></tr>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<div id=\"div"<<getBlockID()<<"\" class=\"unhidden\">\n";
  dbg.flush();
  dbg.userAccessing();  
}

void scope::printExit() {
 // Close this scope
  dbg.ownerAccessing();
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</td></tr>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</table>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"</td></tr>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  dbg.userAccessing();  
}

/* // Indicates that the application has entered or exited a file
void enterFile(std::string funcName)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  dbg.enterFileLevel(funcName);
}

void exitFile(std::string funcName)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  dbg.exitFileLevel(funcName);
}
*/

// Indicates that the application has entered or exited a function
/*void enterBlock(block* b)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  dbg.enterBlock(b);
}

block* exitBlock()
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  return dbg.exitBlock(b);
}*/

// Adds an image to the output with the given extension and returns the path of this image
// so that the caller can write to it.
std::string addImage(std::string ext)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  return dbg.addImage(ext);
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
std::string addDOT(dottable& obj)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  return dbg.addDOT(obj);
}

// Given a representation of a graph in dot format, create an image of it and return the string
// that must be added to the output to include this image.
std::string addDOTStr(dottable& obj)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  return dbg.addDOTStr(obj);
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
std::string addDOT(std::string dot) {
  if(!initializedDebug) initializeDebug("Debug Output", "dbg");
  return dbg.addDOT(dot);
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
    else                 out += s[i];
  }
  return out;
}

} // namespace dbglog
