// Licence information included in file LICENCE
#include "../../sight_layout.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

using namespace std;

namespace sight {
namespace layout {

// Record the layout handlers in this file
void* flowgraphEnterHandler(properties::iterator props) { return new flowgraph(props); }
void  flowgraphExitHandler(void* obj) { flowgraph* g = static_cast<flowgraph*>(obj); delete g; }
  
flowgraphLayoutHandlerInstantiator::flowgraphLayoutHandlerInstantiator() {
  (*layoutEnterHandlers)["flowgraph"]   = &flowgraphEnterHandler;
  (*layoutExitHandlers )["flowgraph"]   = &flowgraphExitHandler;
  (*layoutEnterHandlers)["dotEncodingFG"] = &flowgraph::setFlowGraphEncoding;
  (*layoutExitHandlers )["dotEncodingFG"] = &defaultExitHandler;
  (*layoutEnterHandlers)["dirEdgeFG"]     = &flowgraph::addDirEdgeFG;
  (*layoutExitHandlers )["dirEdgeFG"]     = &defaultExitHandler;
  (*layoutEnterHandlers)["undirEdgeFG"]   = &flowgraph::addUndirEdgeFG;
  (*layoutExitHandlers )["undirEdgeFG"]   = &defaultExitHandler;
  (*layoutEnterHandlers)["invisEdgeFG"]   = &flowgraph::addInvisEdgeFG;
  (*layoutExitHandlers )["invisEdgeFG"]   = &defaultExitHandler;
  (*layoutEnterHandlers)["nodeFG"]        = &flowgraph::addNodeFG;
  (*layoutExitHandlers )["nodeFG"]        = &defaultExitHandler;
  (*layoutEnterHandlers)["subGraphFG"]    = &flowgraph::startSubFlowGraph;
  (*layoutExitHandlers )["subGraphFG"]    = &flowgraph::endSubFlowGraph;
}
flowgraphLayoutHandlerInstantiator flowgraphLayoutHandlerInstance;

// The path the directory where output files of the graph widget are stored
// Relative to current path
std::string flowgraph::outDir="";
// Relative to root of HTML document
std::string flowgraph::htmlOutDir="";

// Maps the IDs of the currently active graphs to their graph objects
std::map<int, flowgraph*> flowgraph::active;

flowgraph::flowgraph(properties::iterator props) : block(properties::next(props)) {
  dbg.enterBlock(this, false, true);
  //imgPath = dbg.addImage("svg");
  maxNodeID = 0;
  
  initEnvironment();  
    
  flowgraphID = properties::getInt(props, "flowgraphID");
  
  dbg.ownerAccessing();
  dbg << "<div id=\"graph_container_"<<flowgraphID<<"\"></div>\n";
  dbg.userAccessing();
  
  // If the dot encoding of the graph is already provided, emit it immediately
  if(props.exists("dotText")) {
    outputCanvizDotFlowGraph(properties::get(props, "dotText"));
    flowgraphOutput = true;
  // Otherwise, wait to observe the nodes and edges of the graph before emitting it in the destructor
  } else
    flowgraphOutput = false;
 
  subFlowGraphCounter=0;
  
  // Add the current graph to the map of ative graphs
  active[flowgraphID] = this;

    dbg << "<iframe id=\"flGrFrame\" src=\"widgets/flowgraph/index.html\" width=\"1300\" height=\"1200\"></iframe>\n";
    // node file
	ostringstream tFName;
	tFName   << outDir << "/node.txt";
	tFile.open(tFName.str().c_str());
	// input_output file
	ostringstream inouFName;
	inouFName << outDir << "/inout.txt";
	inouFile.open(inouFName.str().c_str());
	// data for statistic visualization
	ostringstream datFName;
	datFName << outDir << "/dat.txt";
	datFile.open(datFName.str().c_str());
	// data for input and output variable information of modules
	ostringstream ioInfoFName;
	ioInfoFName << outDir << "/ioInfo.txt";

}

// Initialize the environment within which generated graphs will operate, including
// the JavaScript files that are included as well as the directories that are available.
void flowgraph::initEnvironment() {
  static bool initialized=false;
  
  if(initialized) return;
  initialized = true;
  
  pair<string, string> paths = dbg.createWidgetDir("flowgraph");
  outDir = paths.first;
  htmlOutDir = paths.second;
  
  dbg.includeFile("canviz-0.1");
  
  dbg.includeWidgetScript("canviz-0.1/prototype/prototype.js", "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/path/path.js",           "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/canviz.css",             "text/css");
  dbg.includeWidgetScript("canviz-0.1/canviz.js",              "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/x11colors.js",           "text/javascript");

  dbg.includeFile("flowgraph/processing.js"); dbg.includeWidgetScript("flowgraph/processing.js", "text/javascript");
  dbg.includeFile("flowgraph/flgr.js"); dbg.includeWidgetScript("flowgraph/flgr.js", "text/javascript");
  dbg.includeFile("flowgraph/hoaViz.pde");
  dbg.includeFile("flowgraph/index.html");

}

flowgraph::~flowgraph() {
  if(!flowgraphOutput) {
    outputCanvizDotFlowGraph(genDotFlowGraph());
  }

  tFile.close();
  inouFile.close();
  datFile.close();
  ioInfoFile.close();
  
  dbg.exitBlock();
  
  // Remove the current graph from the map of active graphs
  assert(active.size()>0);
  assert(active.find(flowgraphID) != active.end());
  assert(active[flowgraphID] == this);
  active.erase(flowgraphID);
}

// Generates and returns the dot graph code for this graph
string flowgraph::genDotFlowGraph() {
  ostringstream dot;
  dot << "digraph G {"<<endl;

  /*cout << "graph::genDotGraph() #nodes="<<nodes.size()<<", #edges="<<edges.size()<<endl;

  cout << "nodes("<<nodes.size()<<")="<<endl;
  for(map<anchor, string>::iterator b=nodes.begin(); b!=nodes.end(); b++)
    cout << "    " << b->first.getID()<< " => [" << b->second << ", " << b->first.getLinkJS() << "]"<<endl;

  cout << "edges("<<edges.size()<<")="<<endl;
  for(list<graphEdge>::iterator e=edges.begin(); e!=edges.end(); e++)
    cout << "    "<<e->getFrom().str()<<" => "<<e->getTo().str()<<endl;
*/
  for(map<anchor, string>::iterator b=nodes.begin(); b!=nodes.end(); b++)
  {
    dot << "\tnode_"<<b->first.getID()<<" [shape=box, label=\""<<b->second<<"\", href=\"javascript:"<<b->first.getLinkJS()<<"\"];\n";

    //tFile << moduleID <<":"<< moduleName <<":"<< numInputs <<":"<<numOutputs<<":"<<containerModuleID<<endl;
    tFile << b->first.getID() <<":"<< b->first.getID() <<":"<< "1" <<":"<<"1"<<":"<<b->second<<endl;
    string vizl = "scatter3d:ccp:pcp";
    datFile << b->first.getID() <<","<< b->first.getID() <<","<<vizl<<endl;
  }
  // Between the time when an edge was inserted into edges and now, the anchors on both sides of each
  // edge should have been located (attached to a concrete location in the output). This means that
  // some of the edges are now redundant (e.g. multiple forward edges from one location that end up
  // arriving at the same location). We thus create a new set of edges based on the original list.
  // The set's equality checks will eliminate all duplicates.

  set<flowgraphEdge> uniqueEdges;
  for(list<flowgraphEdge>::iterator e=edges.begin(); e!=edges.end(); e++)
    uniqueEdges.insert(*e);

  /*cout << "edges="<<endl;
  for(set<graphEdge>::iterator e=uniqueEdges.begin(); e!=uniqueEdges.end(); e++) {
    cout << "    from="<<e->from.str("    ")<<" : found="<<(nodes.find(e->from.getLocation())!=nodes.end())<<" : "<<dbg.blockGlobalStr(e->from.getLocation())<<endl;
    cout << "    to="<<e->to.str("    ")    <<" : found="<<(nodes.find(e->to.getLocation())!=nodes.end())  <<" : "<<dbg.blockGlobalStr(e->to.getLocation())  <<endl;
    cout << "    from="<<e->from.str("    ")<<" : "<<nodes[e->from.getLocation()].first<<endl;
    cout << "    to="<<e->to.str("    ")    <<" : "<<nodes[e->to.getLocation()].first<<endl;
  }*/
  for(set<flowgraphEdge>::iterator e=uniqueEdges.begin(); e!=uniqueEdges.end(); e++)
  {
    dot << "\tnode_" << e->from.getID() << 
           " -> "<<
           "node_" << e->to.getID();
    ostringstream style; bool emptyStyle=true;
    if(!e->directed) { if(!emptyStyle) { style << " "; } style << "dir=none";    emptyStyle=false; }
    if(!e->visible)  { if(!emptyStyle) { style << " "; } style << "style=invis"; emptyStyle=false; }
    if(!emptyStyle) dot << "[" << style.str() << "]";
    dot << ";\n";

    inouFile <<"0"<<"_output_"<<e->to.getID()<<":"<<"0"<<"_input_"<<e->to.getID()<< endl;
  }

  dot << " }";

  return dot.str();
}

// Given a string representation of a dot graph, emits the graph's visual representation 
// as a Canviz widget into the debug output.
void flowgraph::outputCanvizDotFlowGraph(std::string dot) {
//  #ifdef DOT_PATH
  ostringstream origDotFName;   origDotFName   << outDir << "/orig."   << flowgraphID << ".dot";
  ostringstream placedDotFName; placedDotFName << outDir << "/placed." << flowgraphID << ".dot";

  ofstream dotFile;
  dotFile.open(origDotFName.str().c_str());
  dotFile << dot;
  dotFile.close();

  // Create the SVG file's picture of the dot file
  //ostringstream cmd; cmd << DOT_PATH << "dot -Tsvg -o"<<imgPath<<" "<<dotFName.str() << "-Tcmapx -o"<<mapFName.str()<<"&"; 
  // Create the explicit DOT file that details the graph's layout
  //ostringstream cmd; cmd << DOT_PATH << "dot "<<origDotFName.str()<<" -Txdot -o"<<placedDotFName.str()<<"&"; 
  ostringstream cmd; cmd << ROOT_PATH << "/widgets/graphviz/bin/dot "<<origDotFName.str()<<" -Txdot -o"<<placedDotFName.str();//<<"&"; 
  //cout << "Command \""<<cmd.str()<<"\"\n";
  system(cmd.str().c_str());
  
  dbg.widgetScriptCommand(txt() << 
     "  var canviz_"<<flowgraphID<<";\n" <<
     "  canviz_"<<flowgraphID<<" = new Canviz('graph_container_"<<flowgraphID<<"');\n" <<
     //dbg << "  canviz_"<<graphID<<".setImagePath('graphs/images/');\n";
     "  canviz_"<<flowgraphID<<".setScale(1);\n" <<
     "  canviz_"<<flowgraphID<<".load('"<<htmlOutDir<<"/placed." << flowgraphID << ".dot');\n");

  flowgraphOutput = true;
}

// Sets the structure of the current graph by specifying its dot encoding
void flowgraph::setFlowGraphEncoding(string dotText) {
  outputCanvizDotFlowGraph(dotText);
}

void* flowgraph::setFlowGraphEncoding(properties::iterator props) {
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->setFlowGraphEncoding(properties::get(props, "dot"));
  return NULL;
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void flowgraph::addDirEdgeFG(anchor from, anchor to) {
  //cout << "graph::addDirEdge("<<from.getID()<<" => "<<to.getID()<<")"<<endl;
  edges.push_back(flowgraphEdge(from, to, true, true));
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addDirEdge() in the currently active graph
void* flowgraph::addDirEdgeFG(properties::iterator props) {
  anchor from(/*false,*/ properties::getInt(props, "from"));
  anchor to(/*false,*/ properties::getInt(props, "to"));
  
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->addDirEdgeFG(from, to);
  return NULL;
}

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void flowgraph::addUndirEdgeFG(anchor a, anchor b) {
  edges.push_back(flowgraphEdge(a, b, false, true));
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* flowgraph::addUndirEdgeFG(properties::iterator props) {
  anchor a(/*false,*/ properties::getInt(props, "a"));
  anchor b(/*false,*/ properties::getInt(props, "b"));
  
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->addUndirEdgeFG(a, b);
  return NULL;
}

// Add an invisible undirected edge between the location of the a anchor and the location of the b anchor
void flowgraph::addInvisEdgeFG(anchor a, anchor b) {
  edges.push_back(flowgraphEdge(a, b, false, false));
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* flowgraph::addInvisEdgeFG(properties::iterator props) {
  anchor a(/*false,*/ properties::getInt(props, "a"));
  anchor b(/*false,*/ properties::getInt(props, "b"));
  
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->addInvisEdgeFG(a, b);
  return NULL;
}

// Add a node to the graph
void flowgraph::addNodeFG(anchor a, string label) {
  nodes[a] = label;
}

void* flowgraph::addNodeFG(properties::iterator props) {
  int flowgraphID = properties::getInt(props, "flowgraphID");
    /*cout << "addNode() props="<<properties::str(props)<<endl;
    cout << "active="<<endl;
    for(map<int, graph*>::iterator i=active.begin(); i!=active.end(); i++)
      cout << "    "<<i->first<<": "<<i->second->getLabel()<<endl;*/
  if(active.find(flowgraphID) == active.end()) {
    cerr << "ERROR: graph with ID "<<flowgraphID<<" is not active when the following node was added: "<<props.str()<<endl;
    cerr << "active(#"<<active.size()<<")=<";
    for(map<int, flowgraph*>::const_iterator a=active.begin(); a!=active.end(); a++) {
      if(a!=active.begin()) cerr << ", ";
      cerr << a->first;
    }
    cerr << ">"<<endl;
    assert(active.find(flowgraphID) != active.end());
  }
  
  active[flowgraphID]->addNodeFG(anchor(/*false,*/ properties::getInt(props, "anchorID")),
                           properties::get(props, "label"));
  return NULL;
}

// Start a sub-graph
void flowgraph::startSubFlowGraph() {
//  dot << "subgraph cluster_"<<clusterCntr<<" {"<<endl;
  subFlowGraphCounter++;
}

void flowgraph::startSubFlowGraph(const std::string& label) {
/*  dot << "subgraph cluster_"<<clusterCntr<<" {"<<endl;
  dot << "label = \""<<label<<"\";"<<endl;*/
  subFlowGraphCounter++;
}

void* flowgraph::startSubFlowGraph(properties::iterator props) {
  int flowgraphID = props.getInt("flowgraphID");
  if(props.exists("label"))
    active[flowgraphID]->startSubFlowGraph(props.get("label"));
  else
    active[flowgraphID]->startSubFlowGraph();
  return NULL;
}

// End a sub-graph
void flowgraph::endSubFlowGraph(void* obj) {

}
}; // namespace layout
}; // namespace sight
