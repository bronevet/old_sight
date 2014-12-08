#include "sight.h"
#include <map>
#include <map>
#include <assert.h>
#include <unistd.h>
using namespace std;
using namespace sight;

// Fibonacci, where we create flowgraph edges from each call to its children.
int fibGraph(int a, int numIters, flowgraph& g, anchor* parent);

int main(int argc, char** argv)
{
	
	if(argc<=1) { cerr << "Usage: 12. Hyperlink HoaVizAPI numIters"<<endl; exit(-1); }
	long numIters = strtol(argv[1], NULL, 10);

	SightInit(argc, argv, txt()<<"12.Hyperlink_HoaVizAPI, "<<numIters<<" iterations", 
	                    txt()<<"dbg.12.Hyperlink_HoaVizAPI.numIters_"<<numIters);

	dbg << "<h1>Example 12: Hyperlink_HoaVizAPI</h1>" << endl;
	dbg << numIters << " Iterations."<<endl;

	// example recursive Fibonacci
	{
		// Call a recursive Fibonacci, where we create a graph of the recursion hierarchy
		{
			scope s("Recursive Fibonacci", scope::medium);
			flowgraph g;
			fibGraph(0, numIters, g, NULL);
		}
	}
}

// Fibonacci, where we create graph edges from each call to its children.
int fibGraph(int a, int numIters, flowgraph& g, anchor* parent) 
{
	attr jAttr("depth", a);
	scope s(txt()<<"fib("<<a<<")");
	anchor sAnchor = s.getAnchor();

	// Create a link from the calling scope to this one
	if(parent) g.addDirEdgeFG(*parent, sAnchor);

	if(a>=numIters) 
	{ 
		dbg << "="<<1<<endl;
		attr jAttr("val", 1);
		return 1;
	} 
	else 
	{
		int val = fibGraph(a+1, numIters, g, &sAnchor) + fibGraph(a+2, numIters, g, &sAnchor);
		attr jAttr("val", val);
		dbg << "="<<val<<endl;
		return val;
	}
}
