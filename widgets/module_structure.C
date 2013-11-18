// Licence information included in file LICENCE
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
std::map<common::module::context, int> module::knownCtxt;
  
// Maps each context to the number of times it was ever observed
std::map<common::module::context, int> module::ctxtCount;

// Maps each context to the trace that records its performance properties
std::map<common::module::context, trace*> module::ctxtTrace;

// Records all the edges ever observed, mapping them to the number of times each edge was observed
std::map<std::pair<common::module::port, common::module::port>, int> module::edges;

int module::maxModuleID=0;

// Stack of the module graphs that are currently in scope
std::list<module*> module::mStack;

module::module(const context& c,                                                     properties* props) : 
  block(c.name, setProperties(c, inputs(),   NULL, props)), c(c)
{ init(c, inputs()); }

module::module(const context& c, const port& in,                                     properties* props): 
  block(c.name, setProperties(c, inputs(in), NULL, props)), c(c)
{ init(c, inputs(in)); }

module::module(const context& c, const std::vector<port>& in,                        properties* props) :
  block(c.name, setProperties(c, in,               NULL, props)), c(c)
{ init(c, in); }

module::module(const context& c,                              const attrOp& onoffOp, properties* props) : 
  block(c.name, setProperties(c, inputs(),   &onoffOp, props)), c(c)
{ init(c, inputs()); }

module::module(const context& c, const port& in,              const attrOp& onoffOp, properties* props): 
  block(c.name, setProperties(c, inputs(in), &onoffOp, props)), c(c)
{ init(c, inputs(in)); }

module::module(const context& c, const std::vector<port>& in, const attrOp& onoffOp, properties* props) :
  block(c.name, setProperties(c, in,               &onoffOp, props)), c(c)
{ init(c, in); }

void module::init(const context& c, const std::vector<port>& in) {
  if(this->props->active) {
    mStack.push_back(this);
    
  	//cout << "module c="<<c.str()<<endl;
    addNode(c, maxModuleID);
    maxModuleID++;
    
    int idx=0;
    for(vector<port>::const_iterator i=in.begin(); i!=in.end(); i++, idx++) {
    	addEdge(*i, port(c, input, idx));
    }
    
    // Create an instance of the trace attribute that is unique to this context but common to all of its instances
    //instanceAttr = new attr(c.UID(), ctxtCount[c]);
    
    // Begin measuring this instance
    moduleMeasure = startMeasure(ctxtTrace[c], "measure");
    
  } else {
    moduleMeasure = NULL;
    //instanceAttr = NULL;
  }
}

// Sets the properties of this object
properties* module::setProperties(const context& c, const std::vector<port>& inputs, const attrOp* onoffOp, properties* props)  {
  if(props==NULL) props = new properties();
  
  assert(c.numInputs==inputs.size());
  
  // We only emit tags at the very end of the outer-most module
  props->emitTag = (props->active && mStack.size()==0);
  
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or it evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
  	props->active = true;
    
    if(props->emitTag) {
      /*map<string, string> pMap = c->first.getProperties();
      //newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
      newProps["numInputs"]  = txt()<<numInputs;
      newProps["numOutputs"] = txt()<<numOutputs;*/
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
  // Complete the measurement of application's behavior during the module's lifetime
  if(props->active) {
    //cout << "module "<<c.str()<<" measured. ctxtTrace[c] "<<(ctxtTrace.find(c) != ctxtTrace.end())<<endl;
    // Complete measuring this instance
    assert(moduleMeasure);
    endMeasure(moduleMeasure);
    
    // Destroy the trace attribute associated with this instance to inform the trace that we've collected 
    // all the data for this value of the attribute
    //assert(instanceAttr);
    //delete instanceAttr;
  }
  
  if(props->active && props->emitTag) {
    /*cout << "knownCtxt="<<endl;
    for(std::map<context, int>::iterator c=knownCtxt.begin(); c!=knownCtxt.end(); c++)
      cout << "    "<<c->first.UID()<<" ==> "<<c->second<<endl;
    
    */
    
    // Emit the information on all the known contexts
    for(std::map<context, int>::iterator c=knownCtxt.begin(); c!=knownCtxt.end(); c++) {
      properties nodeP;
      map<string, string> pMap = c->first.getProperties();
      pMap["ID"] = txt()<<c->second;
      
      assert(ctxtCount.find(c->first) != ctxtCount.end());
      pMap["count"] = txt()<<ctxtCount[c->first];
      
      nodeP.add("moduleNode", pMap);
      dbg.tag(nodeP);
      
      // Deallocate the trace associated with this module, calling its destructor and emitting its tags
      /*assert(ctxtTrace.find(c->first) != ctxtTrace.end());
      cout << "deleting ctxtTrace["<<c->first.str()<<"]"<<endl;  cout.flush();
      delete ctxtTrace[c->first];
      cout << "deleted ctxtTrace["<<c->first.str()<<"]"<<endl;  cout.flush();
      ctxtTrace.erase(c->first);*/
      delete ctxtTrace[c->first];
      ctxtTrace.erase(c->first);
    }
    
    // Emit all the context->context edges collected while this module object was live.
    // Note that this guarantees that edges are guaranteed to be placed after or inside the modules they connect.
    for(std::map<pair<port, port>, int>::iterator e=edges.begin(); e!=edges.end(); e++) {
      properties edgeP;
      map<string, string> pMap;
      pMap["fromCID"] = txt()<<knownCtxt[e->first.first.c];
      pMap["fromT"]   = txt()<<e->first.first.type;
      pMap["fromP"]   = txt()<<e->first.first.index;
      pMap["toCID"]   = txt()<<knownCtxt[e->first.second.c];
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


// Returns a list of the module's input ports
std::vector<common::module::port> module::inputPorts() const {
	std::vector<port> ports;
  for(int i=0; i<numInputs(); i++)
  	ports.push_back(port(c, input, i));
	return ports;
}

// Returns a list of the module's output ports
std::vector<common::module::port> module::outputPorts() const {
	std::vector<port> ports;
  for(int i=0; i<numOutputs(); i++)
  	ports.push_back(port(c, output, i));
  return ports;
}

// Returns the current module on the stack and NULL if the stack is empty
module* module::getCurrent() {
  if(mStack.size()>0) return mStack.back();
  else                return NULL;
}

// Records the mapping from a module's context to its unique ID
void module::addNode(const context& c, int nodeID) {
  // If this context doesn't yet have an ID, set one
  if(knownCtxt.find(c) == knownCtxt.end()) {
    cout << "addNode() new node c="<<c.str()<<", nodeID="<<nodeID<<endl;
    knownCtxt[c] = nodeID;
    ctxtCount[c] = 1;
    //string contextAttr = txt()<<"moduleInstance_"<<nodeID;
    //ctxtTrace[c] = new trace(txt()<<"module_"<<nodeID, contextAttr, trace::showBegin, trace::table, trace::disjMerge);
    
    // Create a trace for this module that is common to all instances of the module
    //ctxtTrace[c] = new trace(c.UID(), c.UID(), trace::showBegin, trace::table, trace::disjMerge);
    ctxtTrace[c] = new trace(c.UID(), trace::showBegin, trace::table, trace::disjMerge);
  } else
    ctxtCount[c]++;
}

// Removes a module node from consideration
/*void module::removeNode(const context& c) {
  assert(knownCtxt.find(c) != knownCtxt.end())
  knownCtxt.erase(c);
}*/

void module::addEdge(port from, port to) {
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

void module::addEdge(context fromC, ioT fromT, int fromP, context toC, ioT toT, int toP) {
  addEdge(port(fromC, fromT, fromP), port(toC, toT, toP));
}

/************************m
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
                                       vector<map<string, streamRecord*> >& inStreamRecords,
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
