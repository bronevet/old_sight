#include "sight.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;

int main(int argc, char** argv)
{
	SightInit(argc, argv, "12.HoaVizAPI", "dbg.12.HoaVizAPI");

	// It is possible to write arbitrary text to the debug output
	dbg << "<h1>Example 12: HoaVizAPI</h1>" << endl;
    
    // genegrate graph by vertical/horizontal layout
    flowgraph g;
    g.graphNodeStart("a",0,0);
    g.graphNodeStart("c",1,0);
    g.graphNodeEnd("c");
    g.graphNodeStart("k",1,1);
    g.graphNodeEnd("k");    
    g.graphNodeStart("b",1,2);
    g.graphNodeStart("d",2,2);
    g.graphNodeEnd("d");
    g.graphNodeStart("e",3,2);
    g.graphNodeEnd("e");
    g.graphNodeEnd("b");
    g.graphNodeEnd("a");
    g.addEdge("a","d");
    g.addEdge("c","e");

    //generate graph 1 by method 1
    flowgraph fg;
    fg.graphNodeStart("a");
    fg.graphNodeStart("b");
    fg.graphNodeStart("c");
    fg.addEdge("a", "c");
    fg.graphNodeEnd("c");
    fg.graphNodeStart("d");
    fg.graphNodeStart("e");
    fg.graphNodeStart("f");
    fg.graphNodeEnd("f");
    fg.graphNodeStart("k");
    fg.addEdge("c","k");
    fg.graphNodeEnd("k");
    fg.graphNodeEnd("e");
    fg.graphNodeEnd("d");
    fg.graphNodeEnd("b");
    fg.graphNodeEnd("a");
    fg.addEdge("a", "f");

    // generate graph 2 by method 2
	flowgraph gr;
	gr.addNode("a");
	gr.addNode("b","a");
	gr.addNode("c","b");
	gr.addNode("d","b");
	gr.addNode("e","d");
	gr.addEdge("a","c");
	gr.addEdge("a","e");

        /*
	// generate graph 3
	flowgraph g3;
	g3.genFlowGraph("{a-b-c;b-d-e-f;e-k;e-m}");
	g3.addNode("time");
	g3.addEdge("time","c");
	g3.addEdge("time","k");

	// generate graph 4
	flowgraph flg;
	flg.genFlowGraph("{m1-m2-m3;m2-m4}");
        */
  return 0;
}
