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
  (*layoutEnterHandlers)["dataEncodingFG"] = &flowgraph::setFlowGraphEncoding;
  (*layoutExitHandlers )["dataEncodingFG"] = &defaultExitHandler;
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
  if(props.exists("dataText")) {
    outputDataFlowGraph(properties::get(props, "dataText"));
    flowgraphOutput = true;
  // Otherwise, wait to observe the nodes and edges of the graph before emitting it in the destructor
  } else
    flowgraphOutput = false;
 
  subFlowGraphCounter=0;
  
  // Add the current graph to the map of ative graphs
  active[flowgraphID] = this;

    dbg << "<iframe id=\"flGrFrame\" src=\"widgets/flowgraph/index.html\" width=\"2300\" height=\"1200\"></iframe>\n";

    /*
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
	ioInfoFile.open(ioInfoFName.str().c_str());
	*/
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
  
  dbg.includeFile("flowgraph/processing.js"); dbg.includeWidgetScript("flowgraph/processing.js", "text/javascript");
  dbg.includeFile("flowgraph/flgr.js"); dbg.includeWidgetScript("flowgraph/flgr.js", "text/javascript");
  dbg.includeFile("flowgraph/hoaViz.pde");
  dbg.includeFile("flowgraph/index.html");

}

flowgraph::~flowgraph() {
  if(!flowgraphOutput) {
    outputDataFlowGraph(genDataFlowGraph());
  }

  /*
  tFile.close();
  inouFile.close();
  datFile.close();
  ioInfoFile.close();
  */
  
  dbg.exitBlock();
  
  // Remove the current graph from the map of active graphs
  assert(active.size()>0);
  assert(active.find(flowgraphID) != active.end());
  assert(active[flowgraphID] == this);
  active.erase(flowgraphID);
}

// Generates and returns the data graph code for this graph
string flowgraph::genDataFlowGraph() {

  ostringstream data;

  for(map<anchor, string>::iterator b=nodes.begin(); b!=nodes.end(); b++)
  {
	data <<  b->first.getID() <<":"<< b->first.getID() <<":"<< "1" <<":"<<"1"<<":"<<b->second << endl;
    tFile << b->first.getID() <<":"<< b->first.getID() <<":"<< "1" <<":"<<"1"<<":"<<b->second << endl;
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

  for(set<flowgraphEdge>::iterator e=uniqueEdges.begin(); e!=uniqueEdges.end(); e++)
  {
	inouFile <<"0"<<"_output_"<<e->to.getID()<<":"<<"0"<<"_input_"<<e->to.getID()<< endl;
    data << "0"<<"_output_"<<e->to.getID()<<":"<<"0"<<"_input_"<<e->to.getID()<< endl;

    ostringstream style; bool emptyStyle=true;
    if(!e->directed) { if(!emptyStyle) { style << " "; } style << "dir=none";    emptyStyle=false; }
    if(!e->visible)  { if(!emptyStyle) { style << " "; } style << "style=invis"; emptyStyle=false; }
    if(!emptyStyle) data << "[" << style.str() << "]";
  }

  return data.str();
}

// Given a string representation of a data flowgraph, emits the graph's visual representation
void flowgraph::outputDataFlowGraph(std::string data) {
	ostringstream dataFName;   dataFName << outDir << "/data.txt";

	ofstream dataFile;
	dataFile.open(dataFName.str().c_str());
	dataFile << data;
	dataFile.close();

	string vizlist = "scatter3d:ccp:pcp";
   	 int nodeID = 0;
   	int parentID = -1;
    	nodesFG.clear();
	parentsFG.clear();
	int br_num = 0;
	int nod_num = 0;
	std::istringstream ss(data);
	std::string token;
	int oldnode = 0;

	//process branches
	while(std::getline(ss, token, ';'))	{
		std::istringstream s(token);
		std::string t;
		br_num++;

		while(std::getline(s, t, '-')) {
			if(oldnode == 0)
				nod_num = nodeID-1;
			else
				oldnode = 0;					
			
			if(nodeID == 0)	{
				nodesFG.push_back(std::make_pair(nodeID, t));
				parentsFG.push_back(std::make_pair(nodeID, parentID));
				nodeID++;
			}
			else {
				oldnode = 0;
				for(int i=0; i < (int)nodesFG.size(); i++) {
					if(t.compare(nodesFG[i].second)==0)
					{
						oldnode = 1;
						nod_num = nodesFG[i].first;
					}
				}
				if(oldnode == 0) {
					nodesFG.push_back(std::make_pair(nodeID, t));
					
					parentsFG.push_back(std::make_pair(nodeID, nod_num));
					nodeID++;
				}

			}
		}
	}

	for(int i=0; i < (int)nodesFG.size(); i++) {
		   add_node(nodesFG[i].first, nodesFG[i].second, 0, 0, parentsFG[i].second);
		   //add_viz(nodesFG[i].first, nodesFG[i].first, vizlist);
	}

	ostringstream datFName;
        datFName << outDir << "/dat.txt";
        datFile.open(datFName.str().c_str(), std::fstream::app);
        //datFile << nodeID << "," << buttonID << "," << viz << endl;
        datFile.close();

	ostringstream inouFName;
	inouFName << outDir << "/inout.txt";
	inouFile.open(inouFName.str().c_str(), std::fstream::app);
	//inouFile <<fromID<<"_output_"<<from_nodeID<<":"<<toID<<"_input_"<<to_nodeID<< endl;
	inouFile.close();

	ostringstream ioInfoFName;
	ioInfoFName << outDir << "/ioInfo.txt";
	ioInfoFile.open(ioInfoFName.str().c_str(), std::fstream::app);
	//ioInfoFile << nodeID <<";"<< num_polyFit<<";"<<fitText<<endl;
	ioInfoFile.close();

	// end process branches

	flowgraphOutput = true;
}

// Sets the structure of the current graph by specifying its dot encoding
void flowgraph::setFlowGraphEncoding(string dataText) {
  outputDataFlowGraph(dataText);
}

void* flowgraph::setFlowGraphEncoding(properties::iterator props) {
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->setFlowGraphEncoding(properties::get(props, "data"));
  return NULL;
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void flowgraph::addDirEdgeFG(anchor from, anchor to) {
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

void flowgraph::add_node(int nodeID, string nodeName, int num_inputs, int num_outputs, int parentID)
{
	ostringstream tFName;
	tFName   << outDir << "/node.txt";
	tFile.open(tFName.str().c_str(), std::fstream::app);
	tFile << nodeID <<":"<< nodeName <<":"<< num_inputs <<":"<<num_outputs<<":"<<parentID<<endl;
	tFile.close();
}

// input data for statistic visualization
void flowgraph::add_viz(int nodeID, int buttonID, string viz)
{
	ostringstream datFName;
	datFName << outDir << "/dat.txt";
	datFile.open(datFName.str().c_str(), std::fstream::app);
	datFile << nodeID << "," << buttonID << "," << viz << endl;
	datFile.close();
}

// connection between input fromID of from_nodeID and output toID of to_nodeID
void flowgraph::add_inout(int fromID, int from_nodeID, int toID, int to_nodeID)
{
	ostringstream inouFName;
	inouFName << outDir << "/inout.txt";
	inouFile.open(inouFName.str().c_str(), std::fstream::app);
	inouFile <<fromID<<"_output_"<<from_nodeID<<":"<<toID<<"_input_"<<to_nodeID<< endl;
	inouFile.close();
}

// data for input and output variable information of modules
void flowgraph::add_ioInfo(int nodeID, int num_polyFit, string fitText)
{
	ostringstream ioInfoFName;
	ioInfoFName << outDir << "/ioInfo.txt";
	ioInfoFile.open(ioInfoFName.str().c_str(), std::fstream::app);
	ioInfoFile << nodeID <<";"<< num_polyFit<<";"<<fitText<<endl;
	ioInfoFile.close();
}

// Add a node to the graph
void flowgraph::addNodeFG(anchor a, string label) {
  nodes[a] = label;
  tFile << a.getID() <<":"<< label <<":"<< "1" <<":"<<"1"<<":"<<"1"<<endl;
  datFile << a.getID() << "," << a.getID() << "," << "scatter3d:ccp:pcp" << endl;
}

void* flowgraph::addNodeFG(properties::iterator props) {
  int flowgraphID = properties::getInt(props, "flowgraphID");

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
  subFlowGraphCounter++;
}

void flowgraph::startSubFlowGraph(const std::string& label) {
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
