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

//#ifndef MODULE_STRUCTURE_C
// Rename for contexts, groups and ports that enables users to refer to them without prepending common::module
//typedef common::module::group group;
typedef common::module::context context;
typedef common::module::context::config config;
//typedef common::module::port port;
//#endif

class instance {
  public:
  std::string name;

  int numInputs;
  int numOutputs;

  instance() {}
  instance(std::string name, int numInputs, int numOutputs) : name(name), numInputs(numInputs), numOutputs(numOutputs) {}
  instance(const instance& that) : name(that.name), numInputs(that.numInputs), numOutputs(that.numOutputs) {}

  instance(properties::iterator props);

  // Returns the properties map that describes this instance object;
  std::map<std::string, std::string> getProperties() const;

  bool operator==(const instance& that) const
  { return name==that.name; }

  bool operator<(const instance& that) const
  { return name<that.name; }

  // Returns a human-readable string that describes this instance
  std::string str() const;
}; // class instance

class module;

// A group represents the granularity at which we differentiate instances of modules.
// Currently this is done by considering the name and count of inputs and outputs of a given module instance
// as well as the nesting hierarchy of instances within which a given instance is executed.
class group {
  public:
  // Stack of module instances that uniquely identifies this module grouping
  std::list<instance> stack;

  group() { }
  group(const std::list<instance>& stack): stack(stack) {}

  // Creates a group given the current stack of modules and a new module instance
  group(const std::list<module*>& mStack, const instance& inst);
  
  void init(const std::list<module*>& mStack, const instance& inst);
  
  group& operator=(const group& that) { 
    stack = that.stack;
    return *this;
  }
  
  // Add the given instance to this group's instance stack
  void push(const instance& inst) { stack.push_back(inst); }
  
  // Remove the last instance from this group's instance stack
  void pop() { assert(stack.size()>0); stack.pop_back(); }
  
  // Returns the name of the most deeply nested instance within this group
  std::string name() const;

  // Returns the number of inputs of the most deeply nested instance within this group
  int numInputs() const;
  
  // Returns the number of outnputs of the most deeply nested instance within this group
  int numOutputs() const;
  
  // Returns the most deeply nested instance within this group
  const instance& getInst() const;
  
  // Returns the depth of the callstack
  int depth() const;
  
  bool operator==(const group& that) const
  { return stack == that.stack; }
  
  bool operator<(const group& that) const
  { return stack < that.stack; }

  // Returns whether the group's descriptor is empty, meaning that this object does not denote any specific group
  bool isNULL() const { return stack.size()==0; }
  
  // Returns a human-readable string that describes this group
  std::string str() const;
}; // class group

class port {
  public:
  group g;
  context ctxt;
  sight::common::module::ioT type;
  int index;

  port() {}
  port(const context& ctxt) : ctxt(ctxt), type(sight::common::module::output), index(-1) {}
  port(const group& g, const context& ctxt, sight::common::module::ioT type, int index) : g(g), ctxt(ctxt), type(type), index(index) {}

  bool operator==(const port& that) const
  { return g==that.g && ctxt==that.ctxt && type==that.type && index==that.index; }

  bool operator<(const port& that) const
  { return (g< that.g) ||
           (g==that.g && ctxt< that.ctxt) ||
           (g==that.g && ctxt==that.ctxt && type< that.type) ||
           (g==that.g && ctxt==that.ctxt && type==that.type && index<that.index); }

  // Erase the context within this port. This is important for data-structures that ignore context details
  void clearContext() { ctxt.configuration.clear(); }

  // Returns a human-readable string that describes this context
  std::string str() const;
}; // class port

// Syntactic sugar for specifying inputs
typedef common::easyvector<port> inputs;

class inport : public port {
  public:
  inport() {}
  inport(const group& g, const context& c, int index) : port(g, c, sight::common::module::input, index) {}
};

class outport : public port {
  public:
  outport() {}
  outport(const group& g, const context& c, int index) : port(g, c, sight::common::module::output, index) {}
};

// Records the hierarchy of nesting observed for module instances
class instanceTree {
  std::map<instance, instanceTree*> m;
  
  public:
  instanceTree() {}
  
  void add(const group& g);
  
  // Types for callback functions to be executed on entry to / exit from an instance during the call to iterate
  typedef void (*instanceEnterFunc)(const group& g);
  typedef void (*instanceExitFunc) (const group& g);
  
  // Iterate through all the groups encoded by this tree, where each group corresponds to a stack of instances
  // from the tree's root to one of its leaves.
  void iterate(instanceEnterFunc entry, instanceExitFunc exit) const;
  
  // Recursive version of iterate that takes as an argument the fragment of the current group that corresponds to
  // the stack of instances between the tree's root and the current sub-tree
  void iterate(instanceEnterFunc entry, instanceExitFunc exit, group& g) const;
  
  // Empties out this instanceTree
  void clear();
  
  // The entry and exit functions used in instanceTree::str()
  static void strEnterFunc(const group& g);
  static void strExitFunc(const group& g);
  
  // Returns a human-readable string representation of this object
  std::string str() const;
  
  // The depth of the recursion in instanceTree::str()
  static int instanceTreeStrDepth;

  // The ostringstream into which the output of instanceTree::str() is accumulated
  static std::ostringstream oss;
}; // class instanceTree

class module;

// Base class for functors that generate traceStreams that are specific to different sub-types of module.
// We need this so that we can pass a reference to the correct genTS() method to modularApp::enterModule(). Since this
// call is made inside the constructor of module, we can't use virtual methods to ensure that the correct version
// of genTS will be called by just passing a reference to the current module-derived object
//typedef traceStream* (*generateTraceStream)(int moduleID, const group& g);
class generateTraceStream {
  public:
  virtual traceStream* operator()(int moduleID)=0;
}; // class generateTraceStream

// Represents a modular application, which may contain one or more modules. Only one modular application may be
// in-scope at any given point in time.
class modularApp: public block
{
  friend class group;
  friend class module;
  
  protected:
  // Points to the currently active instance of modularApp. There can be only one.
  static modularApp* activeMA;
    
  // The maximum ID ever assigned to any modular application
  static int maxModularAppID;
  
  // The maximum ID ever assigned to any module group
  static int maxModuleGroupID;

  // Records all the known contexts, mapping each context to its unique ID
  static std::map<group, int> group2ID;
  	
  // Maps each context to the number of times it was ever observed
  static std::map<group, int> group2Count;
  
  // The trace that records performance observations of different modules and contexts
  static std::map<group, traceStream*> moduleTrace;
  
  // Tree that records the hierarchy of module instances that were observed during the execution of this
  // modular application. Each path from the tree's root to a leaf is a stack of module instances that
  // corresponds to some observed module group.
  static instanceTree tree;
  
  // Maps each module to the list of the names of its input and output context attributes. 
  // This enables us to verify that all the modules are used consistently.
  static std::map<group, std::vector<std::list<std::string> > > moduleInCtxtNames;
  static std::map<group, std::vector<std::list<std::string> > > moduleOutCtxtNames;
  
  // The properties object that describes each module group. This object is created by calling each group's
  // setProperties() method and each call to this method for the same module group must return the same properties.
  static std::map<group, properties*> moduleProps;
    
  // Records all the edges ever observed, mapping them to the number of times each edge was observed
  static std::map<std::pair<port, port>, int> edges;
  
  // Stack of the modules that are currently in scope
  static std::list<module*> mStack;
  
  public:
  static const std::list<module*>& getMStack() { return mStack; }
  
  // The unique ID of this application
  int appID;
  
  // The name of this application
  std::string appName;
  
  // The set of measurements that will be collected for all the modules within this modular app
  namedMeasures meas;

  public:
  modularApp(const std::string& appName,                                                   properties* props=NULL);
  modularApp(const std::string& appName, const attrOp& onoffOp,                            properties* props=NULL);
  modularApp(const std::string& appName,                        const namedMeasures& meas, properties* props=NULL);
  modularApp(const std::string& appName, const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  
  // Stack used while we're emitting the nesting hierarchy of module groups to keep each module group's 
  // sightObject between the time the group is entered and exited
  static std::list<sightObj*> moduleEmitStack;

  // Emits the entry tag for a module group during the execution of ~modularApp()
  static void enterModuleGroup(const group& g);
  // Emits the exit tag for a module group during the execution of ~modularApp()
  static void exitModuleGroup(const group& g);
  
  ~modularApp();
  
  private:
  // Common initialization logic
  void init();
    
  // Sets the properties of this object
  static properties* setProperties(const std::string& appName, const attrOp* onoffOp, properties* props);
  
  public:
  // Returns the module ID of the given module group, generating a fresh one if one has not yet been assigned
  static int genModuleID(const group& g);
    
  // Returns whether the current instance of modularApp is active
  static bool isInstanceActive();
  
  // Assigns a unique ID to the given module group, as needed and returns this ID
  static int addModuleGroup(const group& g);
  
  // Registers the names of the contexts of the given module's inputs or outputs and if this is not the first time this module is called, 
  // verifies that these context names are consistent across different calls.
  // g - the module group for which we're registering inputs/outputs
  // inouts - the vector of input or output ports
  // toT - identifies whether inouts is the vector of inputs or outputs
  static void registerInOutContexts(const group& g, const std::vector<port> inouts, sight::common::module::ioT io);
  
  // Add an edge between one module's output port and another module's input port
  static void addEdge(port from, port to);

  // Add an edge between one module's output port and another module's input port
  static void addEdge(group fromG, sight::common::module::ioT fromT, int fromP, 
                      group toG,   sight::common::module::ioT toT,   int toP);

  // Returns the current module on the stack and NULL if the stack is empty
  static module* getCurModule();
  
  // Adds the given module object to the modules stack
  static void enterModule(module* m, int moduleID, properties* props, generateTraceStream& tsGenerator);
  
  // Removes the given module object from the modules stack
  static void exitModule(module* m);
}; // class modularApp

class module: /*public sightObj, */public common::module
{
  friend class group;
  friend class modularApp;
  
  protected:
  
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
  
  // Information that describes the class that derives from this on. 
  class derivInfo {
    public:
    
    // A pointer to a properties object that can be used to create a tag for the derived object that 
    // includes info about its parents.
    properties* props;
    
    // Fields that must be included within the context of any trace observations made during the execution of this module.
    std::map<std::string, attrValue> ctxt;
    
    // Points to a function that generates the trace stream instance specific to the given derivation of module
    generateTraceStream& tsGenerator;
    
    derivInfo(generateTraceStream& tsGenerator) : tsGenerator(tsGenerator) {
      props = new properties;
    }
    
    derivInfo(properties* props, const std::map<std::string, attrValue>& ctxt, generateTraceStream& tsGenerator) :
                  props(props), ctxt(ctxt), tsGenerator(tsGenerator) { }
  }; // class derivInfo
  
  public:
  // inputs - ports from other modules that are used as inputs by this module.
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  // derivInfo: Information that describes the class that derives from this on.
  module(const instance& inst,                                                                                                                        derivInfo* deriv=NULL);
  module(const instance& inst, const port& inputs,                                                                                                    derivInfo* deriv=NULL);
  module(const instance& inst, const std::vector<port>& inputs,                                                                                       derivInfo* deriv=NULL);
  module(const instance& inst,                                                                      const attrOp& onoffOp,                            derivInfo* deriv=NULL);
  module(const instance& inst, const port& inputs,                                                  const attrOp& onoffOp,                            derivInfo* deriv=NULL);
  module(const instance& inst, const std::vector<port>& inputs,                                     const attrOp& onoffOp,                            derivInfo* deriv=NULL);
  module(const instance& inst,                                  std::vector<port>& externalOutputs,                                                   derivInfo* deriv=NULL);
  module(const instance& inst, const port& inputs,              std::vector<port>& externalOutputs,                                                   derivInfo* deriv=NULL);
  module(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,                                                   derivInfo* deriv=NULL);
  module(const instance& inst,                                  std::vector<port>& externalOutputs, const attrOp& onoffOp,                            derivInfo* deriv=NULL);
  module(const instance& inst, const port& inputs,              std::vector<port>& externalOutputs, const attrOp& onoffOp,                            derivInfo* deriv=NULL);
  module(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, const attrOp& onoffOp,                            derivInfo* deriv=NULL);
    
  module(const instance& inst,                                                                                             const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst, const port& inputs,                                                                         const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst, const std::vector<port>& inputs,                                                            const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst,                                                                      const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst, const port& inputs,                                                  const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst, const std::vector<port>& inputs,                                     const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst,                                  std::vector<port>& externalOutputs,                        const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst, const port& inputs,              std::vector<port>& externalOutputs,                        const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,                        const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst,                                  std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst, const port& inputs,              std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv=NULL);
  module(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv=NULL);
  
  void init(const std::vector<port>& in, derivInfo* deriv);
  
  private:
  // Sets the properties of this object
  //static properties* setProperties(const instance& inst, const std::vector<port>& inputs, const attrOp* onoffOp, derivInfo* deriv);
  
  public:
  ~module();
  
  // The variant of the generateTraceStream functor specialized to generating moduleTraceStreams
  class generateModuleTraceStream : public generateTraceStream {
    protected:
    const group* g;
    public:
    generateModuleTraceStream() {}
    generateModuleTraceStream(const group& g): g(&g) { }
    
    void init(const group& g) { this->g = &g; }
    
    traceStream* operator()(int moduleID);
  }; // class generateModuleTraceStream

  // Returns the properties of this object
  //properties* setProperties(int moduleID, derivInfo* deriv=NULL);
  
  const std::vector<context>& getContext() const { return ctxt; }
  int numInputs()  const { return g.numInputs(); }
  int numOutputs() const { return g.numOutputs(); }
  
  // Sets the context of the given output port
  void setOutCtxt(int idx, const context& c);
  
  // Returns a list of the module's input ports
  //std::vector<port> inputPorts() const;
 
  // Returns a list of the module's output ports
  std::vector<port> outPorts() const;
  port outPort(int idx) const;
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
}; // moduleGraph

class compModule: public structure::module
{
  protected:
  // Records whether this is the reference configuration of the moculde
  //bool isReference;
    
  // The context that describes the configuration options of this module
  //context options;
  
  public:
  
  // isReference: Records whether this is the reference configuration of the moculde
  // options: The context that describes the configuration options of this module
  // derivInfo: Information that describes the class that derives from this on. It includes a pointer to a properties
  //    object that can be used to create a tag for the derived object that includes info about its parents. Further,
  //    it includes fields that must be included within the context of any trace observations made during the execution
  //    of this module.
  compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
             bool isReference, const context& options,
                                                               derivInfo* deriv=NULL);
  compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
             bool isReference, const context& options, 
             const attrOp& onoffOp,                            derivInfo* deriv=NULL);
  compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
             bool isReference, const context& options,
                                    const namedMeasures& meas, derivInfo* deriv=NULL);
  compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
             bool isReference, const context& options, 
             const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv=NULL);
  
  // Sets the properties of this object
  derivInfo* setProperties(const instance& inst, bool isReference, const context& options, const attrOp* onoffOp, derivInfo* deriv=NULL);
  
  // The variant of the generateTraceStream functor specialized to generating moduleTraceStreams
  class generateCompModuleTraceStream : public generateModuleTraceStream {
    bool isReference;
    const context* options;
    public:
    generateCompModuleTraceStream() {}
    
    void init(const instance& inst, bool isReference, const context& options) {
      static group g;
      g.init(modularApp::getMStack(), inst);
      generateModuleTraceStream::init(g);
      this->isReference = isReference;
      this->options = &options;
    }
    
    traceStream* operator()(int moduleID);
  }; // class generateModuleTraceStream
  
  // Static instance of generateModuleTraceStream. It is initialized inside calls to setProperties() and utilized inside
  // the module() constructor in its call to modularApp::enterModule(). As such, its state needs to remain valid during
  // the course of construction but is irrelevant other than that.
  static generateCompModuleTraceStream gcmts;
}; // class compModule

// Specialization of traceStreams for the case where they are hosted by a module node
class moduleTraceStream: public traceStream
{
  public:
  moduleTraceStream(int moduleID, std::string name, int numInputs, int numOutputs, vizT viz, mergeT merge, properties* props=NULL);
  
  static properties* setProperties(int moduleID, std::string name, int numInputs, int numOutputs, vizT viz, mergeT merge, properties* props);
};

// Specialization of moduleTraceStream for the case where they are hosted by a compModule node
class compModuleTraceStream: public moduleTraceStream
{
  public:
  compModuleTraceStream(int moduleID, std::string name, int numInputs, int numOutputs, bool isReference, const context& options, vizT viz, mergeT merge, properties* props=NULL);
  
  static properties* setProperties(int moduleID, std::string name, int numInputs, int numOutputs, bool isReference, const context& options, vizT viz, mergeT merge, properties* props);
};

class ModuleMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  ModuleMergeHandlerInstantiator();
};
extern ModuleMergeHandlerInstantiator ModuleMergeHandlerInstance;

std::map<std::string, streamRecord*> ModuleGetMergeStreamRecord(int streamID);

class ModularAppMerger : public BlockMerger {
  public:
  ModularAppMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ModularAppMerger(tags, outStreamRecords, inStreamRecords, props); }
  
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
}; // class ModularAppMerger

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
  friend class ModularAppMerger;
  friend class ModuleNodeMerger;
  friend class ModuleEdgeMerger;
  friend class ModuleNodeTraceStreamMerger;
  
  /*
  // We allow modules within different modularApps to use independent ID schemes (i.e. the module IDs within
  // different apps may independently start from 0) because we anticipate that in the future this may make it easier
  // to match up instances of the same module within the same modularApp.
  // The stack of streamRecords that record for each currently active moduluarApp
  // (they are nested hierarchically), the mappings of nodes IDs from incoming to 
  // outgoing streams.
  std::list<streamRecord*> mStack;*/
  
  public:
  ModuleStreamRecord(int vID)              : streamRecord(vID, "module") { }
  ModuleStreamRecord(const variantID& vID) : streamRecord(vID, "module") { }
  ModuleStreamRecord(const ModuleStreamRecord& that, int vSuffixID);
  
  // Called to record that we've entered/exited a module
  /*void enterModularApp();
  static void enterModularApp(std::map<std::string, streamRecord*>& outStreamRecords,
                     std::vector<std::map<std::string, streamRecord*> >& incomingStreamRecords);
  void exitModularApp();
  static void exitModularApp(std::map<std::string, streamRecord*>& outStreamRecords,
                    std::vector<std::map<std::string, streamRecord*> >& incomingStreamRecords);
  */
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  /* // Given a vector of streamRecord maps, collects the streamRecords associated with the currently active module (top of the gStack)
  // within each stream into nodeStreamRecords and returns the height of the gStacks on all the streams (must be the same number)
  static int collectNodeStreamRecords(std::vector<std::map<std::string, streamRecord*> >& streams,
                                      std::vector<std::map<std::string, streamRecord*> >& nodeStreamRecords);
  
  // Applies streamRecord::mergeIDs to the nodeIDs of the currently active module
  static int mergeNodeIDs(std::string objName, 
                          std::map<std::string, std::string>& pMap, 
                          const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                          std::map<std::string, streamRecord*>& outStreamRecords,
                          std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  */
  
  // Given multiple streamRecords from several variants of the same stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
      
  std::string str(std::string indent="") const;
}; // class ModuleStreamRecord

}; // namespace structure
}; // namespace sight
