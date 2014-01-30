#include "sight.h"
#include <map>
#include <assert.h>
#include <unistd.h>
using namespace std;
using namespace sight;

int main(int argc, char** argv) {
  int selectedClientID=-1;
  if(argc>=2) selectedClientID = atoi(argv[1]);
    
  int selNumIters=-1;
  int numIters;
  if(argc>=3) { 
    selNumIters = atoi(argv[2]);
    numIters = selNumIters;
  } else
    numIters = 100;
    
  int numServers=3;
  int numClients=10;
  string requests[] = {"read", "write", "status"};
  
  srand(time(NULL)+getpid());
  
  SightInit(argc, argv, "4.AttributeAnnotationFiltering", 
                        "dbg.4.AttributeAnnotationFiltering"+
                            (selectedClientID==-1 ? string(""):
                                                    txt()<<".client_"<<selectedClientID)+
                            (selNumIters==-1 ? string(""):
                                                    txt()<<".numIters_"<<selNumIters));
  stepClock c(1);
  
  dbg << "<h1>Example 4: Attribute-Based Filtering</h1>" << endl;
  
  dbg << "This example includes 100 randomly generated requests of type read, write or status from some client to some server. "<<
         "Each line of the emitted output it annotated by the user with the client, server and request type of that line. This"<<
         "Makes it possible to choose during log generation which lines are enabled and which are disabled. Further, the +, - and O "<<
         "buttons at the top of the page make it possible to filter the output based on these attributes as the log is viewed. "<<
         "Finally, each piece of emitted text can be formatted based on the values of attributes. This capability has been used to "<<
         "<ul>"<<
           "<li>vary each line's indent based on the server ID, "<<
           "<li> vary the color of the client ID text, "<<
           "<li> the background color of the request type text and <li> the border color of the server ID"<<
         "</ul>"<<endl;
  
  // Selector objects allow formatting that depends on the values of attributes. Specifically, selectors can
  // consider all the values that an attribute has ever taken when assigning a value to a particular value of
  // the attribute at a particular point during output generation. Below are selectors that emit colors along
  // some gradient.
  colorSelector clientColor(.3,0,0,1,0,0); // Red gradient
  // Version of selector that always looks at the current value of the request attribute
  colorSelector requestColor("request", 0,.3,0,0,1,0); // Blue gradient
  colorSelector serverColor(0,0,.3,0,0,1); // Green gradient
  
  int clientID;
  if(selectedClientID>=0) clientID = selectedClientID;
  else                    clientID = rand()%numClients;
  
  for(int i=0; i<numIters; i++) {
    //cout << "clock="<<c.str()<<endl;
    int serverID = rand()%numServers;
    string request = requests[rand()%3];
    
    // Create attributes that identify the server, client and request type for sight
    attr sid("serverID", serverID);
    attr cid("clientID", clientID);
    attr r("request", request);
    
    if(i==0) dbg << "We use attrIf to emit debug output only for write and read requests, but not status requests."<<endl;
    //attrIf aif(new attrNEQ("request", string("status")));

    if(i==0)
      dbg << "Each line uses conditional formatting that depends on current application state. The indentation is controlled "<<
             "by the server ID. Further, the text color of the Client ID, background color of the request text and the border "<<
             "color of the Server ID depend on the values of these variables. Color selection is done in two steps. First, "<<
             "the user must declare a common repository for all values that will fall within the same gradient. This repository "<<
             "is called a valueSelector and this example uses colorSelectors. When the user wishes to colorize a given piece of "<<
             "text based on a dynamic value, they write \"sight &lt;&lt; textColor::start(curSelector, curAttribute.getVal()) "<<
             "&lt;&lt \"some text\" &lt;&lt; textClor::end()\". This identifies the text to be colored and specifies the value "<<
             "that controls the color choice and the selector that will record all the values that will form the given color "<<
             "gradient. Once the application is done executing the colorSelector will consider all the values it ever observed "<<
             "and only then choose the actual color along the gradient that will be assigned to each value."<<endl;

    if(i==0) for(int j=0; j<clientID; j++) c.step(0);
    
    indent ind(sid.getVInt());
    dbg << textColor::start(clientColor, cid.getVal()) << "Client "<<clientID<<textColor::end()<<" "<<"sending "<<
           bgColor::start(requestColor) << request << bgColor::end()<< " request to "<<
           borderColor::start(serverColor, sid.getVal()) << "Server "<<serverID<<borderColor::end()<<endl;
    
    
    for(int j=0; j<10; j++) c.step(0);
  }
}
