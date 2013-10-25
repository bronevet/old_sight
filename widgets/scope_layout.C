// Licence information included in file LICENCE
#include "../dbglog_layout.h"
#include "../dbglog_common.h"
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
using namespace dbglog::common;

namespace dbglog {
namespace layout {

// Record the layout handlers in this file
void* scopeEnterHandler(properties::iterator props) { return new scope(props); }
void  scopeExitHandler(void* obj) { scope* s = static_cast<scope*>(obj); delete s; }
  
scopeLayoutHandlerInstantiator::scopeLayoutHandlerInstantiator() { 
  layoutEnterHandlers["scope"] = &scopeEnterHandler;
  layoutExitHandlers ["scope"] = &scopeExitHandler;
}

std::vector<std::string> scope::colors;
int scope::colorIdx=0; // The current index into the list of colors 

scope::scope(properties::iterator props) : block(properties::next(props))
{
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
  
  this->level = (scopeLevel)properties::getInt(props, "level");
  // If this block corresponds to a new file, this string will be set to the Javascript command to 
  // load this file into the current view
  string loadCmd="";
  if(level == high) {
    colorIdx++; // Advance to a new color for this func
    loadCmd = dbg.enterFileLevel(this);
  } else if(level == medium) {
    colorIdx++; // Advance to a new color for this func
    dbg.enterBlock(this, false, true);
  }
  else if(level == low || level == minimum)
    dbg.enterBlock(this, false, true);
}

scope::~scope()
{ 
  if(level == high) {
    dbg.exitFileLevel();
    colorIdx--; // Return to the last color for this func's parent
  }
  else if(level == medium) {
    dbg.exitBlock();
    colorIdx--; // Return to the last color for this func's parent
  } else if(level == low || level == minimum)
    dbg.exitBlock();
  assert(colorIdx>=0);
}

// Called to enable the block to print its entry and exit text
void scope::printEntry(string loadCmd) {
  dbg.ownerAccessing();
  //dbg << "blockID="<<getBlockID()<<endl;
  if(dbg.blockIndex()==0) dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"</td></tr>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth())<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<table bgcolor=\"#"<<colors[(colorIdx-1)%colors.size()]<<"\" width=\"100%\" id=\"table"<<getBlockID()<<"\" style=\"border:1px solid white\" onmouseover=\"this.style.border='1px solid black'; highlightLink('"<<getBlockID()<<"', '#F4FBAA');\" onmouseout=\"this.style.border='1px solid white'; highlightLink('"<<getBlockID()<<"', '#FFFFFF');\" onclick=\"focusLinkSummary('"<<getBlockID()<<"', event);\">\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">";
  if(level == high || level == medium || level == low) {
    dbg <<"<h2>\n";
    dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<a name=\"anchor"<<getBlockID()<<"\" href=\"javascript:unhide('"<<getBlockID()<<"');\">";
  }
  
  dbg.userAccessing();
  dbg << getLabel();
  dbg.ownerAccessing();
  if(level == high || level == medium || level == low) {
    dbg << "</a>\n";
    /*if(false) { //if(appExecInfo) {
      dbg << "<script type=\"text/javascript\">\n";
      dbg << "  document.write(\"<a href=\\\"http://\"+hostname+\""<<":"<<GDB_PORT<<"/gdbwrap.cgi?execFile="<<execFile<<"&tgtCount="<<blockCount<<"&args=";
  //    dbg << "(<a href=\"http://"<<hostname<<":"<<GDB_PORT<<"/gdbwrap.cgi?execFile="<<execFile<<"&tgtCount="<<blockCount<<"&args=";
      for(int i=1; i<argc; i++) {
        if(i!=1) dbg << " ";
        dbg << argv[i];
      }
      dbg << "\\\"><b>GDB</b></a>\");\n";
      dbg << "</script>\n";
    }*/
    #if REMOTE_ENABLED
    if(saved_appExecInfo) {
      ostringstream setGDBLink; 
      setGDBLink << "\"javascript:setGDBLink(this, ':"<<GDB_PORT<<"/gdbwrap.cgi?execFile="<<execFile<<"&tgtCount="<<blockCount<<"&args=";
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
    dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</h2>";
  }
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"</td></tr>\n";
  dbg << "\t\t\t"<<tabs(dbg.blockDepth()+1)<<"<tr width=\"100%\"><td width=50></td><td width=\"100%\">\n";
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

/*********************
 ***** workscope *****
 ********************* /

workscope::workscope(std::string label,                                   scopeLevel level, const attrOp& onoffOp) :
  scope(label, level, onoffOp) {}
workscope::workscope(std::string label, const anchor& pointsTo,           scopeLevel level, const attrOp& onoffOp) :
  scope(label, pointsTo, level, onoffOp) {}
workscope::workscope(std::string label, const std::set<anchor>& pointsTo, scopeLevel level, const attrOp& onoffOp) :
  scope(label, pointsTo, level, onoffOp) {}
workscope::workscope(std::string label,                                                     const attrOp& onoffOp) :
  scope(label, onoffOp) {}
workscope::workscope(std::string label, const anchor& pointsTo,                             const attrOp& onoffOp) :
  scope(label, pointsTo, onoffOp) {}
workscope::workscope(std::string label, const std::set<anchor>& pointsTo,                   const attrOp& onoffOp) :
  scope(label, pointsTo, onoffOp) {}
workscope::workscope(std::string label,                                   scopeLevel level :
  scope(label, level) {}
workscope::workscope(std::string label, const anchor& pointsTo,           scopeLevel level) :
  scope(label, pointsTo, level) {}
workscope::workscope(std::string label, const std::set<anchor>& pointsTo, scopeLevel level) :
  scope(label, pointsTo, level) {}

workscope::~workscope() {
  for(set<dataItem>::iterator d=data.begin(); d!=data.end(); d++) {
    dbg << d->name<<": "<<
  }
}

void workscope::data(std::string name, void* data, DiffFunctor* diff) {
  data.insert(dataItem(name, data, diff));
}
*/

}; // namespace layout
}; // namespace dbglog
