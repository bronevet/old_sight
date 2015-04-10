#include "sight.h"
#include <map>
#include <map>
#include <assert.h>
#include <unistd.h>
using namespace std;
using namespace sight;

int main(int argc, char** argv)
{
	SightInit(argc, argv, "12.HoaVizAPI", "dbg.12.HoaVizAPI");

	// It is possible to write arbitrary text to the debug output
	dbg << "<h1>Example 12: HoaVizAPI</h1>" << endl;
    
    // node link graph example
    
    // generate graph by vertical/horizontal layout - example 1
	
	flowgraph gra;
	gra.addNode("a", 0, 0);
	scope s("test scope");
	anchor sAnchor = s.getAnchor();
	gra.addNodeHyperlink("a",sAnchor);	

	gra.addNode("b","a", 1, 0);
	gra.addNode("c","b", 2, 0);
	gra.addNode("d","b", 3, 0);
	gra.addNode("e","b", 4, 0);
	gra.addNode("f","b", 5, 0);
	
	gra.addEdge("a","c");
	gra.addEdge("a","e");

	// scope s("test scope");
	// anchor sAnchor = s.getAnchor();
	// gra.addNodeHyperlink("a",sAnchor);	
	
	/*
	flowgraph g;
       
	g.graphNodeStart("a",0,0);
	g.graphNodeStart("c",1,0);
	g.graphNodeEnd("c");
	g.graphNodeStart("b",1,1);
	g.graphNodeStart("d",2,1);
	g.graphNodeEnd("d");
	g.graphNodeStart("e",3,1);
	g.graphNodeEnd("e");
	g.graphNodeEnd("b");
	g.graphNodeEnd("a");

    g.graphNodeStart("f",0,2);
    g.graphNodeStart("g",1,2);
    g.graphNodeStart("h",2,2);
    g.graphNodeEnd("h");
    g.graphNodeStart("m",2,3);
    g.graphNodeEnd("m");
    g.graphNodeStart("x",3,2);
    g.graphNodeStart("y",4,2);
    g.graphNodeEnd("y");
    g.graphNodeEnd("x");
    g.graphNodeEnd("g");
    g.graphNodeEnd("f");

	g.graphNodeStart("t1",4,0);
	g.graphNodeStart("t2",5,0);
	g.graphNodeStart("t3",6,0);
	g.graphNodeEnd("t3");
	g.graphNodeEnd("t2");
	g.graphNodeEnd("t1");
	*/

	 // generate graph by vertical/horizontal layout - example 2
	/*
	flowgraph gr;
	gr.graphNodeStart("a",0,0);
	gr.graphNodeStart("c",1,0);
	gr.graphNodeEnd("c");
	gr.graphNodeStart("k",1,1);
	gr.graphNodeEnd("k");
	gr.graphNodeStart("b",1,2);
	gr.graphNodeStart("d",2,2);
	gr.graphNodeEnd("d");
    gr.graphNodeStart("e",3,2);
    gr.graphNodeEnd("e");
    gr.graphNodeEnd("b");
    gr.graphNodeEnd("a");
    gr.addEdge("a","d");
    gr.addEdge("c","e");
    */
     
	/*
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
	*/
    
    // generate graph 2 by method 2
	/*
	flowgraph gra;
	gra.addNode("a");
	gra.addNode("b","a");
	gra.addNode("c","b");
	gra.addNode("d","b");
	gra.addNode("e","d");
	gra.addEdge("a","c");
	gra.addEdge("a","e");
	*/
	
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
