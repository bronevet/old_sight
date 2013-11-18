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
#include "module_common.h"
#include "../sight_structure_internal.h"
#include "Callpath.h"

namespace sight {
namespace structure {

// Syntactic sugar for specifying inputs
typedef common::easyvector<common::module::port> inputs;

class module: public block, public common::module
{
  protected:
  // Records all the known contexts, mapping each context to its count of input and output ports
  //std::map<context, std::pair<int, int> > knownCtxt;
  // Records all the known contexts, mapping each context to its unique ID
  static std::map<context, int> knownCtxt;
  	
  // Maps each context to the number of times it was ever observed
  static std::map<context, int> ctxtCount;
  
  // Maps each context to the trace that records its performance properties
  static std::map<context, trace*> ctxtTrace;
  
  // Records all the edges ever observed, mapping them to the number of times each edge was observed
  static std::map<std::pair<port, port>, int> edges;
  
  // The maximum ID assigned to any module
  static int maxModuleID;
  
  // Stack of the modules that are currently in scope
  static std::list<module*> mStack;
  	
  context c;
  
  // Measure object that captures the app's behavior during the module's lifetime
  measure* moduleMeasure;
  
  // Attribute that records the instance number of this module object
  //attr* instanceAttr;
  
  public:
  // inputs - ports from other modules that are used as inputs by this module.
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  module(const context& c,                                                         properties* props=NULL);
  module(const context& c, const port& inputs,                                     properties* props=NULL);
  module(const context& c, const std::vector<port>& inputs,                        properties* props=NULL);
  module(const context& c,                                  const attrOp& onoffOp, properties* props=NULL);
  module(const context& c, const port& inputs,              const attrOp& onoffOp, properties* props=NULL);
  module(const context& c, const std::vector<port>& inputs, const attrOp& onoffOp, properties* props=NULL);
  
  void init(const context& c, const std::vector<port>& in);
  
  private:
  // Sets the properties of this object
  static properties* setProperties(const context& c, const std::vector<port>& inputs, const attrOp* onoffOp, properties* props);
  
  public:
  ~module();
  
  const context& getContext() const { return c; }
  int numInputs()  const { return c.numInputs; }
  int numOutputs() const { return c.numOutputs; }
  
  // Returns a list of the module's input ports
  std::vector<port> inputPorts() const;
 
  // Returns a list of the module's output ports
  std::vector<port> outputPorts() const;
  
  // Returns the current moduleGraph on the stack and NULL if the stack is empty
  static module* getCurrent();
    
  // Records the mapping from a module's context to its unique ID
	static void addNode(const context& c, int nodeID);
		
	// Removes a module node from consideration
	//static void removeNode(const context& c);
    
  static void addEdge(port from, port to);
  static void addEdge(context fromC, common::module::ioT fromT, int fromP, 
                      context toC,   common::module::ioT toT,   int toP);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
}; // moduleGraph

class ModuleMerger : public BlockMerger {
  public:
  ModuleMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
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

}; // namespace structure
}; // namespace sight
