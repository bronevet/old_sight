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
  void addNode(std::string nodeName, int verID, int horID);
  void addNode(std::string childNode, std::string parentNode);
  void addNode(std::string childNode, std::string parentNode, int verID, int horID);
  void addNode(std::string childNode, std::string parentNode, int verID, int horID, anchor toAnchor);
  void addEdge(std::string startNode, std::string endNode);
  void endGraph();

  void graphNodeStart(std::string nodeName);
  void graphNodeStart(std::string nodeName, std::string texNode);
  void graphNodeStart(std::string nodeName, int verID, int horID);
  void graphNodeStart(std::string nodeName, std::string texNode, int verID, int horID);
  void graphNodeEnd(std::string nodeName);
  void addNodeEdge(std::string startNode, std::string endNode);
  void endNodeGraph();

  void addNodeHyperlink(std::string nodeName, anchor to);
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


// merging
class FlowGraphMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  FlowGraphMergeHandlerInstantiator();
};
extern FlowGraphMergeHandlerInstantiator FlowGraphMergeHandlerInstance;

std::map<std::string, streamRecord*> FlowGraphGetMergeStreamRecord(int streamID);

class FlowGraphMerger : public BlockMerger {
  public:
  FlowGraphMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
              
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new FlowGraphMerger(tags, outStreamRecords, inStreamRecords, props); }
              
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class GraphMerger

class FlowGraphStreamRecord: public streamRecord {
  friend class FlowGraphMerger;
  friend class DirEdgeMergerFG;
  friend class UndirEdgeMergerFG;
  friend class NodeHyperlinkFG;
  
  class streamFlowGraphEdge : public printable {
    public:
    streamAnchor fromAnchor;
    streamAnchor toAnchor;
    bool directed;
    streamFlowGraphEdge(streamAnchor fromAnchor, streamAnchor toAnchor, bool directed) : fromAnchor(fromAnchor), toAnchor(toAnchor), directed(directed) {}
    
    bool operator==(const streamFlowGraphEdge& that) const { 
      return fromAnchor == that.fromAnchor &&
             toAnchor   == that.toAnchor &&
             directed   == that.directed;
    }
    
    bool operator<(const streamFlowGraphEdge& that) const { 
      return (fromAnchor < that.fromAnchor) ||
             (fromAnchor == that.fromAnchor && toAnchor <  that.toAnchor) ||
             (fromAnchor == that.fromAnchor && toAnchor == that.toAnchor && directed < that.directed);
    }
    
    std::string str(std::string indent="") const {
      std::ostringstream s;
      s << "[streamFlowGraphEdge: "<<fromAnchor.str()<<" =&gt; "<<toAnchor.str()<<" dir="<<directed<<"]"<<std::endl;
      return s.str();
    }
  };
  
  // Records the maximum GraphID ever generated on a given outgoing stream
  int maxGraphID;
  
  // Stack of the IDs of currently active graphs
  std::list<int> gStack;
  
  // Maps the ID of each currently active graph to the list of its known edges
  std::map<int, std::set<streamFlowGraphEdge> > edges;
  
  public:
  FlowGraphStreamRecord(int vID)              : streamRecord(vID, "flowgraph") { maxGraphID=0; }
  FlowGraphStreamRecord(const variantID& vID) : streamRecord(vID, "flowgraph") { maxGraphID=0; }
  FlowGraphStreamRecord(const FlowGraphStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
  
  // Marge the IDs of the next graph (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
  // updating each incoming stream's mappings from its IDs to the outgoing stream's IDs
  /*static void mergeIDs(std::map<std::string, std::string>& pMap, 
                       const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);*/
  
  // Indicates that we've entered a graph (for incoming streams)
  static void enterFlowGraph(std::vector<std::pair<properties::tagType, properties::iterator> > tags, 
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  // Indicates that we've entered a graph (for outgoing streams)
  static void enterFlowGraph(int graphID,
                         std::map<std::string, streamRecord*>& outStreamRecord);
  
  // Indicates that we've exited a graph (for incoming streams)
  static void exitFlowGraph(std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  // Indicates that we've exited a graph (for outgoing streams)
  static void exitFlowGraph(std::map<std::string, streamRecord*>& outStreamRecord);
    
  // Adds an edge to the current graph
  void addEdgeFG(int graphID, streamFlowGraphEdge edge);
  
  // Returns a reference to all of the edges of the most deeply nested graph in this incoming stream
  const std::set<streamFlowGraphEdge>& getEdgesFG(int graphID) const;
    
  std::string str(std::string indent="") const;
}; // class GraphStreamRecord

class NodeHyperlinkFG : public Merger {
  public:
  NodeHyperlinkFG(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                std::map<std::string, streamRecord*>& outStreamRecords,
                std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new NodeHyperlinkFG(tags, outStreamRecords, inStreamRecords, props); }

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class NodeHyperlinkFG

class DirEdgeMergerFG : public Merger {
  public:
  DirEdgeMergerFG(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                std::map<std::string, streamRecord*>& outStreamRecords,
                std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new DirEdgeMergerFG(tags, outStreamRecords, inStreamRecords, props); }

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class DirEdgeMerger

class UndirEdgeMergerFG : public Merger {
  public:
  UndirEdgeMergerFG(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                  std::map<std::string, streamRecord*>& outStreamRecords,
                  std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                  properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new UndirEdgeMergerFG(tags, outStreamRecords, inStreamRecords, props); }

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class UndirEdgeMerger

class NodeMergerFG : public Merger {
  public:
  NodeMergerFG(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
             std::map<std::string, streamRecord*>& outStreamRecords,
             std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
             properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new NodeMergerFG(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class NodeMerger

class NodeStreamRecordFG: public streamRecord {
  friend class NodeMergerFG;
  public:
  NodeStreamRecordFG(int vID)              : streamRecord(vID, "nodeFG") { }
  NodeStreamRecordFG(const variantID& vID) : streamRecord(vID, "nodeFG") { }
  NodeStreamRecordFG(const NodeStreamRecordFG& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  std::string str(std::string indent="") const;
}; // class NodeStreamRecord



} // namespace structure
} // namespace sight
