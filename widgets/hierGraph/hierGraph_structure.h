// Copyright (c) 203 Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory
// Written by Greg Bronevetsky <bronevetsky1@llnl.gov>
//  
// LLNL-CODE-642002.
// All rights reserved.
//  
// This file is part of Sight. For details, see https://github.com/bronevet/sight. 
// Please read the COPYRIGHT file for Our Notice and
// for the BSD License.
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
#include "hierGraph_common.h"
#include "../../sight_structure_internal.h"
#include "Callpath.h"
#include <pthread.h>

namespace sight {
namespace structure {

class hierGraphConfHandlerInstantiator : common::confHandlerInstantiator {
public:
  hierGraphConfHandlerInstantiator();
};

extern hierGraphConfHandlerInstantiator hierGraphConfHandlerInstance;

class hierGraphTraceStream;

//#ifndef HIERGRAPH_STRUCTURE_C
// Rename for HG_contexts, HG_groups and HG_ports 
// that enables users to refer to them without prepending common::hierGraph
//typedef common::hierGraph::HG_group HG_group;
typedef common::hierGraph::context HG_context;
//typedef common::hierGraph::HG_context::config config;
//typedef common::hierGraph::HG_port HG_port;
//#endif


class HG_instance {
public:
  std::string name;
  int numInputs;
  int numOutputs;
  // kyushick edit
  int hierGraphID;  // object ID
  int horID;  // horizontal ID
  int verID;  // vertical ID
public:
  HG_instance() {}
  HG_instance(const std::string& name, int numInputs, int numOutputs, int hierGraphID, int horID, int verID) 
		: name(name), numInputs(numInputs), numOutputs(numOutputs), hierGraphID(hierGraphID), horID(horID), verID(verID) {}
  HG_instance(const HG_instance& that) 
		: name(that.name), numInputs(that.numInputs), numOutputs(that.numOutputs), hierGraphID(that.hierGraphID), horID(that.horID), verID(that.verID) {}
  HG_instance(properties::iterator props);

  // Returns the properties map that describes this HG_instance object;
  std::map<std::string, std::string> getProperties() const;

  bool operator==(const HG_instance& that) const
  { return name==that.name; }

  bool operator<(const HG_instance& that) const
  { return name<that.name; }

  // Returns a human-readable string that describes this HG_instance
  std::string str() const;
}; // class HG_instance

class hierGraph;

// HG_group represents the granularity at which we differentiate HG_instances of hierGraphs.
// Currently this is done by considering the name and count of HG_inputs and outputs of a given hierGraph HG_instance
// (Name) + (# of input/output)
// as well as the nesting hierarchy of HG_instances within which a given HG_instance is executed.
class HG_group {
public:
  // Stack of hierGraph HG_instances that uniquely identifies this hierGraph HG_grouping
  std::list<HG_instance> stack;
public:
  HG_group() { }
  HG_group(const std::list<HG_instance>& stack): stack(stack) {}
  HG_group(const HG_group& that) : stack(that.stack) {}
  // Creates a HG_group given the current stack of hierGraphs and a new hierGraph HG_instance
  HG_group(const std::list<hierGraph*>& hgStack, const HG_instance& inst);
  
  HG_group& operator=(const HG_group& that) { 
    stack = that.stack;
    return *this;
  }
  
  bool operator==(const HG_group& that) const
  { return stack == that.stack; }
  
  bool operator!=(const HG_group& that) const { return !(*this == that); }
  
  bool operator<(const HG_group& that) const
  { return stack < that.stack; }

  void init(const std::list<hierGraph*>& hgStack, const HG_instance& inst);

  // Add the given HG_instance to this HG_group's HG_instance stack
  void push(const HG_instance& inst) { stack.push_back(inst); }
  
  // Remove the last HG_instance from this HG_group's HG_instance stack
  void pop() { assert(stack.size()>0); stack.pop_back(); }
  
  // Returns the name of the most deeply nested HG_instance within this HG_group
  std::string name() const;

  // Returns the number of HG_inputs of the most deeply nested HG_instance within this HG_group
  int numInputs() const;
  
  // Returns the number of outnputs of the most deeply nested HG_instance within this HG_group
  int numOutputs() const;

  // Kyushick edit -------------------------------------------------------------------------------

  int hierGraphID() const;
  int horID() const;
  int verID() const;

  //----------------------------------------------------------------------------------------------
  
  // Returns the most deeply nested HG_instance within this HG_group
  const HG_instance& getInst() const;
  
  // Returns the depth of the callstack
  int depth() const;

  // Returns whether the HG_group's descriptor is empty, 
	// meaning that this object does not denote any specific HG_group
  bool isNULL() const { return stack.size()==0; }
  
  // Returns a human-readable string that describes this HG_group
  std::string str() const;
}; // class HG_group

class HG_port {
  public:
  HG_group g;
  // Points to a dynamically-allocated HG_instance of a HG_context object, which may be any class that derives from 
  // HG_context for use by different types of hierGraphs with their own notion of HG_context. 
	// This object is deallocated in this HG_port's destructor.
  HG_context* ctxt;
  sight::common::hierGraph::ioT type;
  int index;

  HG_port() 
		: ctxt(NULL) {}
  HG_port(const HG_port& that) 
		: g(that.g), ctxt(that.ctxt? that.ctxt->copy() : NULL), type(that.type), index(that.index) {}
  HG_port(const HG_context& ctxt) 
		: ctxt(ctxt.copy()), type(sight::common::hierGraph::output), index(-1) { }
  HG_port(const HG_group& g, const HG_context& ctxt, sight::common::hierGraph::ioT type, int index) 
		: g(g), ctxt(ctxt.copy()), type(type), index(index) {}
  HG_port(const HG_group& g, sight::common::hierGraph::ioT type, int index) : g(g), ctxt(NULL), type(type), index(index) {}
  ~HG_port() { if(ctxt) delete ctxt; }
  
  HG_port& operator=(const HG_port& that) {
    if(ctxt) delete ctxt;
    g    = that.g;
    ctxt = that.ctxt->copy();
    type = that.type;
    index = that.index;
    return *this;
  }
  
  bool operator==(const HG_port& that) const
  { return g==that.g && *ctxt==*that.ctxt && type==that.type && index==that.index; }

  bool operator<(const HG_port& that) const
  { return (g< that.g) ||
           (g==that.g && *ctxt< *that.ctxt) ||
           (g==that.g && *ctxt==*that.ctxt && type< that.type) ||
           (g==that.g && *ctxt==*that.ctxt && type==that.type && index<that.index); }

  // Sets the HG_context field
  void setCtxt(const HG_context& newCtxt) {
    if(ctxt) delete ctxt;
    ctxt = newCtxt.copy();
  }
  
  // Adds the given key/attrValue pair to the HG_port's HG_context
  void addCtxt(const std::string& key, const attrValue& val) {
    assert(ctxt);
    ctxt->configuration[key] = val;
  }
  
  // Erase the HG_context within this HG_port. 
	// This is important for data-structures that ignore HG_context details
  void clearContext() { ctxt->configuration.clear(); }

  // Returns a human-readable string that describes this HG_context
  std::string str() const;
}; // class HG_port

// Syntactic sugar for specifying HG_inputs
typedef common::easyvector<HG_port> HG_inputs;

class HG_inport : public HG_port {
  public:
  HG_inport() {}
  HG_inport(const HG_group& g, const HG_context& c, int index) : HG_port(g, c, sight::common::hierGraph::input, index) {}
};

class HG_outport : public HG_port {
  public:
  HG_outport() {}
  HG_outport(const HG_group& g, const HG_context& c, int index) : HG_port(g, c, sight::common::hierGraph::output, index) {}
};

// Records the hierarchy of nesting observed for hierGraph HG_instances
class HG_instanceTree {
  std::map<HG_instance, HG_instanceTree*> m;
  
public:
  HG_instanceTree() {}
  
  void add(const HG_group& g);
  
  // Types for callback functions to be executed on entry to / exit from an HG_instance during the call to iterate
  typedef void (*HG_instanceEnterFunc)(const HG_group& g);
  typedef void (*HG_instanceExitFunc) (const HG_group& g);
  
  // Iterate through all the HG_groups encoded by this tree, where each HG_group corresponds to a stack of HG_instances
  // from the tree's root to one of its leaves.
  void iterate(HG_instanceEnterFunc entry, HG_instanceExitFunc exit) const;
  
  // Recursive version of iterate that takes as an argument the fragment of the current HG_group that corresponds to
  // the stack of HG_instances between the tree's root and the current sub-tree
  void iterate(HG_instanceEnterFunc entry, HG_instanceExitFunc exit, HG_group& g) const;
  
  // Empties out this HG_instanceTree
  void clear();
  
  // The entry and exit functions used in HG_instanceTree::str()
  static void strEnterFunc(const HG_group& g);
  static void strExitFunc(const HG_group& g);
  
  // Returns a human-readable string representation of this object
  std::string str() const;
  
  // The depth of the recursion in HG_instanceTree::str()
  static int HG_instanceTreeStrDepth;

  // The ostringstream into which the output of HG_instanceTree::str() is accumulated
  static std::ostringstream oss;
}; // class HG_instanceTree


class hierGraph;

// Base class for functors that generate traceStreams that are specific to different sub-types of hierGraph.
// We need this so that we can pass a reference to the correct genTS() method to hierGraphApp::enterHierGraph(). Since this
// call is made inside the constructor of hierGraph, we can't use virtual methods to ensure that the correct version
// of genTS will be called by just passing a reference to the current hierGraph-derived object
//typedef traceStream* (*HG_generateTraceStream)(int hierGraphID, const HG_group& g);
class HG_generateTraceStream {
  public:
  virtual traceStream* operator()(int hierGraphID)=0;
}; // class HG_generateTraceStream


// Represents a modular application, which may contain one or more hierGraphs. 
// Only one modular application may be in-scope at any given point in time.
class hierGraphApp: public block
{
  friend class HG_group;
  friend class hierGraph;
  
  protected:
  // Points to the currently active HG_instance of hierGraphApp. There can be only one.
  static hierGraphApp* activeMA;
    
  // The maximum ID ever assigned to any modular application
  static int maxHierGraphAppID;
  
  // The maximum ID ever assigned to any hierGraph HG_group
  static int maxHierGraphGroupID;

  // Records all the known HG_contexts, mapping each HG_context to its unique ID
  static std::map<HG_group, int> HG_group2ID;
  	
  // Maps each HG_context to the number of times it was ever observed
  static std::map<HG_group, int> HG_group2Count;
  
  // The trace that records performance observations of different hierGraphs and HG_contexts
  static std::map<HG_group, traceStream*> hierGraphTrace;
  static std::map<HG_group, int>          hierGraphTraceID;
  
  // Tree that records the hierarchy of hierGraph HG_instances that were observed during the execution of this
  // modular application. Each path from the tree's root to a leaf is a stack of hierGraph HG_instances that
  // corresponds to some observed hierGraph HG_group.
  static HG_instanceTree tree;
  
  // Maps each hierGraph to the list of the names of its input and output HG_context attributes. 
  // This enables us to verify that all the hierGraphs are used consistently.
  static std::map<HG_group, std::vector<std::list<std::string> > > hierGraphInCtxtNames;
  static std::map<HG_group, std::vector<std::list<std::string> > > hierGraphOutCtxtNames;
  
  // The properties object that describes each hierGraph HG_group. This object is created by calling each HG_group's
  // setProperties() method and each call to this method for the same hierGraph HG_group must return the same properties.
  static std::map<HG_group, properties*> hierGraphProps;
    
  // Records all the edges ever observed, mapping them to the number of times each edge was observed
  static std::map<std::pair<HG_port, HG_port>, int> edges;
  
  // Stack of the hierGraphs that are currently in scope
  static std::list<hierGraph*> hgStack;
  
public:
  // Returns a constant reference to the current stack of hierGraphs
  static const std::list<hierGraph*>& getMStack() { return hgStack; }
  
  // The unique ID of this application
  int appID;
  
  // The name of this application
  std::string appName;
  
  // The set of measurements that will be collected for all the hierGraphs within this modular app
  namedMeasures meas;

public:
  hierGraphApp(const std::string& appName,                                                   properties* props=NULL);
  hierGraphApp(const std::string& appName, const attrOp& onoffOp,                            properties* props=NULL);
  hierGraphApp(const std::string& appName,                        const namedMeasures& meas, properties* props=NULL);
  hierGraphApp(const std::string& appName, const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  
  // Stack used while we're emitting the nesting hierarchy of hierGraph HG_groups to keep each hierGraph HG_group's 
  // sightObject between the time the HG_group is entered and exited
  static std::list<sightObj*> hierGraphEmitStack;

  // Emits the entry tag for a hierGraph HG_group during the execution of ~hierGraphApp()
  static void enterHierGraphGroup(const HG_group& g);
  // Emits the exit tag for a hierGraph HG_group during the execution of ~hierGraphApp()
  static void exitHierGraphGroup(const HG_group& g);
  
  ~hierGraphApp();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
private:
  // Common initialization logic
  void init();
    
  // Sets the properties of this object
  static properties* setProperties(const std::string& appName, const attrOp* onoffOp, properties* props);
  
public:
  // Returns the hierGraph ID of the given hierGraph HG_group, generating a fresh one if one has not yet been assigned
  static int genHierGraphID(const HG_group& g);
 
  // kyushick edit 
  static int recvHierGraphID(const HG_group& g);

  // Returns whether the current HG_instance of hierGraphApp is active
  static bool isInstanceActive();

  // Returns a pointer to the current HG_instance of hierGraphApp
  static hierGraphApp* getInstance() { assert(activeMA); return activeMA; }
  
  // Assigns a unique ID to the given hierGraph HG_group, as needed and returns this ID
  static int addHierGraphGroup(const HG_group& g);
  
  // Registers the names of the HG_contexts of the given hierGraph's HG_inputs or outputs 
	// and if this is not the first time this hierGraph is called, 
  // verifies that these HG_context names are consistent across different calls.
	// 
  // g 			- the hierGraph HG_group for which we're registering HG_inputs/outputs
  // inouts - the vector of input or output HG_ports
  // toT 		- identifies whether inouts is the vector of HG_inputs or outputs
  static void registerInOutContexts(const HG_group& g, const std::vector<HG_port>& inouts, sight::common::hierGraph::ioT io);
  
  // Add an edge between one hierGraph's output HG_port and another hierGraph's input HG_port
  static void addEdge(HG_port from, HG_port to);

  // Add an edge between one hierGraph's output HG_port and another hierGraph's input HG_port
  static void addEdge(HG_group fromG, sight::common::hierGraph::ioT fromT, int fromP, 
                      HG_group toG,   sight::common::hierGraph::ioT toT,   int toP);

  // Returns the current hierGraph on the stack and NULL if the stack is empty
  static hierGraph* getCurHierGraph();
  
  // Adds the given hierGraph object to the hierGraphs stack
  static void enterHierGraph(hierGraph* m, int hierGraphID, properties* props/*, HG_generateTraceStream& tsGenerator*/);
  
  // Returns whether a traceStream has been registered for the given hierGraph HG_group
  static bool isTraceStreamRegistered(const HG_group& g);

  // Registers the given traceStream for the given hierGraph HG_group
  static void registerTraceStream(const HG_group& g, traceStream* ts);
  
  // Registers the ID of the traceStream that will be used for the given hierGraph HG_group
  static void registerTraceStreamID(const HG_group& g, int traceID);
  
  // Returns the currently registered the ID of the traceStream that will be used for the given hierGraph HG_group
  static int getTraceStreamID(const HG_group& g);

  // Removes the given hierGraph object from the hierGraphs stack
  static void exitHierGraph(hierGraph* m);

  // -------------------------
  // ----- Configuration -----
  // -------------------------
  // Currently there isn't anything that can be configured but in the future we may wish to
  // add measurements that will be taken on all hierGraphs
public:
  class HierGraphAppConfiguration : public common::Configuration{
  public:
    HierGraphAppConfiguration(properties::iterator props) 
			: common::Configuration(props.next()) 
		{}
  };

  static common::Configuration* configure(properties::iterator props) {
    // Create a HierGraphConfiguration object, using the invocation of the constructor hierarchy to
    // record the configuration details with the respective widgets from which hierGraphs inherit
    HierGraphAppConfiguration* c = new HierGraphAppConfiguration(props);
    delete c;
    return NULL;
  }
}; // class hierGraphApp

class hierGraph: public sightObj, public common::hierGraph
{
  friend class HG_group;
  friend class hierGraphApp;
  
protected:
  
  int hierGraphID_;
  HG_group g;
  
  // The HG_context of this hierGraph execution, which is a combination of the HG_contexts of all of its HG_inputs
  std::vector<HG_context> ctxt;
  
  // The HG_context in a format that traces can understand, set in setTraceCtxt() of the the class that
  // ultimately derives from hierGraph.
  std::map<std::string, attrValue> traceCtxt;

public:
  const std::map<std::string, attrValue>& getTraceCtxt() { return traceCtxt; }

protected:
  
  // The input HG_ports of this hierGraph
  std::vector<HG_port> ins;
  
  // The  output HG_ports of this hierGraph
  std::vector<HG_port> outs;
  
  // Set of all the outputs that have already been set by the user. Used to issue
  // warnings when an output has been set multiple times or has not been set.
  std::set<int> outsSet;
  
  // We allow the user to provide a pointer to an output vector, which is populated by the hierGraph
  // object. This is a pointer to this vector.
  std::vector<HG_port>* externalOutputs;
  
  // List of all the measurements that should be taken during the execution of this hierGraph. This is a list of pointers to
  // measure objects. When the hierGraph HG_instance is deleted, its measure objects are automatically deleted as well.
  namedMeasures meas;
  
  // Records the observation made during the execution of this hierGraph. obs may be filled in
  // all at once in the destructor. Alternately, if users call completeMeasurement() directly,
  // the measurements are written to obs then and the outputs are written in the destructor.
  std::list<std::pair<std::string, attrValue> > obs;
  
  // Information that describes the class that derives from this on. 
/*
	class derivInfo {
    public:
    
    // A pointer to a properties object that can be used to create a tag for the derived object that 
    // includes info about its parents.
    properties* props;
    
    // Fields that must be included within the HG_context of any trace observations made during the execution of this hierGraph.
    std::map<std::string, attrValue> ctxt;
    
    // Points to a function that generates the trace stream HG_instance specific to the given derivation of hierGraph
    //HG_generateTraceStream& tsGenerator;
    
    derivInfo(/ *HG_generateTraceStream& tsGenerator* /)/ * : tsGenerator(tsGenerator)* / {
      props = new properties();
    }
    
    derivInfo(properties* props, const std::map<std::string, attrValue>& ctxt/ *, HG_generateTraceStream& tsGenerator* /) :
                  props(props), ctxt(ctxt)/ *, tsGenerator(tsGenerator)* / { }
  }; // class derivInfo 
*/
  
  // Records whether this class has been derived by another. 
	// In this case ~hierGraph() relies on the destructor of that class to create an appropriate traceStream.
  bool isDerived;
  
public:
  // HG_inputs - HG_ports from other hierGraphs that are used as HG_inputs by this hierGraph.
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  // derivInfo: Information that describes the class that derives from this on.
  hierGraph(const HG_instance& inst,                                                                                                                        properties* props=NULL);
  hierGraph(const HG_instance& inst, const HG_port& HG_inputs,                                                                                                    properties* props=NULL);
  hierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,                                                                                       properties* props=NULL);
  hierGraph(const HG_instance& inst,                                                                      const attrOp& onoffOp,                            properties* props=NULL);
  hierGraph(const HG_instance& inst, const HG_port& HG_inputs,                                                  const attrOp& onoffOp,                            properties* props=NULL);
  hierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,                                     const attrOp& onoffOp,                            properties* props=NULL);
  hierGraph(const HG_instance& inst,                                  std::vector<HG_port>& externalOutputs,                                                   properties* props=NULL);
  hierGraph(const HG_instance& inst, const HG_port& HG_inputs,              std::vector<HG_port>& externalOutputs,                                                   properties* props=NULL);
  hierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs,                                                   properties* props=NULL);
  hierGraph(const HG_instance& inst,                                  std::vector<HG_port>& externalOutputs, const attrOp& onoffOp,                            properties* props=NULL);
  hierGraph(const HG_instance& inst, const HG_port& HG_inputs,              std::vector<HG_port>& externalOutputs, const attrOp& onoffOp,                            properties* props=NULL);
  hierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, const attrOp& onoffOp,                            properties* props=NULL);
    
  hierGraph(const HG_instance& inst,                                                                                             const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst, const HG_port& HG_inputs,                                                                         const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,                                                            const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst,                                                                      const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst, const HG_port& HG_inputs,                                                  const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,                                     const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst,                                  std::vector<HG_port>& externalOutputs,                        const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst, const HG_port& HG_inputs,              std::vector<HG_port>& externalOutputs,                        const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs,                        const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst,                                  std::vector<HG_port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst, const HG_port& HG_inputs,              std::vector<HG_port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  hierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);
  
  properties* setProperties(const HG_instance& inst, properties* props, const attrOp* onoffOp, hierGraph* me);

  void init(const std::vector<HG_port>& in, properties* props);
  
private:
  // Sets the properties of this object
  //static properties* setProperties(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, const attrOp* onoffOp, properties* props);
  
public:
  ~hierGraph();
  
protected:
  // Records whether we've completed measuring this hierGraph's behavior
  bool measurementCompleted;
  
public:
  // Called to complete the measurement of this hierGraph's execution. This measurement may be completed before
  // the hierGraph itself completes to enable users to separate the HG_portion of the hierGraph's execution that 
  // represents its core behavior (and thus should be measured) from the HG_portion where the hierGraph's outputs
  // are computed.
  virtual void completeMeasurement();
  
  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  // The variant of the HG_generateTraceStream functor specialized to generating hierGraphTraceStreams
  /*class generateHierGraphTraceStream : public HG_generateTraceStream {
    protected:
    const HG_group* g;
    public:
    generateHierGraphTraceStream() {}
    generateHierGraphTraceStream(const HG_group& g): g(&g) { }
    
    void init(const HG_group& g) { this->g = &g; }
    
    traceStream* operator()(int hierGraphID);
  }; // class generateHierGraphTraceStream
*/
  
  // Returns the properties of this object
  //properties* setProperties(int hierGraphID, properties* props=NULL);
  
  const std::vector<HG_context>& getContext() const { return ctxt; }
  std::string name() const { return g.name(); }
  int numInputs()    const { return g.numInputs(); }
  int numOutputs()   const { return g.numOutputs(); }
  // kyushick edit
  int hierGraphID()  const { return hierGraphID_; }
  int horID()        const { return g.horID(); }
  int verID()        const { return g.verID(); }

  const HG_group& getGroup() const { return g; }
  
  // Sets the HG_context of the given output HG_port
  virtual void setInCtxt(int idx, const HG_context& c);
  
  // Adds the given key/attrValue pair to the HG_context of the given output HG_port
  virtual void addInCtxt(int idx, const std::string& key, const attrValue& val);
  
  // Adds the given HG_port to this hierGraph's HG_inputs
  virtual void addInCtxt(const HG_port& p);
  
  // Sets the HG_context of the given output HG_port
  virtual void setOutCtxt(int idx, const HG_context& c);
  
  
  // Returns a list of the hierGraph's input HG_ports
  //std::vector<HG_port> inputPorts() const;
 
  // Returns a list of the hierGraph's output HG_ports
  std::vector<HG_port> outPorts() const;
  HG_port outPort(int idx) const;

  // Sets the traceCtxt map, which contains the HG_context attributes to be used in this hierGraph's measurements 
  // by combining the HG_context provided by the classes that this object derives from with its own unique 
  // HG_context attributes.
  void setTraceCtxt();
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }


  // -------------------------
  // ----- Configuration -----
  // -------------------------
  // Currently there isn't anything that can be configured but in the future we may wish to
  // add measurements that will be taken on all hierGraphs
  public:
  class HierGraphConfiguration : public common::Configuration{
    public:
    HierGraphConfiguration(properties::iterator props) : common::Configuration(props.next()) {}
  };
  static common::Configuration* configure(properties::iterator props) { 
    // Create a HierGraphConfiguration object, using the invocation of the constructor hierarchy to
    // record the configuration details with the respective widgets from which hierGraphs inherit
    HierGraphConfiguration* c = new HierGraphConfiguration(props); 
    delete c;
    return NULL;
  }
}; // hierGraph



// Extends the normal HG_context by allowing the caller to specify a description of the comparator to be used
// for each key
class HG_compContext: public HG_context {
  public:
  // Maps each HG_context key to the pair <comparator name, comparator description>
  std::map<std::string, std::pair<std::string, std::string> > comparators;

  HG_compContext() {}

  HG_compContext(const HG_compContext& that) : HG_context((const HG_context&) that), comparators(that.comparators) {}
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0) : 
    HG_context(name0, val0)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1) : 
    HG_context(name0, val0, name1, val1)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1, const std::string& name2, const attrValue& val2, const comparatorDesc& cdesc2) :
    HG_context(name0, val0, name1, val1, name2, val2)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); comparators[name2] = std::make_pair(cdesc2.name(), cdesc2.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1, const std::string& name2, const attrValue& val2, const comparatorDesc& cdesc2, const std::string& name3, const attrValue& val3, const comparatorDesc& cdesc3) :
    HG_context(name0, val0, name1, val1, name2, val2, name3, val3)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); comparators[name2] = std::make_pair(cdesc2.name(), cdesc2.description()); comparators[name3] = std::make_pair(cdesc3.name(), cdesc3.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1, const std::string& name2, const attrValue& val2, const comparatorDesc& cdesc2, const std::string& name3, const attrValue& val3, const comparatorDesc& cdesc3, const std::string& name4, const attrValue& val4, const comparatorDesc& cdesc4) :
    HG_context(name0, val0, name1, val1, name2, val2, name3, val3, name4, val4)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); comparators[name2] = std::make_pair(cdesc2.name(), cdesc2.description()); comparators[name3] = std::make_pair(cdesc3.name(), cdesc3.description()); comparators[name4] = std::make_pair(cdesc4.name(), cdesc4.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1, const std::string& name2, const attrValue& val2, const comparatorDesc& cdesc2, const std::string& name3, const attrValue& val3, const comparatorDesc& cdesc3, const std::string& name4, const attrValue& val4, const comparatorDesc& cdesc4, const std::string& name5, const attrValue& val5, const comparatorDesc& cdesc5) :
    HG_context(name0, val0, name1, val1, name2, val2, name3, val3, name4, val4, name5, val5)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); comparators[name2] = std::make_pair(cdesc2.name(), cdesc2.description()); comparators[name3] = std::make_pair(cdesc3.name(), cdesc3.description()); comparators[name4] = std::make_pair(cdesc4.name(), cdesc4.description()); comparators[name5] = std::make_pair(cdesc5.name(), cdesc5.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1, const std::string& name2, const attrValue& val2, const comparatorDesc& cdesc2, const std::string& name3, const attrValue& val3, const comparatorDesc& cdesc3, const std::string& name4, const attrValue& val4, const comparatorDesc& cdesc4, const std::string& name5, const attrValue& val5, const comparatorDesc& cdesc5, const std::string& name6, const attrValue& val6, const comparatorDesc& cdesc6) :
    HG_context(name0, val0, name1, val1, name2, val2, name3, val3, name4, val4, name5, val5, name6, val6)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); comparators[name2] = std::make_pair(cdesc2.name(), cdesc2.description()); comparators[name3] = std::make_pair(cdesc3.name(), cdesc3.description()); comparators[name4] = std::make_pair(cdesc4.name(), cdesc4.description()); comparators[name5] = std::make_pair(cdesc5.name(), cdesc5.description()); comparators[name6] = std::make_pair(cdesc6.name(), cdesc6.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1, const std::string& name2, const attrValue& val2, const comparatorDesc& cdesc2, const std::string& name3, const attrValue& val3, const comparatorDesc& cdesc3, const std::string& name4, const attrValue& val4, const comparatorDesc& cdesc4, const std::string& name5, const attrValue& val5, const comparatorDesc& cdesc5, const std::string& name6, const attrValue& val6, const comparatorDesc& cdesc6, const std::string& name7, const attrValue& val7, const comparatorDesc& cdesc7) :
    HG_context(name0, val0, name1, val1, name2, val2, name3, val3, name4, val4, name5, val5, name6, val6, name7, val7)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); comparators[name2] = std::make_pair(cdesc2.name(), cdesc2.description()); comparators[name3] = std::make_pair(cdesc3.name(), cdesc3.description()); comparators[name4] = std::make_pair(cdesc4.name(), cdesc4.description()); comparators[name5] = std::make_pair(cdesc5.name(), cdesc5.description()); comparators[name6] = std::make_pair(cdesc6.name(), cdesc6.description()); comparators[name7] = std::make_pair(cdesc7.name(), cdesc7.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1, const std::string& name2, const attrValue& val2, const comparatorDesc& cdesc2, const std::string& name3, const attrValue& val3, const comparatorDesc& cdesc3, const std::string& name4, const attrValue& val4, const comparatorDesc& cdesc4, const std::string& name5, const attrValue& val5, const comparatorDesc& cdesc5, const std::string& name6, const attrValue& val6, const comparatorDesc& cdesc6, const std::string& name7, const attrValue& val7, const comparatorDesc& cdesc7, const std::string& name8, const attrValue& val8, const comparatorDesc& cdesc8) :
    HG_context(name0, val0, name1, val1, name2, val2, name3, val3, name4, val4, name5, val5, name6, val6, name7, val7, name8, val8)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); comparators[name2] = std::make_pair(cdesc2.name(), cdesc2.description()); comparators[name3] = std::make_pair(cdesc3.name(), cdesc3.description()); comparators[name4] = std::make_pair(cdesc4.name(), cdesc4.description()); comparators[name5] = std::make_pair(cdesc5.name(), cdesc5.description()); comparators[name6] = std::make_pair(cdesc6.name(), cdesc6.description()); comparators[name7] = std::make_pair(cdesc7.name(), cdesc7.description()); comparators[name8] = std::make_pair(cdesc8.name(), cdesc8.description()); }
  
  HG_compContext(const std::string& name0, const attrValue& val0, const comparatorDesc& cdesc0, const std::string& name1, const attrValue& val1, const comparatorDesc& cdesc1, const std::string& name2, const attrValue& val2, const comparatorDesc& cdesc2, const std::string& name3, const attrValue& val3, const comparatorDesc& cdesc3, const std::string& name4, const attrValue& val4, const comparatorDesc& cdesc4, const std::string& name5, const attrValue& val5, const comparatorDesc& cdesc5, const std::string& name6, const attrValue& val6, const comparatorDesc& cdesc6, const std::string& name7, const attrValue& val7, const comparatorDesc& cdesc7, const std::string& name8, const attrValue& val8, const comparatorDesc& cdesc8, const std::string& name9, const attrValue& val9, const comparatorDesc& cdesc9) :
    HG_context(name0, val0, name1, val1, name2, val2, name3, val3, name4, val4, name5, val5, name6, val6, name7, val7, name8, val8, name9, val9)
  { comparators[name0] = std::make_pair(cdesc0.name(), cdesc0.description()); comparators[name1] = std::make_pair(cdesc1.name(), cdesc1.description()); comparators[name2] = std::make_pair(cdesc2.name(), cdesc2.description()); comparators[name3] = std::make_pair(cdesc3.name(), cdesc3.description()); comparators[name4] = std::make_pair(cdesc4.name(), cdesc4.description()); comparators[name5] = std::make_pair(cdesc5.name(), cdesc5.description()); comparators[name6] = std::make_pair(cdesc6.name(), cdesc6.description()); comparators[name7] = std::make_pair(cdesc7.name(), cdesc7.description()); comparators[name8] = std::make_pair(cdesc8.name(), cdesc8.description()); comparators[name9] = std::make_pair(cdesc9.name(), cdesc9.description()); }
  
  HG_compContext(const std::map<std::string, attrValue>& configuration, const std::map<std::string, std::pair<std::string, std::string> >& comparators) : 
    HG_context(configuration), comparators(comparators) {}

  // Loads this HG_context from the given properties map. The names of all the fields are assumed to be prefixed
  // with the given string.
  HG_compContext(properties::iterator props, std::string prefix="");
  
  // Returns a dynamically-allocated copy of this object. This method must be implemented by all classes
  // that inherit from HG_context to make sure that appropriate copies of them can be created.
  virtual HG_context* copy() const { return new HG_compContext(*this); }
  
  // These comparator routines must be implemented by all classes that inherit from HG_context to make sure that
  // their additional details are reflected in the results of the comparison. Implementors may assume that 
  // the type of that is their special derivation of HG_context rather than a generic HG_context and should dynamically
  // cast from const HG_context& to their sub-type.
  virtual bool operator==(const HG_context& that) const;

  virtual bool operator<(const HG_context& that) const;

  // Adds the given key/attrValue pair to this HG_context
  void add(std::string key, const attrValue& val, const comparatorDesc& cdesc);

  // Add all the key/attrValue pairs from the given HG_context to this one, overwriting keys as needed
  void add(const HG_compContext& that);

  // Returns the properties map that describes this HG_context object.
  // The names of all the fields in the map are prefixed with the given string.
  std::map<std::string, std::string> getProperties(std::string prefix="") const;
  
  // Returns a human-readable string that describes this HG_context
  virtual std::string str() const;
}; // class HG_compContext

class compHierGraphTraceStream;

class HG_compNamedMeasures {
  public:
  
  // Records all the information about a named measures and their comparators
  class info {
    public:
    measure* meas; // Points to the measure object
    std::string compName; // The name of the comparator to be used with this measure
    std::string compDesc; // The description of the comparator to be used with this measure
  
    info() {}
    info(measure* meas, std::string compName, std::string compDesc) : meas(meas), compName(compName), compDesc(compDesc) {}
  };
    
  // Maps the name of each measure to its info
  std::map<std::string, info> measures;

  HG_compNamedMeasures() {}
  
  HG_compNamedMeasures(const HG_compNamedMeasures& that) : measures(that.measures) {}
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1, const std::string& name2, measure* meas2, const comparatorDesc& cdesc2)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); measures[name2] = info(meas2, cdesc2.name(), cdesc2.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1, const std::string& name2, measure* meas2, const comparatorDesc& cdesc2, const std::string& name3, measure* meas3, const comparatorDesc& cdesc3)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); measures[name2] = info(meas2, cdesc2.name(), cdesc2.description()); measures[name3] = info(meas3, cdesc3.name(), cdesc3.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1, const std::string& name2, measure* meas2, const comparatorDesc& cdesc2, const std::string& name3, measure* meas3, const comparatorDesc& cdesc3, const std::string& name4, measure* meas4, const comparatorDesc& cdesc4)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); measures[name2] = info(meas2, cdesc2.name(), cdesc2.description()); measures[name3] = info(meas3, cdesc3.name(), cdesc3.description()); measures[name4] = info(meas4, cdesc4.name(), cdesc4.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1, const std::string& name2, measure* meas2, const comparatorDesc& cdesc2, const std::string& name3, measure* meas3, const comparatorDesc& cdesc3, const std::string& name4, measure* meas4, const comparatorDesc& cdesc4, const std::string& name5, measure* meas5, const comparatorDesc& cdesc5)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); measures[name2] = info(meas2, cdesc2.name(), cdesc2.description()); measures[name3] = info(meas3, cdesc3.name(), cdesc3.description()); measures[name4] = info(meas4, cdesc4.name(), cdesc4.description()); measures[name5] = info(meas5, cdesc5.name(), cdesc5.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1, const std::string& name2, measure* meas2, const comparatorDesc& cdesc2, const std::string& name3, measure* meas3, const comparatorDesc& cdesc3, const std::string& name5, const std::string& name4, measure* meas4, const comparatorDesc& cdesc4, measure* meas5, const comparatorDesc& cdesc5, const std::string& name6, measure* meas6, const comparatorDesc& cdesc6)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); measures[name2] = info(meas2, cdesc2.name(), cdesc2.description()); measures[name3] = info(meas3, cdesc3.name(), cdesc3.description()); measures[name4] = info(meas4, cdesc4.name(), cdesc4.description()); measures[name5] = info(meas5, cdesc5.name(), cdesc5.description()); measures[name6] = info(meas6, cdesc6.name(), cdesc6.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1, const std::string& name2, measure* meas2, const comparatorDesc& cdesc2, const std::string& name3, measure* meas3, const comparatorDesc& cdesc3, const std::string& name5, const std::string& name4, measure* meas4, const comparatorDesc& cdesc4, measure* meas5, const comparatorDesc& cdesc5, const std::string& name6, measure* meas6, const comparatorDesc& cdesc6, const std::string& name7, measure* meas7, const comparatorDesc& cdesc7)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); measures[name2] = info(meas2, cdesc2.name(), cdesc2.description()); measures[name3] = info(meas3, cdesc3.name(), cdesc3.description()); measures[name4] = info(meas4, cdesc4.name(), cdesc4.description()); measures[name5] = info(meas5, cdesc5.name(), cdesc5.description()); measures[name6] = info(meas6, cdesc6.name(), cdesc6.description()); measures[name7] = info(meas7, cdesc7.name(), cdesc7.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1, const std::string& name2, measure* meas2, const comparatorDesc& cdesc2, const std::string& name3, measure* meas3, const comparatorDesc& cdesc3, const std::string& name5, const std::string& name4, measure* meas4, const comparatorDesc& cdesc4, measure* meas5, const comparatorDesc& cdesc5, const std::string& name6, measure* meas6, const comparatorDesc& cdesc6, const std::string& name7, measure* meas7, const comparatorDesc& cdesc7, const std::string& name8, measure* meas8, const comparatorDesc& cdesc8)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); measures[name2] = info(meas2, cdesc2.name(), cdesc2.description()); measures[name3] = info(meas3, cdesc3.name(), cdesc3.description()); measures[name4] = info(meas4, cdesc4.name(), cdesc4.description()); measures[name5] = info(meas5, cdesc5.name(), cdesc5.description()); measures[name6] = info(meas6, cdesc6.name(), cdesc6.description()); measures[name7] = info(meas7, cdesc7.name(), cdesc7.description()); measures[name8] = info(meas8, cdesc8.name(), cdesc8.description()); }
  
  HG_compNamedMeasures(const std::string& name0, measure* meas0, const comparatorDesc& cdesc0, const std::string& name1, measure* meas1, const comparatorDesc& cdesc1, const std::string& name2, measure* meas2, const comparatorDesc& cdesc2, const std::string& name3, measure* meas3, const comparatorDesc& cdesc3, const std::string& name5, const std::string& name4, measure* meas4, const comparatorDesc& cdesc4, measure* meas5, const comparatorDesc& cdesc5, const std::string& name6, measure* meas6, const comparatorDesc& cdesc6, const std::string& name7, measure* meas7, const comparatorDesc& cdesc7, const std::string& name8, measure* meas8, const comparatorDesc& cdesc8, const std::string& name9, measure* meas9, const comparatorDesc& cdesc9)
  { measures[name0] = info(meas0, cdesc0.name(), cdesc0.description()); measures[name1] = info(meas1, cdesc1.name(), cdesc1.description()); measures[name2] = info(meas2, cdesc2.name(), cdesc2.description()); measures[name3] = info(meas3, cdesc3.name(), cdesc3.description()); measures[name4] = info(meas4, cdesc4.name(), cdesc4.description()); measures[name5] = info(meas5, cdesc5.name(), cdesc5.description()); measures[name6] = info(meas6, cdesc6.name(), cdesc6.description()); measures[name7] = info(meas7, cdesc7.name(), cdesc7.description()); measures[name8] = info(meas8, cdesc8.name(), cdesc8.description()); measures[name9] = info(meas9, cdesc9.name(), cdesc9.description()); }
  
  // Returns the properties map that describes this HG_context object.
  // The names of all the fields in the map are prefixed with the given string.
  //std::map<std::string, std::string> getProperties(std::string prefix="") const;
  
  // Returns a map that maps the names of all the named measurements to the pair of strings that records the 
  // name and description of the comparison to be used with each measurement
  std::map<std::string, std::pair<std::string, std::string> > getComparators() const {
    std::map<std::string, std::pair<std::string, std::string> > name2comp;
    for(std::map<std::string, info>::const_iterator m=measures.begin(); m!=measures.end(); m++)
      name2comp[m->first] = make_pair(m->second.compName, m->second.compDesc);
    return name2comp;
  }
  
  // Returns a mapping from the names of all the named measurements to pointers of their corresponding measure objects
  namedMeasures getNamedMeasures() const {
    namedMeasures nm;
    for(std::map<std::string, info>::const_iterator m=measures.begin(); m!=measures.end(); m++)
      nm[m->first] = m->second.meas;
    return nm;
  }
}; // class HG_compNamedMeasures

// Represents a modular application, which may contain one or more hierGraphs. Only one modular application may be
// in-scope at any given point in time.
class compHierGraphApp : public hierGraphApp
{
  // Maps the names of of all the measurements that should be taken during the execution of compHierGraphs within
  // this modular app to the names and descriptors of the comparisons that should be performed for them.
  std::map<std::string, std::pair<std::string, std::string> > measComp;
  
  public:
  compHierGraphApp(const std::string& appName,                                                        properties* props=NULL);
  compHierGraphApp(const std::string& appName, const attrOp& onoffOp,                                 properties* props=NULL);
  compHierGraphApp(const std::string& appName,                        const HG_compNamedMeasures& cMeas, properties* props=NULL);
  compHierGraphApp(const std::string& appName, const attrOp& onoffOp, const HG_compNamedMeasures& cMeas, properties* props=NULL);

  ~compHierGraphApp();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();

  // -------------------------
  // ----- Configuration -----
  // -------------------------
  // Currently there isn't anything that can be configured but in the future we may wish to
  // add measurements that will be taken on all hierGraphs
  public:
  class CompHierGraphAppConfiguration : public HierGraphAppConfiguration{
    public:
    CompHierGraphAppConfiguration(properties::iterator props) : HierGraphAppConfiguration(props.next()) {}
  };
}; // class compHierGraphApp


class compHierGraph: public structure::hierGraph
{
  friend class compHierGraphTraceStream;
public:
  
  protected:
  // Records whether this is the reference configuration of the moculde
  bool isReference;
    
  // The HG_context that describes the configuration options of this hierGraph
  HG_context options;
  
  // Records whether this class has been derived by another. In this case ~hierGraph() relies on the destructor of
  // that class to create an appropriate traceStream.
  bool isDerived;
  
  // Maps the names of of all the measurements that should be taken during the execution of this hierGraph to the 
  // names and descriptors of the comparisons that should be performed for them.
  std::map<std::string, std::pair<std::string, std::string> > measComp;
  
  public:
  
  // Information that describes the class that derives from this on. 
/*
	class compDerivInfo : public derivInfo {
    public:
    // The options that the derived class wishes to pass to this compHierGraph. The issue is that because we pass
    // options by reference if a derived class wishes to extend them, it cannot do so from within a constructor.
    // Adding options to derivInfo make this possible;
    HG_context options;
    
    compDerivInfo() : derivInfo() {}
    
    compDerivInfo(properties* props, const std::map<std::string, attrValue>& ctxt, const HG_context& options) : 
      derivInfo(props, ctxt), options(options) {}
  };
*/
  
  // isReference: Records whether this is the reference configuration of the moculde
  // options: The HG_context that describes the configuration options of this hierGraph
  // meas: The measurements that should be performed during the execution of this compHierGraph
  // derivInfo: Information that describes the class that derives from this on. It includes a pointer to a properties
  //    object that can be used to create a tag for the derived object that includes info about its parents. Further,
  //    it includes fields that must be included within the HG_context of any trace observations made during the execution
  //    of this hierGraph.
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,  
             bool isReference,
                                                               properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,  
             bool isReference, 
             const attrOp& onoffOp,                            properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,  
             bool isReference,
                                    const HG_compNamedMeasures& meas, properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,  
             bool isReference, 
             const attrOp& onoffOp, const HG_compNamedMeasures& meas, properties* props=NULL);

  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             bool isReference,
                                                               properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             bool isReference, 
             const attrOp& onoffOp,                            properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             bool isReference,
                                    const HG_compNamedMeasures& meas, properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             bool isReference, 
             const attrOp& onoffOp, const HG_compNamedMeasures& meas, properties* props=NULL);

  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,  
             bool isReference, HG_context options,
                                                               properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,  
             bool isReference, HG_context options, 
             const attrOp& onoffOp,                            properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,  
             bool isReference, HG_context options,
                                    const HG_compNamedMeasures& meas, properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs,  
             bool isReference, HG_context options, 
             const attrOp& onoffOp, const HG_compNamedMeasures& meas, properties* props=NULL);

  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             bool isReference, HG_context options,
                                                               properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             bool isReference, HG_context options, 
             const attrOp& onoffOp,                            properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             bool isReference, HG_context options,
                                    const HG_compNamedMeasures& meas, properties* props=NULL);
  compHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             bool isReference, HG_context options, 
             const attrOp& onoffOp, const HG_compNamedMeasures& meas, properties* props=NULL);

  // Sets the properties of this object
  properties* setProperties(const HG_instance& inst, 
														bool isReference, 
														HG_context options, 
														const attrOp* onoffOp, 
														properties* props=NULL);
  
  void init(properties* props);
  
  ~compHierGraph();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  // Sets the HG_context of the given option
  virtual void setOptionCtxt(std::string name, attrValue val);
  
  // Sets the isReference flag to indicate whether this is a reference HG_instance of this compHierGraph or not
  void setIsReference(bool newVal);

  // Sets the HG_context of the given output HG_port. This variant ensures that the outputs of compHierGraphs can only
  // be set with HG_compContexts.
  void setOutCtxt(int idx, const HG_context& c) 
	{ std::cerr << "ERROR: compHierGraph::setOutCtxt() can only be called with a HG_compContext argument!"<<std::endl; assert(0); }

  virtual void setOutCtxt(int idx, const HG_compContext& c);
  
  // Sets the traceCtxt map, which contains the HG_context attributes to be used in this hierGraph's measurements 
  // by combining the HG_context provided by the classes that this object derives from with its own unique 
  // HG_context attributes.
  void setTraceCtxt();

  // -------------------------
  // ----- Configuration -----
  // -------------------------
  // We can configure the values that modifiable options take within each hierGraph
  // hierGraph name -> modifiable option name -> option value
  static std::map<std::string, std::map<std::string, attrValue> > modOptValue;
  
  public:
  class CompHierGraphConfiguration : public HierGraphConfiguration {
    public:
    CompHierGraphConfiguration(properties::iterator props) : HierGraphConfiguration(props.next()) {
      int numModOpts = props.getInt("numModOpts");
      for(int i=0; i<numModOpts; i++) {
        modOptValue[props.get(txt()<<"mo_HierGraph_"<<i)][props.get(txt()<<"mo_Key_"<<i)] = 
            attrValue(props.get(txt()<<"mo_Val_"<<i), attrValue::unknownT);
      }
    }
  };
  static common::Configuration* configure(properties::iterator props) { 
    // Create a HierGraphConfiguration object, using the invocation of the constructor hierarchy to
    // record the configuration details with the respective widgets from which hierGraphs inherit
    CompHierGraphConfiguration* c = new CompHierGraphConfiguration(props); 
    delete c;
    return NULL;
  }
  
  // Checks whether a modifiable option with the given name was specified for this hierGraph. 
  // This option may be set in a configuration file and then applications may use this 
  // function to query for its value and act accordingly
  virtual bool existsModOption(std::string name);
  
  // Returns the value of the given modifiable option of this hierGraph.
  virtual attrValue getModOption(std::string name);
  
  
}; // class compHierGraph

class springHierGraph;

// Represents a modular application, which may contain one or more hierGraphs. Only one modular application may be
// in-scope at any given point in time.
class springHierGraphApp : public compHierGraphApp
{
  friend class springHierGraph;
  long bufSize;
  char* data;
  pthread_t interfThread;
  
  public:
  springHierGraphApp(const std::string& appName,                                                        properties* props=NULL);
  springHierGraphApp(const std::string& appName, const attrOp& onoffOp,                                 properties* props=NULL);
  springHierGraphApp(const std::string& appName,                        const HG_compNamedMeasures& cMeas, properties* props=NULL);
  springHierGraphApp(const std::string& appName, const attrOp& onoffOp, const HG_compNamedMeasures& cMeas, properties* props=NULL);
  
  void init();
  
  ~springHierGraphApp();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  static void* Interference(void *arg);

  // Returns a pointer to the current HG_instance of hierGraphApp
  static springHierGraphApp* getInstance() { 
    springHierGraphApp* springActiveMA = dynamic_cast<springHierGraphApp*>(hierGraphApp::getInstance());
    assert(springActiveMA);
    return springActiveMA;
  }
}; // class springHierGraphApp

class springHierGraph: public compHierGraph {
/*  // The options passed into the constructor, extended with the configuration options from Spring.
  // extendedOptions is cleared at the end of the constructor sincee it is only used to communicate
  // the full set of options to the compHierGraph constructor.
  HG_context extendedOptions;*/

  public:
  
  // options: The HG_context that describes the configuration options of this hierGraph
  // meas: The measurements that should be performed during the execution of this springHierGraph
  // derivInfo: Information that describes the class that derives from this on. It includes a pointer to a properties
  //    object that can be used to create a tag for the derived object that includes info about its parents. Further,
  //    it includes fields that must be included within the HG_context of any trace observations made during the execution
  //    of this hierGraph.
  springHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
               const HG_context& options,
                                                               properties* props=NULL);
  springHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             const HG_context& options, 
             const attrOp& onoffOp,                            properties* props=NULL);
  springHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             const HG_context& options,
                                    const HG_compNamedMeasures& meas, properties* props=NULL);
  springHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             const HG_context& options, 
             const attrOp& onoffOp, const HG_compNamedMeasures& meas, properties* props=NULL);
  
  static bool isSpringReference();
  static HG_context extendOptions(const HG_context& options);

  ~springHierGraph();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();

  // Returns the HG_context attributes to be used in this hierGraph's measurements by combining the HG_context provided by the classes
  // that this object derives from with its own unique HG_context attributes.
  //static std::map<std::string, attrValue> getTraceCtxt(const std::vector<HG_port>& HG_inputs, bool isReference, const HG_context& options);
}; // class springHierGraph

class processedHierGraphTraceStream;

class processedHierGraph : public structure::hierGraph {
  friend class processedHierGraphTraceStream;
  
  protected:
  processedTrace::commands processorCommands;
  
  public:
  // processorCommands - list of executables to be run on the information of the hierGraph HG_instances to process/filter them
  processedHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             const processedTrace::commands& processorCommands,
                                                               properties* props=NULL);
  processedHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             const processedTrace::commands& processorCommands,
             const attrOp& onoffOp,                            properties* props=NULL);
  processedHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             const processedTrace::commands& processorCommands,
                                    const namedMeasures& meas, properties* props=NULL);
  processedHierGraph(const HG_instance& inst, const std::vector<HG_port>& HG_inputs, std::vector<HG_port>& externalOutputs, 
             const processedTrace::commands& processorCommands,
             const attrOp& onoffOp, const namedMeasures& meas, properties* props=NULL);

  // Sets the properties of this object
  /*derivInfo* setProperties(const HG_instance& inst, const processedTrace::commands& processorCommands, 
                           const attrOp* onoffOp, properties* props=NULL);*/
  
  void init(properties* props);
  
  ~processedHierGraph();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();

  // Returns the HG_context attributes to be used in this hierGraph's measurements by combining the HG_context provided by the classes
  // that this object derives from with its own unique HG_context attributes.
  //static std::map<std::string, attrValue> getTraceCtxt();
}; // class processedHierGraph

// Specialization of traceStreams for the case where they are hosted by a hierGraph node
class hierGraphTraceStream: public traceStream
{
  public:
  hierGraphTraceStream(int hierGraphID, hierGraph* m, vizT viz, mergeT merge, int traceID, properties* props=NULL);
  
  static properties* setProperties(int hierGraphID, hierGraph* m, vizT viz, mergeT merge, properties* props);

  ~hierGraphTraceStream();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
};

// Specialization of hierGraphTraceStream for the case where they are hosted by a compHierGraph node
class compHierGraphTraceStream: public hierGraphTraceStream
{
  public:
  compHierGraphTraceStream(int hierGraphID, compHierGraph* cm, vizT viz, mergeT merge, int traceID, properties* props=NULL);
  
  static properties* setProperties(int hierGraphID, compHierGraph* cm, vizT viz, mergeT merge, properties* props);

  ~compHierGraphTraceStream();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
};

// Specialization of hierGraphTraceStream for the case where they are hosted by a processedHierGraph node
class processedHierGraphTraceStream: public hierGraphTraceStream
{
  public:
  processedHierGraphTraceStream(int hierGraphID, processedHierGraph* pm, vizT viz, mergeT merge, int traceID, properties* props=NULL);
  
  static properties* setProperties(int hierGraphID, processedHierGraph* pm, vizT viz, mergeT merge, properties* props);

  ~processedHierGraphTraceStream();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
};











// ---------- hierGraph Merge Handler ---------------------------------------------------------------------------------

class HierGraphMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  HierGraphMergeHandlerInstantiator();
};

extern HierGraphMergeHandlerInstantiator HierGraphMergeHandlerInstance;

std::map<std::string, streamRecord*> HierGraphGetMergeStreamRecord(int streamID);

class HierGraphAppMerger : public BlockMerger {
public:
  HierGraphAppMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new HierGraphAppMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which HG_instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class HierGraphAppMerger

class HierGraphAppStructureMerger : public Merger {
public:
  HierGraphAppStructureMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new HierGraphAppStructureMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which HG_instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class HierGraphAppStructureMerger

class HierGraphMerger : public Merger {
public:
  HierGraphMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new HierGraphMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which HG_instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
  
}; // class HierGraphMerger

class HierGraphCtrlMerger : public Merger {
public:
  HierGraphCtrlMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new HierGraphCtrlMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which HG_instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
  
}; // class HierGraphCtrlMerger


class HierGraphEdgeMerger : public Merger {
public:
  HierGraphEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new HierGraphEdgeMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which HG_instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class HierGraphEdgeMerger

class HierGraphTraceStreamMerger : public TraceStreamMerger {
public:
  HierGraphTraceStreamMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
    
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new HierGraphTraceStreamMerger(tags, outStreamRecords, inStreamRecords, props); }
              
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);

  // Sets a list of strings that denotes a unique ID according to which HG_instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class HierGraphTraceStreamMerger

class CompHierGraphMerger : public HierGraphMerger {
  public:
  CompHierGraphMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new CompHierGraphMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which HG_instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class CompHierGraphMerger

class CompHierGraphTraceStreamMerger : public HierGraphTraceStreamMerger {
  public:
  CompHierGraphTraceStreamMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
    
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new CompHierGraphTraceStreamMerger(tags, outStreamRecords, inStreamRecords, props); }
              
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);

  // Sets a list of strings that denotes a unique ID according to which HG_instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class CompHierGraphTraceStreamMerger





class HierGraphStreamRecord: public streamRecord {
  friend class HierGraphAppMerger;
  friend class HierGraphMerger;
  friend class HierGraphEdgeMerger;
  friend class HierGraphTraceStreamMerger;
  friend class CompHierGraphTraceStreamMerger;
  
/*
  // We allow hierGraphs within different hierGraphApps to use independent ID schemes (i.e. the hierGraph IDs within
  // different apps may independently start from 0) because we anticipate that in the future this may make it easier
  // to match up HG_instances of the same hierGraph within the same hierGraphApp.
  // The stack of streamRecords that record for each currently active moduluarApp
  // (they are nested hierarchically), the mappings of nodes IDs from incoming to 
  // outgoing streams.
  std::list<streamRecord*> hgStack;
*/
  
  // The information that uniquely identifies a hierGraph
  /*class hierGraphInfo {
    public:
    int hierGraphID;
    std::string name;
    int numInputs;
    int numOutputs;
    
    hierGraphInfo(int hierGraphID, std::string name, int numInputs, int numOutputs) : hierGraphID(hierGraphID), name(name), numInputs(numInputs), numOutputs(numOutputs) {}
    
    bool operator==(const hierGraphInfo& that) const {
      return hierGraphID==that.hierGraphID;//name==that.name && numInputs==that.numInputs && numOutputs==that.numOutputs;
    }
    
    bool operator<(const hierGraphInfo& that) const {
      return hierGraphID < that.hierGraphID;/ *
      return (name< that.name) ||
             (name==that.name && numInputs< that.numInputs) ||
             (name==that.name && numInputs==that.numInputs && numOutputs<that.numOutputs);* /
    }
    
    std::string str() const {
      return txt()<<"name=\""<<name<<"\" numInputs="<<numInputs<<" numOutputs="<<numOutputs;
    }
  };*/
  // Stack of the hierGraph HG_instances that are currently in scope
  std::list<HG_instance> iStack;
  
  // Mapping from the unique information of all the observed hierGraphStreamRecords to their hierGraphIDs.
  // This is maintained on the outgoing stream to ensure that even if we fail to accurately align
  // two hierGraphTS tags that actually belong to the same hierGraph, we only keep the record for the first
  // and ignore the subsequent HG_instances of the tag.
  std::map<HG_group, int> observedHierGraphs;  // hierGraph HG_group -> hierGraphID
  std::map<HG_group, int> observedHierGraphsTS; // hierGraph HG_group -> traceID
  // The maximum ID ever assigned to a hierGraph HG_group within this stream
  int maxHierGraphID;
  
  // Returns a fresh hierGraphID
  int genHierGraphID() { return maxHierGraphID++; }

  // kyushick edit
  int recvHierGraphID() { return iStack.back().hierGraphID; }
  
  // Called when a hierGraph is entered/exited along the given stream to record the current hierGraph HG_group
  HG_group enterHierGraph(const HG_instance& inst);
  // Record that we've entered the given hierGraph HG_instance on all the given incoming streams
  static HG_group enterHierGraph(const HG_instance& inst, const std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);

  void exitHierGraph();
  // Record that we've exited the given hierGraph HG_instance on all the given incoming streams
  static void exitHierGraph(const std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  
  // Returns the HG_group denoted by the current stack of hierGraph HG_instances
  HG_group getGroup();
  // Returns the HG_group denoted by the current stack of hierGraph HG_instances in all the given incoming streams 
  // (must be identical on all of them).
  static HG_group getGroup(const std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  
  
  // Returns true if the given hierGraph HG_group has already been observed and false otherwise
  bool isHierGraphObserved(const HG_group& g) const { 
    assert((observedHierGraphs.find(g)   != observedHierGraphs.end()) ==
           (observedHierGraphsTS.find(g) != observedHierGraphsTS.end()));
    return observedHierGraphs.find(g) != observedHierGraphs.end();
  }
  
  // If the given hierGraph HG_group has been observed, returns its hierGraphID
  int getHierGraphID(const HG_group& g)
  { assert(observedHierGraphs.find(g) != observedHierGraphs.end()); return observedHierGraphs[g]; }
  
  // If the given hierGraph HG_group has been observed, returns its traceID
  int getHierGraphTraceID(const HG_group& g)
  { assert(observedHierGraphsTS.find(g) != observedHierGraphsTS.end()); return observedHierGraphsTS[g]; }
  
  // Records the ID of a given hierGraph HG_group
  void setHierGraphID(const HG_group& g, int hierGraphID, int traceID) { 
    assert(observedHierGraphs.find(g)   == observedHierGraphs.end()); 
    assert(observedHierGraphsTS.find(g) == observedHierGraphsTS.end()); 
    observedHierGraphs[g]   = hierGraphID;
    observedHierGraphsTS[g] = traceID;
  }
  
  public:
  HierGraphStreamRecord(int vID)              : streamRecord(vID, "hierGraph") { maxHierGraphID=0; }
  HierGraphStreamRecord(const variantID& vID) : streamRecord(vID, "hierGraph") { maxHierGraphID=0; }
  HierGraphStreamRecord(const HierGraphStreamRecord& that, int vSuffixID);
  
  // Called to record that we've entered/exited a hierGraph
  /*void enterHierGraphApp();
  static void enterHierGraphApp(std::map<std::string, streamRecord*>& outStreamRecords,
                     std::vector<std::map<std::string, streamRecord*> >& incomingStreamRecords);
  void exitHierGraphApp();
  static void exitHierGraphApp(std::map<std::string, streamRecord*>& outStreamRecords,
                    std::vector<std::map<std::string, streamRecord*> >& incomingStreamRecords);
  */
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  /* // Given a vector of streamRecord maps, collects the streamRecords associated with the currently active hierGraph (top of the gStack)
  // within each stream into nodeStreamRecords and returns the height of the gStacks on all the streams (must be the same number)
  static int collectNodeStreamRecords(std::vector<std::map<std::string, streamRecord*> >& streams,
                                      std::vector<std::map<std::string, streamRecord*> >& nodeStreamRecords);
  
  // Applies streamRecord::mergeIDs to the nodeIDs of the currently active hierGraph
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
}; // class HierGraphStreamRecord

} // namespace structure
} // namespace sight
