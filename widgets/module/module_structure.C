// Licence information included in file LICENCE
#define MODULE_STRUCTURE_C
#include "../../sight_structure.h"
#include "module_structure.h"
#include "../../sight_common.h"
#include "module_common.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <typeinfo>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

using namespace std;
using namespace sight::common;
  
namespace sight {
namespace structure {

/********************
 ***** instance *****
 ********************/
 
instance::instance(properties::iterator props) {
  name       = properties::get(props, "name");
  numInputs  = properties::getInt(props, "numInputs");
  numOutputs = properties::getInt(props, "numOutputs");
}

// Returns the properties map that describes this group object;
std::map<std::string, std::string> instance::getProperties() const {
  map<string, string> pMap;
  pMap["name"]       = name;
  pMap["numInputs"]  = txt()<<numInputs;
  pMap["numOutputs"] = txt()<<numOutputs;  
  return pMap;
}

// Returns a human-readable string that describes this context
std::string instance::str() const {
  return txt()<<"[instance "<<name<<", #inputs="<<numInputs<<", #outputs="<<numOutputs<<"]";
}

/*****************
 ***** group *****
 *****************/

// Creates a group given the current stack of modules and a new module instance
group::group(const std::list<module*>& mStack, const instance& inst) {
  for(std::list<module*>::const_iterator m=mStack.begin(); m!=mStack.end(); m++)
    stack.push_back((*m)->g.getInst());
  stack.push_back(inst);
}


void group::init(const std::list<module*>& mStack, const instance& inst) {
  stack.clear();
  for(std::list<module*>::const_iterator m=mStack.begin(); m!=mStack.end(); m++)
    stack.push_back((*m)->g.getInst());
  stack.push_back(inst);
}

// Returns the name of the most deeply nested instance within this group
std::string group::name() const {
  assert(stack.size()>0);
  return stack.back().name;
}

// Returns the number of inputs of the most deeply nested instance within this group
int group::numInputs() const {
  assert(stack.size()>0);
  return stack.back().numInputs;
}

// Returns the number of outnputs of the most deeply nested instance within this group
int group::numOutputs() const {
  assert(stack.size()>0);
  return stack.back().numOutputs;
}

// Returns the most deeply nested instance within this group
const instance& group::getInst() const {
  assert(stack.size()>0);
  return stack.back();
}

// Returns the depth of the callstack
int group::depth() const {
  return stack.size();
}

// Returns a human-readable string that describes this group
std::string group::str() const {
  ostringstream s;
  
  s << "[group: "<<endl;
  for(list<instance>::const_iterator i=stack.begin(); i!=stack.end(); i++)
    s << "    "<<i->str()<<endl;
  s << "]";
  
  return s.str();
}

/****************
 ***** port *****
 ****************/

// Returns a human-readable string that describes this context
std::string port::str() const {
  return txt() << "[port: g="<<g.str()<<", ctxt="<<ctxt->str() << " : "<<(type==sight::common::module::input?"In":"Out")<<" : "<<index<<"]";
}
  
/************************
 ***** instanceTree *****
 ************************/

void instanceTree::add(const group& g) {
  // The current instance tree. We'll keep walking t deeper into the depths of the tree until we find the leaf
  // for this group or find a missing node where this group should go
  instanceTree* t = this;
  for(std::list<instance>::const_iterator i=g.stack.begin(); i!=g.stack.end(); i++) {
    // If the current instance within this group's stack exists within the current level of the tree
    if(t->m.find(*i) != t->m.end()) {
      // Recurse deeper into the tree
      t = t->m[*i];
      assert(t);
    // Otherwise, we've identified the missing sub-node within the tree where we need to add g
    } else {
      // Iterate through the rest of g's stack, adding deeper sub-trees for each instance
      while(i!=g.stack.end()) {
        t->m[*i] = new instanceTree();
        t = t->m[*i];
        i++;
      }
      return;
    }
  }
}

// Iterate through all the groups encoded by this tree, where each group corresponds to a stack of instances
// from the tree's root to one of its leaves.
void instanceTree::iterate(instanceEnterFunc entry, instanceExitFunc exit) const {
  group g;
  iterate(entry, exit, g);
}

// Recursive version of iterate that takes as an argument the fragment of the current group that corresponds to
// the stack of instances between the tree's root and the current sub-tree
void instanceTree::iterate(instanceEnterFunc entry, instanceExitFunc exit, group& g) const {
  // Iterate over all the instances at this level of the tree
  for(std::map<instance, instanceTree*>::const_iterator i=m.begin(); i!=m.end(); i++) {
    // Enter this instance
    g.push(i->first);
    entry(g);

    // Iterate on the next deeper level of the tree
    i->second->iterate(entry, exit, g);

    // Exit this instance()
    exit(g);
    g.pop();
  }
}

// Empties out this instanceTree
void instanceTree::clear() {
  // Empty out each of this tree's sub-trees and then delete them
  for(std::map<instance, instanceTree*>::const_iterator i=m.begin(); i!=m.end(); i++) {
    i->second->clear();
    delete(i->second);
  }
  // Clear out this tree's sub-tree map
  m.clear();
}

// The depth of the recursion in instanceTree::str()
int instanceTree::instanceTreeStrDepth;

// The ostringstream into which the output of instanceTree::str() is accumulated
std::ostringstream instanceTree::oss;

// The entry and exit functions used in instanceTree::str()
void instanceTree::strEnterFunc(const group& g) {
  instanceTreeStrDepth++;
  // Print out the indentation that corresponds to the current depth of recursion
  for(int i=0; i<instanceTreeStrDepth; i++) oss << "    ";
  oss << g.getInst().str()<<endl;
}
void instanceTree::strExitFunc(const group& g)
{
  instanceTreeStrDepth--;
}

// Returns a human-readable string representation of this object
std::string instanceTree::str() const {
  // Reset the recursion depth and the ostringstream
  instanceTreeStrDepth=0;
  oss.str("");
  oss.clear();
  
  iterate(strEnterFunc, strExitFunc);
  
  return oss.str();
}

/**********************
 ***** modularApp *****
 **********************/

// Points to the currently active instance of modularApp. There can be only one.
modularApp* modularApp::activeMA = NULL;

// The maximum ID ever assigned to any modular application
int modularApp::maxModularAppID=0;

// The maximum ID ever assigned to any module group
int modularApp::maxModuleGroupID=0;

// Records all the known contexts, mapping each context to its unique ID
std::map<group, int> modularApp::group2ID;

// Maps each context to the number of times it was ever observed
std::map<group, int> modularApp::group2Count;

// The trace that records performance observations of different modules and contexts
std::map<group, traceStream*> modularApp::moduleTrace;
std::map<group, int>          modularApp::moduleTraceID;

// Tree that records the hierarchy of module instances that were observed during the execution of this
// modular application. Each path from the tree's root to a leaf is a stack of module instances that
// corresponds to some observed module group.
instanceTree modularApp::tree;

// Maps each module to the list of the names of its input and output context attributes. 
// This enables us to verify that all the modules are used consistently.
std::map<group, std::vector<std::list<std::string> > > modularApp::moduleInCtxtNames;
std::map<group, std::vector<std::list<std::string> > > modularApp::moduleOutCtxtNames;

// The properties object that describes each module group. This object is created by calling each group's
// setProperties() method and each call to this method for the same module group must return the same properties.
std::map<group, properties*> modularApp::moduleProps;

// Records all the edges ever observed, mapping them to the number of times each edge was observed
std::map<std::pair<port, port>, int> modularApp::edges;

// Stack of the module graphs that are currently in scope
std::list<module*> modularApp::mStack;

modularApp::modularApp(const std::string& appName,                                                   properties* props) :
    block(appName, setProperties(appName, NULL, props)), appName(appName), meas(meas)
{ init(); }
        
modularApp::modularApp(const std::string& appName, const attrOp& onoffOp,                            properties* props) :
    block(appName, setProperties(appName, &onoffOp, props)), appName(appName), meas(meas)
{ init(); }

modularApp::modularApp(const std::string& appName,                        const namedMeasures& meas, properties* props) :
    block(appName, setProperties(appName, NULL, props)), appName(appName), meas(meas)
{ init(); }

modularApp::modularApp(const std::string& appName, const attrOp& onoffOp, const namedMeasures& meas, properties* props) :
    block(appName, setProperties(appName, &onoffOp, props)), appName(appName), meas(meas)
{ init(); }

// Common initialization logic
void modularApp::init() {
  // Register this modularApp instance (there can be only one)
  if(activeMA != NULL) { cerr << "ERROR: multiple modularApps are active at the same time! Make sure to complete one modularApp before starting another."<<endl; }
  assert(activeMA == NULL);
  activeMA = this;
  
  appID = maxModularAppID;
  maxModularAppID++;
  
  // If this modularApp is not active, deallocate all the provided measurements
  if(!props->active) {
    for(namedMeasures::iterator m=meas.begin(); m!=meas.end(); m++)
      delete m->second;
  }
}

// Stack used while we're emitting the nesting hierarchy of module groups to keep each module group's 
// sightObj between the time the group is entered and exited
list<sightObj*> modularApp::moduleEmitStack;

// Emits the entry tag for a module group during the execution of ~modularApp()
void modularApp::enterModuleGroup(const group& g) {
  properties* props = new properties();
  
  /*map<string, string> pMap;
  pMap["moduleID"]   = txt()<<modularApp::group2ID[g];
  pMap["name"]       = g.name();
  pMap["numInputs"]  = txt()<<g.numInputs();
  pMap["numOutputs"] = txt()<<g.numOutputs();
  assert(modularApp::group2Count.find(g) != modularApp::group2Count.end());
  pMap["count"] = txt()<<modularApp::group2Count[g];
  props->add("module", pMap);*/
  
  // Add the count property to the map of "module".
  /*properties::iterator moduleIter = moduleProps[g]->find("module");
  properties::set(moduleIter, "count", txt()<<modularApp::group2Count[g]);*/
  moduleProps[g]->set("module", "count", txt()<<modularApp::group2Count[g]);
  moduleEmitStack.push_back(new sightObj(moduleProps[g]));
}

// Emits the exit tag for a module group during the execution of ~modularApp()
void modularApp::exitModuleGroup(const group& g) {
  assert(moduleEmitStack.size()>0);
  delete moduleEmitStack.back();
  moduleEmitStack.pop_back();
  
  // Erase this group from moduleProps to clearly keep track of the properties objects that are currently allocated
  // (the sightObj destructor will deallocate them)
  moduleProps.erase(g);
}

modularApp::~modularApp() {
  // Unregister this modularApp instance (there can be only one)
  assert(activeMA);
  activeMA = NULL;
  
  // All the modules that were entered inside this modularApp instance must have already been exited
  assert(mStack.size()==0);
  
  if(props->active) {
    /*cout << "group2ID="<<endl;
    for(std::map<context, int>::iterator c=group2ID.begin(); c!=group2ID.end(); c++)
      cout << "    "<<c->first.UID()<<" ==> "<<c->second<<endl;
    */
    // Delete the moduleTraces associated with each module group, forcing them to emit their respective end tags
    for(map<group, traceStream*>::iterator m=moduleTrace.begin(); m!=moduleTrace.end(); m++) {
      delete m->second;
    }
    
    // -------------------------------------------------------
    // Emit the tags of all modules and the edges between them
    // -------------------------------------------------------
    
    // Emit the hierarchy of module groups observed during this modularApp's execution
    tree.iterate(enterModuleGroup, exitModuleGroup);
    
    // Emit all the edges between module groups collected while this modularApp was live.
    // Note that this guarantees that edges are guaranteed to be placed after or inside the modules they connect.
    for(std::map<pair<port, port>, int>::iterator e=edges.begin(); e!=edges.end(); e++) {
      // If either group is NULL, don't generate an edge. Users can specify a NULL group if they don't want to bother
      // documenting where a given input came from
      if(e->first.first.g.isNULL() || e->first.second.g.isNULL()) continue;
      
      properties edgeP;
      map<string, string> pMap;
      pMap["fromCID"] = txt()<<group2ID[e->first.first.g];
      pMap["fromT"]   = txt()<<e->first.first.type;
      pMap["fromP"]   = txt()<<e->first.first.index;
      pMap["toCID"]   = txt()<<group2ID[e->first.second.g];
      pMap["toT"]     = txt()<<e->first.second.type;
      pMap["toP"]     = txt()<<e->first.second.index;
      
      // Number of time we've entere the edge's source module group
      pMap["fromCount"] = txt()<<group2Count[e->first.first.g];
      
      // Fraction of times that we've entered the edge's source module group and took this outgoing edge
      assert(group2Count.find(e->first.first.g) != group2Count.end());
      pMap["prob"]    = txt()<<(e->second / group2Count[e->first.first.g]);
      
      edgeP.add("moduleEdge", pMap);
      dbg.tag(edgeP);
    }
    
    // ------------------------
    // Clean up sata structures
    // ------------------------
    
    // Deallocate all the measurements provided to this modularApp since we won't need them any longer
    for(namedMeasures::iterator m=meas.begin(); m!=meas.end(); m++)
      delete m->second;
    
    // We do not deallocate all the module group properties because these are deallocated in the destructors
    // of the sightObjs created in modularApp::enterModuleGroup(). Further, moduleProps should be completely 
    // emptied by all our calls to modularApp::exitModuleGroup().
    assert(moduleProps.size()==0);
    /*for(std::map<group, properties*>::iterator p=moduleProps.begin(); p!=moduleProps.end(); p++)
      delete p->second;*/
    
    // Clear out all of the static datastructures of modularApp
    group2ID.clear();
    group2Count.clear();
    moduleTrace.clear();
    tree.clear();
    moduleInCtxtNames.clear();
    moduleOutCtxtNames.clear();
    edges.clear();
    meas.clear();
  } else {
    // If this modularApp instance is inactive, all the static datastructures must be empty
    assert(group2ID.size()==0);
    assert(group2Count.size()==0);
    assert(moduleTrace.size()==0);
    assert(moduleInCtxtNames.size()==0);
    assert(moduleOutCtxtNames.size()==0);
    assert(moduleProps.size()==0);
    assert(edges.size()==0);
    assert(meas.size()==0);
  }
}

// Sets the properties of this object
properties* modularApp::setProperties(const std::string& appName, const attrOp* onoffOp, properties* props) {
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    // If the current attribute query evaluates to true (we're emitting debug output) AND
    // either onoffOp is not provided or it evaluates to true
    if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    	props->active = true;
      
      if(props->emitTag) {
        /*map<string, string> pMap = c->first.getProperties();
        //newProps["callPath"] = cp2str(CPRuntime.doStackwalk());*/
        map<string, string> pMap;
        pMap["appName"] = appName;
        pMap["appID"]   = txt()<<maxModularAppID;
        props->add("modularApp", pMap);
      }
    } else
    	props->active = false;
  }
  
  return props;
}

// Returns the module ID of the given module group, generating a fresh one if one has not yet been assigned
int modularApp::genModuleID(const group& g) {
  // If this module group doesn't yet have an ID, set one
  if(group2ID.find(g) == group2ID.end()) {
    group2ID[g] = maxModuleGroupID;
    maxModuleGroupID++;
    group2Count[g] = 1;
    
    // Add this group to the instance tree
    tree.add(g);
  } else
    // Increment the number of times this group has been executed.
    group2Count[g]++;
  
  return group2ID[g];
}

// Returns whether the current instance of modularApp is active
bool modularApp::isInstanceActive() {
  //assert(activeMA);
  return activeMA!=NULL && activeMA->isActive();
}

// Assigns a unique ID to the given module group, as needed and returns this ID
int modularApp::addModuleGroup(const group& g) {
  
  return group2ID[g];
}

// Registers the names of the contexts of the given module's inputs or outputs and if this is not the first time this module is called, 
// verifies that these context names are consistent across different calls.
// g - the module group for which we're registering inputs/outputs
// inouts - the vector of input or output ports
// toT - identifies whether inouts is the vector of inputs or outputs
void modularApp::registerInOutContexts(const group& g, const std::vector<port>& inouts, sight::common::module::ioT io)
{
  // Exactly one modularAppInstance must be active
  assert(activeMA);
  assert(activeMA->isActive());
  
  // Refers to either moduleInCtxtNames or moduleOutCtxtNames, depending on the value of io;
  map<group, vector<list<string> > >& moduleCtxtNames = (io==sight::common::module::input? moduleInCtxtNames: moduleOutCtxtNames);
  
  // If moduleInCtxtNames/moduleOutCtxtNames has not yet been initialized, set it to contain the names of the context attributes in inputs
  if(moduleCtxtNames.find(g) == moduleCtxtNames.end()) {
    vector<list<string> > ctxtNames;
    for(int i=0; i<inouts.size(); i++) {
      ctxtNames.push_back(list<string>());
      for(std::map<std::string, attrValue>::const_iterator c=inouts[i].ctxt->configuration.begin(); c!=inouts[i].ctxt->configuration.end(); c++)
        ctxtNames[i].push_back(c->first);
    }
    moduleCtxtNames[g] = ctxtNames;
  // If moduleInCtxtNames has already been initialized, verify inputs against it
  } else {
    if(inouts.size() != moduleCtxtNames[g].size()) { cerr << "ERROR: Inconsistent "<<(io==sight::common::module::input?"inputs":"outputs")<<" for module "<<g.name()<<"! Prior instances has "<<moduleCtxtNames[g].size()<<" "<<(io==sight::common::module::input?"inputs":"outputs")<<" while this instance has "<<inouts.size()<<" "<<(io==sight::common::module::input?"inputs":"outputs")<<"!"<<endl; assert(0); }
    for(int i=0; i<inouts.size(); i++) {
      if(inouts[i].ctxt->configuration.size() != moduleCtxtNames[g][i].size()) { cerr << "ERROR: Inconsistent number of context attributes for "<<(io==sight::common::module::input?"input":"output")<<" "<<i<<" of module "<<g.name()<<"! Prior instances has "<<moduleCtxtNames[g][i].size()<<" "<<(io==sight::common::module::input?"inputs":"outputs")<<" while this instance has "<<inouts[i].ctxt->configuration.size()<<" "<<(io==sight::common::module::input?"inputs":"outputs")<<"!"<<endl; assert(0); }
      std::map<std::string, attrValue>::const_iterator c=inouts[i].ctxt->configuration.begin();
      list<std::string>::const_iterator m=moduleCtxtNames[g][i].begin();
      int idx=0;
      for(; c!=inouts[i].ctxt->configuration.end(); c++, m++, idx++) {
        if(c->first != *m) { cerr << "ERROR: Inconsistent names for context attribute "<<idx<<" of "<<(io==sight::common::module::input?"input":"output")<<" "<<i<<" of module "<<g.name()<<"! Prior instances has "<<*m<<" while this instance has "<<c->first<<"!"<<endl; assert(0); }
      }
    }
  }
}


// Add an edge between one module's output port and another module's input port
void modularApp::addEdge(port from, port to) {
  // Exactly one modularAppInstance must be active
  assert(activeMA);
  assert(activeMA->isActive());
  
  // Clear the contexts of from and to since edges is not sensitive to contexts
  from.clearContext();
  to.clearContext();
  
  pair<port, port> edge(from, to);
  if(edges.find(edge) == edges.end())
    edges[edge] = 1;
  else
    edges[edge]++;
  
  /*cout << "    module::addEdge()"<<endl;
  cout << "      edge="<<from.str()<<" => "<<to.str()<<endl;
  for(map<pair<port, port>, int>::iterator e=edges.begin(); e!=edges.end(); e++) {
    cout << "        "<<e->first.first.str()<<" => "<<e->first.second.str()<<" : "<<e->second<<endl;
  }*/
}

// Add an edge between one module's output port and another module's input port
void modularApp::addEdge(group fromG, sight::common::module::ioT fromT, int fromP, 
                         group toG,   sight::common::module::ioT toT,   int toP) {
  addEdge(port(fromG, context(), fromT, fromP), port(toG, context(), toT, toP));
}

// Returns the current module on the stack and NULL if the stack is empty
module* modularApp::getCurModule() {
  if(mStack.size()>0) return mStack.back();
  else                return NULL;
}

// Adds the given module object to the modules stack
void modularApp::enterModule(module* m, int moduleID, properties* props/*, generateTraceStream& tsGenerator*/) {
  // Exactly one modularAppInstance must be active
  assert(isInstanceActive());
  
  mStack.push_back(m);
  
  // If we have not yet recorded the properties of this module group, do so now
  if(moduleProps.find(m->g) == moduleProps.end())
    moduleProps[m->g] = props;
  // Otherwise, make sure that every module within the same group has the same properties
  else {
    /*cout << "props="<<props->str()<<endl;
    cout << "moduleProps["<<m->g.str()<<"]="<<moduleProps[m->g]->str()<<endl;*/
    assert(*(moduleProps[m->g]) == *props);
    // Delete props since it is no longer useful
    delete props;
  }
}

// Returns whether a traceStream has been registered for the given module group
bool modularApp::isTraceStreamRegistered(const group& g) {
  return moduleTrace.find(g) != moduleTrace.end();
}

// Registers the given traceStream for the given module group
void modularApp::registerTraceStream(const group& g, traceStream* ts) {
  // No other traceStream may be currently registered for this module group;
  assert(moduleTrace.find(g) == moduleTrace.end());
  moduleTrace[g] = ts;
}

// Registers the ID of the traceStream that will be used for the given module group
void modularApp::registerTraceStreamID(const group& g, int traceID) {
  // No other traceStream ID may be currently registered for this module group;
  assert(moduleTraceID.find(g) == moduleTraceID.end());
  moduleTraceID[g] = traceID;
}

// Returns the currently registered the ID of the traceStream that will be used for the given module group
int modularApp::getTraceStreamID(const group& g) {
  // A traceID must be registered with this module group
  if(moduleTraceID.find(g) == moduleTraceID.end()) { cerr << "ERROR: No traceStream registered for module group "<<g.str()<<"!"<<endl; }
  assert(moduleTraceID.find(g) != moduleTraceID.end());
  return moduleTraceID[g];
}

// Removes the given module object from the modules stack
void modularApp::exitModule(module* m) {
  // Pop this module from the mStack
  assert(mStack.size()>0);
  assert(mStack.back()==m);
  mStack.pop_back();
}

/******************
 ***** module *****
 ******************/

module::module(const instance& inst,                                                     derivInfo* deriv) : 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(),   NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(inputs(), deriv); }

module::module(const instance& inst, const port& in,                                     derivInfo* deriv): 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(in), NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(inputs(in), deriv); }

module::module(const instance& inst, const std::vector<port>& in,                        derivInfo* deriv) :
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, in,         NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(in, deriv); }

module::module(const instance& inst,                              const attrOp& onoffOp, derivInfo* deriv) : 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(),   &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(inputs(), deriv); }

module::module(const instance& inst, const port& in,              const attrOp& onoffOp, derivInfo* deriv): 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(in), &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(inputs(in), deriv); }

module::module(const instance& inst, const std::vector<port>& in, const attrOp& onoffOp, derivInfo* deriv) :
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, in,         &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(in, deriv); }


module::module(const instance& inst,                              std::vector<port>& externalOutputs,                        derivInfo* deriv) : 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(),   NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(inputs(), deriv); }

module::module(const instance& inst, const port& in,              std::vector<port>& externalOutputs,                        derivInfo* deriv): 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(in), NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(inputs(in), deriv); }

module::module(const instance& inst, const std::vector<port>& in, std::vector<port>& externalOutputs,                        derivInfo* deriv) :
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, in,         NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(in, deriv); }

module::module(const instance& inst,                              std::vector<port>& externalOutputs, const attrOp& onoffOp, derivInfo* deriv) : 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(),   &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(inputs(), deriv); }

module::module(const instance& inst, const port& in,              std::vector<port>& externalOutputs, const attrOp& onoffOp, derivInfo* deriv): 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(in), &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(inputs(in), deriv); }

module::module(const instance& inst, const std::vector<port>& in, std::vector<port>& externalOutputs, const attrOp& onoffOp, derivInfo* deriv) :
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, in,         &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(in, deriv); }



module::module(const instance& inst,                                                     const namedMeasures& meas, derivInfo* deriv) : 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(),   NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(inputs(), deriv); }

module::module(const instance& inst, const port& in,                                     const namedMeasures& meas, derivInfo* deriv): 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(in), NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(inputs(in), deriv); }

module::module(const instance& inst, const std::vector<port>& in,                        const namedMeasures& meas, derivInfo* deriv) :
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, in,         NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(in, deriv); }

module::module(const instance& inst,                              const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv) : 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(),   &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(inputs(), deriv); }

module::module(const instance& inst, const port& in,              const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv): 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(in), &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(inputs(in), deriv); }

module::module(const instance& inst, const std::vector<port>& in, const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv) :
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, in,         &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(in, deriv); }


module::module(const instance& inst,                              std::vector<port>& externalOutputs,                        const namedMeasures& meas, derivInfo* deriv) : 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(),   NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(inputs(), deriv); }

module::module(const instance& inst, const port& in,              std::vector<port>& externalOutputs,                        const namedMeasures& meas, derivInfo* deriv): 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(in), NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(inputs(in), deriv); }

module::module(const instance& inst, const std::vector<port>& in, std::vector<port>& externalOutputs,                        const namedMeasures& meas, derivInfo* deriv) :
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, in,         NULL, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(in, deriv); }

module::module(const instance& inst,                              std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv) : 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(),   &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(inputs(), deriv); }

module::module(const instance& inst, const port& in,              std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv): 
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, inputs(in), &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(inputs(in), deriv); }

module::module(const instance& inst, const std::vector<port>& in, std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv) :
  sightObj(setProperties(inst, NULL, this)), /*sightObj(setProperties(inst, in,         &onoffOp, derivInfo),*/ g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(in, deriv); }

properties* module::setProperties(const instance& inst, properties* props, module* me) {
  group g(modularApp::mStack, inst);
  bool isDerived = (props!=NULL); // This is an instance of an object that derives from module if its constructor sets props to non-NULL
  if(props==NULL) {
    props = new properties();
  }

  map<string, string> pMap;
  //pMap["moduleID"] = txt()<<modularApp::genModuleID(g);
  pMap["name"]       = g.name();
  pMap["numInputs"]  = txt()<<g.numInputs();
  pMap["numOutputs"] = txt()<<g.numOutputs();
  
  // If this is an instance of module rather than a class that derives from module
  if(modularApp::isInstanceActive() && props->active && !isDerived) {
    props->active = true;
    
    // If no traceStream has been registered for this module group
    if(!modularApp::isTraceStreamRegistered(g)) {
      // We'll create a new traceStream in the destructor but first, generate and record the ID of that 
      // traceStream so that we can include it in the tag and record it in the module object for use in its destructor
      int traceID = traceStream::genTraceID();
      modularApp::registerTraceStreamID(g, traceID);
      pMap["traceID"] = txt()<<traceID;
    } else {
      // Reuse the previously registered traceID
      pMap["traceID"] = txt()<<modularApp::getTraceStreamID(g);
    }
  } else
    props->active = false;
  
  props->add("moduleMarker", pMap);

  return props;
}

void module::init(const std::vector<port>& in, derivInfo* deriv) {
  //generateModuleTraceStream gmts(g);
  isDerived = deriv!=NULL; // This is an instance of an object that derives from module if its constructor sets deriv to non-NULL
  if(deriv==NULL) {
    deriv = new derivInfo(/*gmts*/);
  }
  
  ins = in;
  
  if(modularApp::isInstanceActive() && deriv->props->active) {
    moduleID = modularApp::genModuleID(g);
    
    // Add the properties of this module to props
    map<string, string> pMap;
    pMap["moduleID"]   = txt()<<moduleID;
    pMap["name"]       = g.name();
    pMap["numInputs"]  = txt()<<g.numInputs();
    pMap["numOutputs"] = txt()<<g.numOutputs();
    deriv->props->add("module", pMap);
    
    // Add this module instance to the current stack of modules
    modularApp::enterModule(this, moduleID, deriv->props/*, deriv->tsGenerator*/);
    
    // Set the context attributes to be used in this module's measurements by combining the context provided by any
    // class that may derive from this one as well as the contexts of its inputs
    
    // Start with the derived context
    traceCtxt = deriv->ctxt;
    
    // Add to it the context of this module based on the contexts of its inputs
    for(int i=0; i<in.size(); i++) {
      for(std::map<std::string, attrValue>::const_iterator c=in[i].ctxt->configuration.begin(); c!=in[i].ctxt->configuration.end(); c++)
        traceCtxt[encodeCtxtName("module", txt()<<"In"<<i, c->first)] = c->second;
    }
    
    // Make sure that the input contexts have the same names across all the invocations of this module group
    modularApp::registerInOutContexts(g, in, sight::common::module::input);
    
    // Add edges between the modules from which this module's inputs came and this module
    for(int i=0; i<in.size(); i++)
    	modularApp::addEdge(in[i], port(g, context(), input, i));
    
    // Initialize the output ports of this module
    if(externalOutputs) externalOutputs->clear(); 
    for(int i=0; i<g.numOutputs(); i++) {
      outs.push_back(port(g, context(), output, i));
      // If the user provided an output vector, initialize it as well
      if(externalOutputs) { 
        externalOutputs->push_back(port(g, context(), output, i));
      }
    }
    
    // Add the default measurements recored in modularApp to meas
    for(namedMeasures::iterator m=modularApp::activeMA->meas.begin(); m!=modularApp::activeMA->meas.end(); m++) {
      // If the name of the current measurement in modularApp isn't already specified for the given module group, add it
      if(meas.find(m->first) == meas.end())
        meas[m->first] = m->second->copy();
    }
    
    // Begin measuring this module instance
    for(namedMeasures::iterator m=meas.begin(); m!=meas.end(); m++) {
      m->second->start(); 
    }
  }
}

// Sets the properties of this object
/*properties* module::setProperties(const instance& inst, const std::vector<port>& inputs, const attrOp* onoffOp, properties* props)  {
  if(props==NULL) props = new properties();
  
  if(inst.numInputs!=inputs.size()) cerr << "ERROR: module group \""<<inst.name<<"\" has "<<inst.numInputs<<" inputs but "<<inputs.size()<<" inputs were provided!"<<endl; 
  assert(inst.numInputs==inputs.size());
  
  // This module instance is active only if the current modularApp instance is active
  props->active = modularApp::isInstanceActive() && props->active;
  
  // If this module instance is active according to the classes that derive from it, AND
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // Either onoffOp is not provided or it evaluates to true
  if(props->active && attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    // Register this module and get its unique ID
    group g(modularApp::mStack, inst);
    int moduleID = modularApp::addModuleGroup(g);
    
    // If we're supposed to actually emit a tag for this module instance
    if(props->emitTag) {
      /*map<string, string> pMap = c->first.getProperties();
      //newProps["callPath"] = cp2str(CPRuntime.doStackwalk());* /
      map<string, string> pMap;
      pMap["moduleID"]   = txt()<<moduleID;
      pMap["name"]       = inst.name;
      pMap["numInputs"]  = txt()<<inst.numInputs;
      pMap["numOutputs"] = txt()<<inst.numOutputs;
      assert(modularApp::group2Count.find(g) != modularApp::group2Count.end());
      pMap["count"] = txt()<<modularApp::group2Count[g];
      props->add("module", pMap);
    }
  } else
    props->active = false;
  
  return props;
}*/

module::~module()
{ 
  //cout << "~module() props->active="<<props->active<<endl;

  if(ins.size() != g.numInputs()) { cerr << "ERROR: module \""<<g.name()<<"\" specifies "<<g.numInputs()<<" inputs but "<<ins.size()<<" inputs are actually provided!"<<endl; }
  
  // Complete the measurement of application's behavior during the module's lifetime
  if(props->active) {
    // If this is an instance of module rather than a class that derives from module
    if(!isDerived) {
      // Register a traceStream for this compModule's module group, if one has not already been registered
      if(!modularApp::isTraceStreamRegistered(g)) {
        modularApp::registerTraceStream(g, new moduleTraceStream(moduleID, this, trace::lines, trace::disjMerge, modularApp::getTraceStreamID(g)));
      }
    }
    
    // Complete measuring this instance and collect the observations into props
    list<pair<string, attrValue> > obs;
    int measIdx=0;
    for(namedMeasures::iterator m=meas.begin(); m!=meas.end(); m++, measIdx++) {
      // Complete the m's measurement
      list<pair<string, attrValue> > curObs = m->second->endGet();
      // Push each measurement onto the observation
      for(list<pair<string, attrValue> >::iterator o=curObs.begin(); o!=curObs.end(); o++) {
        obs.push_back(make_pair(encodeCtxtName("measure", m->first, o->first), o->second));
      }
      delete m->second;
    }
    
    // Add to the trace observation the properties of all of the module's outputs
    for(int i=0; i<outs.size(); i++) {
      for(map<std::string, attrValue>::iterator c=outs[i].ctxt->configuration.begin();
          c!=outs[i].ctxt->configuration.end(); c++) {
        obs.push_back(make_pair(encodeCtxtName("output", txt()<<i, c->first), c->second));
      }
    }
    
    // Record the observation into this module group's trace
    modularApp::moduleTrace[g]->traceFullObservation(traceCtxt, obs, anchor::noAnchor);

    // Make sure that the output contexts have the same names across all the invocations of this module group
    modularApp::registerInOutContexts(g, outs, sight::common::module::output);
  
    modularApp::exitModule(this);
  }
}

/*traceStream* module::generateModuleTraceStream::operator()(int moduleID) {
  return new moduleTraceStream(moduleID, g->name(), g->numInputs(), g->numOutputs(), trace::lines, trace::disjMerge);
}*/

// Sets the context of the given output port
void module::setInCtxt(int idx, const context& c) {
  if(!props->active) return;
  if(idx>=g.numInputs()) { cerr << "ERROR: cannot set context of input "<<idx<<" of module group "<<g.str()<<"! This module was declared to have "<<g.numInputs()<<" inputs."<<endl; }
  assert(idx<g.numInputs());
  ins[idx].setCtxt(c);
}

// Adds the given key/attrValue pair to the context of the given output port
void module::addInCtxt(int idx, const std::string& key, const attrValue& val) {
  if(!props->active) return;
  if(idx>=g.numInputs()) { cerr << "ERROR: cannot add context to input "<<idx<<" of module group "<<g.str()<<"! This module was declared to have "<<g.numInputs()<<" inputs."<<endl; }
  assert(idx<g.numInputs());
  ins[idx].addCtxt(key, val);
}

// Adds the given port to this module's inputs
void module::addInCtxt(const port& p) {
  ins.push_back(p);
}

// Sets the context of the given output port
void module::setOutCtxt(int idx, const context& c) { 
  if(!props->active) return;
  if(idx>=g.numOutputs()) { cerr << "ERROR: cannot set context of output "<<idx<<" of module group "<<g.str()<<"! This module was declared to have "<<g.numOutputs()<<" outputs."<<endl; }
  assert(idx<g.numOutputs());
  outs[idx].setCtxt(c);
  // If the user provided an output vector, update it as well
  if(externalOutputs)
    (*externalOutputs)[idx].setCtxt(c);
}

// Returns a list of the module's input ports
/*std::vector<port> module::inputPorts() const {
	std::vector<port> ports;
  for(int i=0; i<numInputs(); i++)
  	ports.push_back(port(c, input, i));
	return ports;
}*/

// Returns a list of the module's output ports
std::vector<port> module::outPorts() const {
	return outs;
}

port module::outPort(int idx) const {
  if(!props->active) return port();
  if(idx>=g.numOutputs()) { cerr << "ERROR: cannot get port "<<idx<<" of module group "<<g.str()<<"! This module was declared to have "<<g.numOutputs()<<" outputs."<<endl; }
  assert(idx < g.numOutputs());
  return outs[idx];  
}

/***********************
 ***** compContext *****
 ***********************/

// Loads this context from the given properties map. The names of all the fields are assumed to be prefixed
// with the given string.
compContext::compContext(properties::iterator props, std::string prefix) : context(props, prefix) {
  // Read out just the fields related to the comparator description
  int numCfgKeys = properties::getInt(props, txt()<<prefix<<"numCfgKeys");
  for(int i=0; i<numCfgKeys; i++) {
    comparators[properties::get(props, txt()<<prefix<<"key_"<<i)] = 
            make_pair(properties::get(props, txt()<<prefix<<"compName_"<<i),
                      properties::get(props, txt()<<prefix<<"compDesc_"<<i));
  }
}

// Returns the properties map that describes this context object.
// The names of all the fields in the map are prefixed with the given string.
map<string, string> compContext::getProperties(std::string prefix) const {
  // Initialize pMap with the properties of all the keys and values
  map<string, string> pMap = context::getProperties(prefix);
  // Add to it the properties of each key's comparator
  int i=0;
  for(map<string, pair<string, string> >::const_iterator comp=comparators.begin(); comp!=comparators.end(); comp++, i++) {
    pMap[txt()<<prefix<<"compName_"<<i]  = comp->second.first;
    pMap[txt()<<prefix<<"compDesc_"<<i]  = comp->second.second;
  }
  return pMap;
}

// These comparator routines must be implemented by all classes that inherit from context to make sure that
// their additional details are reflected in the results of the comparison. Implementors may assume that 
// the type of that is their special derivation of context rather than a generic context and should dynamically
// cast from const context& to their sub-type.
bool compContext::operator==(const context& that_arg) const {
  try {
    const compContext& that = dynamic_cast<const compContext&>(that_arg);
    return (*(const context*)this)==that_arg && comparators==that.comparators;
  } catch(std::bad_cast exp) {
    cerr << "ERROR: comparing compContext and a different sub-type of context using == operator!"<<endl;
    assert(0);
  }
}

bool compContext::operator<(const context& that_arg) const { 
  try {
    const compContext& that = dynamic_cast<const compContext&>(that_arg);
    return ((*(const context*)this)< that_arg) ||
           ((*(const context*)this)==that_arg && comparators<that.comparators);
  } catch(std::bad_cast exp) {
    cerr << "ERROR: comparing compContext and a different sub-type of context using < operator!"<<endl;
    assert(0);
  }
}

// Returns a human-readable string that describes this context
std::string compContext::str() const {
  ostringstream s;
  s << "[compContext: "<<endl;
  for(map<string, pair<string, string> >::const_iterator comp=comparators.begin(); comp!=comparators.end(); comp++) {
    if(comp!=comparators.begin()) s << " ";
    s << "("<<comp->second.first<<": "<<comp->second.second<<")";
  }
  s << "]";
  return s.str();
}

/**************************
 ***** compModularApp *****
 **************************/

compModularApp::compModularApp(const std::string& appName,                                                       properties* props) : 
  modularApp(appName,                                   props)
{}

compModularApp::compModularApp(const std::string& appName, const attrOp& onoffOp,                                properties* props) : 
  modularApp(appName, onoffOp,                          props)
{}

compModularApp::compModularApp(const std::string& appName,                        const compNamedMeasures& cMeas, properties* props) :
  modularApp(appName,          cMeas.getNamedMeasures(), props), measComp(cMeas.getComparators())
{}

compModularApp::compModularApp(const std::string& appName, const attrOp& onoffOp, const compNamedMeasures& cMeas, properties* props) :
  modularApp(appName, onoffOp, cMeas.getNamedMeasures(), props), measComp(cMeas.getComparators())
{}


/**********************
 ***** compModule *****
 **********************/

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, context options,
                                                                         derivInfo* deriv) :
          module(inst, inputs, externalOutputs, setProperties(inst, isReference, options, NULL, deriv)),           isReference(isReference), options(options)
{ init(deriv); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, context options, 
                       const attrOp& onoffOp,                            derivInfo* deriv) :
          module(inst, inputs, externalOutputs, setProperties(inst, isReference, options, &onoffOp, deriv)),       isReference(isReference), options(options)
{ init(deriv); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, context options, 
                                              const compNamedMeasures& cMeas, derivInfo* deriv) :
          module(inst, inputs, externalOutputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, options, NULL, deriv)),     isReference(isReference), options(options), measComp(cMeas.getComparators())
{ init(deriv); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, context options, 
                       const attrOp& onoffOp, const compNamedMeasures& cMeas, derivInfo* deriv) : 
          module(inst, inputs, externalOutputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, options, &onoffOp, deriv)), isReference(isReference), options(options), measComp(cMeas.getComparators())
{ init(deriv); }

// Sets the properties of this object
module::derivInfo* compModule::setProperties(const instance& inst, bool isReference, context options, const attrOp* onoffOp, derivInfo* deriv) {
  // Set gcmts to refer to the properties of this instance of compModule
  //gcmts.init(inst, isReference, options);
  if(deriv==NULL) {
    deriv = new derivInfo(/*gcmts*/);
  }
  
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    /* // Initialize pMap to contain the properties of options
    map<string, string> pMap;// = options.getProperties("op");
    //pMap["isReference"]   = txt()<<isReference;

    deriv->props->add("compModule", pMap);*/
    
    // Add isReference and the options to the trace context attributes in derivInfo
    deriv->ctxt["compModule:isReference"] = attrValue((long)isReference);
    for(map<std::string, attrValue>::const_iterator o=options.getCfg().begin(); o!=options.getCfg().end(); o++)
      deriv->ctxt[encodeCtxtName("compModule", "options", o->first)] = o->second;
  }
  else
    deriv->props->active = false;
  
  return deriv;
}

void compModule::init(derivInfo* deriv) {
  isDerived = deriv!=NULL; // This is an instance of an object that derives from module if its constructor sets deriv to non-NULL
}

compModule::~compModule() {
  // If this is an instance of compModule rather than a class that derives from compModule
  if(props->active && !isDerived) {
    // Register a traceStream for this compModule's module group, if one has not already been registered
    if(!modularApp::isTraceStreamRegistered(g))
      modularApp::registerTraceStream(g, new compModuleTraceStream(moduleID, this, trace::lines, trace::disjMerge, modularApp::getTraceStreamID(g)));
  }
}

// Sets the context of the given output port. This variant ensures that the outputs of compModules can only
// be set with compContexts.
void compModule::setOutCtxt(int idx, const compContext& c) {
  if(!props->active) return;
  module::setOutCtxt(idx, (const context&)c);
}

/*****************************
 ***** springModularApp  *****
 *****************************/

springModularApp::springModularApp(const std::string& appName,                                                        properties* props) :
    compModularApp(appName, props)
{ init(); }

springModularApp::springModularApp(const std::string& appName, const attrOp& onoffOp,                                 properties* props)  :
    compModularApp(appName, props)
{ init(); }

springModularApp::springModularApp(const std::string& appName,                        const compNamedMeasures& cMeas, properties* props) :
    compModularApp(appName, props)
{ init(); }

springModularApp::springModularApp(const std::string& appName, const attrOp& onoffOp, const compNamedMeasures& cMeas, properties* props)  :
    compModularApp(appName, props)
{ init(); }

#include <sched.h>

void *springModularApp::Interference(void *arg) {
  int oldstate; pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
  int oldtype; pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
  
  //cout << "Interference CPU: "<<sched_getcpu()<<endl;
  
  springModularApp* spring = static_cast<springModularApp*>(arg);
  assert(spring);
  char* data = spring->data;
  long long numLL = spring->bufSize / sizeof(long long);
  
  // Cache capacity interference
  struct random_data* rand_states = (struct random_data*)calloc(1, sizeof(struct random_data));
  char* rand_statebufs = (char*)calloc(1, 8);
  initstate_r(random(), rand_statebufs, 8, rand_states);
  int32_t r1;
  while(1) {
    random_r(rand_states, &r1);
    ((long long*)data)[r1%numLL]=((long long*)data)[r1%numLL]+1;
  }
  
  /*//cout << "numLL="<<numLL<<endl;
  long long i=0;
  int j=0; 
  double res=2;
  while(1) {
    ((long long*)data)[i] = (((long long*)data)[i]+1) % numLL;
    i = ((long long*)data)[i];
    //res = sin(exp(res));
    j++;
    //if(j%1000000==0) { cout << "."; cout.flush(); }
  }
  //return NULL;
  
  return &res;*/
  /*double res=2;
  int j=0;
  while(1) {
    res = sin(exp(res));
    j++;
    if(j%1000000==0) { cout << "."; cout.flush(); }
  }
  return &res;*/
}

long long randomLL() {
  long long r = 0;
  for (int i = 0; i < sizeof(long long)/sizeof(int); i++)
  {
      r = r << (sizeof(int) * 8);
      r |= rand();
  }
  return r;
}

void springModularApp::init() {
  if(!props->active) return;
  
  bufSize=0;
  if(getenv("SPRING_BUF_SIZE"))
    bufSize = strtol(getenv("SPRING_BUF_SIZE"), NULL, 10);
  
  //cout << "SPRING_BUF_SIZE="<<(getenv("SPRING_BUF_SIZE")? getenv("SPRING_BUF_SIZE"): "NULL")<<", bufSize="<<bufSize<<endl;
  //cout << "App CPU: "<<sched_getcpu()<<endl;
  
  if(bufSize>sizeof(long long)) {
    data = new char[bufSize];
    long long numLL = bufSize / sizeof(long long);
    // Initialize buf with random indexes withiin buf
    for(int i=0; i<numLL; i++) {
      ((long long*)data)[i] = randomLL() % numLL;
    }
    
    int ret = pthread_create(&interfThread, NULL, Interference, (void *) this);
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    //for (j = 0; j < 8; j++) CPU_SET(j, &cpuset);
    int appCPU = sched_getcpu();
    //int domainSize = 6;
    //CPU_SET((appCPU/domainSize)*domainSize + (appCPU+1)%domainSize, 
    //        &cpuset);
    CPU_SET(appCPU, &cpuset);
    pthread_setaffinity_np(interfThread, sizeof(cpu_set_t), &cpuset);
    
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    
    assert(ret==0);
  } else
    data = NULL;
}

springModularApp::~springModularApp() {
  if(props->active && bufSize>sizeof(long long)) {
    int ret = pthread_cancel(interfThread);
    if(ret!=0) { cerr << "ERROR: return code from pthread_cancel() is "<<ret<<", thread %d\n"; assert(0); }
    
    int status;
    ret = pthread_join(interfThread, (void **)&status); 
    if(ret!=0) { cerr << "ERROR: return code from pthread_join() is "<<ret<<", thread %d\n"; assert(0); }
   
    delete data;
  }
}

/************************
 ***** springModule *****
 ************************/

springModule::springModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
                       const context& options,
                                                                         derivInfo* deriv) :
          compModule(inst, inputs, externalOutputs, isSpringReference(), extendOptions(options), compNamedMeasures(), deriv)
{ 
/*springModularApp* app = springModularApp::getInstance();
if(app->bufSize>0) { cout << "bufSize="<<app->bufSize<<" sleep("<<(log(app->bufSize)/log(10))<<")"<<endl; sleep(log(app->bufSize)/log(10)); }*/
}

springModule::springModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
                       const context& options,
                       const attrOp& onoffOp,                            derivInfo* deriv) :
          compModule(inst, inputs, externalOutputs, isSpringReference(), extendOptions(options), onoffOp, compNamedMeasures(), deriv)
{ 
/*springModularApp* app = springModularApp::getInstance();
if(app->bufSize>0) { cout << "bufSize="<<app->bufSize<<" sleep("<<(log(app->bufSize)/log(10))<<")"<<endl; sleep(log(app->bufSize)/log(10)); }*/
}

springModule::springModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
                       const context& options,
                                              const compNamedMeasures& cMeas, derivInfo* deriv) :
          compModule(inst, inputs, externalOutputs, isSpringReference(), extendOptions(options), cMeas, deriv)
{ 
/*springModularApp* app = springModularApp::getInstance();
if(app->bufSize>0) { cout << "bufSize="<<app->bufSize<<" sleep("<<(log(app->bufSize)/log(10))<<")"<<endl; sleep(log(app->bufSize)/log(10)); }*/
}

springModule::springModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
                       const context& options,
                       const attrOp& onoffOp, const compNamedMeasures& cMeas, derivInfo* deriv) :
          compModule(inst, inputs, externalOutputs, isSpringReference(), extendOptions(options), cMeas, deriv)
{ 
/*springModularApp* app = springModularApp::getInstance();
if(app->bufSize>0) { cout << "bufSize="<<app->bufSize<<" sleep("<<(log(app->bufSize)/log(10))<<")"<<endl; sleep(log(app->bufSize)/log(10)); }*/
}

bool springModule::isSpringReference() {
  springModularApp* app = springModularApp::getInstance();
  if(app) return app->bufSize==0;
  else    return false;
}

context springModule::extendOptions(const context& options) {
  context extendedO = options;
  springModularApp* app = springModularApp::getInstance();
  if(app) {
    extendedO.configuration["Spring:bufSize"] = attrValue(app->bufSize);
    return extendedO;
  } else
    return options;
}

/***************************
 ***** processedModule *****
 ***************************/

processedModule::processedModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       const processedTrace::commands& processorCommands,
                                                                         derivInfo* deriv) :
          module(inst, inputs, externalOutputs, setProperties(inst, processorCommands, NULL, deriv)),           processorCommands(processorCommands)
{ init(deriv); }

processedModule::processedModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       const processedTrace::commands& processorCommands, 
                       const attrOp& onoffOp,                            derivInfo* deriv) :
          module(inst, inputs, externalOutputs, setProperties(inst, processorCommands, &onoffOp, deriv)),       processorCommands(processorCommands)
{ init(deriv); }

processedModule::processedModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       const processedTrace::commands& processorCommands, 
                                              const namedMeasures& meas, derivInfo* deriv) :
          module(inst, inputs, externalOutputs, meas, setProperties(inst, processorCommands, NULL, deriv)),     processorCommands(processorCommands)
{ init(deriv); }

processedModule::processedModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       const processedTrace::commands& processorCommands, 
                       const attrOp& onoffOp, const namedMeasures& meas, derivInfo* deriv) : 
          module(inst, inputs, externalOutputs, meas, setProperties(inst, processorCommands, &onoffOp, deriv)), processorCommands(processorCommands)
{ init(deriv); }

// Sets the properties of this object
module::derivInfo* processedModule::setProperties(const instance& inst, const processedTrace::commands& processorCommands, const attrOp* onoffOp, derivInfo* deriv) {
/*  if(deriv==NULL) {
    deriv = new derivInfo();
  }
  
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    deriv->props->active = true;
    
    map<string, string> pMap;
    pMap["numCmds"] = txt()<<processorCommands.size();
    int i=0;
    for(list<string>::const_iterator c=processorCommands.begin(); c!=processorCommands.end(); c++)
      pMap[txt()<<"cmd"<<i] = *c;
    
    deriv->props->add("processedModule", pMap);
  }
  else
    deriv->props->active = false;
  */
  return deriv;
}

void processedModule::init(derivInfo* deriv) {
  isDerived = deriv!=NULL; // This is an instance of an object that derives from module if its constructor sets deriv to non-NULL
}

processedModule::~processedModule() {
  // If this is an instance of processedModule rather than a class that derives from processedModule
  if(props->active) {
    // Register a traceStream for this processedModule's module group, if one has not already been registered
    if(!modularApp::isTraceStreamRegistered(g))
      modularApp::registerTraceStream(g, new processedModuleTraceStream(moduleID, this, trace::lines, trace::disjMerge, modularApp::getTraceStreamID(g)));
  }
}

/*****************************
 ***** moduleTraceStream *****
 *****************************/

moduleTraceStream::moduleTraceStream(int moduleID, module* m, vizT viz, mergeT merge, int traceID, properties* props) : 
  traceStream(viz, merge, traceID, setProperties(moduleID, m, viz, merge, props))
{ }

properties* moduleTraceStream::setProperties(int moduleID, module* m, vizT viz, mergeT merge, properties* props) {
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    map<string, string> pMap;
    pMap["moduleID"]   = txt()<<moduleID;
    pMap["name"]       = m->name();
    pMap["numInputs"]  = txt()<<m->numInputs();
    pMap["numOutputs"] = txt()<<m->numOutputs();
    props->add("moduleTS", pMap);
  }
  
  return props;
}

/*********************************
 ***** compModuleTraceStream *****
 *********************************/

compModuleTraceStream::compModuleTraceStream(int moduleID, 
                                             compModule* cm,
                                             vizT viz, mergeT merge, 
                                             int traceID, properties* props) : 
  moduleTraceStream(moduleID, (module*)cm, viz, merge, 
                    traceID, setProperties(moduleID, cm, viz, merge, props))
{ }

properties* compModuleTraceStream::setProperties(int moduleID, 
                                                 compModule* cm, vizT viz, mergeT merge, properties* props) {
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    map<string, string> pMap;// = cm->options.getProperties("op");
    
    // Record the number of outputs of this compMduole. This repeats the same attribute in moduleTS but it is easier
    // let both moduleTS and compModuleTS to have their own than to allow functions of compModuleTS access to properties of moduleTS.
    pMap["numOutputs"] = txt()<<cm->numOutputs(); 
    
    // Add the comparators to be used for each output attribute
    int outIdx=0;
    for(std::vector<port>::iterator o=cm->outs.begin(); o!=cm->outs.end(); o++, outIdx++) {
      if(o->ctxt==NULL) { cerr << "ERROR in module "<<moduleID<<"! Context of output "<<outIdx<<" not provided!"<<endl; assert(0); }
      compContext* ctxt = dynamic_cast<compContext*>(o->ctxt);
      assert(ctxt);
      pMap[txt()<<"out"<<outIdx<<":numAttrs"] = txt()<<ctxt->comparators.size();
      int compIdx=0;
      for(map<string, pair<string, string> >::iterator comp=ctxt->comparators.begin(); comp!=ctxt->comparators.end(); comp++, compIdx++) {
        pMap[txt()<<"out"<<outIdx<<":compare"<<compIdx] = 
                    txt()<<escapedStr(comp->first,         ":", escapedStr::unescaped).escape()<<":"<<
                           escapedStr(comp->second.first,  ":", escapedStr::unescaped).escape()<<":"<<
                           escapedStr(comp->second.second, ":", escapedStr::unescaped).escape();
      }
    }
    
    // Add the comparators to be used for each measurement
    pMap["numMeasurements"] = txt()<<cm->measComp.size();
    int measIdx=0;
    for(map<string, pair<string, string> >::iterator mc=cm->measComp.begin(); mc!=cm->measComp.end(); mc++, measIdx++) {
      pMap[txt()<<"measure"<<measIdx] = 
              txt()<<escapedStr(mc->first,         ":", escapedStr::unescaped).escape()<<":"<<
                     escapedStr(mc->second.first,  ":", escapedStr::unescaped).escape()<<":"<<
                     escapedStr(mc->second.second, ":", escapedStr::unescaped).escape();
    }
    
    props->add("compModuleTS", pMap);
  }
  
  return props;
}

/**************************************
 ***** processedModuleTraceStream *****
 **************************************/

processedModuleTraceStream::processedModuleTraceStream(int moduleID, 
                                                       processedModule* pm,
                                                       vizT viz, mergeT merge, 
                                                       int traceID, properties* props) : 
  moduleTraceStream(moduleID, (module*)pm, viz, merge, 
                    traceID, setProperties(moduleID, pm, viz, merge, props))
{ }

properties* processedModuleTraceStream::setProperties(int moduleID, 
                                                      processedModule* pm, vizT viz, mergeT merge, properties* props) {
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    map<string, string> pMap;
    pMap["numCmds"] = txt()<<pm->processorCommands.size();
    int i=0;
    for(list<string>::const_iterator c=pm->processorCommands.begin(); c!=pm->processorCommands.end(); c++)
      pMap[txt()<<"cmd"<<i] = *c;
    
    props->add("processedModuleTS", pMap);
  }
  
  return props;
}

/******************************************
 ***** ModuleMergeHandlerInstantiator *****
 ******************************************/

ModuleMergeHandlerInstantiator::ModuleMergeHandlerInstantiator() { 
  (*MergeHandlers   )["modularApp"]   = ModularAppMerger::create;
  (*MergeKeyHandlers)["modularApp"]   = ModularAppMerger::mergeKey;
  (*MergeHandlers   )["module"]       = ModuleMerger::create;
  (*MergeKeyHandlers)["module"]       = ModuleMerger::mergeKey;    
  (*MergeHandlers   )["moduleMarker"] = ModuleMerger::create;
  (*MergeKeyHandlers)["moduleMarker"] = ModuleMerger::mergeKey;    
  (*MergeHandlers   )["moduleEdge"]   = ModuleEdgeMerger::create;
  (*MergeKeyHandlers)["moduleEdge"]   = ModuleEdgeMerger::mergeKey;
  (*MergeHandlers   )["moduleTS"]     = ModuleTraceStreamMerger::create;
  (*MergeKeyHandlers)["moduleTS"]     = ModuleTraceStreamMerger::mergeKey;
  
  (*MergeHandlers   )["compModule"]   = CompModuleMerger::create;
  (*MergeKeyHandlers)["compModule"]   = CompModuleMerger::mergeKey;    
  (*MergeHandlers   )["compModuleTS"] = CompModuleTraceStreamMerger::create;
  (*MergeKeyHandlers)["compModuleTS"] = CompModuleTraceStreamMerger::mergeKey;
    
  MergeGetStreamRecords->insert(&ModuleGetMergeStreamRecord);
}
ModuleMergeHandlerInstantiator ModuleMergeHandlerInstance;

std::map<std::string, streamRecord*> ModuleGetMergeStreamRecord(int streamID) {
  std::map<std::string, streamRecord*> mergeMap;
  mergeMap["modularApp"] = new ModuleStreamRecord(streamID);
  mergeMap["module"]     = new ModuleStreamRecord(streamID);
  return mergeMap;
}


/****************************
 ***** ModularAppMerger *****
 ****************************/

ModularAppMerger::ModularAppMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        BlockMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* ModularAppMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags);
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging modularApps!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "modularApp");

    // All the merged apps must have the same name
    vector<string> cpValues = getValues(tags, "appName");
    assert(allSame<string>(cpValues));
    pMap["appName"] = *cpValues.begin();
    
    // Merge the app IDs along all the streams
    int appID = streamRecord::mergeIDs("modularApp", "appID", pMap, tags, outStreamRecords, inStreamRecords);
    pMap["appID"] = txt() << appID;
    
    /*vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();*/
    
    //ModuleStreamRecord::enterModularApp(outStreamRecords, inStreamRecords);
    props->add("modularApp", pMap);
  } else {
    //ModuleStreamRecord::exitModularApp(outStreamRecords, inStreamRecords);
    props->add("modularApp", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void ModularAppMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  BlockMerger::mergeKey(type, tag.next(), inStreamRecords, key);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; assert(0); }
  if(type==properties::enterTag) {
    key.push_back(tag.get("appName"));
  }
}

/************************
 ***** ModuleMerger *****
 ************************/

ModuleMerger::ModuleMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        Merger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* ModuleMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Modules!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "moduleMarker" || *names.begin() == "module");
    
    /*vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();*/
    
    // Merge the module IDs along all the streams
    //int nodeID = ModuleStreamRecord::mergeNodeIDs("moduleID", pMap, tags, outStreamRecords, inStreamRecords);
    //pMap["moduleID"] = txt() << nodeID;
    
    pMap["name"]       = getSameValue(tags, "name");
    pMap["numInputs"]  = getSameValue(tags, "numInputs");
    pMap["numOutputs"] = getSameValue(tags, "numOutputs");

    group g = ((ModuleStreamRecord*)(outStreamRecords)["module"])->enterModule(
                           instance(pMap["name"], attrValue::parseInt(pMap["numInputs"]), attrValue::parseInt(pMap["numOutputs"])));

    // If this is a module tag, which is placed at the end of the modularApp and records information
    // about the module's execution
    if(*names.begin() == "module") {
      // Record for every incoming stream that we've entered the given module instance, making sure that the 
      // module group along every incoming stream is the same
      /*group g = ModuleStreamRecord::enterModule(
                           instance(pMap["name"], attrValue::parseInt(pMap["numInputs"]), attrValue::parseInt(pMap["numOutputs"])),
                           inStreamRecords);*/
      
      // We must have previously assigned a streamID in the outgoing stream to this module group
      assert(((ModuleStreamRecord*)(outStreamRecords)["module"])->isModuleObserved(g));
      pMap["moduleID"] = txt() << ((ModuleStreamRecord*)(outStreamRecords)["module"])->getModuleID(g);
      //pMap["moduleID"] = txt()<<streamRecord::mergeIDs("module", "moduleID", pMap, tags, outStreamRecords, inStreamRecords);
      
      pMap["count"] = txt()<<vSum(str2int(getValues(tags, "count")));
      props->add("module", pMap);
    
    } else if(*names.begin() == "moduleMarker") {
      // Merge the IDs of the traceStreams associated with these modules
      pMap["traceID"] = txt()<<streamRecord::mergeIDs("traceStream", "traceID", pMap, tags, outStreamRecords, inStreamRecords);
      
      // Record for every incoming stream that we've entered the given module instance, making sure that the 
      // module group along every incoming stream is the same
      /*group g = ModuleStreamRecord::enterModule(
                           instance(pMap["name"], attrValue::parseInt(pMap["numInputs"]), attrValue::parseInt(pMap["numOutputs"])),
                            inStreamRecords);*/
      // The variants must have the same name and numbers of inputs and outputs
      
      props->add("moduleMarker", pMap);
    }
    
  } else {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "moduleMarker" || *names.begin() == "module");
    
    //if(*names.begin() == "moduleMarker") {
    //ModuleStreamRecord::exitModule(inStreamRecords);
      ((ModuleStreamRecord*)(outStreamRecords)["module"])->exitModule();
    //}
    
    
    props->add(*names.begin(), pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void ModuleMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, key);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // We only merge module tags that correspond to the same module in the outgoing stream. This ID is assigned 
    // to nodes while we're processing their moduleNodeTraceStreams because the module tags are emitted after
    // the moduleTS tags.
    /*assert(((ModuleStreamRecord*)inStreamRecords["module"])->mStack.size()>0);
    assert(((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back());*/

    //if(tag.name() == "moduleMarker") {
      key.push_back(tag.get("name"));
      key.push_back(tag.get("numInputs"));
      key.push_back(tag.get("numOutputs"));
    /*} else if(tag.name() == "module") {
      streamID inSID(properties::getInt(tag, "moduleID"), inStreamRecords["module"]->getVariantID());
      //streamID outSID = ((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back()->in2outID(inSID);
      map<string, string> pMap;
      streamID outSID = inStreamRecords["module"]->in2outID(inSID);
      key.push_back(txt()<<outSID.ID);
    }*/
  }
}

/****************************
 ***** ModuleEdgeMerger *****
 ****************************/

ModuleEdgeMerger::ModuleEdgeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        Merger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* ModuleEdgeMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Module Edges!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "moduleEdge");

    /*vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();*/
    
    // Merge the IDs of the module nodes on both sides of the edge, along all the streams
    int fromNodeID = streamRecord::mergeIDs("module", "fromCID", pMap, tags, outStreamRecords, inStreamRecords);
    //int fromNodeID = ModuleStreamRecord::mergeNodeIDs("fromCID", pMap, tags, outStreamRecords, inStreamRecords);
    pMap["fromCID"] = txt() << fromNodeID;
      
    int toNodeID   = streamRecord::mergeIDs("module", "toCID",   pMap, tags, outStreamRecords, inStreamRecords);
    //int toNodeID = ModuleStreamRecord::mergeNodeIDs("toCID", pMap, tags, outStreamRecords, inStreamRecords);
    pMap["toCID"] = txt() << toNodeID;
    
    // The variants must connect the same port indexes with the same types (input/output)
    vector<string> fromT = getValues(tags, "fromT");
    assert(allSame<string>(fromT));
    pMap["fromT"] = *fromT.begin();
    
    vector<string> fromP = getValues(tags, "fromP");
    assert(allSame<string>(fromP));
    pMap["fromP"] = *fromP.begin();
    
    vector<string> toT = getValues(tags, "toT");
    assert(allSame<string>(toT));
    pMap["toT"] = *toT.begin();
    
    vector<string> toP = getValues(tags, "toP");
    assert(allSame<string>(toP));
    pMap["toP"] = *toP.begin();
    
    // Compute the average of the fraction of times we entered this edge's source module group and then took
    // this edge, weighted by the number of times we enter the source module group within each incoming stream.
    std::vector<long> fromCount = str2int  (getValues(tags, "fromCount"));
    std::vector<double> prob    = str2float(getValues(tags, "prob"));
    assert(fromCount.size() == prob.size());
    
    long sumCount=0;
    double sumProb=0;
    for(int i=0; i<fromCount.size(); i++) {
      sumCount += fromCount[i];
      sumProb  += fromCount[i] * prob[i];
    }
    pMap["fromCount"] = txt()<<sumCount;
    pMap["prob"] = txt()<<(sumProb / sumCount);
    
    props->add("moduleEdge", pMap);
  } else {
    props->add("moduleEdge", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void ModuleEdgeMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                                std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, key);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Edges between nodes that were merged to different nodeIDs in the outgoing streams are separated
    /*assert(((ModuleStreamRecord*)inStreamRecords["module"])->mStack.size()>0);
    assert(((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back());*/
    
    streamID inFromSID(tag.getInt("fromCID"), inStreamRecords["module"]->getVariantID());
    //streamID outFromSID = ((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back()->in2outID(inFromSID);
    streamID outFromSID = inStreamRecords["module"]->in2outID(inFromSID);
    key.push_back(txt()<<outFromSID.ID);
    
    streamID inToSID(tag.getInt("toCID"), inStreamRecords["module"]->getVariantID());
    //streamID outToSID = ((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back()->in2outID(inToSID);
    streamID outToSID = inStreamRecords["module"]->in2outID(inToSID);
    key.push_back(txt()<<outToSID.ID);
    
    // Edges between different port numbers/types are separated
    key.push_back(tag.get("fromT"));
    key.push_back(tag.get("fromP"));
    key.push_back(tag.get("toT"));
    key.push_back(tag.get("toP"));
  }
}

/***********************************
 ***** ModuleTraceStreamMerger *****
 ***********************************/

ModuleTraceStreamMerger::ModuleTraceStreamMerger(
                         std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) :
                    TraceStreamMerger(advance(tags), outStreamRecords, inStreamRecords, 
                                      setProperties(tags, outStreamRecords, inStreamRecords, props))
{
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Module TraceStreams!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    /*properties::iterator moduleTSIter = getProps().find("moduleTS");
    ModuleStreamRecord::moduleInfo info(moduleTSIter.getInt("moduleID"),
                                        moduleTSIter.get("name"),
                                        moduleTSIter.getInt("numInputs"),
                                        moduleTSIter.getInt("numOutputs"));
    
    cout << "< ((ModuleStreamRecord*)outStreamRecords[\"module\"])->observedModules (#"<<(((ModuleStreamRecord*)outStreamRecords["module"])->observedModules.size())<<")="<<endl;
    for(map<group, int>::iterator i=((ModuleStreamRecord*)outStreamRecords["module"])->observedModules.begin(); i!=((ModuleStreamRecord*)outStreamRecords["module"])->observedModules.end(); i++)
      cout << "<     "<<i->first.str()<<" : "<<i->second<<endl;
    
    // Record 
    (ModuleStreamRecord*)outStreamRecords["module"])->observedModules.find(info)
    
    // If we already have a record for this moduleTS on the outgoing stream
    if(((ModuleStreamRecord*)outStreamRecords["module"])->observedModules.find(info) != 
       ((ModuleStreamRecord*)outStreamRecords["module"])->observedModules.end())
    {
      cout << "Found "<<moduleTSIter.get("name")<<endl;
      // Don't emit this tag since it will be a duplicate of the moduleTS that we've already emitted
      dontEmit();
    // If we do not yet have a record for this moduleTS, add one
    } else 
      ((ModuleStreamRecord*)outStreamRecords["module"])->observedModules[info] = moduleTSIter.getInt("moduleID");

    cout << "> ((ModuleStreamRecord*)outStreamRecords[\"module\"])->observedModules (#"<<(((ModuleStreamRecord*)outStreamRecords["module"])->observedModules.size())<<")="<<endl;
    for(map<ModuleStreamRecord::moduleInfo, int>::iterator i=((ModuleStreamRefcord*)outStreamRecords["module"])->observedModules.begin(); i!=((ModuleStreamRecord*)outStreamRecords["module"])->observedModules.end(); i++)
      cout << ">     "<<i->first.str()<<" : "<<i->second<<endl;*/
    
    // If in setProperties() we discovered that a moduleTS tag has already been emitted for this module group,
    // don't emit another one
    if(getProps().find("moduleTS").exists("dontEmit"))
      dontEmit();
  }
}

// Sets the properties of this object
properties* ModuleTraceStreamMerger::setProperties(
                         std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Module TraceStreams!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "moduleTS");
    
    pMap["name"] = getSameValue(tags, "name");
    pMap["numInputs"] = getSameValue(tags, "numInputs");
    pMap["numOutputs"] = getSameValue(tags, "numOutputs");
    
    
    
    // Now check to see if the current group on each incoming stream has already been assigned an ID on the
    // outgoing stream
    /*streamID outSID;
    bool sidAssigned=false; // Records whether the ID on the outgoing stream has been assigned
    for(std::vector<std::map<std::string, streamRecord*> >::iterator in=inStreamRecords.begin(); in!=inStreamRecords.end(); in++) {
      ModuleStreamRecord* ms = (ModuleStreamRecord*)(*in)["module"];
      group g = ms->getGroup();
      if(((ModuleStreamRecord*)outStreamRecords["module"])->isModuleObserved(g)) {
        if(!sidAssigned) {
          outSID = ms->getModuleID(g);
          sidAssigned = true;
        } else {
          if(outSID != ms->getModuleID(g)) { cerr << "ERROR: Attempting to merge module IDs of multiple incoming streams but they are mapped to different anchorIDs in the outgoing stream!"<<endl; assert(0); }
        }
      }
    }*/
    //group g = ModuleStreamRecord::getGroup(inStreamRecords);
    group g = ((ModuleStreamRecord*)(outStreamRecords)["module"])->getGroup();
    
    // If we've previously assigned a streamID in the outgoing stream to this module group
    if(((ModuleStreamRecord*)(outStreamRecords)["module"])->isModuleObserved(g)) {
      // This means that a moduleTS tag for this module group has already been emitted and thus, we will
      // not be emitting another one.
      // Set the dontEmit field in pMap to communicate this fact to the ModuleTraceStreamMerger constructor
      pMap["dontEmit"] = 1;
    // If we've never previously observed this module group
    } else {
      int moduleID = ((ModuleStreamRecord*)(outStreamRecords)["module"])->genModuleID();
      // Record the mapping from the moduleIDs of each incoming stream to this moduleID
      //int moduleID = streamRecord::mergeIDs("module", "moduleID", pMap, tags, outStreamRecords, inStreamRecords);
      
      // Associate the new moduleID with the module group
      ((ModuleStreamRecord*)(outStreamRecords)["module"])->setModuleID(g, moduleID);
      
      pMap["moduleID"] = txt() << moduleID;
    }
  } else if(type==properties::exitTag) {
  }
  props->add("moduleTS", pMap);
  
  return props;
}


// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void ModuleTraceStreamMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, key);
   
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // We only merge moduleNodeTraceStreams that have the same name and numbers of inputs and outputs
    // The same rule is used for their associated module but since those tags appear later in the stream
    // they're split according to the ID in the outgoing stream that was assigned to them while processing
    // their moduleNodeTraceStreams.
    key.push_back(tag.get("name"));
    key.push_back(tag.get("numInputs"));
    key.push_back(tag.get("numOutputs"));
  }
}

/****************************
 ***** CompModuleMerger *****
 ****************************/

CompModuleMerger::CompModuleMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        ModuleMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* CompModuleMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Modules!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "compModule");
    
    /*vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();*/
    
    // Merge the module IDs along all the streams
    //int nodeID = ModuleStreamRecord::mergeNodeIDs("moduleID", pMap, tags, outStreamRecords, inStreamRecords);
    /*int nodeID = streamRecord::mergeIDs("module", "moduleID", pMap, tags, outStreamRecords, inStreamRecords);
    pMap["moduleID"] = txt() << nodeID;*/
    
    // The variants must have the same name and numbers of inputs and outputs
    pMap["compModule:isReference"] = getSameValue(tags, "compModule:isReference");
    
    props->add("compModule", pMap);
  } else {
    props->add("compModule", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void CompModuleMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  ModuleMerger::mergeKey(type, tag.next(), inStreamRecords, key);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    key.push_back(tag.get("compModule:isReference"));
  }
}

/***************************************
 ***** CompModuleTraceStreamMerger *****
 ***************************************/

CompModuleTraceStreamMerger::CompModuleTraceStreamMerger(
                         std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) :
                    ModuleTraceStreamMerger(advance(tags), outStreamRecords, inStreamRecords, 
                                            setProperties(tags, outStreamRecords, inStreamRecords, props))
{ }

// Sets the properties of this object
properties* CompModuleTraceStreamMerger::setProperties(
                         std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Comparison Module TraceStreams!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "compModuleTS");

    //int nodeID = ModuleStreamRecord::mergeNodeIDs("nodeID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // We know that all the tags have the same valuues for isReference, options, output comparators and measurements.
    // Thus, we only consider this information from the first tag.
    
    /*common::module::context options = common::module::context(tags[0].second, "op");
    pMap = options.getProperties("op");*/
    
    //pMap["isReference"] = tags[0].second.get("isReference");

    pMap["numOutputs"] = tags[0].second.get("numOutputs");
    int numOutputs = attrValue::parseInt(pMap["numOutputs"]);
    for(int outIdx=0; outIdx<numOutputs; outIdx++) {
      pMap[txt()<<"out"<<outIdx<<":numAttrs"] = tags[0].second.get(txt()<<"out"<<outIdx<<":numAttrs");
      int numAttrs = attrValue::parseInt(pMap[txt()<<"out"<<outIdx<<":numAttrs"]);
      for(int compIdx=0; compIdx<numAttrs; compIdx++) {
        pMap[txt()<<"out"<<outIdx<<":compare"<<compIdx] = tags[0].second.get(txt()<<"out"<<outIdx<<":compare"<<compIdx);
      }
    }
    
    pMap["numMeasurements"] = tags[0].second.get("numMeasurements");
    int numMeasurements = attrValue::parseInt(pMap["numMeasurements"]);
    for(int m=0; m<numMeasurements; m++)
      pMap[txt()<<"measure"<<m] = tags[0].second.get(txt()<<"measure"<<m);
  }
  props->add("compModuleTS", pMap);
  
  return props;
}


// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void CompModuleTraceStreamMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, key);
   
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // We only merge moduleNodeTraceStreams that have the same values for isReference, (they either are both 
    // examples of the reference configuration or they're bith not), same configuration, same output comparators and 
    // same measurements. 
    // The same rule is used for their associated module but since those tags appear later in the stream
    // they're split according to the ID in the outgoing stream that was assigned to them while processing
    // their moduleNodeTraceStreams.
    //key.push_back(tag.get("isReference"));
    
    // Same options
    /*common::module::context options(tag, "op");
    key.push_back(txt()<<options.configuration.size());
    int cIdx=0;
    for(map<string, attrValue>::iterator c=options.configuration.begin(); c!=options.configuration.end(); c++, cIdx++) {
      key.push_back(c->first);
      key.push_back(c->second.serialize());
    }*/
    
    // Same comparators on outputs
    key.push_back(tag.get("numOutputs"));
    int numOutputs = tag.getInt("numOutputs");
    for(int outIdx=0; outIdx<numOutputs; outIdx++) {
      key.push_back(tag.get(txt()<<"out"<<outIdx<<":numAttrs"));
      int numAttrs = tag.getInt(txt()<<"out"<<outIdx<<":numAttrs");
      for(int compIdx=0; compIdx<numAttrs; compIdx++) {
        key.push_back(tag.get(txt()<<"out"<<outIdx<<":compare"<<compIdx));
      }
    }
    
    // Same measurements
    key.push_back(tag.get("numMeasurements"));
    // Concatenate the descriptions of the measurements
    int numMeasurements = tag.getInt("numMeasurements");
    ostringstream measurements;
    for(int m=0; m<numMeasurements; m++)
      key.push_back(tag.get(txt()<<"measure"<<m));
  }
}

/*****************************
 ***** ModuleStreamRecord *****
 *****************************/

// Called when a module is entered/exited along the given stream to record the current module group
group ModuleStreamRecord::enterModule(const instance& inst) { 
  iStack.push_back(inst);
  //cout << "<("<<this<<")#iStack = "<<iStack.size()<<", iStack.back()="<<(&(iStack.back()))<<endl;
  return getGroup();
}

// Record that we've entered the given module instance on all the given incoming streams
group ModuleStreamRecord::enterModule(const instance& inst, const std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  // Record for every incoming stream that we've entered the given module instance, making sure that the 
  // module group along every incoming stream is the same
  group g;
  for(std::vector<std::map<std::string, streamRecord*> >::const_iterator in=inStreamRecords.begin(); in!=inStreamRecords.end(); in++) {
    std::map<std::string, streamRecord*>::const_iterator modIter = in->find("module");
    assert(modIter != in->end());
    ModuleStreamRecord* ms = (ModuleStreamRecord*)modIter->second;
    group curGroup = ms->enterModule(inst);
    if(in == inStreamRecords.begin()) 
      g = curGroup;
    else if(g != curGroup)
    { cerr << "ERROR: Attempting to merge modules from multiple incoming streams but the module groups on these streams are not identical!"<<endl; assert(0); }
  }
  return g;
}

void ModuleStreamRecord::exitModule() { 
  //cout << ">("<<this<<")#iStack = "<<iStack.size()<<", iStack.back()="<<(&(iStack.back()))<<endl;
  assert(iStack.size()>0);
  iStack.pop_back();
}

// Record that we've exited the given module instance on all the given incoming streams
void ModuleStreamRecord::exitModule(const std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  for(std::vector<std::map<std::string, streamRecord*> >::const_iterator in=inStreamRecords.begin(); in!=inStreamRecords.end(); in++) {
    std::map<std::string, streamRecord*>::const_iterator modIter = in->find("module");
    assert(modIter != in->end());
    ((ModuleStreamRecord*)modIter->second)->exitModule();
  }
}

// Returns the group denoted by the current stack of module instances
group ModuleStreamRecord::getGroup()
{ return group(iStack); }

// Returns the group denoted by the current stack of module instances in all the given incoming streams 
// (must be identical on all of them).
group ModuleStreamRecord::getGroup(const std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) { 
  group g;
  for(std::vector<std::map<std::string, streamRecord*> >::const_iterator in=inStreamRecords.begin(); in!=inStreamRecords.end(); in++) {
    std::map<std::string, streamRecord*>::const_iterator modIter = in->find("module");
    assert(modIter != in->end());
    ModuleStreamRecord* ms = (ModuleStreamRecord*)modIter->second;
    if(in == inStreamRecords.begin()) 
      g = ms->getGroup();
    else if(ms->getGroup() != g)
    { cerr << "ERROR: Attempting to merge modules from multiple incoming streams but the module groups on these streams are not identical!"<<endl; assert(0); }
  }
  return g;
}

ModuleStreamRecord::ModuleStreamRecord(const ModuleStreamRecord& that, int vSuffixID) :
  streamRecord(that, vSuffixID), iStack(that.iStack), observedModules(that.observedModules), maxModuleID(that.maxModuleID)
{}

/*
// Called to record that we've entered/exited a module
void ModuleStreamRecord::enterModularApp() {
  mStack.push_back(new streamRecord(getVariantID(), "moduleNode"));
}

void ModuleStreamRecord::enterModularApp(map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords) {
  for(vector<map<string, streamRecord*> >::iterator i=inStreamRecords.begin();
      i!=inStreamRecords.end(); i++) {
    assert(dynamic_cast<ModuleStreamRecord*>((*i)["module"]));
    dynamic_cast<ModuleStreamRecord*>((*i)["module"])->enterModule();
  }
  dynamic_cast<ModuleStreamRecord*>(outStreamRecords["module"])->enterModule();
}

void ModuleStreamRecord::exitModularApp() {
  assert(mStack.size()>0);
  assert(mStack.back());
  delete mStack.back();
  mStack.pop_back();
}

void ModuleStreamRecord::exitModularApp(map<string, streamRecord*>& outStreamRecords,
                                    vector<map<string, streamRecord*> >& inStreamRecords) {
  for(vector<map<string, streamRecord*> >::iterator i=inStreamRecords.begin();
      i!=inStreamRecords.end(); i++) {
    assert(dynamic_cast<ModuleStreamRecord*>((*i)["module"]));
    dynamic_cast<ModuleStreamRecord*>((*i)["module"])->exitModule();
  }
  dynamic_cast<ModuleStreamRecord*>(outStreamRecords["module"])->exitModule();
}*/

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* ModuleStreamRecord::copy(int vSuffixID) {
  return new ModuleStreamRecord(*this, vSuffixID);
}

/* // Given a vector of streamRecord maps, collects the streamRecords associated with the currently active module (top of the mStack)
// within each stream into nodeStreamRecords and returns the height of the mStacks on all the streams (must be the same number)
int ModuleStreamRecord::collectNodeStreamRecords(std::vector<std::map<std::string, streamRecord*> >& streams,
                                                 std::vector<std::map<std::string, streamRecord*> >& nodeStreamRecords) {
  int stackSize=0;
  
  // Iterate over the mStacks in all the streams
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    // Make sure that the mStack is the same height along all the incoming streams
    if(s==streams.begin()) stackSize = ((ModuleStreamRecord*)(*s)["module"])->mStack.size();
    else assert(stackSize == ((ModuleStreamRecord*)(*s)["module"])->mStack.size());
    
    // Add a reference to nodes streamRecord to nodeInStreamRecords
    if(stackSize>0) {
      map<string, streamRecord*> srMap;
      srMap["moduleNode"] = ((ModuleStreamRecord*)(*s)["module"])->mStack.back();
      nodeStreamRecords.push_back(srMap);
    }
  }
  
  / * // The mStack must contain at least one module
  assert(stackSize > 0);* /
  
  return stackSize;
}

// Applies streamRecord::mergeIDs to the nodeIDs of the currently active module
int ModuleStreamRecord::mergeNodeIDs(
                         std::string IDName, 
                         std::map<std::string, std::string>& pMap, 
                         const vector<pair<properties::tagType, properties::iterator> >& tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  
  // Iterate over the mStacks in all the incoming streams
  vector<map<string, streamRecord*> > nodeInStreamRecords;
  int stackSize = collectNodeStreamRecords(inStreamRecords, nodeInStreamRecords);
  
  // Make sure that the size of the mStack on the incoming streams as the same as it is on the outgoing stream
  assert(stackSize == ((ModuleStreamRecord*)outStreamRecords["module"])->mStack.size());
  
  if(stackSize>0) {
    // Set nodeOutStreamRecords to refer to just the streamRecord of the currently active module
    std::map<std::string, streamRecord*> nodeOutStreamRecords;
    nodeOutStreamRecords["module"] = ((ModuleStreamRecord*)outStreamRecords["module"])->mStack.back();

    // Call mergeIDs on the nodeInStreamRecords to merge this nodeIS    
    return mergeIDs("module", IDName, pMap, tags, nodeOutStreamRecords, nodeInStreamRecords);
  } else
    return -1;
}*/

// Given multiple streamRecords from several variants of the same stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void ModuleStreamRecord::resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams) {
  streamRecord::resumeFrom(streams);
    
  /*vector<map<string, streamRecord*> > nodeStreamRecords;
  int stackSize = collectNodeStreamRecords(streams, nodeStreamRecords);
  
  streamRecord::resumeFrom(nodeStreamRecords);*/
  
  // Set iStack to be equal to its counterparts in streams, which should have identical iStacks
  iStack.clear();
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    ModuleStreamRecord* ms = (ModuleStreamRecord*)(*s)["module"];
    if(s==streams.begin())
      iStack = ms->iStack;
    else if(iStack != ms->iStack) { cerr << "ERROR: inconsistent stacks of module instances along different incoming streams!"<<endl; }
  }
  
  // Set observedModules to be the union of its counterparts in streams
  observedModules.clear();
  
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    ModuleStreamRecord* ms = (ModuleStreamRecord*)(*s)["module"];

    /*cout << "| | ModuleStreamRecord::resumeFrom observedModules ms (#"<<ms->observedModules.size()<<")="<<endl;
    for(map<ModuleStreamRecord::moduleInfo, int>::iterator i=ms->observedModules.begin(); i!=ms->observedModules.end(); i++)
      cout << "|       "<<i->first.str()<<" : "<<i->second<<endl;*/
    
    for(map<group, int>::const_iterator i=ms->observedModules.begin(); i!=ms->observedModules.end(); i++)
      observedModules.insert(*i);
  }
  
  /*cout << "| ModuleStreamRecord::resumeFrom observedModules (#"<<observedModules.size()<<")="<<endl;
  for(map<ModuleStreamRecord::moduleInfo, int>::iterator i=observedModules.begin(); i!=observedModules.end(); i++)
    cout << "|     "<<i->first.str()<<" : "<<i->second<<endl;*/
  
  // Set maxModuleID to be the maximum of its counterparts in streams
  maxModuleID = -1;
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    ModuleStreamRecord* ms = (ModuleStreamRecord*)(*s)["module"];
    maxModuleID = (ms->maxModuleID > maxModuleID? ms->maxModuleID: maxModuleID);
  }
}
  
std::string ModuleStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[ModuleStreamRecord: ";
  s << streamRecord::str(indent+"    ") << endl;  
  /*int i=0;
  s << indent << "mStack(#"<<mStack.size()<<")="<<endl;
  for(std::list<streamRecord*>::const_iterator m=mStack.begin(); m!=mStack.end(); m++, i++)
    s << indent << i << ": "<< (*m)->str(indent) << endl;*/
  s << indent << "]";
  
  return s.str();
}

}; // namespace structure
}; // namespace sight
