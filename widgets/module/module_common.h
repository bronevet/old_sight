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
  
    
  class context {
    public:
    std::map<std::string, attrValue> configuration;
    
    typedef common::easymap<std::string, attrValue> config;
    
    context() {}
    
    context(const context& that) : configuration(that.configuration) {}
    
    context(const std::map<std::string, attrValue>& configuration) : configuration(configuration) {}
    
    // Loads this context from the given properties map. The names of all the fields are assumed to be prefixed
    // with the given string.
    context(properties::iterator props, std::string prefix="");
    
    bool operator==(const context& that) const
    { return configuration==that.configuration; }
    
    bool operator<(const context& that) const
    { return configuration<that.configuration; }
    
    void add(std::string key, const attrValue& val)
    { configuration[key] = val; }
    
    const std::map<std::string, attrValue>& getCfg() const { return configuration; }
    
    // Returns whether the given key is mapped within this context
    bool isKey(std::string key) const
    { return config.find(key) != config.end(); }
    
    // Returns the properties map that describes this context object.
    // The names of all the fields in the map are prefixed with the given string.
    std::map<std::string, std::string> getProperties(std::string prefix="") const;
    
    // Returns a human-readable string that describes this context
    std::string str() const;
  }; // class context

  // The different types of ports: input and output
  typedef enum {input, output} ioT;

}; // class module

}; // namespace common
}; // namespace sight
