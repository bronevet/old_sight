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

// -------------------------
// ----- Configuration -----
// -------------------------

// Record the configuration handlers in this file
moduleConfHandlerInstantiator::moduleConfHandlerInstantiator() {
  (*enterHandlers)["modularApp"] = &modularApp::configure;
  (*exitHandlers )["modularApp"] = &moduleConfHandlerInstantiator::defaultExitFunc;
  /*(*confEnterHandlers)["modularAppBody"]      = &defaultConfEntryHandler;
  (*confExitHandlers )["modularAppBody"]      = &defaultConfExitHandler;
  (*confEnterHandlers)["modularAppStructure"] = &defaultConfEntryHandler;
  (*confExitHandlers )["modularAppStructure"] = &defaultConfExitHandler;
  (*confEnterHandlers)["moduleTS"]            = &moduleTraceStream::enterTraceStream;
  (*confExitHandlers )["moduleTS"]            = &defaultConfExitHandler;
  (*confEnterHandlers)["module"]              = &modularApp::enterModule;
  (*confExitHandlers )["module"]              = &modularApp::exitModule;
  (*confEnterHandlers)["moduleMarker"]        = &defaultConfEntryHandler;
  (*confExitHandlers )["moduleMarker"]        = &defaultConfExitHandler;
  (*confEnterHandlers)["moduleCtrl"]          = &defaultConfEntryHandler;
  (*confExitHandlers )["moduleCtrl"]          = &defaultConfExitHandler;
  (*confEnterHandlers)["moduleEdge"]          = &modularApp::addEdge;
  (*confExitHandlers )["moduleEdge"]          = &defaultConfExitHandler;
  (*confEnterHandlers)["compModuleTS"]        = &compModuleTraceStream::enterTraceStream;
  (*confExitHandlers )["compModuleTS"]        = &defaultConfExitHandler;
  (*confEnterHandlers)["processedModuleTS"]   = &processedModuleTraceStream::enterTraceStream;
  (*confExitHandlers )["processedModuleTS"]   = &defaultConfExitHandler;*/
  (*enterHandlers)["module"]          = &module::configure;
  (*exitHandlers )["module"]          = &moduleConfHandlerInstantiator::defaultExitFunc;
  (*enterHandlers)["compModule"]      = &compModule::configure;
  (*exitHandlers )["compModule"]      = &moduleConfHandlerInstantiator::defaultExitFunc;
}
moduleConfHandlerInstantiator moduleConfHandlerInstance;


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
  
  s << "[group: ";
  for(list<instance>::const_iterator i=stack.begin(); i!=stack.end(); i++) {
    if(i!=stack.begin()) s << " ";
    s << "    "<<i->str();
  }
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
    block(appName, setProperties(appName, NULL, props)), appName(appName)
{ init(); }
        
modularApp::modularApp(const std::string& appName, const attrOp& onoffOp,                            properties* props) :
    block(appName, setProperties(appName, &onoffOp, props)), appName(appName)
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
  
  // Emit the tag that starts the description of the modularApp's body
  map<string, string> pMapMABody;
  properties propsMABody;
  propsMABody.add("modularAppBody", pMapMABody);
  dbg.enter(propsMABody);
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

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void modularApp::destroy() {
  this->~modularApp();
}

modularApp::~modularApp() {
  assert(!destroyed);
  
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
    
    // Emit the tag that ends the description of the modularApp's body
    map<string, string> pMapMABody;
    properties propsMABody;
    propsMABody.add("modularAppBody", pMapMABody);
    dbg.exit(propsMABody);
    
    // Emit the tag that starts the description of the modularApp's structure
    map<string, string> pMapMAStructure;
    properties propsMAStructure;
    propsMAStructure.add("modularAppStructure", pMapMAStructure);
    dbg.enter(propsMAStructure);
    
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
    
    // Emit the exit tag that denotes the end of the description of the modularApp's structure
    dbg.exit(propsMAStructure);
    
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
      if(inouts[i].ctxt->configuration.size() != moduleCtxtNames[g][i].size()) { 
         cerr << "ERROR: Inconsistent number of context attributes for "<<(io==sight::common::module::input?"input":"output")<<" "<<i<<" of module "<<g.name()<<"!"<<endl;
         cerr << "Prior instances has "<<moduleCtxtNames[g][i].size()<<" "<<(io==sight::common::module::input?"inputs":"outputs")<<":"<<endl;
         for(list<string>::const_iterator c=moduleCtxtNames[g][i].begin(); c!=moduleCtxtNames[g][i].end(); c++)
           cerr << "    "<<*c<<endl; 
         cerr << "This instance has "<<inouts[i].ctxt->configuration.size()<<" "<<(io==sight::common::module::input?"inputs":"outputs")<<"!"<<endl; 
         for(map<std::string, attrValue>::const_iterator c=inouts[i].ctxt->configuration.begin(); c!=inouts[i].ctxt->configuration.end(); c++)
           cerr << "    "<<c->first<<endl; 
         assert(0);
      }
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
void modularApp::enterModule(module* m, int moduleID, properties* props) {
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
    //assert(*(moduleProps[m->g]) == *props);
    if(*(moduleProps[m->g]) != *props) {
      cerr << "ERROR: module "<<m->g.str()<<" observed multiple times with different properties (count of inputs and outputs)!\n";
      assert(0);
    }
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
  if(moduleTraceID.find(g) == moduleTraceID.end()) { cerr << "ERROR: No traceStream registered for module \""<<g.str()<<"\"!"<<endl; }
  assert(moduleTraceID.find(g) != moduleTraceID.end());
  return moduleTraceID[g];
}

// Removes the given module object from the modules stack
void modularApp::exitModule(module* m) {
  // Exactly one modularAppInstance must be active
  assert(isInstanceActive());

  // Pop this module from the mStack
  assert(mStack.size()>0);
  assert(mStack.back()==m);
  mStack.pop_back();
}

/******************
 ***** module *****
 ******************/

module::module(const instance& inst,                                                     properties* props) : 
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(inputs(), props); }

module::module(const instance& inst, const port& in,                                     properties* props): 
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(inputs(in), props); }

module::module(const instance& inst, const std::vector<port>& in,                        properties* props) :
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(in, props); }

module::module(const instance& inst,                              const attrOp& onoffOp, properties* props) : 
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(inputs(), props); }

module::module(const instance& inst, const port& in,              const attrOp& onoffOp, properties* props): 
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(inputs(in), props); }

module::module(const instance& inst, const std::vector<port>& in, const attrOp& onoffOp, properties* props) :
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(NULL)
{ init(in, props); }


module::module(const instance& inst,                              std::vector<port>& externalOutputs,                        properties* props) : 
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(inputs(), props); }

module::module(const instance& inst, const port& in,              std::vector<port>& externalOutputs,                        properties* props): 
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(inputs(in), props); }

module::module(const instance& inst, const std::vector<port>& in, std::vector<port>& externalOutputs,                        properties* props) :
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(in, props); }

module::module(const instance& inst,                              std::vector<port>& externalOutputs, const attrOp& onoffOp, properties* props) : 
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(inputs(), props); }

module::module(const instance& inst, const port& in,              std::vector<port>& externalOutputs, const attrOp& onoffOp, properties* props): 
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(inputs(in), props); }

module::module(const instance& inst, const std::vector<port>& in, std::vector<port>& externalOutputs, const attrOp& onoffOp, properties* props) :
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs)
{ init(in, props); }



module::module(const instance& inst,                                                     const namedMeasures& meas, properties* props) : 
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(inputs(), props); }

module::module(const instance& inst, const port& in,                                     const namedMeasures& meas, properties* props): 
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(inputs(in), props); }

module::module(const instance& inst, const std::vector<port>& in,                        const namedMeasures& meas, properties* props) :
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(in, props); }

module::module(const instance& inst,                              const attrOp& onoffOp, const namedMeasures& meas, properties* props) : 
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(inputs(), props); }

module::module(const instance& inst, const port& in,              const attrOp& onoffOp, const namedMeasures& meas, properties* props): 
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(inputs(in), props); }

module::module(const instance& inst, const std::vector<port>& in, const attrOp& onoffOp, const namedMeasures& meas, properties* props) :
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(NULL), meas(meas)
{ init(in, props); }


module::module(const instance& inst,                              std::vector<port>& externalOutputs,                        const namedMeasures& meas, properties* props) : 
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(inputs(), props); }

module::module(const instance& inst, const port& in,              std::vector<port>& externalOutputs,                        const namedMeasures& meas, properties* props): 
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(inputs(in), props); }

module::module(const instance& inst, const std::vector<port>& in, std::vector<port>& externalOutputs,                        const namedMeasures& meas, properties* props) :
  sightObj(setProperties(inst, props, NULL, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(in, props); }

module::module(const instance& inst,                              std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props) : 
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(inputs(), props); }

module::module(const instance& inst, const port& in,              std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props): 
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(inputs(in), props); }

module::module(const instance& inst, const std::vector<port>& in, std::vector<port>& externalOutputs, const attrOp& onoffOp, const namedMeasures& meas, properties* props) :
  sightObj(setProperties(inst, props, &onoffOp, this)), g(modularApp::mStack, inst), externalOutputs(&externalOutputs), meas(meas)
{ init(in, props); }

properties* module::setProperties(const instance& inst, properties* props, const attrOp* onoffOp, module* me) {
  group g(modularApp::mStack, inst);
  bool isDerived = (props!=NULL); // This is an instance of an object that derives from module if its constructor sets props to non-NULL
 
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    if(!modularApp::isInstanceActive()) {
      cerr << "ERROR: module "<<inst.str()<<" entered while there no instance of modularApp is active!"<<endl;
      assert(0);
    }
 
    // The properties object for the moduleMarker that will be emitted at this location in the log
    properties* markerProps = new properties();
  
    map<string, string> pMap;
    //pMap["moduleID"] = txt()<<modularApp::genModuleID(g);
    pMap["name"]       = g.name();
    pMap["numInputs"]  = txt()<<g.numInputs();
    pMap["numOutputs"] = txt()<<g.numOutputs();

    // If this is an instance of module rather than a class that derives from module
    //if(modularApp::isInstanceActive() && !isDerived) {
      markerProps->active = true;

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
    //}
  
    markerProps->add("moduleMarker", pMap);
    
    return markerProps;
  } else
    return NULL;
}

void module::init(const std::vector<port>& ins, properties* derivedProps) {
  isDerived = (derivedProps!=NULL); // This is an instance of an object that derives from module if its constructor sets props to non-NULL
  if(derivedProps==NULL) derivedProps = new properties();
  this->ins = ins;
  
  if(modularApp::isInstanceActive() && derivedProps->active) {
    moduleID = modularApp::genModuleID(g);
    
    // Add the properties of this module to derivedProps
    map<string, string> pMap;
    pMap["moduleID"]   = txt()<<moduleID;
    pMap["name"]       = g.name();
    pMap["numInputs"]  = txt()<<g.numInputs();
    pMap["numOutputs"] = txt()<<g.numOutputs();
    derivedProps->add("module", pMap);
    
    // Add this module instance to the current stack of modules
    modularApp::enterModule(this, moduleID, derivedProps);
    
    // Make sure that the input contexts have the same names across all the invocations of this module group
    modularApp::registerInOutContexts(g, ins, sight::common::module::input);

    // Record any publicized inputs
    addPublicizedInputs(ins, numPublicizedInputs, modularApp::getInstance()->publicizedInputs,
                  modularApp::getInstance()->publicizedInputNotes);
    
    // Add edges between the modules from which this module's inputs came and this module
    for(int i=0; i<ins.size(); i++)
    	modularApp::addEdge(ins[i], port(g, context(), input, i));
    
    // Initialize the output ports of this module
    if(externalOutputs) externalOutputs->clear(); 
    for(int i=0; i<g.numOutputs(); i++) {
      outs.push_back(port(g, /*context(),*/ output, i));
      // If the user provided an output vector, initialize it as well
      if(externalOutputs) { 
        externalOutputs->push_back(port(g, /*context(),*/ output, i));
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
  
  // We've not yet completed measuring this module's behavior
  measurementCompleted = false;
  
  //cout << "g="<<g.str()<<", ins.size()="<<ins.size()<<endl;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void module::destroy() {
  this->~module();
}

module::~module() {
  assert(!destroyed);
  
  //cout << "~module() props->active="<<props->active<<endl;
  if(ins.size() != g.numInputs()) { cerr << "WARNING: module \""<<g.name()<<"\" specifies "<<g.numInputs()<<" inputs but "<<ins.size()<<" inputs are actually provided!"<<endl; }
  
  if(outsSet.size() != g.numOutputs()) { 
    cerr << "WARNING: module \""<<g.name()<<"\" specifies "<<g.numOutputs()<<" outputs but "<<outsSet.size()<<" outputs are actually provided! ";
    cerr << "Missing outputs:";
    for(int i=0; i<outs.size(); i++) if(outsSet.find(i)==outsSet.end()) cerr << " "<<i;
    cerr << endl;
  }
  
  if(props->active) {
    // If this is an instance of module rather than a class that derives from module
    if(!isDerived) {
      // Wrap the portion of the moduleMarker tag where we place the control information (the traceStream)
      // in its own tag to make them easier to match across different streams
      properties moduleCtrl;
      moduleCtrl.add("moduleCtrl", map<string, string>());

      dbg.enter(moduleCtrl);

      // Register a traceStream for this module's group, if one has not already been registered
      if(!modularApp::isTraceStreamRegistered(g)) {
        modularApp::registerTraceStream(g, new moduleTraceStream(moduleID, this, trace::lines, trace::disjMerge, 
                                        modularApp::getTraceStreamID(g)));
      }
    }

    // Set the context attributes to be used in this module's measurements by combining the context provided by any
    // class that may derive from this one as well as the contexts of its inputs
    //map<string, attrValue> traceCtxt = getTraceCtxt();
    if(!isDerived) setTraceCtxt();
    /*cout << "traceCtxt="<<endl;
    for(map<string, attrValue>::iterator tc=traceCtxt.begin(); tc!=traceCtxt.end(); tc++)
      cout << "    "<<tc->first << ": "<<tc->second.serialize()<<endl;*/

    // Complete the measurement of application's behavior during the module's lifetime
    completeMeasurement();
    
    // Add to the trace observation the properties of all of the module's outputs
    for(int i=0; i<outs.size(); i++) {
      if(outs[i].ctxt==NULL) continue;

      for(map<std::string, attrValue>::iterator c=outs[i].ctxt->configuration.begin();
          c!=outs[i].ctxt->configuration.end(); c++) {
        obs.push_back(make_pair(encodeCtxtName("module", "output", txt()<<i, c->first), c->second));
      }
    }
    
    // Clear all records of publicized inputs
    clearPublicized(numPublicizedInputs, modularApp::getInstance()->publicizedInputs,
                    modularApp::getInstance()->publicizedInputNotes);
    
    /*cout << "obs="<<endl;
  for(list<pair<string, attrValue> >::iterator tc=obs.begin(); tc!=obs.end(); tc++)
    cout << "    "<<tc->first << ": "<<tc->second.serialize()<<endl;*/

    // Record the observation into this module group's trace
    modularApp::moduleTrace[g]->traceFullObservation(traceCtxt, obs, anchor::noAnchor);

    /* GB 2014-01-24 - It makes sense to allow different instances of a module group to not provide
     *        the same outputs since in fault injection we may be aborted before all the outputs are 
     *        computed and in other cases the code may break out without computing some output. *?
    // Make sure that the output contexts have the same names across all the invocations of this module group
    modularApp::registerInOutContexts(g, outs, sight::common::module::output); */
  
    modularApp::exitModule(this);
    
    // Close the tag that wraps the control section of this module
    properties moduleCtrl;
    moduleCtrl.add("moduleCtrl", map<string, string>());
    dbg.exit(moduleCtrl);
  }
}

// Called to complete the measurement of this module's execution. This measurement may be completed before
// the module itself completes to enable users to separate the portion of the module's execution that 
// represents its core behavior (and thus should be measured) from the portion where the module's outputs
// are computed.
void module::completeMeasurement() {
  if(!props->active) return;
  
  if(measurementCompleted) return; // { cerr << "ERROR: completing measurement of execution of module "<<g.str()<<" multiple times!"<<endl; assert(0); }
  
  // Complete measuring this instance and collect the observations into props
  //cout << "module::completeMeasurement() "<<g.str()<<", #meas="<<meas.size()<<endl;
  int measIdx=0;
  for(namedMeasures::iterator m=meas.begin(); m!=meas.end(); m++, measIdx++) {
    //cout << "    "<<m->second->str()<<endl;
    // Complete the m's measurement
    list<pair<string, attrValue> > curObs = m->second->endGet();
    // Push each measurement onto the observation
    for(list<pair<string, attrValue> >::iterator o=curObs.begin(); o!=curObs.end(); o++) {
      //cout << "        "<<m->first<<": "<<o->first<<": "<<o->second.getAsStr()<<endl;
      obs.push_back(make_pair(encodeCtxtName("module", "measure", m->first, o->first), o->second));
    }
    delete m->second;
  }
  
  // We've completed measuring this module
  measurementCompleted = true;
}

// Records any publicized inputs or options inside this module and its associated modularApp. Modules
// executed while this module is active will inherit these inputs and options.
void module::addPublicizedInputs(const std::vector<port>& ins, int& numPublicized,
                           std::list<std::pair<std::string, attrValue> >& publicized,
                           std::list<std::pair<std::string, notes> >& publicizedNotes) {
  numPublicized=0;
  for(int i=0; i<ins.size(); i++) {
    addPublicized(*(ins[i].ctxt), numPublicized, publicized, publicizedNotes);
  }
}

void module::addPublicizedOptions(const context& opts, int& numPublicized,
                           std::list<std::pair<std::string, attrValue> >& publicized,
                           std::list<std::pair<std::string, notes> >& publicizedNotes) {
  numPublicized=0;
  addPublicized(opts, numPublicized, publicized, publicizedNotes);
}

void module::addPublicized(const context& insopts, int& numPublicized,
                           std::list<std::pair<std::string, attrValue> >& publicized,
                           std::list<std::pair<std::string, notes> >& publicizedNotes)
{
  for(std::map<std::string, notes>::const_iterator cn=insopts.configNotes.begin(); cn!=insopts.configNotes.end(); cn++) {
    for(notes::const_iterator n=cn->second.begin(); n!=cn->second.end(); n++) {
       if(n->getName() == "publicized") {
        numPublicized++;
        map<string, attrValue>::const_iterator val=insopts.configuration.find(cn->first);
        assert(val != insopts.configuration.end());
        publicized.push_back(make_pair(cn->first, val->second));
        publicizedNotes.push_back(make_pair(cn->first, cn->second));
      }
    }
  }
}

// Clears off any records of publicized inputs or options that were set in addPublicized()
void module::clearPublicized(int& numPublicized,
                             std::list<std::pair<std::string, attrValue> >& publicized,
                             std::list<std::pair<std::string, notes> >& publicizedNotes) {
  assert(publicized.size()>=numPublicized);
  while(numPublicized>0) {
    publicized.pop_back();
    publicizedNotes.pop_back();
    numPublicized--;
  }
}


// Sets the context of the given output port
void module::setInCtxt(int idx, const context& c) {
  if(!props->active) return;
  if(idx>=g.numInputs()) { cerr << "ERROR: cannot set context of input "<<idx<<" of module \""<<g.str()<<"\"! This module was declared to have "<<g.numInputs()<<" inputs."<<endl; }
  assert(idx<g.numInputs());
  ins[idx].setCtxt(c);
}

// Adds the given key/attrValue pair to the context of the given output port
void module::addInCtxt(int idx, const std::string& key, const attrValue& val) {
  if(!props->active) return;
  if(idx>=g.numInputs()) { cerr << "ERROR: cannot add context to input "<<idx<<" of module \""<<g.str()<<"\"! This module was declared to have "<<g.numInputs()<<" inputs."<<endl; }
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
  if(idx>=g.numOutputs()) { cerr << "ERROR: cannot set context of output "<<idx<<" of module \""<<g.str()<<"\"! This module was declared to have "<<g.numOutputs()<<" outputs."<<endl; }
  assert(idx<g.numOutputs());
  outs[idx].setCtxt(c);
  // If the user provided an output vector, update it as well
  if(externalOutputs)
    (*externalOutputs)[idx].setCtxt(c);
    
  // Emit a warning of an output has been set multiple times
  if(outsSet.find(idx) != outsSet.end()) cerr << "WARNING: output "<<idx<<" of module \""<<g.str()<<"\" has been set multiple times! Set this time to context "<<c.str()<<endl;
  else outsSet.insert(idx);
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
  if(idx>=g.numOutputs()) { cerr << "ERROR: cannot get port "<<idx<<" of module \""<<g.str()<<"\"! This module was declared to have "<<g.numOutputs()<<" outputs."<<endl; }
  assert(idx < g.numOutputs());
  return outs[idx];  
}

// Sets the traceCtxt map, which contains the context attributes to be used in this module's measurements 
// by combining the context provided by the classes that this object derives from with its own unique 
// context attributes.
void module::setTraceCtxt() {
  // Add to it the context of this module based on the inputs publicized by containing modules, omitting any
  // from this module.
  int j=0;
  for(list<pair<string, attrValue> >::const_iterator i=modularApp::getInstance()->publicizedInputs.begin(); 
         i!=modularApp::getInstance()->publicizedInputs.end() && 
         j<modularApp::getInstance()->publicizedInputs.size()-numPublicizedInputs; 
      i++, j++)
    traceCtxt[encodeCtxtName("module", "pub_input", "", i->first)] = i->second;

  // Add to it the context of this module based on the contexts of its inputs
  for(int i=0; i<ins.size(); i++) {
    for(std::map<std::string, attrValue>::const_iterator c=ins[i].ctxt->configuration.begin(); c!=ins[i].ctxt->configuration.end(); c++) {
      traceCtxt[encodeCtxtName("module", "input", txt()<<i, c->first)] = c->second;
    }
  }
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
    // If both this and that are compContexts
    const compContext& that = dynamic_cast<const compContext&>(that_arg);
    return context::operator==(that_arg) && comparators==that.comparators;
  } catch(std::bad_cast exp) {
    // Otherwise, relate different context instances based on their typeids
    //cout << "compContext::operator==() typeid(*this)="<<typeid(*this).name()<<", typeid(that_arg)="<<typeid(that_arg).name()<<", == "<<(typeid(*this) == typeid(that_arg))<<endl;
    return typeid(*this) == typeid(that_arg);
    /*cerr << "ERROR: comparing compContext and a different sub-type of context using == operator!"<<endl;
    assert(0);*/
  }
}

bool compContext::operator<(const context& that_arg) const { 
  try {
    // If both this and that are compContexts
    const compContext& that = dynamic_cast<const compContext&>(that_arg);
    return context::operator<(that_arg) ||
           (context::operator==(that_arg) && comparators<that.comparators);
  } catch(std::bad_cast exp) {
    // Otherwise, relate different context instances based on their typeids
    //cout << "compContext::operator<() typeid(*this)="<<typeid(*this).name()<<", typeid(that_arg)="<<typeid(that_arg).name()<<", < "<<(typeid(*this).name() < typeid(that_arg).name())<<endl;
    return typeid(*this).name() < typeid(that_arg).name();
    /*cerr << "ERROR: comparing compContext and a different sub-type of context using < operator!"<<endl;
    assert(0);*/
  }
}

// Adds the given key/attrValue pair to this context
void compContext::add(std::string key, const attrValue& val, const comparatorDesc& cdesc) {
  context::add(key, val);
  comparators[key] = std::make_pair(cdesc.name(), cdesc.description());
}

// Add all the key/attrValue pairs from the given context to this one, overwriting keys as needed
void compContext::add(const compContext& that) {
  context::add(that);
  for(std::map<std::string, std::pair<std::string, std::string> >::const_iterator c=that.comparators.begin();
      c!=that.comparators.end(); c++) {
    comparators[c->first] = c->second;
  }
}

// Returns a human-readable string that describes this context
std::string compContext::str() const {
  ostringstream s;
  s << "[compContext: "<<endl;
  s << "    "<<context::str()<<endl;
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

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void compModularApp::destroy() {
  this->~compModularApp();
}

compModularApp::~compModularApp() {
  assert(!destroyed);
}

/**********************
 ***** compModule *****
 **********************/

compModule::compModule(const instance& inst, const std::vector<port>& inputs,  
                       bool isReference,
                                                                         properties* props) :
            module(inst, inputs, setProperties(inst, isReference, module::context(), NULL, props)),           isReference(isReference), options(module::context())
{ init(context(), props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs,  
                       bool isReference, 
                       const attrOp& onoffOp,                            properties* props) :
          module(inst, inputs, setProperties(inst, isReference, module::context(), &onoffOp, props)),       isReference(isReference), options(module::context())
{ init(context(), props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs,  
                       bool isReference, 
                                              const compNamedMeasures& cMeas, properties* props) :
          module(inst, inputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, module::context(), NULL, props)),     isReference(isReference), options(module::context()), measComp(cMeas.getComparators())
{ init(context(), props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs,  
                       bool isReference, 
                       const attrOp& onoffOp, const compNamedMeasures& cMeas, properties* props) : 
          module(inst, inputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, module::context(), &onoffOp, props)), isReference(isReference), options(module::context()), measComp(cMeas.getComparators())
{ init(context(), props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference,
                                                                         properties* props) :
            module(inst, inputs, externalOutputs, setProperties(inst, isReference, module::context(), NULL, props)),           isReference(isReference), options(module::context())
{ init(context(), props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, 
                       const attrOp& onoffOp,                            properties* props) :
          module(inst, inputs, externalOutputs, setProperties(inst, isReference, module::context(), &onoffOp, props)),       isReference(isReference), options(module::context())
{ init(context(), props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, 
                                              const compNamedMeasures& cMeas, properties* props) :
          module(inst, inputs, externalOutputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, module::context(), NULL, props)),     isReference(isReference), options(module::context()), measComp(cMeas.getComparators())
{ init(context(), props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, 
                       const attrOp& onoffOp, const compNamedMeasures& cMeas, properties* props) : 
          module(inst, inputs, externalOutputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, module::context(), &onoffOp, props)), isReference(isReference), options(module::context()), measComp(cMeas.getComparators())
{ init(context(), props); }


compModule::compModule(const instance& inst, const std::vector<port>& inputs,  
                       bool isReference, context options,
                                                                         properties* props) :
            module(inst, inputs, setProperties(inst, isReference, options, NULL, props)),           isReference(isReference), options(options)
{ init(options, props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs,  
                       bool isReference, context options, 
                       const attrOp& onoffOp,                            properties* props) :
          module(inst, inputs, setProperties(inst, isReference, options, &onoffOp, props)),       isReference(isReference), options(options)
{ init(options, props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs,  
                       bool isReference, context options, 
                                              const compNamedMeasures& cMeas, properties* props) :
          module(inst, inputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, options, NULL, props)),     isReference(isReference), options(options), measComp(cMeas.getComparators())
{ init(options, props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs,  
                       bool isReference, context options, 
                       const attrOp& onoffOp, const compNamedMeasures& cMeas, properties* props) : 
          module(inst, inputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, options, &onoffOp, props)), isReference(isReference), options(options), measComp(cMeas.getComparators())
{ init(options, props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, context options,
                                                                         properties* props) :
            module(inst, inputs, externalOutputs, setProperties(inst, isReference, options, NULL, props)),           isReference(isReference), options(options)
{ init(options, props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, context options, 
                       const attrOp& onoffOp,                            properties* props) :
          module(inst, inputs, externalOutputs, setProperties(inst, isReference, options, &onoffOp, props)),       isReference(isReference), options(options)
{ init(options, props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, context options, 
                                              const compNamedMeasures& cMeas, properties* props) :
          module(inst, inputs, externalOutputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, options, NULL, props)),     isReference(isReference), options(options), measComp(cMeas.getComparators())
{ init(options, props); }

compModule::compModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       bool isReference, context options, 
                       const attrOp& onoffOp, const compNamedMeasures& cMeas, properties* props) : 
          module(inst, inputs, externalOutputs, cMeas.getNamedMeasures(), setProperties(inst, isReference, options, &onoffOp, props)), isReference(isReference), options(options), measComp(cMeas.getComparators())
{ init(options, props); }

// Sets the properties of this object
properties* compModule::setProperties(const instance& inst, bool isReference, context options, const attrOp* onoffOp, properties* props) {
  if(props==NULL) {
    props = new properties();
  }
 /* 
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    / * // Initialize pMap to contain the properties of options
    map<string, string> pMap;// = options.getProperties("op");
    //pMap["isReference"]   = txt()<<isReference;

    deriv->props->add("compModule", pMap);* /
    
 }
  else
    deriv->props->active = false;
 */ 
  return props;
}

void compModule::init(const context& options, properties* props) {
  isDerived = props!=NULL; // This is an instance of an object that derives from module if its constructor sets deriv to non-NULL
  
  if(!isDerived) setTraceCtxt();

  // Record any publicized options
  addPublicizedOptions(options, numPublicizedOptions, modularApp::getInstance()->publicizedOptions,
                       modularApp::getInstance()->publicizedOptionNotes);
    
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void compModule::destroy() {
  this->~compModule();
}

compModule::~compModule() {
  assert(!destroyed);
  
  // If this is an instance of compModule rather than a class that derives from compModule
  if(props->active && !isDerived) {    // Wrap the portion of the moduleMarker tag where we place the control information (the traceStream)
    // in its own tag to make them easier to match across different streams
    properties moduleCtrl;
    moduleCtrl.add("moduleCtrl", map<string, string>());
    dbg.enter(moduleCtrl);

    // Clear all records of publicized options
    clearPublicized(numPublicizedOptions, modularApp::getInstance()->publicizedOptions,
                    modularApp::getInstance()->publicizedOptionNotes);

    // Register a traceStream for this compModule's module group, if one has not already been registered
    if(!modularApp::isTraceStreamRegistered(g))
      modularApp::registerTraceStream(g, new compModuleTraceStream(moduleID, this, trace::lines, trace::disjMerge, modularApp::getTraceStreamID(g)));
  }
  
  if(!isDerived) setTraceCtxt();
}

// Sets the traceCtxt map, which contains the context attributes to be used in this module's measurements 
// by combining the context provided by the classes that this object derives from with its own unique 
// context attributes.
void compModule::setTraceCtxt() {
  // Initialize traceCtxt with the information from module
  module::setTraceCtxt();

  traceCtxt["compModule:isReference"] = attrValue((long)isReference);
  
  // Add to it the context of this module based on the options publicized by containing modules, omitting any
  // from this module.
  int j=0;
  for(list<pair<string, attrValue> >::const_iterator i=modularApp::getInstance()->publicizedOptions.begin(); 
         i!=modularApp::getInstance()->publicizedOptions.end() && 
         j<modularApp::getInstance()->publicizedOptions.size()-numPublicizedOptions; 
      i++, j++)
    traceCtxt[encodeCtxtName("compModule", "pub_option", "", i->first)] = i->second;

  // Add all the options to the context
  for(map<std::string, attrValue>::const_iterator o=options.getCfg().begin(); o!=options.getCfg().end(); o++)
    traceCtxt[encodeCtxtName("compModule", "option", "0", o->first)] = o->second;
}

// Sets the isReference flag to indicate whether this is a reference instance of this compModule or not
void compModule::setIsReference(bool newVal) {
  isReference = newVal;
}

// Sets the context of the given option
void compModule::setOptionCtxt(std::string name, attrValue val) {
  options.configuration[name] = val;
}
   

// Sets the context of the given output port. This variant ensures that the outputs of compModules can only
// be set with compContexts.
void compModule::setOutCtxt(int idx, const compContext& c) {
  if(!props->active) return;
  module::setOutCtxt(idx, (const context&)c);
}

// -------------------------
// ----- Configuration -----
// -------------------------

// We can configure the values that modifiable options take within each module
// module name -> modifiable option name -> option value
std::map<std::string, std::map<std::string, attrValue> > compModule::modOptValue;

// Checks whether a modifiable option with the given name was specified for this module. 
// This option may be set in a configuration file and then applications may use this 
// function to query for its value and act accordingly
bool compModule::existsModOption(std::string name) {
  return (modOptValue.find(g.name()) != modOptValue.end()) &&
         (modOptValue[g.name()].find(name) != modOptValue[g.name()].end());
}

// Returns the value of the given modifiable option of this module.
attrValue compModule::getModOption(std::string name) {
  if(modOptValue.find(g.name()) == modOptValue.end())
  { cerr << "ERROR: no modifiable options specified for module "<<g.name()<<"!"<<endl; assert(0); }
  
  if(modOptValue[g.name()].find(name) == modOptValue[g.name()].end())
  { cerr << "ERROR: no value specified for modifiable option "<<name<<" in module "<<g.name()<<"!"<<endl; assert(0); }
  
  return modOptValue[g.name()][name];
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

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void springModularApp::destroy() {
  this->~springModularApp();
}

springModularApp::~springModularApp() {
  assert(!destroyed);
  
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
                                                                         properties* props) :
          compModule(inst, inputs, externalOutputs, isSpringReference(), extendOptions(options), compNamedMeasures(), props)
{ 
/*springModularApp* app = springModularApp::getInstance();
if(app->bufSize>0) { cout << "bufSize="<<app->bufSize<<" sleep("<<(log(app->bufSize)/log(10))<<")"<<endl; sleep(log(app->bufSize)/log(10)); }*/
}

springModule::springModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
                       const context& options,
                       const attrOp& onoffOp,                            properties* props) :
          compModule(inst, inputs, externalOutputs, isSpringReference(), extendOptions(options), onoffOp, compNamedMeasures(), props)
{ 
/*springModularApp* app = springModularApp::getInstance();
if(app->bufSize>0) { cout << "bufSize="<<app->bufSize<<" sleep("<<(log(app->bufSize)/log(10))<<")"<<endl; sleep(log(app->bufSize)/log(10)); }*/
}

springModule::springModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
                       const context& options,
                                              const compNamedMeasures& cMeas, properties* props) :
          compModule(inst, inputs, externalOutputs, isSpringReference(), extendOptions(options), cMeas, props)
{ 
/*springModularApp* app = springModularApp::getInstance();
if(app->bufSize>0) { cout << "bufSize="<<app->bufSize<<" sleep("<<(log(app->bufSize)/log(10))<<")"<<endl; sleep(log(app->bufSize)/log(10)); }*/
}

springModule::springModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
                       const context& options,
                       const attrOp& onoffOp, const compNamedMeasures& cMeas, properties* props) :
          compModule(inst, inputs, externalOutputs, isSpringReference(), extendOptions(options), cMeas, props)
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

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void springModule::destroy() {
  this->~springModule();
}

springModule::~springModule() {
  assert(!destroyed);
}

// Returns the context attributes to be used in this module's measurements by combining the context provided by the classes
// that this object derives from with its own unique context attributes.
/*std::map<std::string, attrValue> springModule::getTraceCtxt(const std::vector<port>& inputs, bool isReference, const context& options)
{ return compModule::getTraceCtxt(ins, isReference, options); }*/

/***************************
 ***** processedModule *****
 ***************************/

processedModule::processedModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       const processedTrace::commands& processorCommands,
                                                                         properties* props) :
          module(inst, inputs, externalOutputs, props),           processorCommands(processorCommands)
{ init(props); }

processedModule::processedModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       const processedTrace::commands& processorCommands, 
                       const attrOp& onoffOp,                            properties* props) :
          module(inst, inputs, externalOutputs, props),       processorCommands(processorCommands)
{ init(props); }

processedModule::processedModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       const processedTrace::commands& processorCommands, 
                                              const namedMeasures& meas, properties* props) :
          module(inst, inputs, externalOutputs, meas, props),     processorCommands(processorCommands)
{ init(props); }

processedModule::processedModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs, 
                       const processedTrace::commands& processorCommands, 
                       const attrOp& onoffOp, const namedMeasures& meas, properties* props) : 
          module(inst, inputs, externalOutputs, meas, props), processorCommands(processorCommands)
{ init(props); }

/* // Sets the properties of this object
module::derivInfo* processedModule::setProperties(const instance& inst, const processedTrace::commands& processorCommands, const attrOp* onoffOp, properties* props) {
/ *  if(deriv==NULL) {
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
  * /
  return deriv;
}*/ 

void processedModule::init(properties* props) {
  isDerived = props!=NULL; // This is an instance of an object that derives from module if its constructor sets deriv to non-NULL
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void processedModule::destroy() {
  this->~processedModule();
}

processedModule::~processedModule() {
  assert(!destroyed);
  
  // If this is an instance of processedModule rather than a class that derives from processedModule
  if(props->active) {
    // Register a traceStream for this processedModule's module group, if one has not already been registered
    if(!modularApp::isTraceStreamRegistered(g))
      modularApp::registerTraceStream(g, new processedModuleTraceStream(moduleID, this, trace::lines, trace::disjMerge, modularApp::getTraceStreamID(g)));
  }
}

// Returns the context attributes to be used in this module's measurements by combining the context provided by the classes
// that this object derives from with its own unique context attributes.
/*std::map<std::string, attrValue> processedModule::getTraceCtxt(const std::vector<port>& inputs)
{ return module::getTraceCtxt(inputs); }*/

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

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void moduleTraceStream::destroy() {
  this->~moduleTraceStream();
}

moduleTraceStream::~moduleTraceStream() {
  assert(!destroyed);
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
    
    // Record the number of inputs and outputs of this compModule. This repeats the same attributes in moduleTS but 
    // it is easier let both moduleTS and compModuleTS to have their own than to allow functions of compModuleTS
    // access to properties of moduleTS.
    pMap["numInputs"]  = txt()<<cm->numInputs(); 
    pMap["numOutputs"] = txt()<<cm->numOutputs(); 
    
    //cout << "compModuleTraceStream::setProperties() #ins="<<cm->ins.size()<<endl;
    // Add the comparators to be used for each input attribute, where they are provided
    for(int inIdx=0; inIdx<cm->ins.size(); inIdx++) {
      // If the context specified for this input is a compContext
      compContext* ctxt = dynamic_cast<compContext*>(cm->ins[inIdx].ctxt);
      // Skip inputs for which a compContext was not provided
      //cout << "ctxt="<<ctxt<<endl;
      if(!ctxt) {
        pMap[txt()<<"in"<<inIdx<<":numAttrs"] = "0";
        continue;
      }
        
      pMap[txt()<<"in"<<inIdx<<":numAttrs"] = txt()<<ctxt->comparators.size();
      int compIdx=0;
      for(map<string, pair<string, string> >::const_iterator comp=ctxt->comparators.begin(); comp!=ctxt->comparators.end(); comp++, compIdx++)
        pMap[txt()<<"in"<<inIdx<<":compare"<<compIdx] = 
                  txt()<<escapedStr(comp->first,         ":", escapedStr::unescaped).escape()<<":"<<
                         escapedStr(comp->second.first,  ":", escapedStr::unescaped).escape()<<":"<<
                         escapedStr(comp->second.second, ":", escapedStr::unescaped).escape();
    }
    
    // Add the comparators to be used for each output attribute
    for(int outIdx=0; outIdx<cm->outs.size(); outIdx++) {
      //if(o->ctxt==NULL) { cerr << "ERROR in module "<<moduleID<<"! Context of output "<<outIdx<<" not provided!"<<endl; assert(0); }
      // Skip outputs for which contexts were not provided
      if(cm->outs[outIdx].ctxt==NULL) {
        pMap[txt()<<"out"<<outIdx<<":numAttrs"] = "0";
        continue;
      }
      
      compContext* ctxt = dynamic_cast<compContext*>(cm->outs[outIdx].ctxt);
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

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void compModuleTraceStream::destroy() {
  this->~compModuleTraceStream();
}

compModuleTraceStream::~compModuleTraceStream() {
  assert(!destroyed);
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

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void processedModuleTraceStream::destroy() {
  this->~processedModuleTraceStream();
}

processedModuleTraceStream::~processedModuleTraceStream() {
  assert(!destroyed);
}


/******************************************
 ***** ModuleMergeHandlerInstantiator *****
 ******************************************/

ModuleMergeHandlerInstantiator::ModuleMergeHandlerInstantiator() { 
  (*MergeHandlers   )["modularApp"]          = ModularAppMerger::create;
  (*MergeKeyHandlers)["modularApp"]          = ModularAppMerger::mergeKey;
  (*MergeHandlers   )["modularAppBody"]      = ModularAppStructureMerger::create;
  (*MergeKeyHandlers)["modularAppBody"]      = ModularAppStructureMerger::mergeKey;
  (*MergeHandlers   )["modularAppStructure"] = ModularAppStructureMerger::create;
  (*MergeKeyHandlers)["modularAppStructure"] = ModularAppStructureMerger::mergeKey;
  (*MergeHandlers   )["module"]              = ModuleMerger::create;
  (*MergeKeyHandlers)["module"]              = ModuleMerger::mergeKey;    
  (*MergeHandlers   )["moduleMarker"]        = ModuleMerger::create;
  (*MergeKeyHandlers)["moduleMarker"]        = ModuleMerger::mergeKey;
  (*MergeHandlers   )["moduleCtrl"]          = ModuleCtrlMerger::create;
  (*MergeKeyHandlers)["moduleCtrl"]          = ModuleCtrlMerger::mergeKey;
  (*MergeHandlers   )["moduleEdge"]          = ModuleEdgeMerger::create;
  (*MergeKeyHandlers)["moduleEdge"]          = ModuleEdgeMerger::mergeKey;
  (*MergeHandlers   )["moduleTS"]            = ModuleTraceStreamMerger::create;
  (*MergeKeyHandlers)["moduleTS"]            = ModuleTraceStreamMerger::mergeKey;
  
  (*MergeHandlers   )["compModule"]          = CompModuleMerger::create;
  (*MergeKeyHandlers)["compModule"]          = CompModuleMerger::mergeKey;    
  (*MergeHandlers   )["compModuleTS"]        = CompModuleTraceStreamMerger::create;
  (*MergeKeyHandlers)["compModuleTS"]        = CompModuleTraceStreamMerger::mergeKey;
    
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
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  // We do not call the mergeKey method of the BlockMerger because we wish to keep the API of modularApps simple:
  // If there are two modular apps with the same name, they should be merged even if their call stacks are different.
  // This is because for the most part users define a single modularApp variable in their application and details
  // of call stack, etc. should not make a difference.
  //BlockMerger::mergeKey(type, tag.next(), inStreamRecords, info);
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; assert(0); }
  if(type==properties::enterTag) {
    info.add(tag.get("appName"));
  }
}

/*************************************
 ***** ModularAppStructureMerger *****
 *************************************/

ModularAppStructureMerger::ModularAppStructureMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        Merger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* ModularAppStructureMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags);
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging modularAppStructure!"<<endl; exit(-1); }
  if(type==properties::enterTag || type==properties::exitTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "modularAppBody" || 
           *names.begin() == "modularAppStructure");
    
    props->add(*names.begin(), pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void ModularAppStructureMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; assert(0); }
  if(type==properties::enterTag) {
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
//      pMap["traceID"] = txt()<<streamRecord::mergeIDs("traceStream", "traceID", pMap, tags, outStreamRecords, inStreamRecords);
      
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
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // We only merge module tags that correspond to the same module in the outgoing stream. This ID is assigned 
    // to nodes while we're processing their moduleNodeTraceStreams because the module tags are emitted after
    // the moduleTS tags.
    /*assert(((ModuleStreamRecord*)inStreamRecords["module"])->mStack.size()>0);
    assert(((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back());*/

    //if(tag.name() == "moduleMarker") {
      info.add(tag.get("name"));
      info.add(tag.get("numInputs"));
      info.add(tag.get("numOutputs"));
    /*} else if(tag.name() == "module") {
      streamID inSID(properties::getInt(tag, "moduleID"), inStreamRecords["module"]->getVariantID());
      //streamID outSID = ((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back()->in2outID(inSID);
      map<string, string> pMap;
      streamID outSID = inStreamRecords["module"]->in2outID(inSID);
      info.add(txt()<<outSID.ID);
    }*/
  }
}

/*****************************
 ***** ModuleCtrlMerger *****
 ****************************/

ModuleCtrlMerger::ModuleCtrlMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        Merger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* ModuleCtrlMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Module Control tags!"<<endl; exit(-1); }
  if(type==properties::enterTag || type==properties::exitTag) {
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "moduleCtrl");
  }
  props->add("moduleCtrl", pMap);
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void ModuleCtrlMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge control attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    info.setUniversal(true);
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
                                std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Edges between nodes that were merged to different nodeIDs in the outgoing streams are separated
    /*assert(((ModuleStreamRecord*)inStreamRecords["module"])->mStack.size()>0);
    assert(((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back());*/
    
    streamID inFromSID(tag.getInt("fromCID"), inStreamRecords["module"]->getVariantID());
    //streamID outFromSID = ((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back()->in2outID(inFromSID);
    streamID outFromSID = inStreamRecords["module"]->in2outID(inFromSID);
    info.add(txt()<<outFromSID.ID);
    
    streamID inToSID(tag.getInt("toCID"), inStreamRecords["module"]->getVariantID());
    //streamID outToSID = ((ModuleStreamRecord*)inStreamRecords["module"])->mStack.back()->in2outID(inToSID);
    streamID outToSID = inStreamRecords["module"]->in2outID(inToSID);
    info.add(txt()<<outToSID.ID);
    
    // Edges between different port numbers/types are separated
    info.add(tag.get("fromT"));
    info.add(tag.get("fromP"));
    info.add(tag.get("toT"));
    info.add(tag.get("toP"));
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
    int moduleID;
    
    // The tags of the traceStream objects along the incoming streams, which are the parent objects of
    // the moduleTraceStreams considered in this function.
    std::vector<std::pair<properties::tagType, properties::iterator> > TraceStreamTags = advance(tags);
    
    // If we've previously assigned a streamID in the outgoing stream to this module group
    if(((ModuleStreamRecord*)(outStreamRecords)["module"])->isModuleObserved(g)) {
      // This means that a moduleTS tag for this module group has already been emitted and thus, we will
      // not be emitting another one.
      // Set the dontEmit field in pMap to communicate this fact to the ModuleTraceStreamMerger constructor
      pMap["dontEmit"] = "1";
      
      moduleID = ((ModuleStreamRecord*)(outStreamRecords)["module"])->getModuleID(g);
      
      // We get into this case where the merger cannot align different instances of ModuleTraceStream,
      //   causing one instance to appear on one incoming stream before the same ModuleTraceStream appear
      //   on another incoming stream. Since we've already chosen the traceID for this module group,
      //   simply retrieve it and and use mergeIDs() to force all the newly-observed ModuleTraceStreams
      //   to use it.
      // When TraceStreamMerger calls mergeIDs() in TraceStreamMerger::TraceStreamMerger() call right 
      //   after this function ends it will get this traceID.
      int traceID = ((ModuleStreamRecord*)(outStreamRecords)["module"])->getModuleTraceID(g);
      streamRecord::mergeIDs("traceStream", "traceID", pMap, TraceStreamTags, outStreamRecords, inStreamRecords, traceID);
    // If we've never previously observed this module group
    } else {
      // Generate a fresh ID for all the modules along the incoming streams
      moduleID = ((ModuleStreamRecord*)(outStreamRecords)["module"])->genModuleID();
      
      // Make sure that the traceIDs of the traceStreams associated with the aligned modules along the 
      //   incoming streams are mapped to the same traceID along the outgoing stream. 
      // When TraceStreamMerger calls mergeIDs() in TraceStreamMerger::TraceStreamMerger() call right 
      //   after this function ends it will get this traceID.
      int traceID = streamRecord::mergeIDs("traceStream", "traceID", pMap, TraceStreamTags, outStreamRecords, inStreamRecords);
      
      // Associate the new moduleID and traceID with the module group
      ((ModuleStreamRecord*)(outStreamRecords)["module"])->setModuleID(g, moduleID, traceID);
      
      pMap["moduleID"] = txt() << moduleID;
    }
    
    // Record the mapping from the moduleIDs of each incoming stream to this moduleID
    int mergedID = streamRecord::mergeIDs("module", "moduleID", pMap, tags, outStreamRecords, inStreamRecords, moduleID);
    assert(mergedID == moduleID);
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
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
   
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // We only merge moduleNodeTraceStreams that have the same name and numbers of inputs and outputs
    // The same rule is used for their associated module but since those tags appear later in the stream
    // they're split according to the ID in the outgoing stream that was assigned to them while processing
    // their moduleNodeTraceStreams.
    info.add(tag.get("name"));
    info.add(tag.get("numInputs"));
    info.add(tag.get("numOutputs"));
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
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  ModuleMerger::mergeKey(type, tag.next(), inStreamRecords, info);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    info.add(tag.get("compModule:isReference"));
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
    assert(getSameName(tags) == "compModuleTS");

    //int nodeID = ModuleStreamRecord::mergeNodeIDs("nodeID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // We know that all the tags have the same values for isReference, options, output comparators and measurements.
    // Thus, we only consider this information from the first tag.
    
    /*common::module::context options = common::module::context(tags[0].second, "op");
    pMap = options.getProperties("op");*/
    
    //pMap["isReference"] = tags[0].second.get("isReference");

    // All the tags have the same values for input comparators, so we only consider this information from the first tag.
    pMap["numInputs"] = tags[0].second.get("numInputs");
    long numInputs = attrValue::parseInt(pMap["numInputs"]);
    for(int inIdx=0; inIdx<numInputs; inIdx++) {
      pMap[txt()<<"in"<<inIdx<<":numAttrs"] = tags[0].second.get(txt()<<"in"<<inIdx<<":numAttrs");
      int numAttrs = attrValue::parseInt(pMap[txt()<<"in"<<inIdx<<":numAttrs"]);
      for(int compIdx=0; compIdx<numAttrs; compIdx++) {
        pMap[txt()<<"in"<<inIdx<<":compare"<<compIdx] = tags[0].second.get(txt()<<"in"<<inIdx<<":compare"<<compIdx);
      }
    }
    
    /*pMap["numOutputs"] = tags[0].second.get("numOutputs");
    int numOutputs = attrValue::parseInt(pMap["numOutputs"]);
    for(int outIdx=0; outIdx<numOutputs; outIdx++) {
      pMap[txt()<<"out"<<outIdx<<":numAttrs"] = tags[0].second.get(txt()<<"out"<<outIdx<<":numAttrs");
      int numAttrs = attrValue::parseInt(pMap[txt()<<"out"<<outIdx<<":numAttrs"]);
      for(int compIdx=0; compIdx<numAttrs; compIdx++) {
        pMap[txt()<<"out"<<outIdx<<":compare"<<compIdx] = tags[0].second.get(txt()<<"out"<<outIdx<<":compare"<<compIdx);
      }
    }*/

    // Collect the union of all the comparators associated with each output. Each attribute of each output
    // does not need to have a comparator specified for it on all incoming streams. However, if it is specified
    // it must have the same value on all the incoming streams.
    pMap["numOutputs"] = getSameValue(tags, "numOutputs");
    long numOutputs = attrValue::parseInt(pMap["numOutputs"]);
    //vector<int> maxNumAttrs(numOutputs, 0); // The maximum number of attributes observed for any output

    for(int outIdx=0; outIdx<numOutputs; outIdx++) {
      // Maps the name of each output attribute of the current output to the descriptor of its comparator
      map<string, pair<string, string> > comparators;
      
      for(vector<pair<properties::tagType, properties::iterator> >::iterator t=tags.begin(); t!=tags.end(); t++) {
        // Iterate over all the attributes that were specified for the current output along the given incoming stream
        long numAttrs = t->second.getInt(txt()<<"out"<<outIdx<<":numAttrs");
        for(int compIdx=0; compIdx<numAttrs; compIdx++) {
           vector<string> fields = escapedStr(t->second.get(txt()<<"out"<<outIdx<<":compare"<<compIdx), ":", escapedStr::escaped).unescapeSplit(":");
           assert(fields.size()==3);
           // The name of the attribute
           string attrName = fields[0];
           // The name of the comparator to be used for this attribute
           string compName = fields[1];
           // The description of the comparator
           string compDesc = fields[2];

           // Record the comparator to be used for the current attribute of the current output

           // If we haven't yet recorded the comparator for this attribute, do so now
           if(comparators.find(attrName) == comparators.end())
             comparators[attrName] = make_pair(compName, compDesc);
           // Otherwise, ensure that all the comparators specified for this attribute/output are identical
           else {
             if(comparators[attrName] != make_pair(compName, compDesc)) { cerr << "ERROR: mismatched comparators for attribute "<<attrName<<"! Before observed "<<comparators[attrName].first<<" / "<<comparators[attrName].second<<" but now observing "<<compName<<" / "<<compDesc<<endl; }
             assert(comparators[attrName] == make_pair(compName, compDesc));
           }

/*          string key = txt()<<"out"<<outIdx<<":compare"<<compIdx;
cout << "outIdx="<<outIdx<<", compIdx="<<compIdx<<". key="<<key<<": "<<t->second.get(txt()<<"out"<<outIdx<<":compare"<<compIdx)<<endl;
          // If we haven't yet recorded the comparator for this attribute of this output, do so now
          if(pMap.find(key) == pMap.end()) pMap[key] = t->second.get(txt()<<"out"<<outIdx<<":compare"<<compIdx);
          // Ensure that all the comparators specified for this attribute/output are identical
          else                      assert(pMap[key] == t->second.get(txt()<<"out"<<outIdx<<":compare"<<compIdx));*/
        }
        
        // Update the record of the maximum number of attributes
        //if(maxNumAttrs[outIdx] < numAttrs) maxNumAttrs[outIdx] = numAttrs;
      }

      // We've now collected the comparators for all the attributes of this output. Now add them to pMap[]
      pMap[txt()<<"out"<<outIdx<<":numAttrs"] = txt()<<comparators.size();
      int compIdx=0;
      for(map<string, pair<string, string> >::iterator comp=comparators.begin(); comp!=comparators.end(); comp++, compIdx++) {
        pMap[txt()<<"out"<<outIdx<<":compare"<<compIdx] = txt() <<
                           escapedStr(comp->first,         ":", escapedStr::unescaped).escape()<<":"<<
                           escapedStr(comp->second.first,  ":", escapedStr::unescaped).escape()<<":"<<
                           escapedStr(comp->second.second, ":", escapedStr::unescaped).escape();
      }
    }
    
    // Record the number of attributes for each output
    /*for(int outIdx=0; outIdx<maxNumAttrs.size(); outIdx++)
      pMap[txt()<<"out"<<outIdx<<":numAttrs"] = txt()<<maxNumAttrs[outIdx];*/
    
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
                                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
   
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // We only merge moduleNodeTraceStreams that have the same values for isReference, (they either are both 
    // examples of the reference configuration or they're bith not), same configuration, same output comparators and 
    // same measurements. 
    // The same rule is used for their associated module but since those tags appear later in the stream
    // they're split according to the ID in the outgoing stream that was assigned to them while processing
    // their moduleNodeTraceStreams.
    //info.add(tag.get("isReference"));
    
    // Same options
    /*common::module::context options(tag, "op");
    info.add(txt()<<options.configuration.size());
    int cIdx=0;
    for(map<string, attrValue>::iterator c=options.configuration.begin(); c!=options.configuration.end(); c++, cIdx++) {
      info.add(c->first);
      info.add(c->second.serialize());
    }*/
    
    // Same number of inputs
    info.add(tag.get("numInputs"));
    // Same comparators on all inputs. Inputs are not required to have comparators specified for them, so
    // if a given input has no comparators specified we merely make sure that this holds along all incoming streams.
    int numInputs = tag.getInt("numInputs");
    for(int inIdx=0; inIdx<numInputs; inIdx++) {
      info.add(tag.get(txt()<<"in"<<inIdx<<":numAttrs"));
      int numAttrs = tag.getInt(txt()<<"in"<<inIdx<<":numAttrs");
      for(int compIdx=0; compIdx<numAttrs; compIdx++) {
        info.add(tag.get(txt()<<"in"<<inIdx<<":compare"<<compIdx));
      }
    }
    
    // Same number of outputs
    info.add(tag.get("numOutputs"));
    /*
    // Same comparators on outputs
    int numOutputs = tag.getInt("numOutputs");
    for(int outIdx=0; outIdx<numOutputs; outIdx++) {
      info.add(tag.get(txt()<<"out"<<outIdx<<":numAttrs"));
      int numAttrs = tag.getInt(txt()<<"out"<<outIdx<<":numAttrs");
      for(int compIdx=0; compIdx<numAttrs; compIdx++) {
        info.add(tag.get(txt()<<"out"<<outIdx<<":compare"<<compIdx));
      }
    }*/
    
    // Same measurements
    info.add(tag.get("numMeasurements"));
    // Concatenate the descriptions of the measurements
    int numMeasurements = tag.getInt("numMeasurements");
    ostringstream measurements;
    for(int m=0; m<numMeasurements; m++)
      info.add(tag.get(txt()<<"measure"<<m));
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
  streamRecord(that, vSuffixID), iStack(that.iStack), observedModules(that.observedModules), observedModulesTS(that.observedModulesTS), maxModuleID(that.maxModuleID)
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
  observedModulesTS.clear();
  
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    ModuleStreamRecord* ms = (ModuleStreamRecord*)(*s)["module"];

    /*cout << "| | ModuleStreamRecord::resumeFrom observedModules ms (#"<<ms->observedModules.size()<<")="<<endl;
    for(map<ModuleStreamRecord::moduleInfo, int>::iterator i=ms->observedModules.begin(); i!=ms->observedModules.end(); i++)
      cout << "|       "<<i->first.str()<<" : "<<i->second<<endl;*/
    
    for(map<group, int>::const_iterator i=ms->observedModules.begin(); i!=ms->observedModules.end(); i++)
      observedModules.insert(*i);
    
    for(map<group, int>::const_iterator i=ms->observedModulesTS.begin(); i!=ms->observedModulesTS.end(); i++)
      observedModulesTS.insert(*i);
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
