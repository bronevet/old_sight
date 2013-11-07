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
#include "../sight_structure_internal.h"

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
  
  public:
  
  graph(                                                                        properties* props=NULL);
  graph(                                                 const attrOp& onoffOp, properties* props=NULL);
  graph(                     anchor& pointsTo,                                  properties* props=NULL);
  graph(                     std::set<anchor>& pointsTo, const attrOp& onoffOp, properties* props=NULL);
  graph(std::string dotText,                                                    properties* props=NULL);
  graph(std::string dotText,                             const attrOp& onoffOp, properties* props=NULL);
  graph(std::string dotText, anchor& pointsTo,                                  properties* props=NULL);
  graph(std::string dotText, std::set<anchor>& pointsTo, const attrOp& onoffOp, properties* props=NULL);
    
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
 
  // Sets the structure of the current graph by specifying its dot encoding
  void setGraphEncoding(std::string dotText);
  
  // Add a directed edge from the location of the from anchor to the location of the to anchor
  virtual void addDirEdge(anchor from, anchor to);
  
  // Add an undirected edge between the location of the a anchor and the location of the b anchor
  virtual void addUndirEdge(anchor a, anchor b);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  virtual bool subBlockEnterNotify(block* subBlock);// { return false; }
  virtual bool subBlockExitNotify (block* subBlock) { return false; }
}; // graph

class GraphMerger : public BlockMerger {
  public:
  GraphMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
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
  
  // Maps each currently active graph (there is a stack since they nest hierarchically) to 
  // the list of its known edges
  std::list<std::set<streamGraphEdge> > edges;
  
  // Maps the GraphIDs within an incoming stream to the GraphIDs on its corresponding outgoing stream
  std::map<streamID, streamID> in2outGraphIDs;
  
  public:
  GraphStreamRecord(int vID)              : streamRecord(vID) { maxGraphID=0; }
  GraphStreamRecord(const variantID& vID) : streamRecord(vID) { maxGraphID=0; }
  GraphStreamRecord(const GraphStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
  
  // Marge the IDs of the next graph (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
  // updating each incoming stream's mappings from its IDs to the outgoing stream's IDs
  static void mergeIDs(std::map<std::string, std::string> pMap, 
                       const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  
  // Indicates that we've entered/exited a graph
  static void enterGraph(std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  static void exitGraph(std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
    
  // Adds an edge to the current graph
  void addEdge(streamGraphEdge edge);
  
  // Returns a reference to all of the edges of the most deeply nested graph in this incoming stream
  const std::set<streamGraphEdge>& getEdges() const;
    
  std::string str(std::string indent="") const;
}; // class GraphStreamRecord

class DirEdgeMerger : public Merger {
  public:
  DirEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                std::map<std::string, streamRecord*>& outStreamRecords,
                std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
}; // class DirEdgeMerger

class UndirEdgeMerger : public Merger {
  public:
  UndirEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                  std::map<std::string, streamRecord*>& outStreamRecords,
                  std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
}; // class UndirEdgeMerger

class NodeMerger : public Merger {
  public:
  NodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
             std::map<std::string, streamRecord*>& outStreamRecords,
             std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
}; // class NodeMerger

class NodeStreamRecord: public streamRecord {
  friend class NodeMerger;
  // Records the maximum NodeID ever generated on a given outgoing stream
  int maxNodeID;
  
  // Maps the NodeIDs within an incoming stream to the NodeIDs on its corresponding outgoing stream
  std::map<streamID, streamID> in2outNodeIDs;
  
  public:
  NodeStreamRecord(int vID)              : streamRecord(vID) { maxNodeID=0; }
  NodeStreamRecord(const variantID& vID) : streamRecord(vID) { maxNodeID=0; }
  NodeStreamRecord(const NodeStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
  
  // Marge the IDs of the next Node (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
  // updating each incoming stream's mappings from its IDs to the outgoing stream's IDs
  static void mergeIDs(std::map<std::string, std::string> pMap, 
                       const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);

  std::string str(std::string indent="") const;
}; // class NodeStreamRecord


}; // namespace structure
}; // namespace sight
