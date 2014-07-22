// Licence information included in file LICENCE
#include "../../sight_layout.h"
#include "../../sight_common.h"
#include "cdnode_layout.h"
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
void* CDNodeEnterHandler(properties::iterator props) 
{ 
  return new CDNode(props); 
}

void  CDNodeExitHandler(void* obj)                   
{ 
  CDNode* s = static_cast<CDNode*>(obj); 
  delete s; 
}
  
CDNodeLayoutHandlerInstantiator::CDNodeLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["CDNode"] = &CDNodeEnterHandler;
  (*layoutExitHandlers) ["CDNode"] = &CDNodeExitHandler;
}
CDNodeLayoutHandlerInstantiator CDNodeLayoutHandlerInstance;

CDNode::CDNode(properties::iterator props) : scope(properties::next(props))
{
  // Keeps track of whether we've included the syntax highlighting scripts required by CDNode
  static bool scriptsIncluded=false;
  if(!scriptsIncluded) {
    // Create the directory that holds the CDNode-specific scripts
    dbg.createWidgetDir("cdnode");
    
    dbg.includeWidgetScript("cdnode/sh_main.js",   "text/javascript");  dbg.includeFile("cdnode/sh_main.js");
    dbg.includeWidgetScript("cdnode/sh_cpp.js",    "text/javascript");  dbg.includeFile("cdnode/sh_cpp.js");
    dbg.includeWidgetScript("cdnode/sh_style.css", "text/css");         dbg.includeFile("cdnode/sh_style.css");
    
    dbg.widgetScriptPrologCommand("sh_highlightDocument();");
    scriptsIncluded = true;
  }

  
//  int numRegions = properties::getInt(props, "numRegions");

//  int numInfo = profile_list.size();
 
//  for(int i=0; i<numInfo; i++) {
/*    
    string fName     = properties::get(props, txt()<<"fName_"<<i);
    string startLine = properties::get(props, txt()<<"startLine_"<<i);
    string endLine   = properties::get(props, txt()<<"endLine_"<<i);

    dbg << "<u>"<<fName<<" ["<<startLine<<" - "<<endLine<<"]</u>"<<endl;

    dbg.ownerAccessing();
    //dbg << "<pre><code class=\"language-c++\">"<<endl;
    dbg << "<pre class=\"sh_cpp\">"<<endl;
    //dbg.userAccessing();

    ifstream sourceFile(fName.c_str());
    assert(sourceFile.is_open());


    sourceFile.close(); 
*/
    //dbg.ownerAccessing();
    //dbg << "</code></pre>"<<endl;
    dbg << "</pre>"<<endl;
    dbg.userAccessing();
//  }
}

// Called to enable the block to print its entry and exit text
void CDNode::printEntry(string loadCmd) {
}

void CDNode::printExit() {
}

// -------------------------- StageNode Layout -------------------------

// Record the layout handlers in this file
void* StageNodeEnterHandler(properties::iterator props) 
{ 
  return new StageNode(props); 
}

void  StageNodeExitHandler(void* obj)                   
{ 
  StageNode* s = static_cast<StageNode*>(obj); 
  delete s; 
}
  
StageNodeLayoutHandlerInstantiator::StageNodeLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["StageNode"] = &StageNodeEnterHandler;
  (*layoutExitHandlers) ["StageNode"] = &StageNodeExitHandler;
}
StageNodeLayoutHandlerInstantiator StageNodeLayoutHandlerInstance;

StageNode::StageNode(properties::iterator props) : scope(properties::next(props))
{
  // Keeps track of whether we've included the syntax highlighting scripts required by StageNode
  static bool scriptsIncluded=false;
  if(!scriptsIncluded) {
    // Create the directory that holds the StageNode-specific scripts
//    dbg.createWidgetDir("StageNode");
    
    
//    dbg.widgetScriptPrologCommand("sh_highlightDocument();");
//    scriptsIncluded = true;
  }

  
//  int numRegions = properties::getInt(props, "numRegions");

//  int numInfo = profile_list.size();
 
//  for(int i=0; i<numInfo; i++) {
/*    
    string fName     = properties::get(props, txt()<<"fName_"<<i);
    string startLine = properties::get(props, txt()<<"startLine_"<<i);
    string endLine   = properties::get(props, txt()<<"endLine_"<<i);

    dbg << "<u>"<<fName<<" ["<<startLine<<" - "<<endLine<<"]</u>"<<endl;

    dbg.ownerAccessing();
    //dbg << "<pre><code class=\"language-c++\">"<<endl;
    dbg << "<pre class=\"sh_cpp\">"<<endl;
    //dbg.userAccessing();

    ifstream sourceFile(fName.c_str());
    assert(sourceFile.is_open());


    sourceFile.close(); 
*/
    //dbg.ownerAccessing();
    //dbg << "</code></pre>"<<endl;
    dbg << "</pre>"<<endl;
    dbg.userAccessing();
//  }
}

// Called to enable the block to print its entry and exit text
void StageNode::printEntry(string loadCmd) {
}

void StageNode::printExit() {
}
}; // namespace layout
}; // namespace sight
