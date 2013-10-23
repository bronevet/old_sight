// Licence information included in file LICENCE
#include "../dbglog_structure_internal.h"
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

namespace dbglog {
namespace structure {

// Maximum ID assigned to any graph object
int graph::maxGraphID=0;

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
  graph g(obj.toDOT("graphDbgLog"));
  dbg.tag(&g);
  /*map<string, string> properties;
  // !!! SHOULD CREATE A graph OBJECT !!!
  properties["dot"] = obj.toDOT("graphDbgLog");
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
  dbglogObj *obj = new dbglogObj(new properties());
  
  map<string, string> newProps;
  newProps["from"] = txt()<<from.getID();
  newProps["to"]   = txt()<<to.getID();
  obj->props->add("dirEdge", newProps);
  
  //dbg.tag("dirEdge", properties, false);
  dbg.tag(obj);
  
  delete(obj);
}

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void graph::addUndirEdge(anchor a, anchor b) {
  dbglogObj *obj = new dbglogObj(new properties());
  
  map<string, string> newProps;
  newProps["a"] = txt()<<a.getID();
  newProps["b"] = txt()<<b.getID();
  obj->props->add("undirEdge", newProps);
  //dbg.tag("undEdge", properties, false);
  
  dbg.tag(obj);
  
  delete(obj);
}

}; // namespace structure
}; // namespace dbglog
