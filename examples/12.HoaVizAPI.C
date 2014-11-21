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
    
    // node link graph example

    flowgraph glik;
       
	glik.graphNodeStart("a","a text log",0,0);
	glik.graphNodeStart("c","c text log",1,0);
	glik.graphNodeEnd("c");
	glik.graphNodeStart("b","b text log",1,1);
	glik.graphNodeStart("d","d text log",2,1);
	glik.graphNodeEnd("d");
	glik.graphNodeStart("e","e text log",3,1);
	glik.graphNodeEnd("e");
	glik.graphNodeEnd("b");
	glik.graphNodeEnd("a");

    glik.graphNodeStart("f","f text log",0,2);
    glik.graphNodeStart("g","g text log",1,2);
    glik.graphNodeStart("h","h text log",2,2);
    glik.graphNodeEnd("h");
    glik.graphNodeStart("m","m text log",2,3);
    glik.graphNodeEnd("m");
    glik.graphNodeStart("x","x text log",3,2);
    glik.graphNodeStart("y","y text log",4,2);
    glik.graphNodeEnd("y");
    glik.graphNodeEnd("x");
    glik.graphNodeEnd("g");
    glik.graphNodeEnd("f");

	glik.graphNodeStart("t1","t1 text log",4,0);
	glik.graphNodeStart("t2","t2 text log",5,0);
	glik.graphNodeStart("t3","t3 text log",6,0);
	glik.graphNodeEnd("t3");
	glik.graphNodeEnd("t2");
	glik.graphNodeEnd("t1");

    
    // generate graph by vertical/horizontal layout - example 1
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

	 // generate graph by vertical/horizontal layout - example 2
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
	/*
        flowgraph gra;
	gra.addNode("a");
	gra.addNode("b","a");
	gra.addNode("c","b");
	gra.addNode("d","b");
	gra.addNode("e","d");
	*/
        //gr.addEdge("a","c");
	//gr.addEdge("a","e");

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
