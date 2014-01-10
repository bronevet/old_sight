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
    
    //typedef common::easymap<const std::string&, const attrValue&> config;
    
    context() {}
    
    context(const context& that) : configuration(that.configuration) {}
    
    //context(const config& configuration) : configuration(configuration) {}
    context(const std::map<std::string, attrValue>& configuration) : configuration(configuration) {}
    
    context(const std::string& key0, const attrValue& val0)
    { configuration[key0] = val0; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1)
    { configuration[key0] = val0; configuration[key1] = val1; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7, const std::string& key8, const attrValue& val8)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; configuration[key8] = val8; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7, const std::string& key8, const attrValue& val8, const std::string& key9, const attrValue& val9)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; configuration[key8] = val8; configuration[key9] = val9; }
    
    // Loads this context from the given properties map. The names of all the fields are assumed to be prefixed
    // with the given string.
    context(properties::iterator props, std::string prefix="");
    
    // Returns a dynamically-allocated copy of this object. This method must be implemented by all classes
    // that inherit from context to make sure that appropriate copies of them can be created.
    virtual context* copy() const { return new context(*this); }
    
    // These comparator routines must be implemented by all classes that inherit from context to make sure that
    // their additional details are reflected in the results of the comparison. Implementors may assume that 
    // the type of that is their special derivation of context rather than a generic context and should dynamically
    // cast from const context& to their sub-type.
    virtual bool operator==(const context& that) const
    { return configuration==that.configuration; }
    
    virtual bool operator<(const context& that) const
    { return configuration<that.configuration; }
    
    void add(std::string key, const attrValue& val)
    { configuration[key] = val; }
    
    const std::map<std::string, attrValue>& getCfg() const { return configuration; }
    
    // Returns whether the given key is mapped within this context
    bool isKey(std::string key) const
    { return configuration.find(key) != configuration.end(); }
    
    // Returns the properties map that describes this context object.
    // The names of all the fields in the map are prefixed with the given string.
    std::map<std::string, std::string> getProperties(std::string prefix="") const;
    
    // Returns a human-readable string that describes this context
    virtual std::string str() const;
  }; // class context

  // The different types of ports: input and output
  typedef enum {input, output} ioT;

  // Returns a string encoding of the key information of a context attribute:
  // moduleClass - the class that inherits from module that produced this attribute
  // ctxtGrouping - the name of the grouping of context attributes within the attributes generated by the 
  //    module class. This may be a given input (e.g. input0) or the configuration options of a compModule.
  // attrName - the name of the context attribute itself.
  static std::string encodeCtxtName(const std::string& moduleClass, const std::string& ctxtGrouping, const std::string& attrName);
  // Given a string encoded by encodeCtxtName(), sets moduleClass, ctxtGrouping and attrName to their original values
  static void decodeCtxtName(const std::string& encoded, std::string& moduleClass, std::string& ctxtGrouping, std::string& attrName);
}; // class module

}; // namespace common
}; // namespace sight
