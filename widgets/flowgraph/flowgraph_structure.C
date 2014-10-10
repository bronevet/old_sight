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
void flowgraph::genFlowGraph(std::string dataText) {
  flowgraph g(dataText);
}

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

/*****************************************
 ***** GraphMergeHandlerInstantiator *****
 *****************************************/

FlowGraphMergeHandlerInstantiator::FlowGraphMergeHandlerInstantiator() {
  (*MergeHandlers   )["flowgraph"]     = FlowGraphMerger::create;
  (*MergeKeyHandlers)["flowgraph"]     = FlowGraphMerger::mergeKey;
  (*MergeHandlers   )["node"]      = NodeMergerFG::create;
  (*MergeKeyHandlers)["node"]      = NodeMergerFG::mergeKey;
  (*MergeHandlers   )["dirEdge"]   = DirEdgeMergerFG::create;
  (*MergeKeyHandlers)["dirEdge"]   = DirEdgeMergerFG::mergeKey;
  (*MergeHandlers   )["undirEdge"] = UndirEdgeMergerFG::create;
  (*MergeKeyHandlers)["undirEdge"] = UndirEdgeMergerFG::mergeKey;
  
  MergeGetStreamRecords->insert(&FlowGraphGetMergeStreamRecord);
}
FlowGraphMergeHandlerInstantiator FlowGraphMergeHandlerInstance;

std::map<std::string, streamRecord*> FlowGraphGetMergeStreamRecord(int streamID) {
	std::map<std::string, streamRecord*> mergeMap;
	mergeMap["flowgraph"] = new FlowGraphStreamRecord(streamID);
	mergeMap["node"]  = new NodeStreamRecordFG(streamID);
	return mergeMap;
}

/***********************
 ***** GraphMerger *****
 ***********************/

FlowGraphMerger::FlowGraphMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        BlockMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props))
{
	assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "flowgraph");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Graph!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Merge the graph IDs along all the streams
    FlowGraphStreamRecord::enterFlowGraph(tags, inStreamRecords);
      
    // Record the merged graphID within the outgoing stream's record
    int outGraphID = streamRecord::mergeIDs("flowgraph", "flowgraphID", pMap, tags, outStreamRecords, inStreamRecords);
    FlowGraphStreamRecord::enterFlowGraph(outGraphID, outStreamRecords);
      
    //cout << "GraphMerger::GraphMerger outStreamRecords
  } else {
    // Set of edges within the outgoing stream that correspond to the union of edges along all the input streams,
    // with no duplication
    set<FlowGraphStreamRecord::streamFlowGraphEdge> outEdges;
    
    // Iterate over all the edges that have been recorded for this graph, adding tags to moreTags for each edge
    for(vector<map<string, streamRecord*> >::iterator r=inStreamRecords.begin(); r!=inStreamRecords.end(); r++) {
      FlowGraphStreamRecord* inGS = (FlowGraphStreamRecord*)(*r)["flowgraph"];
      
      // Get the graphID associated with the graph we're currently exiting, along the current incoming stream
      assert(inGS->gStack.size()>0);
      int inGraphID = inGS->gStack.back();
      
      // Iterate over all the edges collected for this graph and add them to the merged graph that will be emitted
      // on the outgoing stream
      const std::set<FlowGraphStreamRecord::streamFlowGraphEdge>& edges = ((FlowGraphStreamRecord*)(*r)["flowgraph"])->getEdges(inGraphID);
      for(std::set<FlowGraphStreamRecord::streamFlowGraphEdge>::const_iterator e=edges.begin(); e!=edges.end(); e++) {
        streamID outAnchorIDFrom = (*r)["anchor"]->in2outID(e->fromAnchor.getID());
        streamID outAnchorIDTo   = (*r)["anchor"]->in2outID(e->toAnchor.getID());
        //cout << "    e->fromAnchor="<<e->fromAnchor.str()<<", e->toAnchor="<<e->toAnchor.str()<<endl;
        //cout << "    outAnchorIDFrom="<<outAnchorIDFrom.str()<<", outAnchorIDTo="<<outAnchorIDTo.str()<<endl;
        outEdges.insert(FlowGraphStreamRecord::streamFlowGraphEdge(
                                 streamAnchor(outAnchorIDFrom, outStreamRecords),
                                 streamAnchor(outAnchorIDTo,   outStreamRecords),
                                 e->directed));
      }
    }
    
    FlowGraphStreamRecord* outGS = (FlowGraphStreamRecord*)outStreamRecords["flowgraph"];
    
    // Get the graphID associated with the graph we're currently exiting, along the current outgoing stream
    assert(outGS->gStack.size()>0);
    int outGraphID = outGS->gStack.back();
      
    // Predefine the properties of exit tags to reduce the cost of emitting them, since they're all identical
    properties dirExitProps("dirEdge");
    properties undirExitProps("undirEdge");
    
    // Iterate over all the edges within the outgoing stream and add them to moreTags to be emitted as 
    // part of the merged graph in the outgoing stream
    for(set<FlowGraphStreamRecord::streamFlowGraphEdge>::iterator e=outEdges.begin(); e!=outEdges.end(); e++) {
      properties enterProps;
      map<string, string> edgePropsMap;
      edgePropsMap["from"]    = txt()<<e->fromAnchor.getID().ID;
      edgePropsMap["to"]      = txt()<<e->toAnchor.getID().ID;
      edgePropsMap["flowgraphID"] = txt()<<outGraphID;
      enterProps.add(e->directed? "dirEdge": "undirEdge", edgePropsMap);
      moreTagsBefore.push_back(make_pair(properties::enterTag, enterProps));
      moreTagsBefore.push_back(make_pair(properties::exitTag, e->directed? dirExitProps: undirExitProps));
    }
    
    FlowGraphStreamRecord::exitFlowGraph(inStreamRecords);
    FlowGraphStreamRecord::exitFlowGraph(outStreamRecords);
  }
}


// Sets the properties of the merged object
properties* FlowGraphMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "flowgraph");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging graph!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Merge the graph IDs along all the streams
    streamRecord::mergeIDs("flowgraph", "flowgraphID", pMap, tags, outStreamRecords, inStreamRecords);
     
    pMap["dataText"] = "";
    for(std::vector<std::pair<properties::tagType, properties::iterator> >::iterator t=tags.begin(); t!=tags.end(); t++) {
      if(t->second.exists("dataText"))
        pMap["dataText"] += properties::get(t->second, "dataText");
    }
    // If the dataText field was not set on any of the incoming streams, remove it from pMap
    if(pMap["dataText"] == "") pMap.erase("dataText");
  }
  props->add("flowgraph", pMap);
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void FlowGraphMerger::mergeKey(properties::tagType type, properties::iterator tag,
                           const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  BlockMerger::mergeKey(type, tag.next(), inStreamRecords, info);
}


FlowGraphStreamRecord::FlowGraphStreamRecord(const FlowGraphStreamRecord& that, int vSuffixID) :
  streamRecord(that, vSuffixID), edges(that.edges), gStack(that.gStack)
{ }

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* FlowGraphStreamRecord::copy(int vSuffixID) {
  return new FlowGraphStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void FlowGraphStreamRecord::resumeFrom(std::vector<std::map<string, streamRecord*> >& streams) {
  streamRecord::resumeFrom(streams);
  
  // Set edges and gStack to be the union of its counterparts in streams
  edges.clear();
  gStack.clear();
  
  //cout << "FlowGraphStreamRecord::resumeFrom()"<<endl;
  
  int idx=0;
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++, idx++) {
    FlowGraphStreamRecord* gs = (FlowGraphStreamRecord*)(*s)["flowgraph"];
    
    // Union gs->edges into edges
    for(map<int, set<streamFlowGraphEdge> >::const_iterator i=gs->edges.begin(); i!=gs->edges.end(); i++) {
      // If the union FlowGraphStreamRecord does not contain edges for the given graphID, initialize it with an empty set
      if(i->first)
        edges[i->first] = set<streamFlowGraphEdge>();
    
      // Fill the edges set for the current graphID in the union FlowGraphStreamRecord with edges for this
      // graphID in the current variant's FlowGraphStreamRecord
      for(set<streamFlowGraphEdge>::iterator e=i->second.begin(); e!=i->second.end(); e++)
        edges[i->first].insert(*e);
    }

    // Union gs->gStack into gStack
    list<int>::iterator unionIt=gStack.begin();
    list<int>::const_iterator curIt=gs->gStack.begin();
    while(curIt!=gs->gStack.end()) {
      // If the union stream's gStack is not as deep as the one within the current stream's,
      // add the next deepest value from the current stream's gStack to the union stream's
      // and update unionIt to point to this new element
      if(unionIt == gStack.end()) {
        gStack.push_back(*curIt);
        unionIt = --gStack.end();
      // Otherwise, ensure that the graphIDs at the same level of all the streams being unioned are consistent
      } else
        assert(*unionIt == *curIt);
      unionIt++;
      curIt++;
    }
  }

}

// Indicates that we've entered a graph (for incoming streams)
void FlowGraphStreamRecord::enterFlowGraph(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  assert(tags.size() == inStreamRecords.size());
  for(int i=0; i<tags.size(); i++) {
    int flowgraphID = properties::getInt(tags[i].second, "flowgraphID");
    FlowGraphStreamRecord* gs = (FlowGraphStreamRecord*)inStreamRecords[i]["flowgraph"];
    gs->edges[flowgraphID] = set<streamFlowGraphEdge>();
    gs->gStack.push_back(flowgraphID);
  }
}

// Indicates that we've entered a graph (for outgoing streams)
void FlowGraphStreamRecord::enterFlowGraph(int graphID,
                                   std::map<std::string, streamRecord*>& outStreamRecord) {
  FlowGraphStreamRecord* gs = (FlowGraphStreamRecord*)outStreamRecord["flowgraph"];
  gs->gStack.push_back(graphID);
}

// Indicates that we've exited a graph (for incoming streams)
void FlowGraphStreamRecord::exitFlowGraph(std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  for(vector<map<std::string, streamRecord*> >::iterator r=inStreamRecords.begin(); r!=inStreamRecords.end(); r++) {
    FlowGraphStreamRecord* gs = (FlowGraphStreamRecord*)(*r)["flowgraph"];
    assert(gs->gStack.size()>0);
    int flowgraphID = gs->gStack.back();
    gs->gStack.pop_back();
    
    assert(gs->edges.find(flowgraphID) != gs->edges.end());
    gs->edges.erase(flowgraphID);
  }
}

// Indicates that we've exited a graph (for outgoing streams)
void FlowGraphStreamRecord::exitFlowGraph(std::map<std::string, streamRecord*>& outStreamRecord) {
  FlowGraphStreamRecord* gs = (FlowGraphStreamRecord*)outStreamRecord["flowgraph"];
  assert(gs->gStack.size()>0);
  gs->gStack.pop_back();
}


// Adds an edge to the current graph
void FlowGraphStreamRecord::addEdge(int flowgraphID, streamFlowGraphEdge edge) {
  assert(edges.find(flowgraphID) != edges.end());
  edges[flowgraphID].insert(edge);
}

// Returns a reference to all of the edges of the most deeply nested graph in this incoming stream
const std::set<FlowGraphStreamRecord::streamFlowGraphEdge>& FlowGraphStreamRecord::getEdges(int flowgraphID) const {
  std::map<int, std::set<streamFlowGraphEdge> >::const_iterator edgeIt = edges.find(flowgraphID);
  assert(edgeIt != edges.end());
  return edgeIt->second;
}

std::string FlowGraphStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[FlowGraphStreamRecord: ";
  s << streamRecord::str(indent+"    ") << endl;
  
  s << indent << "edges="<<endl;
  for(map<int, set<streamFlowGraphEdge> >::const_iterator se=edges.begin(); se!=edges.end(); se++) {
    s << se->first << ": "<<endl;
    for(set<streamFlowGraphEdge>::const_iterator e=se->second.begin(); e!=se->second.end(); e++)
      s << indent << "    "<<e->str(indent+"            ")<<endl;
  }
  s << indent << "gStack=[";
  for(list<int>::const_iterator i=gStack.begin(); i!=gStack.end(); i++) {
    if(i!=gStack.begin()) s << " ";
    s << *i;
  }
  s << "]"<<endl;

  s << indent << "]";
  

  
  return s.str();
}

DirEdgeMergerFG::DirEdgeMergerFG(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                             std::map<std::string, streamRecord*>& outStreamRecords,
                             std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                             properties* props) : 
                                     Merger(advance(tags), outStreamRecords, inStreamRecords, props) {
  if(props==NULL) props = new properties();
  this->props = props;
                                      
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "dirEdge");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging dirEdge!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Iterate over all the incoming streams, adding the current stream edge to the current graph of that stream
    vector<map<std::string, streamRecord*> >::iterator r=inStreamRecords.begin();
    for(int i=0; i<tags.size(); i++, r++) {
      // The ID of the edge's endpoints within the current incoming stream
      streamID fromInSID(properties::getInt(tags[i].second, "from"), inStreamRecords[i]["flowgraph"]->getVariantID());
      streamID toInSID  (properties::getInt(tags[i].second, "to"),   inStreamRecords[i]["flowgraph"]->getVariantID());
      
       ((FlowGraphStreamRecord*)(*r)["flowgraph"])->addEdge(properties::getInt(tags[i].second, "flowgraphID"),
                                                   FlowGraphStreamRecord::streamFlowGraphEdge(
                                                              streamAnchor(fromInSID, inStreamRecords[i]),
                                                              streamAnchor(toInSID,   inStreamRecords[i]),
                                                              true));
    }
  }
  
  // We do not emit edge tags until we're done reading the graph
  dontEmit();
}

UndirEdgeMergerFG::UndirEdgeMergerFG(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                 std::map<std::string, streamRecord*>& outStreamRecords,
                                 std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                 properties* props) : 
                                       Merger(advance(tags), outStreamRecords, inStreamRecords, props) {
  if(props==NULL) props = new properties();
  this->props = props;
  
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "undirEdge");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging undirEdge!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Iterate over all the incoming streams, adding the current stream edge to the current graph of that stream
    vector<map<std::string, streamRecord*> >::iterator r=inStreamRecords.begin();
    for(int i=0; i<tags.size(); i++, r++) {
      // The ID of the edge's endpoints within the current incoming stream
      streamID aInSID(properties::getInt(tags[i].second, "a"), inStreamRecords[i]["flowgraph"]->getVariantID());
      streamID bInSID(properties::getInt(tags[i].second, "b"), inStreamRecords[i]["flowgraph"]->getVariantID());
      ((FlowGraphStreamRecord*)(*r)["flowgraph"])->addEdge(properties::getInt(tags[i].second, "flowgraphID"),
                                                   FlowGraphStreamRecord::streamFlowGraphEdge(
                                                              streamAnchor(aInSID, inStreamRecords[i]),
                                                              streamAnchor(bInSID, inStreamRecords[i]),
                                                              false));
    }
    
    // Merge the graphIDs along all the streams
    streamRecord::mergeIDs("flowgraph", "flowgraphID", pMap, tags, outStreamRecords, inStreamRecords);
  }
  
  // We do not emit edge tags until we're done reading the graph
  dontEmit();
}

NodeMergerFG::NodeMergerFG(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                       properties* props) : 
                                Merger(advance(tags), outStreamRecords, inStreamRecords, props) {
  if(props==NULL) props = new properties();
  this->props = props;

  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "node");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Node!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Merge the graphIDs along all the streams
    streamRecord::mergeIDs("flowgraph", "flowgraphID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // Merge the node IDs along all the streams
    //NodeStreamRecordFG::mergeIDs(pMap, tags, outStreamRecords, inStreamRecords);
    streamRecord::mergeIDs("node", "nodeID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // Merge the IDs of the anchors that target the nodes
    //AnchorStreamRecord::mergeIDs("node", pMap, tags, outStreamRecords, inStreamRecords);
    streamRecord::mergeIDs("anchor", "anchorID", pMap, tags, outStreamRecords, inStreamRecords);
    
    pMap["label"] = getMergedValue(tags, "label");

  }
  
  props->add("node", pMap);
}

NodeStreamRecordFG::NodeStreamRecordFG(const NodeStreamRecordFG& that, int vSuffixID) :
  streamRecord(that, vSuffixID)
{ }

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* NodeStreamRecordFG::copy(int vSuffixID) {
  return new NodeStreamRecordFG(*this, vSuffixID);
}

std::string NodeStreamRecordFG::str(std::string indent) const {
  ostringstream s;
  s << "[NodeStreamRecordFG: ";
  s << streamRecord::str(indent+"    ") << "]";
  return s.str();
}


}; // namespace structure
}; // namespace sight
