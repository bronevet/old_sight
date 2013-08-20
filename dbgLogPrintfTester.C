#include "dbglog.h"
#include "widgets/valSelector.h"
#include "widgets/trace.h"
#include <map>
#include <assert.h>
#include <sys/time.h>

using namespace std;
using namespace dbglog;

int main(int argc, char** argv) {
  initializeDebug(argc, argv);
  
  scope s("scope", scope::medium);
  colorSelector loopColor("loop",0,0,.3,0,0,1); // Blue gradient
  for(int i=0; i<3; i++) {
    scope ss(txt()<<"sub-scope "<<i, scope::low);
    dbg << textColor::start(loopColor, attrValue((long)i));
    dbgprintf("printf i=%d\n", i);
    dbg << textColor::end();
  }
  
  return 0;
}