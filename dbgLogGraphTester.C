#include "dbglog.h"
#include "widgets.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace dbglog;

class dottableExample: public dottable
{
  public:
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  std::string toDOT(std::string graphName) {
    ostringstream oss;
    oss << "graph ethane {"<<endl;
    oss << "     C_0 -- H_0 [type=s];"<<endl;
    oss << "     C_0 -- H_1 [type=s];"<<endl;
    oss << "     C_0 -- H_2 [type=s];"<<endl;
    oss << "     C_0 -- C_1 [type=s];"<<endl;
    oss << "     C_1 -- H_3 [type=s];"<<endl;
    oss << "     C_1 -- H_4 [type=s];"<<endl;
    oss << "     C_1 -- H_5 [type=s];"<<endl;
    oss << " }";
    return oss.str();
  }
};

int main(int argc, char** argv)
{
  initializeDebug(argc, argv);

  // It is possible to write arbitrary text to the debug output
  dbg << "Welcome to the dbgLogLogTester" << endl;
  
  // Dot graph
  dbg << "It is possible to generate dot graphs that describe relevant aspects of the code state.";
  
  {
    scope dotStr("This graph was generated from a string:", scope::medium);
    //string imgPath = addDOT(string("graph graphname {\n     a -- b -- c;\n     b -- d;\n}"));
    //dbg << "imgPath=\""<<imgPath<<"\"\n";
    graph::genGraph(string("graph graphname {\n     a -- b -- c;\n     b -- d;\n}"));
  }
  
  {
    scope dotStr("This graph was generated from a dottable object:", scope::medium);
    dottableExample ex;
    /*string imgPath = addDOT(ex);
    dbg << "imgPath=\""<<imgPath<<"\"\n";*/
    graph::genGraph(ex);
  }
  
  {
    scope s("Graph1: link each number to its multiples", scope::high);
    graph g;
    
    map<int, set<anchor> > pointsTo;
    int maxVal=20;
    for(int i=2; i<maxVal; i++) {
      scope s(txt()<<"s"<<i, pointsTo[i], scope::medium);
      for(int j=i*2; j<maxVal; j+=i) {
        anchor toAnchor;
        //cout << "    toAnchor="<<toAnchor.str()<<endl;
        pointsTo[j].insert(toAnchor);
        g.addDirEdge(s.getAnchor(), toAnchor);
        //cout << "    toAnchor="<<toAnchor.str()<<endl;
      }
    }
  }
  
  {
    int x=5;
    scope s(txt()<<"Graph2: link all numbers (mod "<<x<<") together in a ring", scope::high);
    graph g;
    
    map<int, anchor> ringStartA;
    map<int, anchor> ringNextA;
    int maxVal=20;
    for(int i=0; i<maxVal; i++) {
      scope s(txt()<<"s"<<i, 
              // Scopes that are first in their ring have no incoming link yet (we'll create one when we reach the last scope in the ring)
              i/x==0 ? anchor::noAnchor: 
              // Other scopes are linked to from their predecessor. In particular, their predecessor created the link to this scope
              // before the scope was formed, we terminate it here to complete the connection
                       ringNextA[i%x],
              scope::medium);
      
      // If this is the first scope in its ring
      if(i/x==0) {
        // Record its anchor
        ringStartA[i%x] = s.getAnchor();
      }
      
      // If the next scope in the ring is not its first
      if(i+x < maxVal) {
        // Add a forward link to the next scope in the ring and record the target anchor of the link
        // so that we can identify the scope where it terminates when we reach that scope.
        anchor toAnchor;
        g.addDirEdge(s.getAnchor(), toAnchor);
        ringNextA[i%x] = toAnchor;
      } else {
        // Otherwise, add a backward link to the starting scope of the ring
        g.addDirEdge(s.getAnchor(), ringStartA[i%x]);
      }
    }
  }
    
  return 0;
}

