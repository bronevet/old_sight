#include "dbglog.h"
#include "widgets.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace dbglog;

int main(int argc, char** argv)
{
  initializeDebug(argc, argv);

  // It is possible to write arbitrary text to the debug output
  dbg << "Welcome to the dbgLogLogTester" << endl;
  
  // Keep emitting output at increasingly higher debug levels
  for(int debugLevel=0; debugLevel<3; debugLevel++) {
    scope sdl(txt()<<"Debug Level "<<debugLevel);
    attr dl("debugLevel", (long)debugLevel);
    
    // This text is emitted at debug level 0
    { 
      attrIf aif(new attrEQ("debugLevel", (long)0, attrOp::any));
      for(int i=0; i<6; i++) {
        attr a1("i mod 2", (long)i%2);
        attr a2("i", (long)(i));
        
        // This scope will be created in iterations 0, 1, 2
        scope sOut(txt()<<"Outer[0-3): i="<<i,   scope::medium, attrRange("i", (long)0, (long)3, attrOp::any));
        // This scope will be created in the even iterations
        scope sInEven(txt()<<"Inner:Even i="<<i, scope::medium, attrEQ("i mod 2", (long)0, attrOp::any));
        // This scope will be created in the odd iterations
        scope sInOdd(txt()<<"Inner:Odd i="<<i,   scope::medium, attrEQ("i mod 2", (long)1, attrOp::any));
        
        // The text below will be emitted in odd iterations
        attrAnd condAnd(new attrEQ("i mod 2", (long)1, attrOp::any));
        dbg << "Only Odd: i="<<i<<endl;
        
        // This scope will be created in odd iterations where i is in [3, 6)
        scope sDeep(txt()<<"Deepest: Odd and [3-6): i="<<i,   scope::medium, attrRange("i", (long)3, (long)6, attrOp::any));
        
        // The text below will be emitted in odd iterations, regardless of the debug level
        {
          attrIf condIf(new attrEQ("i mod 2", (long)1, attrOp::any));
          
          // This scope will be created in odd iterations where i is in [3, 6)
          scope sDeeper(txt()<<"Deepest: Odd and [3-6): i="<<i,   scope::medium, attrRange("i", (long)3, (long)6, attrOp::any));
          
          dbg << "Odd and any debug level: i="<<i<<endl;
        }
      }
    }
    
    // This text is emitted at debug level 1
    {
      attrIf aif(new attrEQ("debugLevel", (long)1, attrOp::any));
      
      scope s("Graph1: link each number to its multiples", scope::high);
      graph g;
      
      map<int, set<anchor> > pointsTo;
      int maxVal=4;
      for(int i=2; i<maxVal; i++) {
        scope s(txt()<<"s"<<i, pointsTo[i], scope::medium);
        for(int j=i*2; j<maxVal; j+=i) {
          anchor toAnchor;
          pointsTo[j].insert(toAnchor);
          g.addDirEdge(s.getAnchor(), toAnchor);
        }
      }
    }
  }
}