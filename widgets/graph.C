// Licence information included in file LICENCE
#include "../dbglog.h"
#include "graph.h"
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

namespace dbglog {

// The path the directory where output files of the graph widget are stored
// Relative to current path
std::string graph::outDir="";
// Relative to root of HTML document
std::string graph::htmlOutDir="";

// Maximum ID assigned to any graph object
int graph::maxWidgetID=0;

graph::graph() : block("Graph") {
  // If the current attribute query evaluates to true (we're emitting debug output)
  if(attributes.query())
    init();
  else
    active = false;
}

graph::graph(const attrOp& onoffOp) : block("Graph") {
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && onoffOp.apply())
    init();
  else
    active = false;
}

void graph::init() {
  active = true;
  
  dbg.enterBlock(this, false, true);
  //imgPath = dbg.addImage("svg");
  maxNodeID = 0;
  
  initEnvironment();  
    
  widgetID = maxWidgetID;
  maxWidgetID++;
  
  graphOutput = false;
  
  dbg.ownerAccessing();
  dbg << "<div id=\"graph_container_"<<widgetID<<"\"></div>\n";
  //dbg << "<div id=\"debug_output\"></div>\n";
  dbg.userAccessing();
}

// Initialize the environment within which generated graphs will operate, including
// the JavaScript files that are included as well as the directories that are available.
void graph::initEnvironment() {
  static bool initialized=false;
  
  if(initialized) return;
  initialized = true;
  
  pair<string, string> paths = dbg.createWidgetDir("graph");
  outDir = paths.first;
  htmlOutDir = paths.second;
  //cout << "outDir="<<outDir<<" htmlOutDir="<<htmlOutDir<<endl;
  
  dbg.includeFile("canviz-0.1");
  
  //<!--[if IE]><script type="text/javascript" src="excanvas/excanvas.js"></script><![endif]-->
  //dbg.includeWidgetScript("canviz-0.1/prototype/excanvas/excanvas.js", "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/prototype/prototype.js", "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/path/path.js",         "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/canviz.css",           "text/css");
  dbg.includeWidgetScript("canviz-0.1/canviz.js",            "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/x11colors.js",         "text/javascript");
  //dbg.includeWidgetScript("canviz-0.1/graphs/graphlist.js",  "text/javascript");
  //dbg.includeWidgetScript("canviz-0.1/graphs/layoutlist.js", "text/javascript");
}

graph::~graph() {
  if(!active) return;
  
  if(!graphOutput) {
    outputCanvizDotGraph(genDotGraph());
  }
  
  dbg.exitBlock();
}

// Generates and returns the dot graph code for this graph
string graph::genDotGraph() {
  ostringstream dot;
  dot << "digraph G {"<<endl;

  /*cout << "nodes("<<nodes.size()<<")="<<endl;
  for(map<location, node >::iterator b=nodes.begin(); b!=nodes.end(); b++)
    cout << "    " << dbg.blockGlobalStr(b->first)<< " => [" << b->second.label << ", " << b->second.a.getLinkJS() << "]"<<endl;*/
  for(map<location, node>::iterator b=nodes.begin(); b!=nodes.end(); b++)
    dot << "\tnode_"<<b->second.ID<<" [shape=box, label=\""<<b->second.label<<"\", href=\"javascript:"<<b->second.a.getLinkJS()<<"\"];\n";

  // Between the time when an edge was inserted into edges and now, the anchors on both sides of each
  // edge should have been located (attached to a concrete location in the output). This means that
  // some of the edges are now redundant (e.g. multiple forward edges from one location that end up
  // arriving at the same location). We thus create a new set of edges based on the original list.
  // The set's equality checks will eliminate all duplicates.

  set<graphEdge> uniqueEdges;
  for(list<graphEdge>::iterator e=edges.begin(); e!=edges.end(); e++)
    uniqueEdges.insert(*e);

  /*cout << "edges="<<endl;
  for(set<graphEdge>::iterator e=uniqueEdges.begin(); e!=uniqueEdges.end(); e++) {
    cout << "    from="<<e->from.str("    ")<<" : found="<<(nodes.find(e->from.getLocation())!=nodes.end())<<" : "<<dbg.blockGlobalStr(e->from.getLocation())<<endl;
    cout << "    to="<<e->to.str("    ")    <<" : found="<<(nodes.find(e->to.getLocation())!=nodes.end())  <<" : "<<dbg.blockGlobalStr(e->to.getLocation())  <<endl;
    cout << "    from="<<e->from.str("    ")<<" : "<<nodes[e->from.getLocation()].first<<endl;
    cout << "    to="<<e->to.str("    ")    <<" : "<<nodes[e->to.getLocation()].first<<endl;
  }*/
  for(set<graphEdge>::iterator e=uniqueEdges.begin(); e!=uniqueEdges.end(); e++) {
    dot << "\tnode_" << nodes[e->from.getLocation()].ID << 
           " -> "<<
           "node_" << nodes[e->to.getLocation()].ID << 
           (e->directed? "": "[dir=none]") << ";\n";
  }

  dot << " }";

  return dot.str();
}

// Given a string representation of a dot graph, emits the graph's visual representation 
// as a Canviz widget into the debug output.
void graph::outputCanvizDotGraph(std::string dot) {
//  #ifdef DOT_PATH
  ostringstream origDotFName;   origDotFName   << outDir << "/orig."   << widgetID << ".dot";
  ostringstream placedDotFName; placedDotFName << outDir << "/placed." << widgetID << ".dot";

  ofstream dotFile;
  dotFile.open(origDotFName.str().c_str());
  dotFile << dot;
  dotFile.close();

  // Create the SVG file's picture of the dot file
  //ostringstream cmd; cmd << DOT_PATH << "dot -Tsvg -o"<<imgPath<<" "<<dotFName.str() << "-Tcmapx -o"<<mapFName.str()<<"&"; 
  // Create the explicit DOT file that details the graph's layout
  //ostringstream cmd; cmd << DOT_PATH << "dot "<<origDotFName.str()<<" -Txdot -o"<<placedDotFName.str()<<"&"; 
  ostringstream cmd; cmd << ROOT_PATH << "/widgets/graphviz/bin/dot "<<origDotFName.str()<<" -Txdot -o"<<placedDotFName.str()<<"&"; 
  //cout << "Command \""<<cmd.str()<<"\"\n";
  system(cmd.str().c_str());
  
  dbg.widgetScriptCommand(txt() << 
     "  var canviz_"<<widgetID<<";\n" <<
     "  canviz_"<<widgetID<<" = new Canviz('graph_container_"<<widgetID<<"');\n" <<
     //dbg << "  canviz_"<<widgetID<<".setImagePath('graphs/images/');\n";
     "  canviz_"<<widgetID<<".setScale(1);\n" <<
     "  canviz_"<<widgetID<<".load('"<<htmlOutDir<<"/placed." << widgetID << ".dot');\n"); 

/*  #else
  dbg << "<b>graphviz not available</b>" << endl;
  #endif*/

  graphOutput = true;
}

// Given a reference to an object that can be represented as a dot graph,  create an image from it and add it to the output.
// Return the path of the image.
void graph::genGraph(dottable& obj) {
  graph g;
  g.outputCanvizDotGraph(obj.toDOT("graph_"+g.widgetID));
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
void graph::genGraph(std::string dot) {
  graph g;
  g.outputCanvizDotGraph(dot);
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void graph::addDirEdge(anchor from, anchor to) {
  edges.push_back(graphEdge(from, to, true)); 
}

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void graph::addUndirEdge(anchor a, anchor b) {
  edges.push_back(graphEdge(a, b, false));
}
  
// Called to notify this block that a sub-block was started/completed inside of it. 
// Returns true of this notification should be propagated to the blocks 
// that contain this block and false otherwise.
bool graph::subBlockEnterNotify(block* subBlock) {
  //cout << "graph::subBlockEnterNotify(subBlock="<<subBlock->getLabel()<<")\n";
  // If this block is immediately contained inside this graph
  location common = dbgStream::commonSubLocation(getLocation(), subBlock->getLocation());
  
  /*cout << "subBlock->getLocation().back().second.size()="<<subBlock->getLocation().back().second.size()<<" common.back().second.size()="<<common.back().second.size()<<endl;
  cout << "subBlock->getLocation="<<dbg.blockGlobalStr(subBlock->getLocation())<<endl;*/
  // If subBlock is nested immediately inside the graph's block 
//???  // (the nesting gap is 2 blocks rather than 1 since each block is chopped up into sub-blocks that
//???  //  span the text between adjacent attribute definitions and major block terminal points)
  assert(subBlock->getLocation().size()>0);
  if(common == getLocation() &&
     subBlock->getLocation().size() == common.size()/* &&
     subBlock->getLocation().back().second.size()-1 == common.back().second.size()*/)
  { 
    nodes[subBlock->getLocation()] = node(maxNodeID, subBlock->getLabel(), subBlock->getAnchor());
    maxNodeID++;
  }
  
  return false;
}
bool graph::subBlockExitNotify(block* subBlock)
{
  return false;  
}

} // namespace dbglog
