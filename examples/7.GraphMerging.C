#include "sight.h"
#include "widgets/module_structure.h"
#include <math.h>
#include <map>
#include <assert.h>
#include <unistd.h>
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
  
  //trace t0("MT Root",      "Root",      trace::showBegin, trace::table, trace::disjMerge);
  //trace t1("MT Input",     "Input",     trace::showBegin, trace::table, trace::disjMerge);
  //trace t2("MT Iteration", "Iteration", trace::showBegin, trace::table, trace::disjMerge);
  //trace t3("MT Module A",  "Module A",  trace::showBegin, trace::table, trace::disjMerge);
  //trace t4("MT Module B",  "Module B",  trace::showBegin, trace::table, trace::disjMerge);
  //attr a("ModuleCtxt", 1);
  srand(time(NULL));
  module mg(group("Root", 0, 0));
  
  module mInput(group("Input", 0, 4));
  mInput.setOutCtxt(0, context(config("nonzeros", 0)));
  mInput.setOutCtxt(1, context(config("CondNum", 0)));
  mInput.setOutCtxt(2, context(config("solnErr", 2)));
  mInput.setOutCtxt(3, context(config("solnVal", 5)));
  
  //vector<port> 
  std::vector<port> mtxGenOutputs;
  for(int i=0; i<10; i++)
  {
    module mIter(group("Iteration", 0, 1));
    {
      //cout << "mIter.getContext()="<<mIter.getContext().str()<<endl;
      std::vector<port> ma1Outputs;
      { module ma1(group("Lin Solve", 2, 2), 
                   inputs(//nonzeros
                          (i==0? mInput.outPort(0): mtxGenOutputs[0]),
                          // CondNum
                          (i==0? mInput.outPort(1): mtxGenOutputs[1])),
                   ma1Outputs);
    	  usleep(200+i*i*100 + i*1000/*rand()%50*/);
    	  ma1.setOutCtxt(0, context(config("solnErr", (i+1)*2)));
    	  ma1.setOutCtxt(1, context(config("solnVal", (i+1)*5)));
    	}
    	
    	{ module mb1(group("Mtx Gen", 2, 2),
    	             inputs(ma1Outputs[0], ma1Outputs[1]),
    	             mtxGenOutputs);
    	  mb1.setOutCtxt(0, context(config("nonzeros", i+1)));
    	  mb1.setOutCtxt(1, context(config("CondNum", i*i)));
    	    
    	  usleep(1000+i*100/*rand()%150*/);
    	}
    	
    	/*{ module mb1(group("Mtx Gen", 2, 2),
    	             inputs(mInput.outPort(2), ma1Outputs[1]));
    	  mb1.setOutCtxt(0, context(config("nonzeros", i+1)));
    	  mb1.setOutCtxt(1, context(config("CondNum", i*i)));
    	    
    	  usleep(100+i*10/*rand()%150* /);
    	}*/
  	}
  }
  /* //graph g;
  // Maps each iteration number to the link anchors that will point to this iteration's scope
  map<int, set<anchor> > pointsTo;
  map<int, anchor> iterAnchor;
  
  for(int i=0; i<numIters; i++) {
    // Create iteration i'i*icope, anchoring all incominlinks to it
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
  
  /*list<string> contextAttrs;
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
    fibLinks(0, numIters, NULL, toChild);* /
  }*/
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


