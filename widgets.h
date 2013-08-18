#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "attributes.h"

namespace dbglog {

class scope: public block
{
  static std::vector<std::string> colors;
  static int colorIdx; // The current index into the list of colors 
  
  public:
  // Records whether this scope is included in the emitted output (true) or not (false)
  bool active;
  typedef enum {high, medium, low, min} scopeLevel;
  scopeLevel level;

  // label - the label associated with this scope.
  // pointsTo - the anchor(s) that terminate at this scope. Used to target links to scopes.
  // level - the type of visualization used, with higher levels associated with more amounts of debug output
  //    There are several features that are enabled by the levels:
  //       own_file: Text inside the scope is written to a separate file. Users must manually click a button to see it.
  //       own_color: The background color of the scope is different from its parent scope.
  //       label_shown: The label of this scope is shown in a larger font, along with controls to minimize the scope 
  //              by clicking on the label and open GDB to the point in the execution where the scope started
  //    Different levels
  //    high: own_file, own_color, label_shown
  //    medium: own_color, label_shown
  //    low: label_shown
  //    min: none of the above
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  scope(std::string label,                                   scopeLevel level, const attrOp& onoffOp);
  scope(std::string label, const anchor& pointsTo,           scopeLevel level, const attrOp& onoffOp);
  scope(std::string label, const std::set<anchor>& pointsTo, scopeLevel level, const attrOp& onoffOp);
  scope(std::string label,                                                     const attrOp& onoffOp);
  scope(std::string label, const anchor& pointsTo,                             const attrOp& onoffOp);
  scope(std::string label, const std::set<anchor>& pointsTo,                   const attrOp& onoffOp);
  scope(std::string label,                                   scopeLevel level=medium);
  scope(std::string label, const anchor& pointsTo,           scopeLevel level=medium);
  scope(std::string label, const std::set<anchor>& pointsTo, scopeLevel level=medium);

  
  private:
  // Common initialization code
  void init(scopeLevel level, const attrOp* onoffOp);
  
  public:
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
  
  // Called to enable the block to print its entry and exit text
  void printEntry(std::string loadCmd);
  void printExit();
  
  ~scope();
}; // scope

class graph;

class graphEdge {
  anchor from;
  anchor to;
  bool directed;
  
  friend class graph;
  
  public:
  graphEdge(anchor from, anchor to, bool directed) :
    from(from), to(to), directed(directed)
  {}
  
  const anchor& getFrom() { return from; }
  const anchor& getTo()   { return to; }
  
  bool operator<(const graphEdge& that) const {
    return (from < that.from) ||
           (from == that.from && to < that.to) ||
           (from == that.from && to == that.to && directed < that.directed);
  }
};

class dottable
{
  public:
  virtual ~dottable() {}
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  virtual std::string toDOT(std::string graphName)=0;
};

class graph: public block
{
  protected:
  class node {
    public:
    int ID;
    std::string label;
    anchor a;
      
    node() : ID(-1), label("") {}
    node(int ID, std::string label, const anchor& a) : ID(ID), label(label), a(a) {}
  };
  
  // Maps a block's location to its ID and label
  std::map<location, node> nodes;
  // The maximum ID associated with any node in this graph
  int maxNodeID;
  
  std::list<graphEdge> edges;
  
  // The path the directory where output files of the graph widget are stored
  // Relative to current path
  static std::string outDir;
  // Relative to root of HTML document
  static std::string htmlOutDir;
    
  // Unique ID of this graph object
  int widgetID;
  
  // Maximum ID assigned to any graph object
  static int maxWidgetID;
  
  // Records whether this graph has already been output by a call to outputCanvizDotGraph()
  bool graphOutput;
  
  // Records whether this scope is included in the emitted output (true) or not (false)
  bool active;
  
  public:
  
  graph();
  graph(const attrOp& onoffOp);
  void init();
  ~graph();

  // Generates and returns the dot graph code for this graphgenDotGraph
  virtual std::string genDotGraph();
  
  // Given a string representation of a dot graph, emits the graph's visual representation 
  // as a Canviz widget into the debug output.
  void outputCanvizDotGraph(std::string dot);
  
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
  bool subBlockEnterNotify(block* subBlock);
  bool subBlockExitNotify (block* subBlock);
};

} // namespace dbglog
