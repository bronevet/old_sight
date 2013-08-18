#include "dbglog.h"
#include "widgets/valSelector.h"
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
  
  for(int i=0; i<100; i++) {
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
           textColor::start(requestColor) << request << textColor::end()<< " request to "<<
           textColor::start(serverColor, sid.getVal()) << "Server "<<serverID<<textColor::end()<<endl;
  }
}