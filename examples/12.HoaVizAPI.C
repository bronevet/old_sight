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

	// generate graph 1 by method 1
	flowgraph fg;
    fg.startGraph();
    fg.graphNodeStart("a");
    fg.graphNodeStart("b");
    fg.graphNodeStart("c");
    fg.graphNodeEnd("c");
    fg.graphNodeStart("d");
    fg.graphNodeStart("e");
    fg.graphNodeStart("f");
    fg.graphNodeEnd("f");
    fg.graphNodeStart("k");
    fg.graphNodeEnd("k");
    fg.graphNodeEnd("e");
    fg.graphNodeEnd("d");
    fg.graphNodeEnd("b");
    fg.graphNodeEnd("a");
    fg.endNodeGraph();

    // generate graph 2 by method 2
	flowgraph gr;
	gr.startGraph();
	gr.addNode("a");
	gr.addNode("b","a");
	gr.addNode("c","b");
	gr.addNode("d","b");
	gr.addNode("e","d");
	gr.addEdge("a","c");
	gr.addEdge("a","e");
	gr.endGraph();

	// generate graph 3
	flowgraph g;
	g.genFlowGraph("{a-b-c;b-d-e-f;e-k;e-m}");
	g.addNode("time");
	g.addEdge("time","c");
	g.addEdge("time","k");
	g.endGraph();

	// generate graph 4
	flowgraph flg;
	flg.genFlowGraph("{m1-m2-m3;m2-m4}");
	flg.endGraph();

  return 0;
}
