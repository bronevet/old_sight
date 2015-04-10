#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../sight_common.h"
#include "../../sight_layout.h"

namespace sight {
namespace layout {

class flowgraphLayoutHandlerInstantiator : layoutHandlerInstantiator {
  public:
  flowgraphLayoutHandlerInstantiator();
};
extern flowgraphLayoutHandlerInstantiator flowgraphLayoutHandlerInstance;

typedef common::flowgraphEdge<anchor> flowgraphEdge;

class flowgraph: public block
{
  protected:

  std::vector< std::pair <int, std::string> > nodesFG;
  std::vector< std::pair <int, int> > parentsFG;
  std::string graphName;

	// Maps a block's location to its ID and label
  std::map<anchor, std::string> nodes;
  // The maximum ID associated with any node in this graph
  int maxNodeID;
  
  std::list<flowgraphEdge> edges;
  
  // The path the directory where output files of the graph widget are stored
  // Relative to current path
  static std::string outDir;
  // Relative to root of HTML document
  static std::string htmlOutDir;
    
  // Unique ID of this graph object
  int flowgraphID;
  
  // Maximum ID assigned to any graph object
  //static int maxWidgetID;
  
  // Maps the IDs of the currently active graphs to their graph objects
  static std::map<int, flowgraph*> active;
  
  // Records whether this graph has already been output by a call to outputDataFlowGraph()
  bool flowgraphOutput;
  

  // Counts the number of sub graphs that have been created within this one
  int subFlowGraphCounter;
  
  public:
  
  flowgraph(properties::iterator props);
  ~flowgraph();

  // Generates and returns the graph code for this genDataFlowGraph
  virtual std::string genDataFlowGraph();

  
  // Given a string representation of a dot graph, emits the graph's visual representation 
  void outputDataFlowGraph(std::string dot);
  
  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();
 
  // The txt file that will hold the representation of the graph
  std::ofstream tFile;
  std::ofstream inouFile;
  std::ofstream datFile;
  std::ofstream ioInfoFile;
  std::ofstream hoavizCanvasFile;

  // Sets the structure of the current graph by specifying its dot encoding
  void setFlowGraphEncoding(std::string dataText);
  static void* setFlowGraphEncoding(properties::iterator props);

  // add node
  void add_node(int nodeID, std::string nodeName, int num_inputs, int num_outputs, int parentID);
  // input data for statistic visualization
  void add_viz(int nodeID, int buttonID, std::string viz);
  // connection between input fromID of from_nodeID and output toID of to_nodeID
  void add_inout(int from_nodeID, int fromID, int to_nodeID, int toID);
  // data for input and output variable information
  void add_ioInfo(int nodeID, int num_polyFit, std::string fitText);

  void addNodeHyperlink(std::string fromNode, anchor to);
  static void* addNodeHyperlink(properties::iterator props);
  //void addNodeHyperlink(properties::iterator props);
  // Add a directed edge from the location of the from anchor to the location of the to anchor
  void addDirEdgeFG(anchor from, anchor to);
  static void* addDirEdgeFG(properties::iterator props);
  
  // Add an undirected edge between the location of the a anchor and the location of the b anchor
  void addUndirEdgeFG(anchor a, anchor b);
  static void* addUndirEdgeFG(properties::iterator props);

  // Add an invisible undirected edge between the location of the a anchor and the location of the b anchor
  void addInvisEdgeFG(anchor a, anchor b);
  static void* addInvisEdgeFG(properties::iterator props);

  // Add a node to the graph
  void addNodeFG(anchor a, std::string label);
  static void* addNodeFG(properties::iterator props);

  // Start a sub-graph
  void startSubFlowGraph();
  void startSubFlowGraph(const std::string& label);
  static void* startSubFlowGraph(properties::iterator props);

  // End a sub-graph
  //void endSubGraph();
  static void endSubFlowGraph(void* obj);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return false; }
  bool subBlockExitNotify (block* subBlock) { return false; }

};

/*
class flowGraphNode{
public:
	flowGraphNode(std::string graphName);


};
*/

}; // namespace layout
}; // namespace sight
