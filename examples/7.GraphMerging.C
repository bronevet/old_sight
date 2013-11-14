#include "sight.h"
#include <math.h>
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;

// Fibonacci, where we create graph edges from each call to its children.
int fibGraph(int a, int numIters, graph& g, anchor* parent);
	// Fibonacci, where we create links edges from each call to its children and back
int fibLinks(int a, int numIters, /*graph& g, */anchor* parent, anchor fromParent);

int main(int argc, char** argv)
{
  if(argc<=1) { cerr << "Usage: 7.GraphMerging numIters"<<endl; exit(-1); }
  long numIters = strtol(argv[1], NULL, 10);

  SightInit(argc, argv, txt()<<"7.GraphMerging, "<<numIters<<" iterations", 
                        txt()<<"dbg.7.GraphMerging.numIters_"<<numIters);

  dbg << "<h1>Example 7: Graph Merging</h1>" << endl;
  dbg << numIters << " Iterations."<<endl;
  scope s("We make output more navigable by linking related outer iterations", scope::medium);
  
  /* //graph g;
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
  }*/
  
  list<string> contextAttrs;
  contextAttrs.push_back("depth");
  contextAttrs.push_back("val");
  trace tableTrace("Table", contextAttrs, trace::showBegin, trace::table, trace::disjMerge);
  
  // Call a recursive Fibonacci, where we create a graph of the recursion hierarchy
  {
    scope s("Recursive Fibonacci", scope::medium);
    graph g;
    fibGraph(0, numIters, g, NULL);
    
    /*anchor toChild; 
    toChild.linkImg("Root Node"); dbg << endl;
    fibLinks(0, numIters, NULL, toChild);*/
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


// Fibonacci, where we create links edges from each call to its children and back
int fibLinks(int a, int numIters, /*graph& g, */anchor* parent, anchor fromParent) {
  scope s(txt()<<"fib("<<a<<")", fromParent);
  anchor sAnchor = s.getAnchor();
  
  // Create a link from the calling scope to this one
  if(parent) //g.addDirEdge(*parent, sAnchor);
  	parent->linkImg("Parent"); dbg << endl;
  
  if(a>=numIters) { 
    dbg << "="<<1<<endl;
    return 1;
  } else {
  	anchor left, right;
  	left.linkImg("Left"); right.linkImg("Right"); dbg << endl;
    int val = fibLinks(a+1, numIters, /*g, */&sAnchor, left) + fibLinks(a+2, numIters, /*g, */&sAnchor, right);
    dbg << "="<<val<<endl;
    return val;
  }
}


