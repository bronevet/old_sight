#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../sight_structure_internal.h"
#include "Callpath.h"

namespace sight {
namespace structure {

typedef common::flowgraphEdge<anchor> flowgraphEdge;

class flowgraph: public structure::block
{
  // Unique ID of this graph object
  int flowgraphID;

  // Maximum ID assigned to any graph object
  static ThreadLocalStorage1<int, int> maxFlowGraphID;

  // Maximum ID assigned to any graph node
  static ThreadLocalStorage1<int, int> maxNodeID;
  
  // Records whether this scope is included in the emitted output (true) or not (false)
  bool active;
  
  // Records whether we should include a graph node for each sub-block inside the graph
  bool includeAllSubBlocks;
  
  // Maps the anchor IDs of nodes that have been observed.
  // This is useful for cases where includeAllSubBlocks and thus, we only emit a node if we observe an edge that 
  // involves it.
  std::map<int, std::pair<std::string, int> > nodesObservedNotEmitted;
  
  // Set of the anchorIDs of the nodes that have been connected by edges.
  std::set<anchor> nodesConnected;
  
  public:
  
  flowgraph(                                                                        bool includeAllSubBlocks=false, properties* props=NULL);
  flowgraph(                                                 const attrOp& onoffOp, bool includeAllSubBlocks=false, properties* props=NULL);
  flowgraph(                     anchor& pointsTo,                                  bool includeAllSubBlocks=false, properties* props=NULL);
  flowgraph(                     std::set<anchor>& pointsTo, const attrOp& onoffOp, bool includeAllSubBlocks=false, properties* props=NULL);
  flowgraph(std::string dataText,                                                    bool includeAllSubBlocks=false, properties* props=NULL);
  flowgraph(std::string dataText,                             const attrOp& onoffOp, bool includeAllSubBlocks=false, properties* props=NULL);
  flowgraph(std::string dataText, anchor& pointsTo,                                  bool includeAllSubBlocks=false, properties* props=NULL);
  flowgraph(std::string dataText, std::set<anchor>& pointsTo, const attrOp& onoffOp, bool includeAllSubBlocks=false, properties* props=NULL);
    
  private:
  // Sets the properties of this object
  static properties* setProperties(int flowgraphID, std::string dataText, const attrOp* onoffOp, properties* props);
  
  //void init(const attrOp* onoffOp, properties* props);
  
  public:
  ~flowgraph();
  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();

  // Given a representation of a graph in data text format, create an image from it and add it to the output.
  // Return the path of the image.
  void genFlowGraph(std::string data);
  void startGraph();
  void addNode(std::string nodeName);
  void addNode(std::string childNode, std::string parentNode);
  void addEdge(std::string startNode, std::string endNode);
  void endGraph();

  void graphNodeStart(std::string nodeName);
    void graphNodeStart(std::string nodeName, std::string texNode);
    void graphNodeStart(std::string nodeName, int verID, int horID);
  void graphNodeStart(std::string nodeName, std::string texNode, int verID, int horID);
  void graphNodeEnd(std::string nodeName);
  void addNodeEdge(std::string startNode, std::string endNode);
  void endNodeGraph();

  /*
  static void flowGraphStart(std::string graphName);
  static void flowGraphNodeStart(std::string nameNode);
  static void flowGraphNodeEnd(std::string nameNode);
  static void flowGraphEnd(std::string graphName);
  */


  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();
 
  // Sets the structure of the current graph by specifying its text format encoding
  void setFlowGraphEncoding(std::string dataText);

  // Add a directed edge from the location of the from anchor to the location of the to anchor
  virtual void addDirEdgeFG(anchor from, anchor to);
  
  // Add an undirected edge between the location of the a anchor and the location of the b anchor
  virtual void addUndirEdgeFG(anchor a, anchor b);

  // Add an invisible edge that forces the target to be placed after the source
  virtual void addInvisDepEdgeFG(anchor a, anchor b);

  // Start a graphviz cluster
  virtual void startSubFlowGraph();
  virtual void startSubFlowGraph(const std::string& label);
  // End a graphviz cluster
  virtual void endSubFlowGraph();
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  virtual bool subBlockEnterNotify(block* subBlock);
  virtual bool subBlockExitNotify (block* subBlock) { return false; }

  private:
  // Emits a tag for the given node
  void emitNodeTag(int anchorID, std::string label, int nodeID);
}; // flowgraph

class flowGraphNode{
  flowgraph& g;
  public:
  flowGraphNode(flowgraph& g); 
  ~flowGraphNode();
};

// Creates a graphviz cluster within a given graph
class subflowgraph
{
  flowgraph& g;
  public:
  subflowgraph(flowgraph& g, const std::string& label);
  ~subflowgraph();
}; // subgraph

} // namespace structure
} // namespace sight
