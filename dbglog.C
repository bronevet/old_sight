#include "dbglog.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

//#define ROOT_PATH "/cygdrive/c/work/code/dbglog"

namespace dbglog {

bool initializedDebug=false;

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
  //numDivs = 0;
  parentDivs.empty();
  parentDivs.push_back(0);
  //justSynched = false;
  needIndent = false;
  indents.push_back(std::list<std::string>());
  //cout << "Initially parentDivs (len="<<parentDivs.size()<<")\n";
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
  //cout << "remIndent() #indents="<<indents.size()<<endl;
  assert(indents.size()>0);
  assert(indents.rbegin()->size()>0);
  indents.rbegin()->pop_back();
}

streamsize dbgBuf::xsputn(const char * s, streamsize n)
{
  /*cout << "dbgBuf::xsputn"<<endl;
  cout << "  funcs.size()="<<funcs.size()<<", needIndent="<<needIndent<<endl;
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

void dbgBuf::enterScope(string funcName)
{
  //cout << "enterScope("<<funcName<<") numOpenAngles="<<numOpenAngles<<endl;
  funcs.push_back(funcName);
  
  //indents.push_back(indent);
  // Increment the index of this function within its parent
  //cout << "Incrementing parentDivs (len="<<parentDivs.size()<<") from "<<*(parentDivs.rbegin())<<" to ";
  (*(parentDivs.rbegin()))++;
  /*cout << *(parentDivs.rbegin())<<": div=";
  ostringstream divName;
  for(list<int>::iterator d=parentDivs.begin(); d!=parentDivs.end(); d++) {
    if(d!=parentDivs.end())
      divName << *d << "_";
  }
  cout << divName.str()<<"\n";*/
  // Add a new level to the parentDivs list, starting the index at 0
  parentDivs.push_back(0);
  //numDivs++;

  indents.push_back(std::list<std::string>());
}

void dbgBuf::exitScope(string funcName)
{
  //cout << "exitScope("<<funcName<<") numOpenAngles="<<numOpenAngles<<endl;
  if(funcName != funcs.back()) { 
    cout << "dbgStream::exitScope() ERROR: exiting from function "<<funcName<<" which is not the most recent function entered!\n";
    cout << "funcs=\n";
    for(list<string>::iterator f=funcs.begin(); f!=funcs.end(); f++)
      cout << "    "<<*f<<"\n";
    cout.flush();
    baseBuf->pubsync();
    exit(-1);
  }
  funcs.pop_back();
  //indents.pop_back();
  parentDivs.pop_back();
  indents.pop_back();
  needIndent=false;
}

// Returns the depth of enterScope calls that have not yet been matched by exitScopeCalls
int dbgBuf::funcDepth()
{ return funcs.size(); }

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
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
}

dbgStream::dbgStream(string title, string dbgFileName, string workDir, string imgPath)
  : std::ostream(&defaultFileBuf)
{
  init(title, dbgFileName, workDir, imgPath);
}

void dbgStream::init(string title, string dbgFileName, string workDir, string imgPath)
{
  // Initialize fileLevel with a 0 to make it possible to count top-level files
  fileLevel.push_back(0);
  
  this->title       = title;
  this->dbgFileName = dbgFileName;
  this->workDir     = workDir;
  this->imgPath     = imgPath;

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

  numImages++;
  
  enterFileLevel(title, true);
  
  initialized = true;
}

dbgStream::~dbgStream()
{
  if (!initialized)
    return;

  exitFileLevel(title, true);
}

// Returns the unique ID string of the current file within the hierarchy of files
string dbgStream::fileLevelStr(const std::list<int>& myFileLevel) const
/*{
  ostringstream oss;
  for(list<int>::const_iterator i=fileLevel.begin(); i!=fileLevel.end(); ) { 
    oss << *i;
    i++;
    if(i!=fileLevel.end()) oss << "-";
  }
  return oss.str();
}*/

{
  if(myFileLevel.size()<=1) return "";

  ostringstream oss;
  list<int>::const_iterator i=myFileLevel.begin(); 
  int lastFileID = *i;
  i++;
  while(i!=myFileLevel.end()) { 
    oss << *i;
    lastFileID = *i;
    i++;
    if(i!=myFileLevel.end()) oss << "-";
  }
  return oss.str();
}

const list<int>& dbgStream::getFileLevel() const
{ return fileLevel; }

// Returns a string that encodes a Javascript array of integers that together form the unique ID string of 
// the current file within the hierarchy of files
string dbgStream::fileLevelJSIntArray(const std::list<int>& myFileLevel) const
{
  ostringstream oss;
  oss << "[";
  list<int>::const_iterator i=myFileLevel.begin(); 
  int lastFileID = *i;
  i++;
  while(i!=myFileLevel.end()) { 
    oss << lastFileID;
    lastFileID = *i;
    i++;
    if(i!=myFileLevel.end()) oss << ", ";
  }
  oss << "]";
  return oss.str();
}

// Returns the unique ID string of the current region, including all the files that it may be contained in
string dbgStream::regionGlobalStr() const {
  ostringstream divName;
  
  for(std::list<dbgBuf*>::const_iterator fb=fileBufs.begin(); fb!=fileBufs.end(); ) {
    assert((*fb)->parentDivs.size()>0);
    list<int>::const_iterator d=(*fb)->parentDivs.begin();
    int lastDivID=*d;
    d++;
    while(d!=(*fb)->parentDivs.end()) {
      divName << lastDivID;
      lastDivID=*d;
      d++;
      if(d!=(*fb)->parentDivs.end()) divName << "_";
    }
    fb++;
    if(fb!=fileBufs.end()) divName << ":";
  }
  
  return divName.str();
}

// Enter a new file level
void dbgStream::enterFileLevel(string flTitle, bool topLevel)
{
  assert(fileLevel.size()>0);
  // Increment the index of this file unit within the current nesting level
  (*fileLevel.rbegin())++;
  // Add a fresh level to the fileLevel list
  fileLevel.push_back(0);
  
  // Each file unit consists of three files: 
  //    - the detail file that contains all the text output and regions inside the file,
  //    - the summary file that lists these regions
  //    - the script file that contains all the code that needs to be run as the regions in the detail file 
  //      are loaded (substitute for having an explicit onload callback for each region's div)
  //    - the index file that contains both of these
  // The detail and summary files can be viewed on their own via the index file or can be loaded into higher-level 
  // detail and summary files, where they appear like regular regions.
  // These are the absolute and relative names of these files.
  string fileID = fileLevelStr(fileLevel);
  ostringstream indexAbsFName;   indexAbsFName  << dbgFileName << "."     << fileID << ".html";
  ostringstream detailAbsFName;  detailAbsFName << workDir << "/detail."  << fileID;
  ostringstream detailRelFName;  detailRelFName << "detail."  << fileID;
  ostringstream sumAbsFName;     sumAbsFName    << workDir << "/summary." << fileID;
  ostringstream sumRelFName;     sumRelFName    << "summary."  << fileID;
  ostringstream scriptAbsFName;  scriptAbsFName << workDir << "/script." << fileID;
  ostringstream scriptRelFName;  scriptRelFName << "script."  << fileID;
  
  //cout << "enterFileLevel("<<flTitle<<") topLevel="<<topLevel<<" #flTitles="<<flTitles.size()<<" #fileLevel="<<fileLevel.size()<<endl;
  // Add a new function level within the parent file unit that will refer to the child file unit
  if(!topLevel) enterScope(flTitle, true, fileID, fileLevelJSIntArray(fileLevel)); // detailRelFName.str(), sumRelFName.str());
  flTitles.push_back(flTitle);
  
  // Create the index file, which is a frameset that refers to the detail and summary files
  ofstream indexFile;
  try {
    indexFile.open(indexAbsFName.str().c_str());
  } catch (ofstream::failure e)
  { cout << "dbgStream::init() ERROR opening file \""<<indexAbsFName<<"\" for writing!"; exit(-1); }
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
  printDetailFileContainerHTML(detailAbsFName.str(), fileLevelStr(fileLevel), flTitle);
  
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
    printSummaryFileContainerHTML(sumAbsFName.str(), sumRelFName.str(), flTitle);
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
    (*scriptFiles.back()) << "\trecordFile("<<fileLevelJSIntArray(fileLevel)<<", 'loaded', 1);\n";
  }
}

// Exit a current file level
void dbgStream::exitFileLevel(string flTitle, bool topLevel)
{
  //cout << "exitFileLevel("<<flTitle<<") topLevel="<<topLevel<<" #flTitles="<<flTitles.size()<<" #fileLevel="<<fileLevel.size()<<endl;
  assert(fileLevel.size()>1);
  
  dbgFiles.back()->close();
  
  // Complete the table in the current summary file
  (*summaryFiles.back()) << "\t\t\t</td></tr>\n";
  (*summaryFiles.back()) << "\t\t</table>\n";
  summaryFiles.back()->close();
  
  // Complete the current script file
  scriptFiles.back()->close();
  
  fileLevel.pop_back();
  indexFiles.pop_back();
  dbgFiles.pop_back();
  detailFileRelFNames.pop_back();
  summaryFiles.pop_back();
  scriptFiles.pop_back();
  fileBufs.pop_back();
  // Call the ostream class initialization function to connect it dbgBuf of the parent detail file
  ostream::init(fileBufs.back());
  
  // Exit the function level within the parent file
  assert(flTitle == flTitles.back());
  if(!topLevel) exitScope(flTitle);
  
  flTitles.pop_back();
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
  sum << "\tfunction focusLinkHref(divID) {\n";
  sum << "\t\ttop.detail.location = \"detail." << fileLevelStr(fileLevel) << ".html#anchor\"+divID;\n";
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

void dbgStream::printDetailFileContainerHTML(string absoluteFileName, string fileID, string title)
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
  det << "\t\t\tloadURLIntoDiv(document, 'detail."<<fileID<<".body', 'detailContents');\n";
  det << "\t\t\tloadjscssfile('script."<<fileID<<"', 'js')\n";
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

void dbgStream::enterScope(string funcName, bool advanceColor, string fileID, string fileIDJSArray)//string detailContentURL, string summaryContentURL)
{
  /* // Either both URLs are provided or neither is
  assert((detailContentURL=="" && summaryContentURL=="") ||
         (detailContentURL!="" && summaryContentURL!=""));*/
  
  fileBufs.back()->ownerAccessing();
  
  fileBufs.back()->enterScope(funcName);
  if(advanceColor)
    colorIdx++; // Advance to a new color for this func
  
  // Create the name of the new div that will hold the contents of the new scope.
  // This name is a concatenation of the indexes of all of its parent divs.
  /*ostringstream divName;
  for(std::list<dbgBuf*>::iterator fb=fileBufs.begin(); fb!=fileBufs.end(); ) {
    assert((*fb)->parentDivs.size()>0);
    list<int>::iterator d=(*fb)->parentDivs.begin();
    int lastDivID=*d;
    d++;
    while(d!=(*fb)->parentDivs.end()) {
      divName << lastDivID;
      lastDivID=*d;
      d++;
      if(d!=(*fb)->parentDivs.end()) divName << "_";
    }
    fb++;
    if(fb!=fileBufs.end()) divName << ":";
  }*/
  string divName = regionGlobalStr();
  
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size())<<"</td></tr>\n";
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"<table bgcolor=\"#"<<colors[(colorIdx-1)%colors.size()]<<"\" width=\"100%\" id=\"table"<<divName<<"\" style=\"border:1px solid white\" onmouseover=\"this.style.border='1px solid black'; highlightLink('"<<divName<<"', '#F4FBAA');\" onmouseout=\"this.style.border='1px solid white'; highlightLink('"<<divName<<"', '#FFFFFF');\" onclick=\"focusLink('"<<divName<<"', event);\">\n";
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\"><h2><a name=\"anchor"<<divName<<"\" href=\"javascript:unhide('"<<divName<<"');\">";
  fileBufs.back()->userAccessing();
  *(this) << funcName;
  fileBufs.back()->ownerAccessing();
  *(this) << "</a>\n";
  ostringstream loadCmd;
  if(fileID != "") {
    *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1);
    loadCmd << "loadSubFile(top.detail.document, 'detail."<<fileID<<".body', 'div"<<divName<<"', "<<
                           "top.summary.document, 'summary."<<fileID<<".body', 'sumdiv"<<divName<<"', "<<
                           "'script."<<fileID<<"'";
    *(this) << "<a href=\"javascript:"<<loadCmd.str()<<")\">";
    //*(this) << "<a href=\"javascript:loadURLIntoDiv(top.detail.document, '"<<detailContentURL<<".body', 'div"<<divName<<"'); loadURLIntoDiv(top.summary.document, '"<<summaryContentURL<<".body', 'sumdiv"<<divName<<"')\">";
    *(this) << "<img src=\"img/divDL.gif\" width=25 height=35></a>\n";
    *(this) << "\t\t\t<a target=\"_top\" href=\"index."<<fileLevelStr(fileLevel)<<".html\">";
    *(this) << "<img src=\"img/divGO.gif\" width=35 height=25></a>\n";
  }
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"</h2></td></tr>\n";
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\"><div id=\"div"<<divName<<"\" class=\"unhidden\">\n";
  this->flush();

  //summaryFiles.back() << "<li><a target=\"detail\" href=\"detail.html#"<<divName<<"\">"<<funcName<<"</a><br>\n";
  //summaryFiles.back() << "<ul>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size())<<"</td></tr>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"<table width=\"100%\" style=\"border:0px\">\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"<tr width=\"100%\"><td width=50></td><td id=\"link"<<divName<<"\" width=\"100%\">";
  *summaryFiles.back() <<     "<a name=\"anchor"<<divName<<"\" href=\"javascript:focusLinkHref('"<<divName<<"')\">"<<funcName<<"</a> ("<<divName<<")</td></tr>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  /*// If the corresponding div in the detail page may be filled in with additional content, we create another summary
  // div to fill in with the corresponding summary content
  if(detailContentURL != "") */*summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"<div id=\"sumdiv"<<divName<<"\">\n";
  summaryFiles.back()->flush();
  
  if(fileID != "") {
    (*scriptFiles.back()) << "\trecordFile("<<fileIDJSArray<<", 'loadFunc', function(continuationFunc) {"<<loadCmd.str()<<", continuationFunc)});\n";
    scriptFiles.back()->flush();
  }
  
  fileBufs.back()->userAccessing();
}

void dbgStream::exitScope(string funcName, bool advanceColor)
{
  //{ cout << "exitScope("<<funcName<<", "<<contentURL<<")"<<endl; }
  fileBufs.back()->ownerAccessing();
  fileBufs.back()->exitScope(funcName);
  if(advanceColor)
    colorIdx--; // Return to the last color for this func's parent
  assert(colorIdx>=0);
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"</td></tr>\n";
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"</table>\n";
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size())<<"</td></tr>\n";
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  this->flush();
  fileBufs.back()->userAccessing();

  //summaryFiles.back() << "</ul>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"</div></td></tr>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"</table>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size())<<"</td></tr>\n";
  *summaryFiles.back() << "\t\t\t"<<tabs(fileBufs.back()->funcs.size())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  summaryFiles.back()->flush();
}

// Adds an image to the output with the given extension and returns the path of this image
// so that the caller can write to it.
string dbgStream::addImage(string ext)
{
  assert(fileBufs.size()>0);
  ostringstream imgFName; imgFName << imgPath << "/image_" << numImages << "." << ext;
  fileBufs.back()->ownerAccessing();
  *(this) << "\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"<img src="<<imgFName.str()<<">\n";
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

  ret << /*"\t\t\t"<<tabs(fileBufs.back()->funcs.size()+1)<<"image_"<<numImages<<":"*/"<img src=\""<<imgFName<<"\" "; 
  ret << "><br>\n";
  
  numImages++;
  #else
  scope dotScope("Warning:", scope::medium);\
  *this << "graphviz not available" << endl;
  #endif  
}

// Creates a link to a given scope, which may be in another file
string dbgStream::linkTo(const scopeID& id, string text) const
{
  // Initialize fileIDPrefix to be the common prefix between fileLevel and id.fileLevel
  list<int> fileIDPrefix;
  //cout << "fileLevel = "<<fileLevelJSIntArray(fileLevel)<<endl;
  //cout << "id.fileLevel = "<<fileLevelJSIntArray(id.fileLevel)<<endl;
  list<int>::const_iterator iThis = fileLevel.begin(),
                            iThat = id.fileLevel.begin();
  while(iThis!=fileLevel.end() & iThat!=id.fileLevel.end() &&
        *iThis == *iThat)
  { 
    //cout << "pushing "<<*iThis<<endl;
    fileIDPrefix.push_back(*iThis);
    iThis++, iThat++;
  }
  /*if(iThat!=id.fileLevel.end()) {
    cout << "pushing "<<*iThat<<endl;
    fileIDPrefix.push_back(*iThat);
  }*/
  
  ostringstream ret;
  ret << "<a href=\"javascript:";
  // The command iteratively opens more and more of the target files, beyond the file that is 
  // shared by the current scope and the target scope
  string suffix;
  while(fileIDPrefix.size() < id.fileLevel.size()-1) {
    fileIDPrefix.push_back(*iThat);
    list<int> curID = fileIDPrefix;
    curID.push_back(1);
    //cout << "curID = "<<fileLevelJSIntArray(curID)<<endl;
    
    // The command is a sequence of calls that use getFile to obtain a function pointer to the function that loads a given 
    // file at a given nesting level and then calls this function. Since each step takes time, subsequent steps are 
    // passed in as continuations so that they're only called when the prior steps have completed (requires callbacks from
    // the browser). This ensures that we only attempt to load a file's children once's its text and scripts have been loaded,
    // registering code to load its children in the fileInfo hash.
    if(fileIDPrefix.size()>2) ret << "function() { ";
    ret << "var next = ";
    if(fileIDPrefix.size() == id.fileLevel.size()-1)
      ret << "undefined;";
    
    suffix = string("")+
             "console.debug('Loaded "+fileLevelJSIntArray(curID)+"='+getFile("+fileLevelJSIntArray(curID)+", 'loaded')); "+
             "if(!getFile("+fileLevelJSIntArray(curID)+", 'loaded')) { console.debug('Loading "+fileLevelJSIntArray(curID)+"'); getFile("+fileLevelJSIntArray(curID)+", 'loadFunc')(next); } "+
             "else if(typeof next !== 'undefined') { next(); } undefined; " + 
             (fileIDPrefix.size()>2? "};": "") +
             suffix;
    //suffix << ")";
    iThat++;
  }
    
  ret << suffix << ";\">";
  ret << text;
  ret << "</a>";
  return ret.str();
}


dbgStream dbg;

/******************
 ***** indent *****
 ******************/

indent::indent(std::string space, int curDebugLevel, int targetDebugLevel) {
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

/*****************
 ***** scope *****
 *****************/
scope::scope(std::string label, scopeLevel level, int curDebugLevel, int targetDebugLevel)
{
  if(curDebugLevel >= targetDebugLevel) {
    active = true;
    this->label = label;
    this->level = level;
    if(level == high)
      enterFile(label);
    else if(level == medium)
      enterScope(label, true);
    else if(level == low)
      enterScope(label, false);
  }
  else
    active = false;  
}

scope::~scope()
{
  if(active) {
    if(level == high)
      exitFile(label);
    else if(level == medium)
      exitScope(label, true);
    else if(level == low)
      exitScope(label, false);
  }
}

// Initializes the debug sub-system
void initializeDebug(string title, string workDir, string fName)
{
  if(initializedDebug) return;
  
  {
    ostringstream cmd; cmd << "mkdir -p "<<workDir;
    int ret = system(cmd.str().c_str());
    if(ret == -1) { cout << "Dbg::init() ERROR creating directory \""<<workDir<<"\"!"; exit(-1); }
  }
  
  // Directory where client-generated images will go
  ostringstream imgPath; imgPath << workDir << "/dbg_imgs";
  {
    ostringstream cmd; cmd << "mkdir -p "<<imgPath.str();
    //cout << "Command \""<<cmd.str()<<"\"\n";
    int ret = system(cmd.str().c_str());
    if(ret == -1) { cout << "Dbg::init() ERROR creating directory \""<<imgPath.str()<<"\"!"; exit(-1); }
  }
  
  // Copy the default images directory to the work directory
  {
    ostringstream defImgPath; defImgPath << workDir << "/img";
    
    {
      ostringstream cmd; cmd << "cp -fr "<<ROOT_PATH<<"/img "<<defImgPath.str();
      //cout << "Command \""<<cmd.str()<<"\"\n";
      int ret = system(cmd.str().c_str());
      if(ret == -1) { cout << "Dbg::init() ERROR copying files from directory \""<<ROOT_PATH<<"/img\" directory \""<<defImgPath.str()<<"\"!"; exit(-1); }
    }
  }

  // Copy the default scripts directory to the work directory
  {
    ostringstream defImgPath; defImgPath << workDir << "/script";
    
    {
      ostringstream cmd; cmd << "cp -fr "<<ROOT_PATH<<"/script "<<defImgPath.str();
      //cout << "Command \""<<cmd.str()<<"\"\n";
      int ret = system(cmd.str().c_str());
      if(ret == -1) { cout << "Dbg::init() ERROR copying files from directory \""<<ROOT_PATH<<"/script\" directory \""<<defImgPath.str()<<"\"!"; exit(-1); }
    }
  }

  ostringstream dbgFileName; dbgFileName << workDir << "/" << fName;
  dbg.init(title, dbgFileName.str(), workDir, imgPath.str());
  initializedDebug = true;
}

// Indicates that the application has entered or exited a file
void enterFile(std::string funcName)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
  dbg.enterFileLevel(funcName);
}

void exitFile(std::string funcName)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
  dbg.exitFileLevel(funcName);
}


// Indicates that the application has entered or exited a function
void enterScope(std::string funcName, bool advanceColor)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
  dbg.enterScope(funcName, advanceColor);
}

void exitScope(std::string funcName, bool advanceColor)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
  dbg.exitScope(funcName, advanceColor);
}

// Adds an image to the output with the given extension and returns the path of this image
// so that the caller can write to it.
std::string addImage(std::string ext)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
  return dbg.addImage(ext);
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
std::string addDOT(dottable& obj)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
  return dbg.addDOT(obj);
}

// Given a representation of a graph in dot format, create an image of it and return the string
// that must be added to the output to include this image.
std::string addDOTStr(dottable& obj)
{
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
  return dbg.addDOTStr(obj);
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
std::string addDOT(std::string dot) {
  if(!initializedDebug) initializeDebug("Debug Output", "dbg", "index");
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
