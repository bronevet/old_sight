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

graph::graph(properties* props) : 
  block("Graph", setProperties(maxGraphID, "", NULL, props)) { graphID=++maxGraphID; }
/*  init(NULL, inheritedFrom);
}*/

graph::graph(const attrOp& onoffOp, properties* props) : 
  block("Graph", setProperties(maxGraphID, "", &onoffOp, props)) { graphID=++maxGraphID; }
/*  init(&onoffOp, inheritedFrom);
}*/

graph::graph(anchor& pointsTo, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, "", NULL, props)) { graphID=++maxGraphID; }
/*  init(NULL, inheritedFrom);
}*/

graph::graph(std::set<anchor>& pointsTo, const attrOp& onoffOp, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, "", &onoffOp, props)) { graphID=++maxGraphID; }
/*  init(&onoffOp);
}*/

graph::graph(string dotText,                                                          properties* props) : 
  block("Graph", setProperties(maxGraphID, dotText, NULL, props)) { graphID=++maxGraphID; }

graph::graph(string dotText,                                   const attrOp& onoffOp, properties* props) : 
  block("Graph", setProperties(maxGraphID, dotText, &onoffOp, props)) { graphID=++maxGraphID; }

graph::graph(string dotText, anchor& pointsTo,                                  properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, dotText, NULL, props)) { graphID=++maxGraphID; }

graph::graph(string dotText, std::set<anchor>& pointsTo, const attrOp& onoffOp, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, dotText, &onoffOp, props)) { graphID=++maxGraphID; }

// Sets the properties of this object
properties* graph::setProperties(int graphID, std::string dotText, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    newProps["graphID"] = txt()<<graphID;
    if(dotText != "") newProps["dotText"] = dotText;
    newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
    props->add("graph", newProps);
  }
  else
    props->active = false;
  return props;
}

/*void graph::init(const attrOp* onoffOp, properties* props) {
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp==NULL || onoffOp->apply())) {
    map<string, string> properties;
    dbg.enter("graph", properties, inheritedFrom);
  } else
    active = false; 
}*/

graph::~graph() {
  /*if(props->active) {
    dbg.exit(this);
  }*/
}

// Given a reference to an object that can be represented as a dot graph,  create an image from it and add it to the output.
// Return the path of the image.
void graph::genGraph(dottable& obj) {
  graph g(obj.toDOT("graphsight"));
  dbg.tag(&g);
  /*map<string, string> properties;
  // !!! SHOULD CREATE A graph OBJECT !!!
  properties["dot"] = obj.toDOT("graphsight");
  dbg.tag("graph", properties, false);*/
  //assert(0);
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
void graph::genGraph(std::string dotText) {
  graph g(dotText);
  dbg.tag(&g);
  /*map<string, string> properties;
  // !!! SHOULD CREATE A graph OBJECT !!!
  properties["dot"] = dot;
  dbg.tag("graph", properties, false);*/
  //assert(0);
}

// Sets the structure of the current graph by specifying its dot encoding
void graph::setGraphEncoding(string dotText) {
  sightObj obj(new properties());
  
  map<string, string> newProps;
  newProps["dot"] = dotText;
  obj.props->add("graphEncoding", newProps);
  
  //dbg.tag("dirEdge", properties, false);
  dbg.tag(&obj);
  
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void graph::addDirEdge(anchor from, anchor to) {
  sightObj obj(new properties());
  
  map<string, string> newProps;
  newProps["from"] = txt()<<from.getID();
  newProps["to"]   = txt()<<to.getID();
  obj.props->add("dirEdge", newProps);
  
  //dbg.tag("dirEdge", properties, false);
  dbg.tag(&obj);
}

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void graph::addUndirEdge(anchor a, anchor b) {
  sightObj obj(new properties());
  
  map<string, string> newProps;
  newProps["a"] = txt()<<a.getID();
  newProps["b"] = txt()<<b.getID();
  obj.props->add("undirEdge", newProps);
  //dbg.tag("undEdge", properties, false);
  
  dbg.tag(&obj);
}

/* ADD THIS IF WE WISH TO HAVE NODES THAT EXISTED INSIDE THE GRAPH BUT WERE NOT CONNECTED VIA EDGES*/
// Called to notify this block that a sub-block was started/completed inside of it. 
// Returns true of this notification should be propagated to the blocks 
// that contain this block and false otherwise.
bool graph::subBlockEnterNotify(block* subBlock) {
  //cout << "graph::subBlockEnterNotify(subBlock="<<subBlock->getLabel()<<")\n";
  // If this block is immediately contained inside this graph
//  location common = dbgStream::commonSubLocation(getLocation(), subBlock->getLocation());
  
  /*cout << "subBlock->getLocation().back().second.size()="<<subBlock->getLocation().back().second.size()<<" common.back().second.size()="<<common.back().second.size()<<endl;
  cout << "subBlock->getLocation="<<dbg.blockGlobalStr(subBlock->getLocation())<<endl;*/
  // If subBlock is nested immediately inside the graph's block 
//???  // (the nesting gap is 2 blocks rather than 1 since each block is chopped up into sub-blocks that
//???  //  span the text between adjacent attribute definitions and major block terminal points)
//  assert(subBlock->getLocation().size()>0);
/*  if(common == getLocation() &&
     subBlock->getLocation().size() == common.size()/ * &&
     subBlock->getLocation().back().second.size()-1 == common.back().second.size() * /)*/
  { 
    properties p;
    map<string, string> newProps;
    newProps["nodeID"]   = txt()<<maxNodeID;
    newProps["anchorID"] = txt()<<subBlock->getAnchor().getID();
    newProps["label"]    = txt()<<subBlock->getLabel();
    newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
    p.add("node", newProps);
  //dbg.tag("undEdge", properties, false);
  
    dbg.tag(p);
   //nodes[subBlock->getLocation()] = node(maxNodeID, subBlock->getLabel(), subBlock->getAnchor());
    maxNodeID++;
  }
  
  return false;
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

  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Graph!"<<endl; exit(-1); }
  if(type==properties::enterTag) {    
    GraphStreamRecord::enterGraph(inStreamRecords);
  } else {
    // Set of edges within the outgoing stream that correspond to the union of edges along all the input streams,
    // with no duplication
    set<GraphStreamRecord::streamGraphEdge> outEdges;
    
    // Iterate over all the edges that have been recorded for this graph, adding tags to moreTags for each edge
    for(vector<map<string, streamRecord*> >::iterator r=inStreamRecords.begin(); r!=inStreamRecords.end(); r++) {
      const std::set<GraphStreamRecord::streamGraphEdge>& edges = ((GraphStreamRecord*)(*r)["graph"])->getEdges();
      for(std::set<GraphStreamRecord::streamGraphEdge>::const_iterator e=edges.begin(); e!=edges.end(); e++) {
        streamID outAnchorIDFrom = (*r)["anchor"]->in2outID(e->fromAnchor.getID());
        streamID outAnchorIDTo   = (*r)["anchor"]->in2outID(e->toAnchor.getID());
        
        outEdges.insert(GraphStreamRecord::streamGraphEdge(
                                 streamAnchor(outAnchorIDFrom, outStreamRecords),
                                 streamAnchor(outAnchorIDTo,   outStreamRecords),
                                 e->directed));
      }
    }

    // Predefine the properties of exit tags to reduce the cost of emitting them, since they're all identical
    properties dirExitProps("dirEdge");
    properties undirExitProps("undirEdge");
    
    // Iterate over all the edges within the outgoing stream and add them to moreTags to be emitted as 
    // part of the merged graph in the outgoing stream
    for(set<GraphStreamRecord::streamGraphEdge>::iterator e=outEdges.begin(); e!=outEdges.end(); e++) {
      properties enterProps;
      map<string, string> edgePropsMap;
      edgePropsMap["from"] = txt()<<e->fromAnchor.getID().ID;
      edgePropsMap["to"]   = txt()<<e->toAnchor.getID().ID;
      enterProps.add(e->directed? "dirEdge": "undirEdge", edgePropsMap);
      moreTagsBefore.push_back(make_pair(properties::enterTag, enterProps));
      moreTagsBefore.push_back(make_pair(properties::exitTag, e->directed? dirExitProps: undirExitProps));
    }
    
    GraphStreamRecord::exitGraph(inStreamRecords);
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
  streamRecord(that, vSuffixID), /*maxGraphID(that.maxGraphID), */edges(that.edges)/*, in2outGraphIDs(that.in2outGraphIDs)*/
{ }

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* GraphStreamRecord::copy(int vSuffixID) {
  return new GraphStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void GraphStreamRecord::resumeFrom(std::vector<std::map<string, streamRecord*> >& streams) {
  /* // Compute the maximum maxGraphID among all the streams
  maxGraphID = -1;
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++)
    maxGraphID = (((GraphStreamRecord*)(*s)["graph"])->maxGraphID > maxGraphID? 
                   ((GraphStreamRecord*)(*s)["graph"])->maxGraphID: 
                   maxGraphID);*/
  streamRecord::resumeFrom(streams);
  
  // Set edges and in2outGraphIDs to be the union of its counterparts in streams
  edges.clear();
  //in2outGraphIDs.clear();
  
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    GraphStreamRecord* gs = (GraphStreamRecord*)(*s)["graph"];
    
    list<set<streamGraphEdge> >::iterator outIt=edges.begin();
    list<set<streamGraphEdge> >::const_iterator inIt=gs->edges.begin();
    while(inIt!=gs->edges.end()) {
      // If edges within the outgoing stream is not as deep as edges within the incoming stream, 
      // add an edges set to it and update outIt to point to this new element
      if(outIt == edges.end()) {
        edges.push_back(set<streamGraphEdge>());
        outIt = --edges.end();
      }
      
      for(set<streamGraphEdge>::const_iterator e=inIt->begin(); e!=inIt->end(); e++)
        outIt->insert(*e);
      
      outIt++;
      inIt++;
    }
    
    /*for(map<streamID, streamID>::const_iterator i=gs->in2outGraphIDs.begin(); i!=gs->in2outGraphIDs.end(); i++)
      in2outGraphIDs.insert(*i);*/
  }
}

// Marge the IDs of the next graph (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
// updating each incoming stream's mappings from its IDs to the outgoing stream's IDs
/*void GraphStreamRecord::mergeIDs(std::map<std::string, std::string>& pMap, 
                                 const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                                 std::map<std::string, streamRecord*>& outStreamRecords,
                                 std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  // Assign to the merged graph the next ID for this output stream
  pMap["graphID"] = txt()<<((GraphStreamRecord*)outStreamRecords["graph"])->maxGraphID;
  
  // The graph's ID within the outgoing stream    
  streamID outSID(((GraphStreamRecord*)outStreamRecords["graph"])->maxGraphID,
                  outStreamRecords["graph"]->getVariantID());
  
  // Update inStreamRecords to map the graph's ID within each incoming stream to the assigned ID in the outgoing stream
  for(int i=0; i<tags.size(); i++) {
    // The graph's ID within the current incoming stream
    streamID inSID(properties::getInt(tags[i].second, "graphID"), 
                   inStreamRecords[i]["graph"]->getVariantID());
    
    ((GraphStreamRecord*)inStreamRecords[i]["graph"])->in2outGraphIDs[inSID] = outSID;
  }
  
  // Advance maxBlockID
  ((GraphStreamRecord*)outStreamRecords["graph"])->maxGraphID++;
}*/

// Indicates that we've entered/exited a graph
void GraphStreamRecord::enterGraph(std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  for(vector<map<std::string, streamRecord*> >::iterator r=inStreamRecords.begin(); r!=inStreamRecords.end(); r++) {
    GraphStreamRecord* gs = (GraphStreamRecord*)(*r)["graph"];
    gs->edges.push_back(set<streamGraphEdge>());
  }
}

void GraphStreamRecord::exitGraph(std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  for(vector<map<std::string, streamRecord*> >::iterator r=inStreamRecords.begin(); r!=inStreamRecords.end(); r++) {
    GraphStreamRecord* gs = (GraphStreamRecord*)(*r)["graph"];
    assert(gs->edges.size()>0);
    gs->edges.pop_back();
  }
}

// Adds an edge to the current graph
void GraphStreamRecord::addEdge(streamGraphEdge edge) {
  assert(edges.size()>0);
  edges.back().insert(edge);
}

// Returns a reference to all of the edges of the most deeply nested graph in this incoming stream
const std::set<GraphStreamRecord::streamGraphEdge>& GraphStreamRecord::getEdges() const {
  assert(edges.size()>0);
  return edges.back();
}

std::string GraphStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[GraphStreamRecord: ";
  s << streamRecord::str(indent+"    ") << endl;
  /*maxGraphID="<<maxGraphID<<endl;
  
  s << indent << "in2outGraphIDs="<<endl;
  for(map<streamID, streamID>::const_iterator i=in2outGraphIDs.begin(); i!=in2outGraphIDs.end(); i++)
    s << indent << "    "<<i->first.str()<<" =&gt; "<<i->second.str()<<endl;
  */
  s << indent << "edges="<<endl;
  int i=0; 
  for(list<set<streamGraphEdge> >::const_iterator se=edges.begin(); se!=edges.end(); se++, i++)
    for(set<streamGraphEdge>::const_iterator e=se->begin(); e!=se->end(); e++)
      s << indent << "    "<<i<<": "<<e->str(indent+"            ")<<endl;
  
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
      ((GraphStreamRecord*)(*r)["graph"])->addEdge(GraphStreamRecord::streamGraphEdge(
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
      ((GraphStreamRecord*)(*r)["graph"])->addEdge(GraphStreamRecord::streamGraphEdge(
                                                              streamAnchor(aInSID, inStreamRecords[i]),
                                                              streamAnchor(bInSID, inStreamRecords[i]),
                                                              false));
    }
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
    
    // Merge the node IDs along all the streams
    //NodeStreamRecord::mergeIDs(pMap, tags, outStreamRecords, inStreamRecords);
    streamRecord::mergeIDs("node", "nodeID", pMap, tags, outStreamRecords, inStreamRecords);
      
    // Merge the IDs of the anchors that target the nodes
    //AnchorStreamRecord::mergeIDs("node", pMap, tags, outStreamRecords, inStreamRecords);
    streamRecord::mergeIDs("anchor", "anchorID", pMap, tags, outStreamRecords, inStreamRecords);
    
    pMap["label"] = getMergedValue(tags, "label");
    
    vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();
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

// Given multiple streamRecords from several variants of the same stream, update this streamRecord object
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
