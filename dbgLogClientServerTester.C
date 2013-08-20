#include "dbglog.h"
#include "widgets/valSelector.h"
#include "widgets/trace.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace dbglog;

int main(int argc, char** argv) {
  int numServers=3;
  int numClients=10;
  string requests[] = {"read", "write", "status"};
  
  srand(time(NULL));
  
  initializeDebug(argc, argv);
  
  colorSelector clientColor(.3,0,0,1,0,0); // Red gradient
  // Version of selector that always looks at the current value of the request attribute
  colorSelector requestColor("request", 0,.3,0,0,1,0); // Blue gradient
  colorSelector serverColor(0,0,.3,0,0,1); // Green gradient
  
  // Create a trace. Every call to traceAttr() while this object is in scope will send data to this trace.
  // The values of the traced items will be shown as a function of attribute "i".
  // The visualization of the trace will be shown at the spot in the debug output where t goes out of scope.
  list<string> contextAttrs;
  contextAttrs.push_back("i");
  contextAttrs.push_back("request");
  trace t("Client and Server IDs", contextAttrs, trace::showEnd, trace::table);
  
  for(int i=0; i<100; i++) {
    attr iAttr("i", (long)i);
    
    int serverID = rand()%numServers;
    int clientID = rand()%numClients;
    string request = requests[rand()%3];
    
    attr sid("serverID", (long)serverID);
    attr cid("clientID", (long)clientID);
    attr r("request", request);
    
    // Emit debug output only for write and read requests, not status requests
    attrIf aif(new attrNEQ("request", string("status")));
 //   scope sdl("Request", scope::min);
    
    indent ind(sid.getVInt());
    dbg << textColor::start(clientColor, cid.getVal()) << "Client "<<clientID<<textColor::end()<<" "<<"sending "<<
           bgColor::start(requestColor) << request << bgColor::end()<< " request to "<<
           borderColor::start(serverColor, sid.getVal()) << "Server "<<serverID<<borderColor::end()<<endl;
    
    traceAttr("clientID", attrValue((long)clientID));
    traceAttr("serverID", attrValue((long)serverID));
  }
}