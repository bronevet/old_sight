// Licence information included in file LICENCE
#include "../sight_structure_internal.h"
#include "graph_structure.h"
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
int graph::maxGraphID=0;

// Maximum ID assigned to any graph node
int graph::maxNodeID=0;

graph::graph(bool includeAllSubBlocks, properties* props) : 
  block("Graph", setProperties(maxGraphID, "", NULL, props)), includeAllSubBlocks(includeAllSubBlocks)
{ graphID=maxGraphID++; }

graph::graph(const attrOp& onoffOp, bool includeAllSubBlocks, properties* props) : 
  block("Graph", setProperties(maxGraphID, "", &onoffOp, props)), includeAllSubBlocks(includeAllSubBlocks)
{ graphID=maxGraphID++; }

graph::graph(anchor& pointsTo, bool includeAllSubBlocks, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, "", NULL, props)), includeAllSubBlocks(includeAllSubBlocks)
{ graphID=maxGraphID++; }

graph::graph(std::set<anchor>& pointsTo, const attrOp& onoffOp, bool includeAllSubBlocks, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, "", &onoffOp, props)), includeAllSubBlocks(includeAllSubBlocks)
{ graphID=maxGraphID++; }

graph::graph(string dotText,                                                          bool includeAllSubBlocks, properties* props) : 
  block("Graph", setProperties(maxGraphID, dotText, NULL, props)), includeAllSubBlocks(includeAllSubBlocks)
{ graphID=maxGraphID++; }

graph::graph(string dotText,                                   const attrOp& onoffOp, bool includeAllSubBlocks, properties* props) : 
  block("Graph", setProperties(maxGraphID, dotText, &onoffOp, props)), includeAllSubBlocks(includeAllSubBlocks)
{ graphID=maxGraphID++; }

graph::graph(string dotText, anchor& pointsTo,                                        bool includeAllSubBlocks, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, dotText, NULL, props)), includeAllSubBlocks(includeAllSubBlocks)
{ graphID=maxGraphID++; }

graph::graph(string dotText, std::set<anchor>& pointsTo,       const attrOp& onoffOp, bool includeAllSubBlocks, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, dotText, &onoffOp, props)), includeAllSubBlocks(includeAllSubBlocks)
{ graphID=maxGraphID++; }

// Sets the properties of this object
properties* graph::setProperties(int graphID, std::string dotText, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> pMap;
    pMap["graphID"] = txt()<<maxGraphID;
    if(dotText != "") pMap["dotText"] = dotText;
    //pMap["callPath"] = cp2str(CPRuntime.doStackwalk());
    props->add("graph", pMap);
  }
  else
    props->active = false;
  return props;
}

graph::~graph() {
}

// Given a reference to an object that can be represented as a dot graph,  create an image from it and add it to the output.
// Return the path of the image.
void graph::genGraph(dottable& obj) {
  graph g(obj.toDOT("graphsight"));
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
void graph::genGraph(std::string dotText) {
  graph g(dotText);
}

// Sets the structure of the current graph by specifying its dot encoding
void graph::setGraphEncoding(string dotText) {
  properties p;
  map<string, string> pMap;
  pMap["dot"] = dotText;
  p.add("graphEncoding", pMap);
  
  dbg.tag(p);
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void graph::addDirEdge(anchor from, anchor to) {
  // If the from or two node of this edge have not yet been emitted, do so now
  if(unEmittedNodes.find(from.getID()) != unEmittedNodes.end())
    emitNodeTag(from.getID(), unEmittedNodes[from.getID()].first, unEmittedNodes[from.getID()].second);
  if(unEmittedNodes.find(to.getID()) != unEmittedNodes.end())
    emitNodeTag(to.getID(), unEmittedNodes[to.getID()].first, unEmittedNodes[to.getID()].second);
  
  properties p;
  map<string, string> pMap;
  pMap["from"] = txt()<<from.getID();
  pMap["to"]   = txt()<<to.getID();
  pMap["graphID"] = txt()<<graphID;
  p.add("dirEdge", pMap);
  
  //dbg.tag("dirEdge", properties, false);
  dbg.tag(p);
}

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void graph::addUndirEdge(anchor a, anchor b) {
  // If the either node of this edge have not yet been emitted, do so now
  if(unEmittedNodes.find(a.getID()) != unEmittedNodes.end())
    emitNodeTag(a.getID(), unEmittedNodes[a.getID()].first, unEmittedNodes[a.getID()].second);
  if(unEmittedNodes.find(b.getID()) != unEmittedNodes.end())
    emitNodeTag(b.getID(), unEmittedNodes[b.getID()].first, unEmittedNodes[b.getID()].second);
  
  properties p;
  map<string, string> pMap;
  pMap["a"] = txt()<<a.getID();
  pMap["b"] = txt()<<b.getID();
  pMap["graphID"] = txt()<<graphID;
  p.add("undirEdge", pMap);
  //dbg.tag("undEdge", properties, false);
  
  dbg.tag(p);
}

/* ADD THIS IF WE WISH TO HAVE NODES THAT EXISTED INSIDE THE GRAPH BUT WERE NOT CONNECTED VIA EDGES*/
// Called to notify this block that a sub-block was started/completed inside of it. 
// Returns true of this notification should be propagated to the blocks 
// that contain this block and false otherwise.
bool graph::subBlockEnterNotify(block* subBlock) {
  // If we should include nodes for all sub-blocks inside this graph block
  if(includeAllSubBlocks)
    // Emit a node tag for this block immediately
    emitNodeTag(subBlock->getAnchor().getID(), subBlock->getLabel(), maxNodeID);
  else
    // Otherwise, save its info in unEmittedNodes[] so that we can emit it when we observe an edge that touches it
    unEmittedNodes[subBlock->getAnchor().getID()] = make_pair(subBlock->getLabel(), maxNodeID);
  
  maxNodeID++;
  
  return false;
}

// Emits a tag for the given node
void graph::emitNodeTag(int anchorID, std::string label, int nodeID) {
  properties p;
  map<string, string> pMap;
  pMap["nodeID"]   = txt()<<nodeID;
  pMap["anchorID"] = txt()<<anchorID;
  pMap["label"]    = label;
  pMap["graphID"]  = txt()<<graphID;
  //pMap["callPath"] = cp2str(CPRuntime.doStackwalk());
  p.add("node", pMap);

  dbg.tag(p);
}

GraphMerger::GraphMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        BlockMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props))
{
	assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "graph");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Graph!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Merge the graph IDs along all the streams
    GraphStreamRecord::enterGraph(tags, inStreamRecords);
      
    // Record the merged graphID within the outgoing stream's record
    int outGraphID = streamRecord::mergeIDs("graph", "graphID", pMap, tags, outStreamRecords, inStreamRecords);
    GraphStreamRecord::enterGraph(outGraphID, outStreamRecords);
      
    //cout << "GraphMerger::GraphMerger outStreamRecords
  } else {
    // Set of edges within the outgoing stream that correspond to the union of edges along all the input streams,
    // with no duplication
    set<GraphStreamRecord::streamGraphEdge> outEdges;
    
    // Iterate over all the edges that have been recorded for this graph, adding tags to moreTags for each edge
    for(vector<map<string, streamRecord*> >::iterator r=inStreamRecords.begin(); r!=inStreamRecords.end(); r++) {
      GraphStreamRecord* inGS = (GraphStreamRecord*)(*r)["graph"];
      
      // Get the graphID associated with the graph we're currently exiting, along the current incoming stream
      assert(inGS->gStack.size()>0);
      int inGraphID = inGS->gStack.back();
      
      // Iterate over all the edges collected for this graph and add them to the merged graph that will be emitted
      // on the outgoing stream
      const std::set<GraphStreamRecord::streamGraphEdge>& edges = ((GraphStreamRecord*)(*r)["graph"])->getEdges(inGraphID);
      for(std::set<GraphStreamRecord::streamGraphEdge>::const_iterator e=edges.begin(); e!=edges.end(); e++) {
        streamID outAnchorIDFrom = (*r)["anchor"]->in2outID(e->fromAnchor.getID());
        streamID outAnchorIDTo   = (*r)["anchor"]->in2outID(e->toAnchor.getID());
        //cout << "    e->fromAnchor="<<e->fromAnchor.str()<<", e->toAnchor="<<e->toAnchor.str()<<endl;
        //cout << "    outAnchorIDFrom="<<outAnchorIDFrom.str()<<", outAnchorIDTo="<<outAnchorIDTo.str()<<endl;
        outEdges.insert(GraphStreamRecord::streamGraphEdge(
                                 streamAnchor(outAnchorIDFrom, outStreamRecords),
                                 streamAnchor(outAnchorIDTo,   outStreamRecords),
                                 e->directed));
      }
    }
    
    GraphStreamRecord* outGS = (GraphStreamRecord*)outStreamRecords["graph"];
    
    // Get the graphID associated with the graph we're currently exiting, along the current outgoing stream
    assert(outGS->gStack.size()>0);
    int outGraphID = outGS->gStack.back();
      
    // Predefine the properties of exit tags to reduce the cost of emitting them, since they're all identical
    properties dirExitProps("dirEdge");
    properties undirExitProps("undirEdge");
    
    // Iterate over all the edges within the outgoing stream and add them to moreTags to be emitted as 
    // part of the merged graph in the outgoing stream
    for(set<GraphStreamRecord::streamGraphEdge>::iterator e=outEdges.begin(); e!=outEdges.end(); e++) {
      properties enterProps;
      map<string, string> edgePropsMap;
      edgePropsMap["from"]    = txt()<<e->fromAnchor.getID().ID;
      edgePropsMap["to"]      = txt()<<e->toAnchor.getID().ID;
      edgePropsMap["graphID"] = txt()<<outGraphID;
      enterProps.add(e->directed? "dirEdge": "undirEdge", edgePropsMap);
      moreTagsBefore.push_back(make_pair(properties::enterTag, enterProps));
      moreTagsBefore.push_back(make_pair(properties::exitTag, e->directed? dirExitProps: undirExitProps));
    }
    
    GraphStreamRecord::exitGraph(inStreamRecords);
    GraphStreamRecord::exitGraph(outStreamRecords);
  }
}


// Sets the properties of the merged object
properties* GraphMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "graph");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging graph!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Merge the graph IDs along all the streams
    streamRecord::mergeIDs("graph", "graphID", pMap, tags, outStreamRecords, inStreamRecords);
     
    pMap["dotText"] = "";
    for(std::vector<std::pair<properties::tagType, properties::iterator> >::iterator t=tags.begin(); t!=tags.end(); t++) {
      if(properties::exists(t->second, "dotText"))
        pMap["dotText"] += properties::get(t->second, "dotText");
    }
    // If the dotText field was not set on any of the incoming streams, remove it from pMap
    if(pMap["dotText"] == "") pMap.erase("dotText");
  }
  props->add("graph", pMap);
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void GraphMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  properties::iterator blockTag = tag;
  BlockMerger::mergeKey(type, ++blockTag, inStreamRecords, key);
}


GraphStreamRecord::GraphStreamRecord(const GraphStreamRecord& that, int vSuffixID) : 
  streamRecord(that, vSuffixID), edges(that.edges), gStack(that.gStack)
{ }

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* GraphStreamRecord::copy(int vSuffixID) {
  return new GraphStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void GraphStreamRecord::resumeFrom(std::vector<std::map<string, streamRecord*> >& streams) {
  streamRecord::resumeFrom(streams);
  
  // Set edges and gStack to be the union of its counterparts in streams
  edges.clear();
  gStack.clear();
  
  //cout << "GraphStreamRecord::resumeFrom()"<<endl;
  
  int idx=0;
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++, idx++) {
    GraphStreamRecord* gs = (GraphStreamRecord*)(*s)["graph"];
    
    // Union gs->edges into edges
    for(map<int, set<streamGraphEdge> >::const_iterator i=gs->edges.begin(); i!=gs->edges.end(); i++) {
      // If the union GraphStreamRecord does not contain edges for the given graphID, initialize it with an empty set
      if(i->first)
        edges[i->first] = set<streamGraphEdge>();
    
      // Fill the edges set for the current graphID in the union GraphStreamRecord with edges for this
      // graphID in the current variant's GraphStreamRecord
      for(set<streamGraphEdge>::iterator e=i->second.begin(); e!=i->second.end(); e++)
        edges[i->first].insert(*e);
    }
    
    /*cout << idx<<": gStack=[";
    for(list<int>::iterator i=gs->gStack.begin(); i!=gs->gStack.end(); i++) {
      if(i!=gStack.begin()) cout << " ";
      cout << *i;
    }
    cout << "]"<<endl;*/
    
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
  
  /*cout << "Union gStack=[";
  for(list<int>::iterator i=gStack.begin(); i!=gStack.end(); i++) {
    if(i!=gStack.begin()) cout << " ";
    cout << *i;
  }
  cout << "]"<<endl;*/
}

// Indicates that we've entered a graph (for incoming streams)
void GraphStreamRecord::enterGraph(std::vector<std::pair<properties::tagType, properties::iterator> > tags, 
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  assert(tags.size() == inStreamRecords.size());
  for(int i=0; i<tags.size(); i++) {
    int graphID = properties::getInt(tags[i].second, "graphID");
    GraphStreamRecord* gs = (GraphStreamRecord*)inStreamRecords[i]["graph"];
    gs->edges[graphID] = set<streamGraphEdge>();
    gs->gStack.push_back(graphID);
  }
}

// Indicates that we've entered a graph (for outgoing streams)
void GraphStreamRecord::enterGraph(int graphID,
                                   std::map<std::string, streamRecord*>& outStreamRecord) {
  GraphStreamRecord* gs = (GraphStreamRecord*)outStreamRecord["graph"];
  gs->gStack.push_back(graphID);
}

// Indicates that we've exited a graph (for incoming streams)
void GraphStreamRecord::exitGraph(std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  for(vector<map<std::string, streamRecord*> >::iterator r=inStreamRecords.begin(); r!=inStreamRecords.end(); r++) {
    GraphStreamRecord* gs = (GraphStreamRecord*)(*r)["graph"];
    assert(gs->gStack.size()>0);
    int graphID = gs->gStack.back();
    gs->gStack.pop_back();
    
    assert(gs->edges.find(graphID) != gs->edges.end());
    gs->edges.erase(graphID);
  }
}

// Indicates that we've exited a graph (for outgoing streams)
void GraphStreamRecord::exitGraph(std::map<std::string, streamRecord*>& outStreamRecord) {
  GraphStreamRecord* gs = (GraphStreamRecord*)outStreamRecord["graph"];
  assert(gs->gStack.size()>0);
  gs->gStack.pop_back();
}


// Adds an edge to the current graph
void GraphStreamRecord::addEdge(int graphID, streamGraphEdge edge) {
  assert(edges.find(graphID) != edges.end());
  edges[graphID].insert(edge);
}

// Returns a reference to all of the edges of the most deeply nested graph in this incoming stream
const std::set<GraphStreamRecord::streamGraphEdge>& GraphStreamRecord::getEdges(int graphID) const {
  std::map<int, std::set<streamGraphEdge> >::const_iterator edgeIt = edges.find(graphID);
  assert(edgeIt != edges.end());
  return edgeIt->second;
}

std::string GraphStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[GraphStreamRecord: ";
  s << streamRecord::str(indent+"    ") << endl;
  
  s << indent << "edges="<<endl;
  for(map<int, set<streamGraphEdge> >::const_iterator se=edges.begin(); se!=edges.end(); se++) {
    cout << se->first << ": "<<endl;
    for(set<streamGraphEdge>::const_iterator e=se->second.begin(); e!=se->second.end(); e++)
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

DirEdgeMerger::DirEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
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
      streamID fromInSID(properties::getInt(tags[i].second, "from"), inStreamRecords[i]["graph"]->getVariantID());
      streamID toInSID  (properties::getInt(tags[i].second, "to"),   inStreamRecords[i]["graph"]->getVariantID());
      
      /*cout << "DirEdgeMerger::DirEdgeMerger() "<<i<<": tags="<<properties::str(tags[i].second)<<endl;
      cout << "    fromInSID="<<fromInSID.str()<<", toInSID="<<toInSID.str()<<endl;*/
      
      ((GraphStreamRecord*)(*r)["graph"])->addEdge(properties::getInt(tags[i].second, "graphID"),
                                                   GraphStreamRecord::streamGraphEdge(
                                                              streamAnchor(fromInSID, inStreamRecords[i]),
                                                              streamAnchor(toInSID,   inStreamRecords[i]),
                                                              true));
    }
  }
  
  // We do not emit edge tags until we're done reading the graph
  dontEmit();
}

UndirEdgeMerger::UndirEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
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
      streamID aInSID(properties::getInt(tags[i].second, "a"), inStreamRecords[i]["graph"]->getVariantID());
      streamID bInSID(properties::getInt(tags[i].second, "b"), inStreamRecords[i]["graph"]->getVariantID());
      ((GraphStreamRecord*)(*r)["graph"])->addEdge(properties::getInt(tags[i].second, "graphID"),
                                                   GraphStreamRecord::streamGraphEdge(
                                                              streamAnchor(aInSID, inStreamRecords[i]),
                                                              streamAnchor(bInSID, inStreamRecords[i]),
                                                              false));
    }
    
    // Merge the graphIDs along all the streams
    streamRecord::mergeIDs("graph", "graphID", pMap, tags, outStreamRecords, inStreamRecords);
  }
  
  // We do not emit edge tags until we're done reading the graph
  dontEmit();
}

NodeMerger::NodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
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
    streamRecord::mergeIDs("graph", "graphID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // Merge the node IDs along all the streams
    //NodeStreamRecord::mergeIDs(pMap, tags, outStreamRecords, inStreamRecords);
    streamRecord::mergeIDs("node", "nodeID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // Merge the IDs of the anchors that target the nodes
    //AnchorStreamRecord::mergeIDs("node", pMap, tags, outStreamRecords, inStreamRecords);
    streamRecord::mergeIDs("anchor", "anchorID", pMap, tags, outStreamRecords, inStreamRecords);
    
    pMap["label"] = getMergedValue(tags, "label");
    
    /*vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();*/
  }
  
  props->add("node", pMap);
}

NodeStreamRecord::NodeStreamRecord(const NodeStreamRecord& that, int vSuffixID) : 
  streamRecord(that, vSuffixID), maxNodeID(that.maxNodeID), in2outNodeIDs(that.in2outNodeIDs)
{ }

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* NodeStreamRecord::copy(int vSuffixID) {
  return new NodeStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
/*void NodeStreamRecord::resumeFrom(std::vector<std::map<string, streamRecord*> >& streams) {
  // Compute the maximum maxNodeID among all the streams
  maxNodeID = -1;
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++)
    maxNodeID = (((NodeStreamRecord*)(*s)["node"])->maxNodeID > maxNodeID? 
                   ((NodeStreamRecord*)(*s)["node"])->maxNodeID: 
                   maxNodeID);
  
  // Set edges and in2outNodeIDs to be the union of its counterparts in streams
  in2outNodeIDs.clear();
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    NodeStreamRecord* ns = (NodeStreamRecord*)(*s)["node"];
    for(map<streamID, streamID>::const_iterator i=ns->in2outNodeIDs.begin(); i!=ns->in2outNodeIDs.end(); i++)
      in2outNodeIDs.insert(*i);
  }
}*/

// Marge the IDs of the next Node (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
// updating each incoming stream's mappings from its IDs to the outgoing stream's IDs
/*void NodeStreamRecord::mergeIDs(std::map<std::string, std::string>& pMap, 
                                const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                                std::map<std::string, streamRecord*>& outStreamRecords,
                                std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  // Assign to the merged Node the next ID for this output stream
  pMap["nodeID"] = txt()<<((NodeStreamRecord*)outStreamRecords["node"])->maxNodeID;
 
  // The graph's ID within the outgoing stream    
  streamID outSID(((NodeStreamRecord*)outStreamRecords["node"])->maxNodeID,
                  outStreamRecords["node"]->getVariantID()); 
  
  // Update inStreamRecords to map the Node's ID within each incoming stream to the assigned ID in the outgoing stream
  for(int i=0; i<tags.size(); i++) {
    // The graph's ID within the current incoming stream
    streamID inSID(properties::getInt(tags[i].second, "nodeID"), 
                   inStreamRecords[i]["node"]->getVariantID());
    
    ((NodeStreamRecord*)inStreamRecords[i]["node"])->in2outNodeIDs[inSID] = outSID;
  }
  
  // Advance maxBlockID
  ((NodeStreamRecord*)outStreamRecords["node"])->maxNodeID++;
}*/

std::string NodeStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[NodeStreamRecord: ";
  s << streamRecord::str(indent+"    ") << "]";
  /*maxNodeID="<<maxNodeID<<endl;
  
  s << indent << "in2outNodeIDs="<<endl;
  for(map<streamID, streamID>::const_iterator i=in2outNodeIDs.begin(); i!=in2outNodeIDs.end(); i++)
    s << indent << "    "<<i->first.str()<<" =&gt; "<<i->second.str()<<endl;
  
  s << indent << "]";*/
  
  return s.str();
}


}; // namespace structure
}; // namespace sight
