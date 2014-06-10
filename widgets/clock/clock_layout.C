// Licence information included in file LICENCE
#include "clock_layout.h"
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
void* clockEnterHandler(properties::iterator props) { return NULL;/*return new clock(props);*/ }
void  clockExitHandler(void* obj) { /*clock* c = static_cast<clock*>(obj); delete c;*/ }
  
clockLayoutHandlerInstantiator::clockLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["clock"] = &clockEnterHandler;
  (*layoutExitHandlers) ["clock"] = &clockExitHandler;
}
clockLayoutHandlerInstantiator clockLayoutHandlerInstance;

}; // namespace layout
}; // namespace sight
