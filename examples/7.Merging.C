#include "sight.h"
#include <math.h>
#include <map>
#include <assert.h>
#include <unistd.h>
using namespace std;
using namespace sight;

// Fibonacci, where we create graph edges from each call to its children.
int fibGraph(int a, int numIters, graph& g, anchor* parent);

int main(int argc, char** argv)
{
  if(argc<=1) { cerr << "Usage: 7.Merging numIters"<<endl; exit(-1); }
  long numIters = strtol(argv[1], NULL, 10);

  SightInit(argc, argv, txt()<<"7.Merging, "<<numIters<<" iterations", 
                        txt()<<"dbg.7.Merging.numIters_"<<numIters);

  dbg << "<h1>Example 7: Merging</h1>" << endl;
  dbg << numIters << " Iterations."<<endl;

  {
    scope s("Iteration Sequence");
    dbg << "This segment corresponds to a single loop where text in each iteration contains a link to the "<<
           "preceding iteration and a link to the following iteration. In addition to links the loop creates "<<
           "edges in a graph from each iteration to the one that follows. The numIters parameter determine this "<<
           "loop's iteration count. When logs from multiple such runs with different values of numIters are "<< 
           "merged it is clear that the first iteration is common to all runs, the second to only a subset and "<<
           "so on. Importantly, the forward and backward links, as well as the links from graph nodes to the scopes "<<
           "they refer to, are valid after merging and can be used to navigate within the merged output."<<endl;
    
    graph g(true);
    
    // Maps each iteration number to the link anchors that will point to this iteration's scope
    map<int, set<anchor> > pointsTo;
    map<int, anchor> iterAnchor;
    
    for(int i=0; i<numIters; i++) {
      // Create iteration i'i*icope, anchoring all incominlinks to it
      scope si(txt()<<i, pointsTo[i]);
      
      //cout << i<<": anchor="<<si.getAnchor().str()<<endl;
      
      // We can now erase all the anchors that refer to this scope from pointsTo[] since we've successfully terminated them.
      pointsTo[i].clear(); 
      
      iterAnchor[i] = si.getAnchor();
      
      // Backward link
      if(i>0) {
        //cout << "iterAnchor["<<(i-1)<<"]="<<iterAnchor[i-1].str()<<endl;
        iterAnchor[i-1].linkImg(txt()<<(i-1));
        iterAnchor.erase(i-1);
      }
      
      // Forward Link
      if(i+1 < numIters) {
        // This is an anchor that will be anchored at iteration j of the outer loop once we reach it.
        anchor toAnchor;
        pointsTo[i+1].insert(toAnchor);
        //cout << "forward link toAnchor="<<toAnchor.str()<<endl;
        
        toAnchor.linkImg(txt()<<(i+1)); dbg << endl;
        
        // Add a graph edge from the current scope to the iteration of the number proven to be not prime
        g.addDirEdge(si.getAnchor(), toAnchor);
      }
    }
  }
  
  {
    scope s("Recursion sequence");
    
    dbg << "This segment corresponds to recursive calls to the Fibonacci function. The numIters parameter determines "<<
           "the depth of the recursion. Each recursive call establishes a new scope and sets up an edge in the graph "<<
           "from the scope of the calling function to the scope of the callee. Further, each run maintains a trace that "<<
           "includes one observation for each recursive Fibonacci call, with the depth of the call and its argument as "<<
           "the observation's context and the product of depth and argument as the observation. When multiple runs with "<<
           "multiple values of numIters are merged, the common portions of the recursion tree are visible, as well as "<<
           "the portions that are only executed for larger values of numIters. We also see all the trace observations "<<
           "performed in any of the runs."<<endl;
    
    list<string> contextAttrs;
    contextAttrs.push_back("depth");
    contextAttrs.push_back("val");
    trace tableTrace("Table", contextAttrs, trace::showBegin, trace::table, trace::disjMerge);
    
    // Call a recursive Fibonacci, where we create a graph of the recursion hierarchy
    {
      scope s("Recursive Fibonacci", scope::medium);
      graph g;
      fibGraph(0, numIters, g, NULL);
    }
  }
}

// Fibonacci, where we create graph edges from each call to its children.
int fibGraph(int a, int numIters, graph& g, anchor* parent) {
  attr jAttr("depth", a);
  scope s(txt()<<"fib("<<a<<")");
  anchor sAnchor = s.getAnchor();
  
  // Create a link from the calling scope to this one
  if(parent) g.addDirEdge(*parent, sAnchor);
  
  if(a>=numIters) { 
    dbg << "="<<1<<endl;
    attr jAttr("val", 1);
    traceAttr("Table", "depth*val", attrValue(a*1), s.getAnchor());
    return 1;
  } else {
    int val = fibGraph(a+1, numIters, g, &sAnchor) + fibGraph(a+2, numIters, g, &sAnchor);
    attr jAttr("val", val);
    traceAttr("Table", "depth*val", attrValue(a*val), s.getAnchor());
    dbg << "="<<val<<endl;
    return val;
  }
}
