// Licence information included in file LICENCE
#define MODULE_STRUCTURE_C
#include "../sight_structure.h"
#include "module_structure.h"
#include "../sight_common.h"
#include "module_common.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>


using namespace std;
using namespace sight::common;
  
namespace sight {
namespace structure {

/******************
 ***** module *****
 ******************/

// Records all the known contexts, mapping each context to its unique ID
std::map<module::group, int> module::group2ID;

// Maps each context to the number of times it was ever observed
std::map<module::group, int> module::group2Count;

// The trace that records performance observations of different modules and contexts
//trace* module::tr;
std::map<module::group, traceStream*> module::moduleTrace;

// Maps each module to the list of the names of its input and output context attributes. 
// This enables us to verify that all the modules are used consistently.
std::map<group, std::vector<std::list<std::string> > > module::moduleInCtxtNames;
std::map<group, std::vector<std::list<std::string> > > module::moduleOutCtxtNames;

// Records all the edges ever observed, mapping them to the number of times each edge was observed
std::map<std::pair<common::module::port, common::module::port>, int> module::edges;

int module::maxModuleID=0;

// Stack of the module graphs that are currently in scope
std::list<module*> module::mStack;

module::module(const group& g,                                                     properties* props) : 
  block(g.name, setProperties(g, inputs(),   NULL, props)), g(g), externalOutputs(NULL)
{ init(g, inputs()); }

module::module(const group& g, const port& in,                                     properties* props): 
  block(g.name, setProperties(g, inputs(in), NULL, props)), g(g), externalOutputs(NULL)
{ init(g, inputs(in)); }

module::module(const group& g, const std::vector<port>& in,                        properties* props) :
  block(g.name, setProperties(g, in,         NULL, props)), g(g), externalOutputs(NULL)
{ init(g, in); }

module::module(const group& g,                              const attrOp& onoffOp, properties* props) : 
  block(g.name, setProperties(g, inputs(),   &onoffOp, props)), g(g), externalOutputs(NULL)
{ init(g, inputs()); }

module::module(const group& g, const port& in,              const attrOp& onoffOp, properties* props): 
  block(g.name, setProperties(g, inputs(in), &onoffOp, props)), g(g), externalOutputs(NULL)
{ init(g, inputs(in)); }

module::module(const group& g, const std::vector<port>& in, const attrOp& onoffOp, properties* props) :
  block(g.name, setProperties(g, in,         &onoffOp, props)), g(g), externalOutputs(NULL)
{ init(g, in); }


module::module(const group& g,                              std::vector<port>& externalOutputs,                        properties* props) : 
  block(g.name, setProperties(g, inputs(),   NULL, props)), g(g), externalOutputs(&externalOutputs)
{ init(g, inputs()); }

module::module(const group& g, const port& in,              std::vector<port>& externalOutputs,                        properties* props): 
  block(g.name, setProperties(g, inputs(in), NULL, props)), g(g), externalOutputs(&externalOutputs)
{ init(g, inputs(in)); }

module::module(const group& g, const std::vector<port>& in, std::vector<port>& externalOutputs,                        properties* props) :
  block(g.name, setProperties(g, in,         NULL, props)), g(g), externalOutputs(&externalOutputs)
{ init(g, in); }

module::module(const group& g,                              std::vector<port>& externalOutputs, const attrOp& onoffOp, properties* props) : 
  block(g.name, setProperties(g, inputs(),   &onoffOp, props)), g(g), externalOutputs(&externalOutputs)
{ init(g, inputs()); }

module::module(const group& g, const port& in,              std::vector<port>& externalOutputs, const attrOp& onoffOp, properties* props): 
  block(g.name, setProperties(g, inputs(in), &onoffOp, props)), g(g), externalOutputs(&externalOutputs)
{ init(g, inputs(in)); }

module::module(const group& g, const std::vector<port>& in, std::vector<port>& externalOutputs, const attrOp& onoffOp, properties* props) :
  block(g.name, setProperties(g, in,         &onoffOp, props)), g(g), externalOutputs(&externalOutputs)
{ init(g, in); }



void module::init(const group& g, const std::vector<port>& in) {
  if(this->props->active) {
    mStack.push_back(this);
    
  	//cout << "module c="<<c.str()<<endl;
    addNode(g, maxModuleID);
    maxModuleID++;
    
    // Compute the context of this module based on the contexts of its inputs
    map<string, attrValue> traceCtxt; // The context in a format that traces can understand
    { int idx=0;
    for(vector<port>::const_iterator i=in.begin(); i!=in.end(); i++, idx++) {
      ctxt.push_back(i->ctxt);
      for(std::map<std::string, attrValue>::const_iterator c=i->ctxt.configuration.begin(); c!=i->ctxt.configuration.end(); c++)
        traceCtxt[txt()<<idx<<":"<<c->first] = c->second;
    } }
    
    // Set moduleInCtxtNames based on the context of the inputs or if it already set, verify that
    // the input attributes have the same names as they had last time.
    {
      // If moduleInCtxtNames has not yet been initialized, set it to contain the names of the context attributes in inputs
      if(moduleInCtxtNames.find(g) == moduleInCtxtNames.end()) {
        moduleInCtxtNames[g] = vector<list<string> >();
        int idx=0;
        for(vector<port>::const_iterator i=in.begin(); i!=in.end(); i++, idx++) {
          moduleInCtxtNames[g].push_back(list<string>());
          for(std::map<std::string, attrValue>::const_iterator c=i->ctxt.configuration.begin(); c!=i->ctxt.configuration.end(); c++)
            moduleInCtxtNames[g][idx].push_back(c->first);
        }
      // If moduleInCtxtNames has already been initialized, verify inputs against it
      } else {
        if(in.size() != moduleInCtxtNames[g].size()) { cerr << "ERROR: Inconsistent inputs for module "<<g.name<<"! Prior instances has "<<moduleInCtxtNames[g].size()<<" inputs while this instance has "<<in.size()<<" inputs!"<<endl; assert(0); }
        for(int i=0; i<in.size(); i++) {
          if(in[i].ctxt.configuration.size() != moduleInCtxtNames[g][i].size()) { cerr << "ERROR: Inconsistent number of context attributes for input "<<i<<" of module "<<g.name<<"! Prior instances has "<<moduleInCtxtNames[g][i].size()<<" inputs while this instance has "<<in[i].ctxt.configuration.size()<<" inputs!"<<endl; assert(0); }
          std::map<std::string, attrValue>::const_iterator c=in[i].ctxt.configuration.begin();
          list<std::string>::const_iterator m=moduleInCtxtNames[g][i].begin();
          int idx=0;
          for(; c!=in[i].ctxt.configuration.end(); c++, m++, idx++) {
            if(c->first != *m) { cerr << "ERROR: Inconsistent names for context attribute "<<idx<<" of input "<<i<<" of module "<<g.name<<"! Prior instances has "<<*m<<" while this instance has "<<c->first<<"!"<<endl; assert(0); }
          }
        }
      }
    }
    
    // Add edges between the modules from this the inputs came and this module
    { int idx=0;
    for(vector<port>::const_iterator i=in.begin(); i!=in.end(); i++, idx++) {
    	addEdge(*i, port(g, context(), input, idx));
    } }

    // Initialize the output ports of this module
    for(int i=0; i<g.numOutputs; i++) {
      outputs.push_back(port(g, context(), output, i));
      // If the user provided an output vector, initialize it as well
      if(externalOutputs) externalOutputs->push_back(port(g, context(), output, i));
    }
    
    
    // Begin measuring this module instance
    assert(moduleTrace.find(g) != moduleTrace.end());
    moduleMeasure = startMeasure(moduleTrace[g], "measure", traceCtxt);
    
  } else {
    moduleMeasure = NULL;
  }
}

// Sets the properties of this object
properties* module::setProperties(const group& g, const std::vector<port>& inputs, const attrOp* onoffOp, properties* props)  {
  if(props==NULL) props = new properties();
  
  if(g.numInputs!=inputs.size()) cerr << "ERROR: module group \""<<g.name<<"\" has "<<g.numInputs<<" inputs but "<<inputs.size()<<" inputs were provided!"<<endl; 
  assert(g.numInputs==inputs.size());
  
  // We only emit tags at the very end of the outer-most module
  props->emitTag = (props->active && mStack.size()==0);
  
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or it evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
  	props->active = true;
    
    if(props->emitTag) {
      /*map<string, string> pMap = c->first.getProperties();
      //newProps["callPath"] = cp2str(CPRuntime.doStackwalk());*/
      map<string, string> pMap;
      pMap["moduleID"]   = txt()<<maxModuleID;
      props->add("module", pMap);
    }
  } else
  	props->active = false;
  
  return props;  
}

module::~module()
{ 
  //cout << "~module() props->active="<<props->active<<endl;
  
  // Complete the measurement of application's behavior during the module's lifetime
  if(props->active) {
    // Complete measuring this instance
    assert(moduleMeasure);
    endMeasure(moduleMeasure);
    
    // Set moduleInCtxtNames based on the context of the outputs or if it already set, verify that
    // the outputs attributes either have the same names as they had last time
    /*{
      // If moduleInCtxtNames has not yet been initialized, set it to contain the names of the context attributes in inputs
      if(moduleInCtxtNames.find(g) == moduleInCtxtNames.end()) {
        moduleInCtxtNames[g] = vector<list<string> >();
        int idx=0;
        for(vector<port>::const_iterator i=in.begin(); i!=in.end(); i++, idx++) {
          moduleInCtxtNames[g].push_back(list<string>());
          for(std::map<std::string, attrValue>::const_iterator c=i->ctxt.configuration.begin(); c!=i->ctxt.configuration.end(); c++)
            moduleInCtxtNames[g][idx].push_back(c->first);
        }
      // If moduleInCtxtNames has already been initialized, verify inputs against it
      } else {
        if(in.size() != moduleInCtxtNames[g].size()) { cerr << "ERROR: Inconsistent inputs for module "<<g.name<<"! Prior instances has "<<moduleInCtxtNames[g].size()<<" inputs while this instance has "<<in.size()<<" inputs!"<<endl; assert(0); }
        for(int i=0; i<in.size(); i++) {
          if(in[i].ctxt.configuration.size() != moduleInCtxtNames[g][i].size()) { cerr << "ERROR: Inconsistent number of context attributes for input "<<i<<" of module "<<g.name<<"! Prior instances has "<<moduleInCtxtNames[g][i].size()<<" inputs while this instance has "<<in[i].ctxt.configuration.size()<<" inputs!"<<endl; assert(0); }
          std::map<std::string, attrValue>::const_iterator c=in[i].ctxt.configuration.begin();
          list<std::string>::const_iterator m=moduleInCtxtNames[g][i].begin();
          int idx=0;
          for(; c!=in[i].ctxt.configuration.end(); c++, m++, idx++) {
            if(c->first != *m) { cerr << "ERROR: Inconsistent names for context attribute "<<idx<<" of input "<<i<<" of module "<<g.name<<"! Prior instances has "<<*m<<" while this instance has "<<c->first<<"!"<<endl; assert(0); }
          }
        }
      }
    }*/
  }
  
  if(props->active && props->emitTag) {
    /*cout << "group2ID="<<endl;
    for(std::map<context, int>::iterator c=group2ID.begin(); c!=group2ID.end(); c++)
      cout << "    "<<c->first.UID()<<" ==> "<<c->second<<endl;
    */
    for(map<group, traceStream*>::iterator m=moduleTrace.begin(); m!=moduleTrace.end(); m++) {
      delete m->second;
    }
    
    // Emit the information on all the known module groups
    for(std::map<group, int>::iterator i=group2ID.begin(); i!=group2ID.end(); i++) {
      properties nodeP;
      map<string, string> pMap;// = c->first.getProperties();
      pMap["name"]       = i->first.name;
      pMap["numInputs"]  = txt()<<i->first.numInputs;
      pMap["numOutputs"] = txt()<<i->first.numOutputs;
      pMap["ID"]         = txt()<<i->second;
      
      assert(group2Count.find(i->first) != group2Count.end());
      pMap["count"] = txt()<<group2Count[i->first];
      
      nodeP.add("moduleNode", pMap);
      dbg.tag(nodeP);
    }
    
    // Emit all the context->context edges collected while this module object was live.
    // Note that this guarantees that edges are guaranteed to be placed after or inside the modules they connect.
    for(std::map<pair<port, port>, int>::iterator e=edges.begin(); e!=edges.end(); e++) {
      properties edgeP;
      map<string, string> pMap;
      pMap["fromCID"] = txt()<<group2ID[e->first.first.g];
      pMap["fromT"]   = txt()<<e->first.first.type;
      pMap["fromP"]   = txt()<<e->first.first.index;
      pMap["toCID"]   = txt()<<group2ID[e->first.second.g];
      pMap["toT"]     = txt()<<e->first.second.type;
      pMap["toP"]     = txt()<<e->first.second.index;
      pMap["count"]   = txt()<<e->second;
      edgeP.add("moduleEdge", pMap);
      dbg.tag(edgeP);
    }
    
    //removeNode(c);
  }
  
  if(props->active) {
    // Pop this module from the mStack
    assert(mStack.size()>0);
    assert(mStack.back()==this);
    mStack.pop_back();
  }
}

// Sets the context of the given output port
void module::setOutCtxt(int idx, const context& c) { 
  assert(idx<g.numOutputs);
  outputs[idx].ctxt = c;
  // If the user provided an output vector, update it as well
  if(externalOutputs)
    (*externalOutputs)[idx].ctxt = c;
}

// Returns a list of the module's input ports
/*std::vector<common::module::port> module::inputPorts() const {
	std::vector<port> ports;
  for(int i=0; i<numInputs(); i++)
  	ports.push_back(port(c, input, i));
	return ports;
}*/

// Returns a list of the module's output ports
std::vector<common::module::port> module::outPorts() const {
	return outputs;
}

common::module::port module::outPort(int idx) const {
  assert(idx < g.numOutputs);
  return outputs[idx];  
}

// Returns the current module on the stack and NULL if the stack is empty
module* module::getCurrent() {
  if(mStack.size()>0) return mStack.back();
  else                return NULL;
}

// Records the mapping from a module's context to its unique ID
void module::addNode(const group& g, int nodeID) {
  cout << "addNode() new node g="<<g.str()<<", nodeID="<<nodeID<<endl;
  // If this context doesn't yet have an ID, set one
  if(group2ID.find(g) == group2ID.end()) {
    group2ID[g] = nodeID;
    group2Count[g] = 1;

    traceStream* ts = new traceStream(trace::lines, trace::disjMerge);
    moduleTrace[g] = ts;
    
    // Record the traceStream, along with the name of the module this steam is associated with
    properties streamProps;
    streamProps.add("module_traceStream", ts->getProperties());
    map<string, string> pMap;
    //pMap["ModuleName"] = g.name;
    pMap["nodeID"] = txt()<<nodeID;
    streamProps.add("module_traceStream_name", pMap);
    dbg.tag(streamProps);
  } else {
    group2Count[g]++;
  }
}

// Removes a module node from consideration
/*void module::removeNode(const instance& g) {
  assert(group2ID.find(c) != group2ID.end())
  group2ID.erase(c);
}*/

void module::addEdge(port from, port to) {
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

void module::addEdge(group fromG, ioT fromT, int fromP, group toG, ioT toT, int toP) {
  addEdge(port(fromG, context(), fromT, fromP), port(toG, context(), toT, toP));
}

/************************
 ***** ModuleMerger *****
 ************************/

ModuleMerger::ModuleMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        BlockMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* ModuleMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& greamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);

  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "scope");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Scope!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();
    
    props->add("module", pMap);
  } else {
    props->add("module", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void ModuleMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  properties::iterator blockTag = tag;
  BlockMerger::mergeKey(type, ++blockTag, inStreamRecords, key);
}

}; // namespace structure
}; // namespace sight
