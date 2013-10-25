#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../attributes_structure.h"
#include "../dbglog_structure_internal.h"

namespace dbglog {
namespace structure {

class dottable
{
  public:
  virtual ~dottable() {}
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  virtual std::string toDOT(std::string graphName)=0;
};

class graph: public structure::block
{
  // Unique ID of this graph object
  int graphID;
  
  // Maximum ID assigned to any graph object
  static int maxGraphID;

  // Maximum ID assigned to any graph node
  static int maxNodeID;
  
  // Records whether this scope is included in the emitted output (true) or not (false)
  bool active;
  
  public:
  
  graph(                                                                              properties* props=NULL);
  graph(                                                       const attrOp& onoffOp, properties* props=NULL);
  graph(                     const anchor& pointsTo,                                  properties* props=NULL);
  graph(                     const std::set<anchor>& pointsTo, const attrOp& onoffOp, properties* props=NULL);
  graph(std::string dotText,                                                          properties* props=NULL);
  graph(std::string dotText,                                   const attrOp& onoffOp, properties* props=NULL);
  graph(std::string dotText, const anchor& pointsTo,                                  properties* props=NULL);
  graph(std::string dotText, const std::set<anchor>& pointsTo, const attrOp& onoffOp, properties* props=NULL);
    
  private:
  // Sets the properties of this object
  static properties* setProperties(int graphID, std::string dotText, const attrOp* onoffOp, properties* props);
  
  //void init(const attrOp* onoffOp, properties* props);
  
  public:
  ~graph();
  
  // Given a reference to an object that can be represented as a dot graph,  create an image from it and add it to the output.
  // Return the path of the image.
  static void genGraph(dottable& obj);

  // Given a representation of a graph in dot format, create an image from it and add it to the output.
  // Return the path of the image.
  static void genGraph(std::string dot);
  
  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();
  
  // Add a directed edge from the location of the from anchor to the location of the to anchor
  void addDirEdge(anchor from, anchor to);
  
  // Add an undirected edge between the location of the a anchor and the location of the b anchor
  void addUndirEdge(anchor a, anchor b);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock);// { return false; }
  bool subBlockExitNotify (block* subBlock) { return false; }
};

}; // namespace structure
}; // namespace dbglog
