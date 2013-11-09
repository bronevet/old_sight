#include "sight.h"
#include <math.h>
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;

int main(int argc, char** argv)
{
  if(argc<=1) { cerr << "Usage: 7.GraphMerging numIters"<<endl; exit(-1); }
  long numIters = strtol(argv[1], NULL, 10);

  SightInit(argc, argv, txt()<<"7.GraphMerging, "<<numIters<<" iterations", 
                        txt()<<"dbg.7.GraphMerging.numIters_"<<numIters);

  dbg << "<h1>Example 7: Graph Merging</h1>" << endl;
  dbg << numIters << " Iterations."<<endl;
  scope s("We make output more navigable by linking related outer iterations", scope::medium);
  
  //graph g;
  // Maps each iteration number to the link anchors that will point to this iteration's scope
  map<int, set<anchor> > pointsTo;
  map<int, anchor> iterAnchor;
  
  for(int i=0; i<numIters; i++) {
    // Create iteration i's scope, anchoring all incoming links to it
    scope si(txt()<<i, pointsTo[i]);
    
    // We can now erase all the anchors that refer to this scope from pointsTo[] since we've successfully terminated them.
    pointsTo[i].clear(); 
    
    iterAnchor[i] = si.getAnchor();
    
    if(i>0) {
      iterAnchor[i-1].linkImg(txt()<<(i-1));
      iterAnchor.erase(i-1);
    }
    
    if(i+1 < numIters) {
      // This is an anchor that will be anchored at iteration j of the outer loop once we reach it.
      anchor toAnchor;
      pointsTo[i+1].insert(toAnchor);
      
      toAnchor.linkImg(txt()<<(i+1)); dbg << endl;
      
      // Add a graph edge from the current scope to the iteration of the number proven to be not prime
      //g.addDirEdge(si.getAnchor(), toAnchor);
    }
  }
}
