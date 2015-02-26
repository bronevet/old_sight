// Licence information included in file LICENCE
#include "../../sight_layout.h"
#include "../../sight_common.h"
#include "scopeOMP_layout.h"
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
void* scopeOMPEnterHandler(properties::iterator props) { return new scopeOMP(props); }
void  scopeOMPExitHandler(void* obj) { scopeOMP* s = static_cast<scopeOMP*>(obj); delete s; }
  
scopeOMPLayoutHandlerInstantiator::scopeOMPLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["scopeOMP"] = &scopeOMPEnterHandler;
  (*layoutExitHandlers) ["scopeOMP"] = &scopeOMPExitHandler;
}
scopeOMPLayoutHandlerInstantiator scopeOMPLayoutHandlerInstance;

scopeOMP::scopeOMP(properties::iterator props) : scope(properties::next(props))
{
  string forID = properties::get(props, txt()<<"forID");
  string numIter = properties::get(props, txt()<<"numIter");
  string iterID = properties::get(props, txt()<<"iterID");

  // Keeps track of whether we've included the syntax highlighting scripts required by scopeOMP
  static bool scriptsIncluded=false;
  if(!scriptsIncluded) {
    // Create the directory that holds the scopeOMP-specific scripts
    dbg.createWidgetDir("scopeOMP");
    dbg.includeWidgetScript("scopeOMP/scopeOMP.js",   "text/javascript");  dbg.includeFile("scopeOMP/scopeOMP.js");
    
    dbg.widgetScriptEpilogCommand("unhideLoop('"+forID+"');\n");
    scriptsIncluded = true;
  }
   
    dbg.ownerAccessing();    
    //dbg << "<u>"<<forID<<" ["<<numIter<<" - "<<getBlockID()<<"]</u>"<<endl;
    dbg.widgetScriptCommand("recordLoop('"+forID+"','"+getBlockID()+"');\n");
    
    dbg.userAccessing();
    
}

// Called to enable the block to print its entry and exit text
void scopeOMP::printEntry(string loadCmd) {
}

void scopeOMP::printExit() {
}

}; // namespace layout
}; // namespace sight
