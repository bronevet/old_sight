#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "attributes_common.h"
#include "sight_common.h"

namespace sight {
namespace common {


class module {
  public:
  class context {
    public:
    std::string name;
    std::map<std::string, attrValue> configuration;
    // If two contexts have the same name and configuration, they must have the same values of numInputs and numOutputs
    int numInputs;
    int numOutputs;
    
    context() {}
    
    context(const context& that) : name(that.name), configuration(that.configuration), numInputs(that.numInputs), numOutputs(that.numOutputs) {}
    
    context(const std::string& name, int numInputs=0, int numOutputs=0) :
    	name(name), configuration(configuration), numInputs(numInputs), numOutputs(numOutputs) {}
    
    context(const std::string& name, int numInputs, int numOutputs, const std::map<std::string, attrValue>& configuration) :
      name(name), configuration(configuration), numInputs(numInputs), numOutputs(numOutputs) {}
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1, std::string key2, attrValue val2) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1, std::string key2, attrValue val2, std::string key3, attrValue val3) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1, std::string key2, attrValue val2, std::string key3, attrValue val3, std::string key4, attrValue val4) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1, std::string key2, attrValue val2, std::string key3, attrValue val3, std::string key4, attrValue val4, std::string key5, attrValue val5) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1, std::string key2, attrValue val2, std::string key3, attrValue val3, std::string key5, std::string key4, attrValue val4, attrValue val5, std::string key6, attrValue val6) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1, std::string key2, attrValue val2, std::string key3, attrValue val3, std::string key5, std::string key4, attrValue val4, attrValue val5, std::string key6, attrValue val6, std::string key7, attrValue val7) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1, std::string key2, attrValue val2, std::string key3, attrValue val3, std::string key5, std::string key4, attrValue val4, attrValue val5, std::string key6, attrValue val6, std::string key7, attrValue val7, std::string key8, attrValue val8) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; configuration[key8] = val8; }
    
    context(const std::string& name, int numInputs, int numOutputs, std::string key0, attrValue val0, std::string key1, attrValue val1, std::string key2, attrValue val2, std::string key3, attrValue val3, std::string key5, std::string key4, attrValue val4, attrValue val5, std::string key6, attrValue val6, std::string key7, attrValue val7, std::string key8, attrValue val8, std::string key9, attrValue val9) :
      name(name), numInputs(numInputs), numOutputs(numOutputs) 
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; configuration[key8] = val8; configuration[key9] = val9; }
    
    context(properties::iterator props);
    
    bool operator==(const context& that) const
    { return name==that.name && configuration==that.configuration; }
    
    bool operator<(const context& that) const
    { 
      //std::cout << ":           ("<<((name< that.name) || (name==that.name && configuration<that.configuration)? "<": "!<")<<") this="<<UID()<<", that="<<that.UID()<<std::endl;
      
      return (name< that.name) ||
             (name==that.name && configuration<that.configuration); }
    
    void add(std::string key, const attrValue& val)
    { configuration[key] = val; }
    
    // Returns the properties map that describes this context object;
    std::map<std::string, std::string> getProperties() const;
  
    // Returns a string that uniquely identifies this context
    std::string UID() const;
      
    // Returns a human-readable string that describes this context
    std::string str() const;
  }; // class context
  
  typedef enum {input, output} ioT;
  
  class port {
    public:
    context c;
    ioT type;
    int index;
    
    port() {}
    port(const context& c, ioT type, int index) : c(c), type(type), index(index) {}
      
    bool operator==(const port& that) const
    { return c==that.c && type==that.type && index==that.index; }
    
    bool operator<(const port& that) const
    { return (c< that.c) ||
             (c==that.c && type< that.type) ||
             (c==that.c && type==that.type && index<that.index); }
    
    // Returns a human-readable string that describes this context
    std::string str() const;
  }; // class port
  
  class inport : public port {
    public:
    inport() {}
    inport(const context& c, int index) : port(c, input, index) {}
  };
  
  class outport : public port {
    public:
    outport() {}
    outport(const context& c, int index) : port(c, output, index) {}
  };

  /* // Syntactic sugar for specifying ports that connect to a module's inputs
	class inputs : public std::vector<port> {
		public:
		inputs() {}
		  
		inputs(const port& p0)
		{ push_back(p0); }
		
		inputs(const port& p0, const port& p1)
		{ push_back(p0); push_back(p1); }
		
		inputs(const port& p0, const port& p1, const port& p2)
		{ push_back(p0); push_back(p1); push_back(p2); }
		
		inputs(const port& p0, const port& p1, const port& p2, const port& p3)
		{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); }
		
		inputs(const port& p0, const port& p1, const port& p2, const port& p3, const port& p4)
		{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); }
		
		inputs(const port& p0, const port& p1, const port& p2, const port& p3, const port& p4, const port& p5)
		{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); }
		
		inputs(const port& p0, const port& p1, const port& p2, const port& p3, const port& p4, const port& p5, const port& p6)
		{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); }
		
		inputs(const port& p0, const port& p1, const port& p2, const port& p3, const port& p4, const port& p5, const port& p6, const port& p7)
		{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); }
		
		inputs(const port& p0, const port& p1, const port& p2, const port& p3, const port& p4, const port& p5, const port& p6, const port& p7, const port& p8)
		{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); push_back(p8); }
		
		inputs(const port& p0, const port& p1, const port& p2, const port& p3, const port& p4, const port& p5, const port& p6, const port& p7, const port& p8, const port& p9)
		{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); push_back(p8); push_back(p8); push_back(p9); }
	}; // class inputs */
}; // class module

}; // namespace common
}; // namespace sight
