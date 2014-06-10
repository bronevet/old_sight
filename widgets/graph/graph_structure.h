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

class dottable
{
  public:
  virtual ~dottable() {}
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  virtual std::string toDOT(std::string graphName)=0;
};

typedef common::graphEdge<anchor> graphEdge;

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
  
  // Records whether we should include a graph node for each sub-block inside the graph
  bool includeAllSubBlocks;
  
  // Maps the anchor IDs of nodes that have been observed.
  // This is useful for cases where includeAllSubBlocks and thus, we only emit a node if we observe an edge that 
  // involves it.
  std::map<int, std::pair<std::string, int> > nodesObservedNotEmitted;
  
  // Set of the anchorIDs of the nodes that have been connected by edges.
  std::set<anchor> nodesConnected;
  
  public:
  
  graph(                                                                        bool includeAllSubBlocks=false, properties* props=NULL);
  graph(                                                 const attrOp& onoffOp, bool includeAllSubBlocks=false, properties* props=NULL);
  graph(                     anchor& pointsTo,                                  bool includeAllSubBlocks=false, properties* props=NULL);
  graph(                     std::set<anchor>& pointsTo, const attrOp& onoffOp, bool includeAllSubBlocks=false, properties* props=NULL);
  graph(std::string dotText,                                                    bool includeAllSubBlocks=false, properties* props=NULL);
  graph(std::string dotText,                             const attrOp& onoffOp, bool includeAllSubBlocks=false, properties* props=NULL);
  graph(std::string dotText, anchor& pointsTo,                                  bool includeAllSubBlocks=false, properties* props=NULL);
  graph(std::string dotText, std::set<anchor>& pointsTo, const attrOp& onoffOp, bool includeAllSubBlocks=false, properties* props=NULL);
    
  private:
  // Sets the properties of this object
  static properties* setProperties(int graphID, std::string dotText, const attrOp* onoffOp, properties* props);
  
  //void init(const attrOp* onoffOp, properties* props);
  
  public:
  ~graph();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  // Given a reference to an object that can be represented as a dot graph,  create an image from it and add it to the output.
  // Return the path of the image.
  static void genGraph(dottable& obj);

  // Given a representation of a graph in dot format, create an image from it and add it to the output.
  // Return the path of the image.
  static void genGraph(std::string dot);
  
  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();
 
  // Sets the structure of the current graph by specifying its dot encoding
  void setGraphEncoding(std::string dotText);
  
  // Add a directed edge from the location of the from anchor to the location of the to anchor
  virtual void addDirEdge(anchor from, anchor to);
  
  // Add an undirected edge between the location of the a anchor and the location of the b anchor
  virtual void addUndirEdge(anchor a, anchor b);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  virtual bool subBlockEnterNotify(block* subBlock);
  virtual bool subBlockExitNotify (block* subBlock) { return false; }

  private:
  // Emits a tag for the given node
  void emitNodeTag(int anchorID, std::string label, int nodeID);
}; // graph

class GraphMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  GraphMergeHandlerInstantiator();
};
extern GraphMergeHandlerInstantiator GraphMergeHandlerInstance;

std::map<std::string, streamRecord*> GraphGetMergeStreamRecord(int streamID);

class GraphMerger : public BlockMerger {
  public:
  GraphMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
              
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new GraphMerger(tags, outStreamRecords, inStreamRecords, props); }
              
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
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class GraphMerger

class GraphStreamRecord: public streamRecord {
  friend class GraphMerger;
  friend class DirEdgeMerger;
  friend class UndirEdgeMerger;
  
  class streamGraphEdge : public printable {
    public:
    streamAnchor fromAnchor;
    streamAnchor toAnchor;
    bool directed;
    streamGraphEdge(streamAnchor fromAnchor, streamAnchor toAnchor, bool directed) : fromAnchor(fromAnchor), toAnchor(toAnchor), directed(directed) {}
    
    bool operator==(const streamGraphEdge& that) const { 
      return fromAnchor == that.fromAnchor &&
             toAnchor   == that.toAnchor &&
             directed   == that.directed;
    }
    
    bool operator<(const streamGraphEdge& that) const { 
      return (fromAnchor < that.fromAnchor) ||
             (fromAnchor == that.fromAnchor && toAnchor <  that.toAnchor) ||
             (fromAnchor == that.fromAnchor && toAnchor == that.toAnchor && directed < that.directed);
    }
    
    std::string str(std::string indent="") const {
      std::ostringstream s;
      s << "[streamGraphEdge: "<<fromAnchor.str()<<" =&gt; "<<toAnchor.str()<<" dir="<<directed<<"]"<<std::endl;
      return s.str();
    }
  };
  
  // Records the maximum GraphID ever generated on a given outgoing stream
  int maxGraphID;
  
  // Stack of the IDs of currently active graphs
  std::list<int> gStack;
  
  // Maps the ID of each currently active graph to the list of its known edges
  std::map<int, std::set<streamGraphEdge> > edges;
  
  public:
  GraphStreamRecord(int vID)              : streamRecord(vID, "graph") { maxGraphID=0; }
  GraphStreamRecord(const variantID& vID) : streamRecord(vID, "graph") { maxGraphID=0; }
  GraphStreamRecord(const GraphStreamRecord& that, int vSuffixID);
  
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
  static void enterGraph(std::vector<std::pair<properties::tagType, properties::iterator> > tags, 
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  // Indicates that we've entered a graph (for outgoing streams)
  static void enterGraph(int graphID,
                         std::map<std::string, streamRecord*>& outStreamRecord);
  
  // Indicates that we've exited a graph (for incoming streams)
  static void exitGraph(std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  // Indicates that we've exited a graph (for outgoing streams)
  static void exitGraph(std::map<std::string, streamRecord*>& outStreamRecord);
    
  // Adds an edge to the current graph
  void addEdge(int graphID, streamGraphEdge edge);
  
  // Returns a reference to all of the edges of the most deeply nested graph in this incoming stream
  const std::set<streamGraphEdge>& getEdges(int graphID) const;
    
  std::string str(std::string indent="") const;
}; // class GraphStreamRecord

class DirEdgeMerger : public Merger {
  public:
  DirEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                std::map<std::string, streamRecord*>& outStreamRecords,
                std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new DirEdgeMerger(tags, outStreamRecords, inStreamRecords, props); }

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class DirEdgeMerger

class UndirEdgeMerger : public Merger {
  public:
  UndirEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                  std::map<std::string, streamRecord*>& outStreamRecords,
                  std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                  properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new UndirEdgeMerger(tags, outStreamRecords, inStreamRecords, props); }

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class UndirEdgeMerger

class NodeMerger : public Merger {
  public:
  NodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
             std::map<std::string, streamRecord*>& outStreamRecords,
             std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
             properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new NodeMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class NodeMerger

class NodeStreamRecord: public streamRecord {
  friend class NodeMerger;
  public:
  NodeStreamRecord(int vID)              : streamRecord(vID, "node") { }
  NodeStreamRecord(const variantID& vID) : streamRecord(vID, "node") { }
  NodeStreamRecord(const NodeStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  std::string str(std::string indent="") const;
}; // class NodeStreamRecord


} // namespace structure
} // namespace sight
