// Licence information included in file LICENCE
#include "sight_layout.h"
#include "sight_common.h"
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
void* variantEnterHandler(properties::iterator props) { variant* v=new variant(props); return v; }
void  variantExitHandler(void* obj) { variant* v = static_cast<variant*>(obj); delete v; }
void* interVariantHandler(properties::iterator props) { 
  dbg.ownerAccessing();
  dbg << endl << "</td><td>" << endl;
  dbg.userAccessing();
  return NULL;
}
  
variantLayoutHandlerInstantiator::variantLayoutHandlerInstantiator() { 
  // Called when a group of variants are entered or exited
  (*layoutEnterHandlers)["variants"] = &variantEnterHandler;
  (*layoutExitHandlers) ["variants"] = &variantExitHandler;
  // Called between individual variants within a group
  (*layoutEnterHandlers)["inter_variants"] = &interVariantHandler;
}
variantLayoutHandlerInstantiator variantLayoutHandlerInstance;

variant::variant(properties::iterator props) : 
  scope("Variants", 
        false, // ownFile
        true,  // ownColor
        false, // labelInteractive
        false, // labelShown
        true)  // summaryEntry
{
  numVariants = props.getNumKeys();

  dbg.ownerAccessing();
  dbg << "<table border=1 width=\"100\%\"><tr><td colspan=\""<<numVariants<<"\">Variants</td></tr>"<<endl;
  dbg << "<tr><td>"<<endl;
  //dbg << "<script type=\"text/javascript\">"<<loadCmd<<"</script>"<<endl;

  dbg.flush();
  dbg.userAccessing();  
}

variant::~variant() { 
  // Close this scope
  dbg.ownerAccessing();
  dbg << "</td></tr></table>"<<endl;
  dbg.userAccessing();  
}

}; // namespace layout
}; // namespace sight
