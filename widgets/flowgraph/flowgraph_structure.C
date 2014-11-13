// Licence information included in file LICENCE
#include "flowgraph_structure.h"
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

#include "CallpathRuntime.h"

using namespace std;

namespace sight {
namespace structure {

// Maximum ID assigned to any graph object
ThreadLocalStorage1<int, int> flowgraph::maxFlowGraphID(0);

// Maximum ID assigned to any graph node
ThreadLocalStorage1<int, int> flowgraph::maxNodeID(0);

int stGraph = -1;

flowgraph::flowgraph(bool includeAllSubBlocks, properties* props) :
  block("flowgraph", setProperties(maxFlowGraphID, "", NULL, props)), includeAllSubBlocks(includeAllSubBlocks)
{ flowgraphID=maxFlowGraphID++; }

flowgraph::flowgraph(const attrOp& onoffOp, bool includeAllSubBlocks, properties* props) :
  block("flowgraph", setProperties(maxFlowGraphID, "", &onoffOp, props)), includeAllSubBlocks(includeAllSubBlocks)
{ flowgraphID=maxFlowGraphID++; }

flowgraph::flowgraph(anchor& pointsTo, bool includeAllSubBlocks, properties* props) :
  block("flowgraph", pointsTo, setProperties(maxFlowGraphID, "", NULL, props)), includeAllSubBlocks(includeAllSubBlocks)
{ flowgraphID=maxFlowGraphID++; }

flowgraph::flowgraph(std::set<anchor>& pointsTo, const attrOp& onoffOp, bool includeAllSubBlocks, properties* props) :
  block("flowgraph", pointsTo, setProperties(maxFlowGraphID, "", &onoffOp, props)), includeAllSubBlocks(includeAllSubBlocks)
{ flowgraphID=maxFlowGraphID++; }

flowgraph::flowgraph(string dataText,                                                          bool includeAllSubBlocks, properties* props) :
  block("flowgraph", setProperties(maxFlowGraphID, dataText, NULL, props)), includeAllSubBlocks(includeAllSubBlocks)
{ flowgraphID=maxFlowGraphID++; }

flowgraph::flowgraph(string dataText,                                   const attrOp& onoffOp, bool includeAllSubBlocks, properties* props) :
  block("flowgraph", setProperties(maxFlowGraphID, dataText, &onoffOp, props)), includeAllSubBlocks(includeAllSubBlocks)
{ flowgraphID=maxFlowGraphID++; }

flowgraph::flowgraph(string dataText, anchor& pointsTo,                                        bool includeAllSubBlocks, properties* props) :
  block("flowgraph", pointsTo, setProperties(maxFlowGraphID, dataText, NULL, props)), includeAllSubBlocks(includeAllSubBlocks)
{ flowgraphID=maxFlowGraphID++; }

flowgraph::flowgraph(string dataText, std::set<anchor>& pointsTo,       const attrOp& onoffOp, bool includeAllSubBlocks, properties* props) :
  block("flowgraph", pointsTo, setProperties(maxFlowGraphID, dataText, &onoffOp, props)), includeAllSubBlocks(includeAllSubBlocks)
{ flowgraphID=maxFlowGraphID++; }

// Sets the properties of this object
properties* flowgraph::setProperties(int flowgraphID, std::string dataText, const attrOp* onoffOp, properties* props)
{

  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes->query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> pMap;
    pMap["flowgraphID"] = txt()<<maxFlowGraphID;
    if(dataText != "") pMap["dataText"] = dataText;
    //pMap["callPath"] = cp2str(CPRuntime.doStackwalk());
    props->add("flowgraph", pMap);
  }
  else
    props->active = false;

  if(stGraph == -1 || stGraph != flowgraphID)
  {
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();

	dbg << "<div id=\"hoaViz\"><iframe id=\"flGrFrame\" src=\"widgets/flowgraph/index.html?graName="<< str << "\" width=\"1600\" height=\"1000\"></iframe></div>\n";
	stGraph = flowgraphID;
  }
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void flowgraph::destroy() {

  this->~flowgraph();
}

flowgraph::~flowgraph() {

	assert(!destroyed);

  // Refresh nodesConnected based on all the location information collected for the graph node anchors during execution
  // by filling a new set. Anchor IDs are only updated to become consistent with their final location when they're copied
  // to make sure that updated information about anchor locations doesn't invalidate data structures. Thus, we get
  // the freshest info we have to create a new set from the anchors in nodesConnected
  set<anchor> freshNC;
  for(std::set<anchor>::iterator i=nodesConnected.begin(); i!=nodesConnected.end(); i++)
    freshNC.insert(*i);
  
  // Identify the nodes that were connected but not emitted and emit them
  for(set<anchor>::iterator i=freshNC.begin(); i!=freshNC.end(); i++)
    if(nodesObservedNotEmitted.find(i->getID()) != nodesObservedNotEmitted.end())
      emitNodeTag(i->getID(), nodesObservedNotEmitted[i->getID()].first, nodesObservedNotEmitted[i->getID()].second);
}

// Given a representation of a graph in data text format, create an image from it and add it to the output.
// Return the path of the image.

// generate flow graph by string
void flowgraph::genFlowGraph(std::string dataText) {
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g(str+dataText);
}

void flowgraph::startGraph(){
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g(str+"{}");
}

// add node and edge by method 1
void flowgraph::addNode(std::string nodeName){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g("addnode:"+str+"{"+nodeName+"}");
}

void flowgraph::addNode(std::string childNode, std::string parentNode){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g("addnode:"+str+"{"+parentNode+"-"+childNode+"}");
}

void flowgraph::addEdge(std::string startNode, std::string endNode){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g("addedge:"+str+"{"+startNode+"-"+endNode);
}

void flowgraph::endGraph(){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g("drawgraph:"+str);
}

void flowgraph::graphNodeStart(std::string nodeName){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g("graphNodeStart:"+str+"{"+nodeName);
}

void flowgraph::graphNodeStart(std::string nodeName, int verID, int horID){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();

        stringstream ss1;
        ss1 << verID;
        string str1 = ss1.str();

        stringstream ss2;
        ss2 << horID;;
        string str2 = ss2.str();
	flowgraph g("verhorNodeStart:"+str+"{"+nodeName+"-"+str1+"-"+str2);

}


void flowgraph::graphNodeEnd(std::string nodeName){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g("graphNodeEnd:"+str+"{"+nodeName);
}

void flowgraph::addNodeEdge(std::string startNode, std::string endNode){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g("addNodeEdge:"+str+"{"+startNode+"-"+endNode);
}

void flowgraph::endNodeGraph(){
	stGraph++;
	stringstream ss;
	ss << flowgraphID;
	string str = ss.str();
	flowgraph g("drawNodeGraph:"+str);
}


/*
// generate flow graph by string
void flowgraph::genFlowGraph(std::string dataText) {
  flowgraph g(dataText);
}

// add node and edge by method 1
void flowgraph::addnode(std::string data){
  flowgraph g("addnode:"+data);
}
void flowgraph::addedge(std::string data){
  flowgraph g("addedge:"+data);
}
void flowgraph::drawGraph(std::string graphname){
  flowgraph g("drawgraph:"+graphname);
}

// add node and edge by method 2
void flowgraph::flowGraphStart(std::string graphName){
  flowgraph g("flowGraphStart:"+graphName);
}
void flowgraph::flowGraphNodeStart(std::string nameNode){
  flowgraph g("flowGraphNodeStart:"+nameNode);
}
void flowgraph::flowGraphNodeEnd(std::string nameNode){
  flowgraph g("flowGraphNodeEnd:"+nameNode);
}
void flowgraph::flowGraphEnd(std::string graphName){
  flowgraph g("drawgraph:"+graphName);
}
*/

// Sets the structure of the current graph by specifying its dot encoding
void flowgraph::setFlowGraphEncoding(string dataText) {
  properties p;
  map<string, string> pMap;
  pMap["data"] = dataText;
  p.add("flowgraphEncoding", pMap);
  
  dbg->tag(p);
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void flowgraph::addDirEdgeFG(anchor from, anchor to) {
  // If we only emit nodes that are connected by edges
  if(!includeAllSubBlocks) {
    //cout << "graph::addDirEdge "<<from.getID()<<"=>"<<to.getID()<<", from observed="<<(nodesObservedNotEmitted.find(from.getID()) != nodesObservedNotEmitted.end())<<", from connected="<<(nodesConnected.find(from.getID()) != nodesConnected.end())<<", to observed="<<(nodesObservedNotEmitted.find(to.getID()) != nodesObservedNotEmitted.end())<<", to connected="<<(nodesConnected.find(to.getID()) != nodesConnected.end())<<endl;
    
    // If the node of either side of this edge has been observed but not yet connected, emit its tag now
    if(nodesObservedNotEmitted.find(from.getID()) != nodesObservedNotEmitted.end()/* && nodesConnected.find(from) == nodesConnected.end()*/)
      emitNodeTag(from.getID(), nodesObservedNotEmitted[from.getID()].first, nodesObservedNotEmitted[from.getID()].second);
    if(nodesObservedNotEmitted.find(to.getID()) != nodesObservedNotEmitted.end()/* && nodesConnected.find(to) == nodesConnected.end()*/)
      emitNodeTag(to.getID(), nodesObservedNotEmitted[to.getID()].first, nodesObservedNotEmitted[to.getID()].second);

    // Record that both nodes have been connected
    nodesConnected.insert(from);
    nodesConnected.insert(to);
  }
  
  properties p;
  map<string, string> pMap;
  pMap["from"] = txt()<<from.getID();
  pMap["to"]   = txt()<<to.getID();
  pMap["flowgraphID"] = txt()<<flowgraphID;
  p.add("dirEdge", pMap);
  
  //dbg->tag("dirEdge", properties, false);
  dbg->tag(p);
}

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void flowgraph::addUndirEdgeFG(anchor a, anchor b) {
  // If we only emit nodes that are connected by edges
  if(!includeAllSubBlocks) {
    // If the node of either side of this edge has been observed but not yet connected, emit its tag now
    if(nodesObservedNotEmitted.find(a.getID()) != nodesObservedNotEmitted.end()/* && nodesConnected.find(a) == nodesConnected.end()*/)
      emitNodeTag(a.getID(), nodesObservedNotEmitted[a.getID()].first, nodesObservedNotEmitted[a.getID()].second);
    if(nodesObservedNotEmitted.find(b.getID()) != nodesObservedNotEmitted.end()/* && nodesConnected.find(b) == nodesConnected.end()*/)
      emitNodeTag(b.getID(), nodesObservedNotEmitted[b.getID()].first, nodesObservedNotEmitted[b.getID()].second);
  
    // Record that both nodes have been connected
    nodesConnected.insert(a);
    nodesConnected.insert(b);
  }
  
  properties p;
  map<string, string> pMap;
  pMap["a"] = txt()<<a.getID();
  pMap["b"] = txt()<<b.getID();
  pMap["flowgraphID"] = txt()<<flowgraphID;
  p.add("undirEdge", pMap);
  //dbg->tag("undEdge", properties, false);
  
  dbg->tag(p);
}

// Add an invisible edge that forces the target to be placed after the source
void flowgraph::addInvisDepEdgeFG(anchor a, anchor b) {
  // If we only emit nodes that are connected by edges
  if(!includeAllSubBlocks) {
    // If the node of either side of this edge has been observed but not yet connected, emit its tag now
    if(nodesObservedNotEmitted.find(a.getID()) != nodesObservedNotEmitted.end()/* && nodesConnected.find(a) == nodesConnected.end()*/)
      emitNodeTag(a.getID(), nodesObservedNotEmitted[a.getID()].first, nodesObservedNotEmitted[a.getID()].second);
    if(nodesObservedNotEmitted.find(b.getID()) != nodesObservedNotEmitted.end()/* && nodesConnected.find(b) == nodesConnected.end()*/)
      emitNodeTag(b.getID(), nodesObservedNotEmitted[b.getID()].first, nodesObservedNotEmitted[b.getID()].second);
  
    // Record that both nodes have been connected
    nodesConnected.insert(a);
    nodesConnected.insert(b);
  }
  
  properties p;
  map<string, string> pMap;
  pMap["a"] = txt()<<a.getID();
  pMap["b"] = txt()<<b.getID();
  pMap["flowgraphID"] = txt()<<flowgraphID;
  p.add("invisEdge", pMap);
  //dbg->tag("undEdge", properties, false);
  
  dbg->tag(p);  
}
  

// Start a graphviz cluster
void flowgraph::startSubFlowGraph() {
  properties p;
  map<string, string> pMap;
  pMap["flowgraphID"]  = txt()<<flowgraphID;
  p.add("subFlowGraph", pMap);
  dbg->enter(p);
}

void flowgraph::startSubFlowGraph(const std::string& label) {
  properties p;
  map<string, string> pMap;
  pMap["label"] = label;
  pMap["flowgraphID"]  = txt()<<flowgraphID;
  p.add("subFlowGraph", pMap);
  dbg->enter(p);
}

// End a graphviz cluster
void flowgraph::endSubFlowGraph() {
  properties p;
  map<string, string> pMap;
  p.add("subFlowGraph", pMap);
  dbg->exit(p);
}

/* ADD THIS IF WE WISH TO HAVE NODES THAT EXISTED INSIDE THE GRAPH BUT WERE NOT CONNECTED VIA EDGES*/
// Called to notify this block that a sub-block was started/completed inside of it. 
// Returns true of this notification should be propagated to the blocks 
// that contain this block and false otherwise.
bool flowgraph::subBlockEnterNotify(block* subBlock) {
  //cout << "graph::subBlockEnterNotify() subBlock="<<subBlock->getLabel()<<"/"<<subBlock->getAnchor().getID()<<" includeAllSubBlocks="<<includeAllSubBlocks<<", connected="<<(nodesConnected.find(subBlock->getAnchor().getID()) != nodesConnected.end())<<endl;
  // If we should include nodes for all sub-blocks inside this graph block or this block has already been connected by an edge
  if(includeAllSubBlocks/* || nodesConnected.find(subBlock->getAnchor()) != nodesConnected.end()*/)
    // Emit a node tag for this block immediately
    emitNodeTag(subBlock->getAnchor().getID(), subBlock->getLabel(), maxNodeID);
  
  // If we only emit nodes that are connected by edges
  //if(!includeAllSubBlocks)
  else
    // Record that this node has been observed
    nodesObservedNotEmitted[subBlock->getAnchor().getID()] = make_pair(subBlock->getLabel(), (int)maxNodeID);
  
  maxNodeID++;
  
  return false;
}

// Emits a tag for the given node
void flowgraph::emitNodeTag(int anchorID, std::string label, int nodeID) {
  //cout << "graph::emitNodeTag("<<label<<"/"<<anchorID<<")"<<endl;
  
  properties p;
  map<string, string> pMap;
  pMap["nodeID"]   = txt()<<nodeID;
  pMap["anchorID"] = txt()<<anchorID;
  pMap["label"]    = label;
  pMap["flowgraphID"]  = txt()<<flowgraphID;
  //pMap["callPath"] = cp2str(CPRuntime.doStackwalk());
  p.add("node", pMap);
  
  // This node has now been emitted
  nodesObservedNotEmitted.erase(anchorID);

  dbg->tag(p);
}

/********************
 ***** subflowgraph *****
 ********************/
subflowgraph::subflowgraph(flowgraph& g, const std::string& label): g(g) {
  g.startSubFlowGraph(label);
}

subflowgraph::~subflowgraph() {
  g.endSubFlowGraph();
}

}; // namespace structure
}; // namespace sight
