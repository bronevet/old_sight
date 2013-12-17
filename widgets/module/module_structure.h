#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../attributes/attributes_structure.h"
#include "module_common.h"
#include "../../sight_structure_internal.h"
#include "Callpath.h"

namespace sight {
namespace structure {

// Syntactic sugar for specifying inputs
typedef common::easyvector<common::module::port> inputs;

//#ifndef MODULE_STRUCTURE_C
// Rename for contexts, groups and ports that enables users to refer to them without prepending common::module
typedef common::module::group group;
typedef common::module::context context;
typedef common::module::port port;
typedef common::module::context::config config;
//#endif

class module: public block, public common::module
{
  protected:
  // Records all the known contexts, mapping each context to its count of input and output ports
  //std::map<context, std::pair<int, int> > group2ID;
  // Records all the known contexts, mapping each context to its unique ID
  static std::map<group, int> group2ID;
  	
  // Maps each context to the number of times it was ever observed
  static std::map<group, int> group2Count;
  
  // Maps each context to the trace that records its performance properties
  //static std::map<context, trace*> ctxtTrace;
  
  // The trace that records performance observations of different modules and contexts
  static std::map<group, traceStream*> moduleTrace;
  
  // Maps each module to the list of the names of its input and output context attributes. 
  // This enables us to verify that all the modules are used consistently.
  static std::map<group, std::vector<std::list<std::string> > > moduleInCtxtNames;
  static std::map<group, std::vector<std::list<std::string> > > moduleOutCtxtNames;
    
  // Records all the edges ever observed, mapping them to the number of times each edge was observed
  static std::map<std::pair<port, port>, int> edges;
  
  // The maximum ID assigned to any module
  static int maxModuleID;
  
  // Stack of the modules that are currently in scope
  static std::list<module*> mStack;
  
  group g;	
  // The context of this module execution, which is a combination of the contexts of all of its inputs
  std::vector<context> ctxt;
  
  // The context in a format that traces can understand, set in init() and used in ~module()
  std::map<std::string, attrValue> traceCtxt;
  
  // The  output ports of this module
  std::vector<port> outputs;
  
  // We allow the user to provide a pointer to an output vector, which is populated by the module
  // object. This is a pointer to this vector.
  std::vector<port>* externalOutputs;
  
  // List of all the measurements that should be taken during the execution of this module. This is a list of pointers to
  // measure objects. When the module instance is deleted, its measure objects are automatically deleted as well.
  namedMeasures meas;
  
  public:
  // inputs - ports from other modules that are used as inputs by this module.
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  module(const group& g,                                                                                                                        properties* props=NULL);
  module(const group& g, const port& inputs,                                                                                                    properties* props=NULL);
  module(const group& g, const std::vector<port>& inputs,                                                                                       properties* props=NULL);
  module(const group& g,                                                                      const attrOp& onoffOp,                            properties* props=NULL);
  module(const group& g, const port& inputs,                                                  const attrOp& onoffOp,                            properties* props=NULL);
  module(const group& g, const std::vector<port>& inputs,                                     const attrOp& onoffOp,                            properties* props=NULL);
  module(const group& g,                                  std::vector<port>& externalOutputs,                                                   properties* props=NULL);
  module(const group& g, const port& inputs,              std::vector<port>& externalOutputs,                                                   properties* props=NULL);
  module(const group& g, const std::vector<port>& inputs, std::vector<port>& externalOutputs,                                                   properties* props=NULL);
  module(const group& g,                                  std::vector<port>& externalOutputs, const attrOp& onoffOp,                            properties* props=NULL);
  module(const group& g, const port& inputs,              std::vector<port>& externalOutputs, const attrOp& onoffOp,                            properties* props=NULL);
  module(const group& g, const std::vector<port>& inputs, std::vector<port>& externalOutputs, const attrOp& onoffOp,                            properties* props=NULL);
    
  module(const group& g,                                                                                             const namedMeasures& meas, properties* props=NULL);
  module(const group& g, const port& inputs,                                                                         const namedMeasures& meas, properties* props=NULL);
  module(const group& g, const std::vector<port>& inputs,                                                            const namedMeasures& meas, properties* props=NULL);
  module(const group& g,                                                                      const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  module(const group& g, const port& inputs,                                                  const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  module(const group& g, const std::vector<port>& inputs,                                     const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  module(const group& g,                                  std::vector<port>& externalOutputs,                        const namedMeasures& meas, properties* props=NULL);
  module(const group& g, const port& inputs,              std::vector<port>& externalOutputs,                        const namedMeasures& meas, properties* props=NULL);
  module(const group& g, const std::vector<port>& inputs, std::vector<port>& externalOutputs,                        const namedMeasures& meas, properties* props=NULL);
  module(const group& g,                                  std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  module(const group& g, const port& inputs,              std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  module(const group& g, const std::vector<port>& inputs, std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  
  void init(const group& g, const std::vector<port>& in);
  
  private:
  // Sets the properties of this object
  static properties* setProperties(const group& g, const std::vector<port>& inputs, const attrOp* onoffOp, properties* props);
  
  public:
  ~module();
  
  const std::vector<context>& getContext() const { return ctxt; }
  int numInputs()  const { return g.numInputs; }
  int numOutputs() const { return g.numOutputs; }
  
  // Sets the context of the given output port
  void setOutCtxt(int idx, const context& c);
  
  // Returns a list of the module's input ports
  //std::vector<port> inputPorts() const;
 
  // Returns a list of the module's output ports
  std::vector<port> outPorts() const;
  port outPort(int idx) const;
    
  // Returns the current moduleGraph on the stack and NULL if the stack is empty
  static module* getCurrent();
    
  // Records the mapping from a module's context to its unique ID
	static void addNode(const group& g, int nodeID);
		
	// Removes a module node from consideration
	//static void removeNode(const context& c);
    
  static void addEdge(port from, port to);
  static void addEdge(group fromG, common::module::ioT fromT, int fromP, 
                      group toG,   common::module::ioT toT,   int toP);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
}; // moduleGraph


// Specialization of traceStreams for the case where they are hosted by a module node
class moduleNodeTraceStream: public traceStream
{
  public:
  moduleNodeTraceStream(int nodeID, std::string name, int numInputs, int numOutputs, vizT viz, mergeT merge, properties* props=NULL);
  
  static properties* setProperties(int nodeID, std::string name, int numInputs, int numOutputs, vizT viz, mergeT merge, properties* props);
};

class ModuleMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  ModuleMergeHandlerInstantiator();
};
extern ModuleMergeHandlerInstantiator ModuleMergeHandlerInstance;

std::map<std::string, streamRecord*> ModuleGetMergeStreamRecord(int streamID);

class ModuleMerger : public BlockMerger {
  public:
  ModuleMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ModuleMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key);
}; // class ModuleMerger

class ModuleNodeMerger : public Merger {
  public:
  ModuleNodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ModuleNodeMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key);
}; // class ModuleNodeMerger

class ModuleEdgeMerger : public Merger {
  public:
  ModuleEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ModuleEdgeMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key);
}; // class ModuleEdgeMerger

class ModuleNodeTraceStreamMerger : public TraceStreamMerger {
  public:
  ModuleNodeTraceStreamMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
    
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ModuleNodeTraceStreamMerger(tags, outStreamRecords, inStreamRecords, props); }
              
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
                       std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key);
}; // class ModuleNodeTraceStreamMerger

class ModuleStreamRecord: public streamRecord {
  friend class ModuleMerger;
  friend class ModuleNodeMerger;
  friend class ModuleEdgeMerger;
  friend class ModuleNodeTraceStreamMerger;
  
  // The stack of streamRecords that record for each currently active module group 
  // (they are nested hierarchically), the mappings of nodes IDs from incoming to 
  // outgoing streams.
  std::list<streamRecord*> mStack;
    
  public:
  ModuleStreamRecord(int vID)              : streamRecord(vID, "module") { }
  ModuleStreamRecord(const variantID& vID) : streamRecord(vID, "module") { }
  ModuleStreamRecord(const ModuleStreamRecord& that, int vSuffixID);
  
  // Called to record that we've entered/exited a module
  void enterModule();
  static void enterModule(std::map<std::string, streamRecord*>& outStreamRecords,
                     std::vector<std::map<std::string, streamRecord*> >& incomingStreamRecords);
  void exitModule();
  static void exitModule(std::map<std::string, streamRecord*>& outStreamRecords,
                    std::vector<std::map<std::string, streamRecord*> >& incomingStreamRecords);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given a vector of streamRecord maps, collects the streamRecords associated with the currently active module (top of the gStack)
  // within each stream into nodeStreamRecords and returns the height of the gStacks on all the streams (must be the same number)
  static int collectNodeStreamRecords(std::vector<std::map<std::string, streamRecord*> >& streams,
                                      std::vector<std::map<std::string, streamRecord*> >& nodeStreamRecords);
  
  // Applies streamRecord::mergeIDs to the nodeIDs of the currently active module
  static int mergeNodeIDs(std::string objName, 
                          std::map<std::string, std::string>& pMap, 
                          const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                          std::map<std::string, streamRecord*>& outStreamRecords,
                          std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  
  // Given a streamID in the current incoming stream return its streamID in the outgoing stream, yelling if it is missing.
  streamID in2outNodeID(streamID inSID) const;
  
  // Given multiple streamRecords from several variants of the same stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
      
  std::string str(std::string indent="") const;
}; // class ModuleStreamRecord

}; // namespace structure
}; // namespace sight
