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

	// generate graph g1
	flowgraph::genFlowGraph(string("g1{a-b-c;b-d-e-f;e-k;e-m}"));
	flowgraph::addnode(string("g1{time;temp}"));
	flowgraph::addedge(string("g1{m-x;c-y;y-z}"));
	flowgraph::drawGraph(string("g1"));

	// generate graph g2
	flowgraph::genFlowGraph(string("g2{m1-m2-m3;m2-m4}"));
	flowgraph::drawGraph(string("g2"));

  return 0;
}




