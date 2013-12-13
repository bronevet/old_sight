#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../attributes/attributes_common.h"
#include "sight_common.h"

namespace sight {
namespace common {


class module {
  public:
  
  // Module relations are organized based two data structures: groups and contexts.
  // A group defines the granularity at which different executions of a module are differentiated. A group
  //    may be the module's name, its name+nesting stack within other modules, name+call stack or name+context.
  //    All module groups that map to the same group must have the same number of inputs and outputs.
  // Context defines the properties of a given data item propagated from one module to another. A context is a mapping
  //    from one or more property names to their values. The context of a given group of a module is the combination
  //    of the contexts of its inputs.
    
  class group {
    public:
    std::string name;
    // nesting stack?
    
    int numInputs;
    int numOutputs;
    
    group() {}
    group(std::string name, int numInputs, int numOutputs) : name(name), numInputs(numInputs), numOutputs(numOutputs) {}
    group(const group& that) : name(that.name), numInputs(that.numInputs), numOutputs(that.numOutputs) {}
    
    group(properties::iterator props);
    
    // Returns the properties map that describes this group object;
    std::map<std::string, std::string> getProperties() const;
    
    bool operator==(const group& that) const
    { return name==that.name; }
    
    bool operator<(const group& that) const
    { return name<that.name; }
    
    // Returns a human-readable string that describes this context
    std::string str() const;
  }; // class group
  
  class context {
    public:
    std::map<std::string, attrValue> configuration;
    
    typedef common::easymap<std::string, attrValue> config;
    
    context() {}
    
    context(const context& that) : configuration(that.configuration) {}
    
    context(const std::map<std::string, attrValue>& configuration) : configuration(configuration) {}
    
    context(properties::iterator props);
    
    bool operator==(const context& that) const
    { return configuration==that.configuration; }
    
    bool operator<(const context& that) const
    { return configuration<that.configuration; }
    
    void add(std::string key, const attrValue& val)
    { configuration[key] = val; }
    
    const std::map<std::string, attrValue>& getCfg() const { return configuration; }
    
    // Returns the properties map that describes this context object;
    std::map<std::string, std::string> getProperties() const;
    
    // Returns a human-readable string that describes this context
    std::string str() const;
  }; // class context
  
  
  typedef enum {input, output} ioT;
  
  class port {
    public:
    group g;
    context ctxt;
    ioT type;
    int index;
    
    port() {}
    port(const group& g, const context& ctxt, ioT type, int index) : g(g), ctxt(ctxt), type(type), index(index) {}
      
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
  
  class inport : public port {
    public:
    inport() {}
    inport(const group& g, const context& c, int index) : port(g, c, input, index) {}
  };
  
  class outport : public port {
    public:
    outport() {}
    outport(const group& g, const context& c, int index) : port(g, c, output, index) {}
  };

}; // class module

}; // namespace common
}; // namespace sight
