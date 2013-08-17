#include "dbglog.h"
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
  
  for(int i=0; i<100; i++) {
    int serverID = rand()%numServers;
    int clientID = rand()%numClients;
    string request = requests[rand()%3];
    
    attr sid("serverID", (long)serverID);
    attr cid("clientID", (long)clientID);
    attr r("request", request);
    
    // Emit debug output only for write and read requests, not status requests
    attrIf aif(new attrNEQ("request", string("status")));
    scope sdl("Request", scope::min);
    
    dbg << "Client "<<clientID<<" sending "<<request<<" request to server "<<serverID<<endl;
  }
}