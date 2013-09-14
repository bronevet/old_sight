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

graph::graph() : block("Graph") {
  init(NULL);
}

graph::graph(const attrOp& onoffOp) : block("Graph") {
  init(&onoffOp);
}

graph::graph(const anchor& pointsTo) : block("Graph", pointsTo) {
  init(NULL);
}

graph::graph(const std::set<anchor>& pointsTo, const attrOp& onoffOp) : block("Graph", pointsTo) {
  init(&onoffOp);
}

void graph::init(const attrOp* onoffOp) {
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp==NULL || onoffOp->apply())) {
    map<string, string> properties;
    dbg.enter("graph", properties);
  } else
    active = false; 
}

graph::~graph() {
  if(!active) return;
  
  dbg.exit("graph");
}

// Given a reference to an object that can be represented as a dot graph,  create an image from it and add it to the output.
// Return the path of the image.
void graph::genGraph(dottable& obj) {
  map<string, string> properties;
  properties["dot"] = obj.toDOT("graphDbgLog");
  dbg.tag("graph", properties);
}

// Given a representation of a graph in dot format, create an image from it and add it to the output.
// Return the path of the image.
void graph::genGraph(std::string dot) {
  map<string, string> properties;
  properties["dot"] = dot;
  dbg.tag("graph", properties);
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void graph::addDirEdge(anchor from, anchor to) {
  map<string, string> properties;
  properties["from"] = txt()<<from.getID();
  properties["to"]   = txt()<<to.getID();
  dbg.tag("dirEdge", properties);
}

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void graph::addUndirEdge(anchor a, anchor b) {
  map<string, string> properties;
  properties["a"] = txt()<<a.getID();
  properties["b"] = txt()<<b.getID();
  dbg.tag("undEdge", properties);
}

}; // namespace structure
}; // namespace dbglog
