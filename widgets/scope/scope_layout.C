// Licence information included in file LICENCE
#include "../../sight_layout.h"
#include "../../sight_common.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

using namespace std;
using namespace sight::common;

namespace sight {
namespace layout {

// Record the layout handlers in this file
void* scopeEnterHandler(properties::iterator props) { return new scope(props); }
void  scopeExitHandler(void* obj) { scope* s = static_cast<scope*>(obj); delete s; }
  
scopeLayoutHandlerInstantiator::scopeLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["scope"] = &scopeEnterHandler;
  (*layoutExitHandlers) ["scope"] = &scopeExitHandler;
}
scopeLayoutHandlerInstantiator scopeLayoutHandlerInstance;

std::vector<std::string> scope::colors;
int scope::colorIdx=0; // The current index into the list of colors 

scope::scope(properties::iterator props) : block(properties::next(props))
{
  level2Config((scopeLevel)properties::getInt(props, "level"));
  init();
}

scope::scope(std::string label, scopeLevel level) : block(label) {
  level2Config(level);
  init();
}

scope::scope(std::string label, bool ownFile, bool ownColor, bool labelInteractive, bool labelShown, bool summaryEntry) : 
  block(label), 
  ownFile(ownFile), 
  ownColor(ownColor), 
  labelInteractive(labelInteractive), 
  labelShown(labelShown), 
  summaryEntry(summaryEntry)
{
  init();
}


// Sets the scope's configuration flags based on the given level
void scope::level2Config(scopeLevel level) {
  switch(level) { 
    case high:    ownFile=true;  ownColor=true;  labelInteractive=true;  labelShown=true; summaryEntry=true; break;
    case medium:  ownFile=false; ownColor=true;  labelInteractive=true;  labelShown=true; summaryEntry=true; break;
    case low:     ownFile=false; ownColor=false; labelInteractive=true;  labelShown=true; summaryEntry=true; break;
    case minimum: ownFile=false; ownColor=false; labelInteractive=false; labelShown=true; summaryEntry=true; break;
    default: cerr << "Unknown level "<<level<<"!"<<endl; assert(0);
  }
}

// Common initialization code
void scope::init() {
  // If labelInteractive is true, then labelShown must also be true
  assert(!labelInteractive || labelShown);
  
  //cout << "scope::init() anchor="<<getAnchorRef().str()<<endl;
  
  // If the colors list has not yet been initialized, do so now
  if(colors.size() == 0) {
    // Initialize colors with a list of light pastel colors 
    /*colors.push_back("FF97E8");
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
    colors.push_back("6FFF44");*/
    colors.push_back("8DD3C7");
    colors.push_back("FFFFB3");
    colors.push_back("BEBADA");
    colors.push_back("FB8072");
    colors.push_back("80B1D3");
    colors.push_back("FDB462");
    colors.push_back("B3DE69");
    colors.push_back("FCCDE5");
    colors.push_back("D9D9D9");
    colors.push_back("BC80BD");
    colors.push_back("CCEBC5");
    colors.push_back("FFED6F");
  }
  
  /* // If this block corresponds to a new file, this string will be set to the Javascript command to 
  // load this file into the current view
  string loadCmd="";*/
  
  // Advance to a new color for this func, if needed
  if(ownColor) colorIdx++; 
  
  // Record the index of this block within its parent block
  blockIndex = dbg.blockIndex();
  
  if(ownFile) /*loadCmd = */dbg.enterFileLevel(this);
  else        dbg.enterBlock(this, false, summaryEntry);
}

scope::~scope()
{ 
  // Return to the last color for this func's parent, if needed
  if(ownColor) colorIdx--; 
  
  if(ownFile) dbg.exitFileLevel();
  else        dbg.exitBlock();
  
  assert(colorIdx>=0);
}

// Called to enable the block to print its entry and exit text
void scope::printEntry(string loadCmd) {
  //cout << getLabel()<<": blockIndex="<<blockIndex<<endl;
  dbg.ownerAccessing();
  /*if(blockIndex==0) {
    //if(dbg.blockDepth()>2) dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"</td></tr></table>\n";
    dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"<table>\n";
    dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"<tr width=\"100%\"><td width=0></td><td width=\"100%\">\n";
  } //else
    //dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"</td></tr>\n";
  */
  
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<table bgcolor=\"#"<<colors[(colorIdx-1)%colors.size()]<<"\" width=\"100%\" id=\"table"<<getBlockID()<<"\" style=\"border:1px solid white\" onmouseover=\"this.style.border='1px solid black'; highlightLink('"<<getBlockID()<<"', '#F4FBAA');\" onmouseout=\"this.style.border='1px solid white'; highlightLink('"<<getBlockID()<<"', '#FFFFFF');\" onclick=\"focusLinkSummary('"<<getBlockID()<<"', event);\">\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<tr width=\"100%\"><td width=0></td><td width=\"100%\">";
  if(labelInteractive) {
    dbg <<"<h2>\n";
    dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<a name=\"anchor"<<getBlockID()<<"\" href=\"javascript:unhide('"<<getBlockID()<<"');\">";
  }
  
  if(labelShown) {
    dbg.userAccessing();
    dbg << getLabel();
    dbg.ownerAccessing();
  }
  
  if(labelInteractive) {
    dbg << "</a>\n";
    #if REMOTE_ENABLED
    if(saved_appExecInfo) {
      ostringstream setGDBLink; 
      setGDBLink << "\"javascript:setGDBLink(this, ':"<<GDB_PORT<<"/gdbwrap.cgi?execFile="<<execFile<<"&tgtCount="<<blockIDFromStructure<<"&args=";
      for(int i=1; i<argc; i++) {
        if(i!=1) dbg << " ";
        setGDBLink<< argv[i];
      }
      setGDBLink << "')\"";
  
      dbg << "<a href=\"#\" onclick="<<setGDBLink.str()<<" onmouseover="<<setGDBLink.str()<<"><img src=\"img/gdb.gif\" width=40 height=21 alt=\"GDB\"></a>\n";
    }
    #endif
    if(loadCmd != "") {
      dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1);
      dbg << "<a href=\"javascript:"<<loadCmd<<")\">";
      //dbg << "<a href=\"javascript:loadURLIntoDiv(top.detail.document, '"<<detailContentURL<<".body', 'div"<<getBlockID()<<"'); loadURLIntoDiv(top.summary.document, '"<<summaryContentURL<<".body', 'sumdiv"<<getBlockID()<<"')\">";
      dbg << "<img src=\"img/divDL.gif\" width=25 height=35></a>\n";
      dbg << "\t\t\t<a target=\"_top\" href=\"index."<<getFileID()<<".html\">";
      dbg << "<img src=\"img/divGO.gif\" width=35 height=25></a>\n";
    }
    dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</h2>"<<endl;
  }
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</td></tr>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<tr width=\"100%\"><td width=0></td><td width=\"100%\">\n";
  dbg.flush();
  dbg.userAccessing();  
}

void scope::printExit() {
  // Close this scope
  dbg.ownerAccessing();
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</td></tr>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</table>\n";
  /*if(blockIndex==0) {
    dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"</td></tr>\n";
    dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"</table>\n";
  }*/
  //dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"</td></tr>\n";
  //dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"<tr width=\"100%\"><td width=0></td><td width=\"100%\">\n";
  dbg.userAccessing();  
}

}; // namespace layout
}; // namespace sight
