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

graph::graph(const anchor& pointsTo, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, "", NULL, props)) { graphID=++maxGraphID; }
/*  init(NULL, inheritedFrom);
}*/

graph::graph(const std::set<anchor>& pointsTo, const attrOp& onoffOp, properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, "", &onoffOp, props)) { graphID=++maxGraphID; }
/*  init(&onoffOp);
}*/

graph::graph(string dotText,                                                          properties* props) : 
  block("Graph", setProperties(maxGraphID, dotText, NULL, props)) { graphID=++maxGraphID; }

graph::graph(string dotText,                                   const attrOp& onoffOp, properties* props) : 
  block("Graph", setProperties(maxGraphID, dotText, &onoffOp, props)) { graphID=++maxGraphID; }

graph::graph(string dotText, const anchor& pointsTo,                                  properties* props) : 
  block("Graph", pointsTo, setProperties(maxGraphID, dotText, NULL, props)) { graphID=++maxGraphID; }

graph::graph(string dotText, const std::set<anchor>& pointsTo, const attrOp& onoffOp, properties* props) : 
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
    sightObj obj(new properties());
  
    map<string, string> newProps;
    newProps["nodeID"]   = txt()<<maxNodeID;
    newProps["anchorID"] = txt()<<subBlock->getAnchor().getID();
    newProps["label"]    = txt()<<subBlock->getLabel();
    obj.props->add("node", newProps);
  //dbg.tag("undEdge", properties, false);
  
    dbg.tag(&obj);
   //nodes[subBlock->getLocation()] = node(maxNodeID, subBlock->getLabel(), subBlock->getAnchor());
    maxNodeID++;
  }
  
  return false;
}

int GraphMerger::maxGraphID=0;

GraphMerger::GraphMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags) : BlockMerger(advance(tags)) {
  assert(tags.size()>0);
  set<string> names = getNameSet(tags);
  assert(names.size()==1);
  assert(*names.begin() == "graph");
  map<string, string> pMap;
  
  pMap["graphID"] = txt()<<maxGraphID++;
  
  set<string> dotTextSet;
  for(std::vector<std::pair<properties::tagType, properties::iterator> >::iterator t=tags.begin(); t!=tags.end(); t++) {
    if(properties::exists(t->second, "dotText"))
      dotTextSet.insert(properties::get(t->second, "dotText"));
  }
  if(dotTextSet.size()>0)
  pMap["dotText"] = *dotTextSet.begin();
  
  props->add("graph", pMap);
}

DirEdgeMerger::DirEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags) : Merger(advance(tags)) {
  assert(tags.size()>0);
  set<string> names = getNameSet(tags);
  assert(names.size()==1);
  assert(*names.begin() == "dirEdge");
  map<string, string> pMap;
  
  pMap["from"] = txt()<<0; // setAvg(str2intSet(getValueSet(tags, "from")));
  pMap["to"] = txt()<<0; // setAvg(str2intSet(getValueSet(tags, "from")));
  
  props->add("dirEdge", pMap);
}

UndirEdgeMerger::UndirEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags) : Merger(advance(tags)) {
  assert(tags.size()>0);
  set<string> names = getNameSet(tags);
  assert(names.size()==1);
  assert(*names.begin() == "undirEdge");
  map<string, string> pMap;
  
  pMap["a"] = txt()<<0; // setAvg(str2intSet(getValueSet(tags, "a")));
  pMap["b"] = txt()<<0; // setAvg(str2intSet(getValueSet(tags, "b")));
  
  props->add("undirEdge", pMap);
}

NodeMerger::NodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags) : Merger(advance(tags)) {
  assert(tags.size()>0);
  set<string> names = getNameSet(tags);
  assert(names.size()==1);
  assert(*names.begin() == "node");
  map<string, string> pMap;

  set<long> nodeIDSet = str2intSet(getValueSet(tags, "nodeID"));
  assert(nodeIDSet.size()==1);
  pMap["nodeID"] = txt()<<(*nodeIDSet.begin());
  
  pMap["anchorID"] = txt()<<0; // setAvg(str2intSet(getValueSet(tags, "anchorID")));
  
  set<string> labelSet = getValueSet(tags, "label");
  pMap["label"] = *labelSet.begin();
  
  props->add("node", pMap);
}

}; // namespace structure
}; // namespace sight
