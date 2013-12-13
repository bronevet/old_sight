#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include "module_common.h"

using namespace std;

namespace sight {
namespace common {

/*****************
 ***** group *****
 *****************/
 
module::group::group(properties::iterator props) {
  name       = properties::get(props, "name");
  numInputs  = properties::getInt(props, "numInputs");
  numOutputs = properties::getInt(props, "numOutputs");
}

// Returns the properties map that describes this group object;
std::map<std::string, std::string> module::group::getProperties() const {
  map<string, string> pMap;
  pMap["name"]       = name;
  pMap["numInputs"]  = txt()<<numInputs;
  pMap["numOutputs"] = txt()<<numOutputs;  
  return pMap;
}

// Returns a human-readable string that describes this context
std::string module::group::str() const {
  return txt()<<"[group "<<name<<", #inputs="<<numInputs<<", #outputs="<<numOutputs<<"]";
}

/*******************
 ***** context *****
 *******************/

module::context::context(properties::iterator props) {
  
  int numCfgKeys = properties::getInt(props, "numCfgKeys");
  for(int i=0; i<numCfgKeys; i++) {
    configuration[properties::get(props, txt()<<"key_"<<i)] = 
                              attrValue(properties::get(props, txt()<<"val_"<<i),
                                        (attrValue::valueType)properties::getInt(props, txt()<<"type_"<<i));
  }
}

// Returns the properties map that describes this context object;
map<string, string> module::context::getProperties() const {
  map<string, string> pMap;
  int i=0;
  for(std::map<std::string, attrValue>::const_iterator cfg=configuration.begin(); cfg!=configuration.end(); cfg++, i++) {
    pMap[txt()<<"key_"<<i]  = cfg->first;
    pMap[txt()<<"val_"<<i]  = cfg->second.getAsStr();
    pMap[txt()<<"type_"<<i] = txt()<<cfg->second.getType();
  }
  return pMap;
}

// Returns a human-readable string that describes this context
std::string module::context::str() const {
  ostringstream s;
  for(map<string, attrValue>::const_iterator c=configuration.begin(); c!=configuration.end(); c++) {
    if(c!=configuration.begin()) s << " ";
    s << "("<<c->first<<": "<<c->second.getAsStr()<<")";
  }
  return s.str();
}

/****************
 ***** port *****
 ****************/

// Returns a human-readable string that describes this context
std::string module::port::str() const {
  return txt() << "[port: g="<<g.str()<<", ctxt="<<ctxt.str() << " : "<<(type==input?"In":"Out")<<" : "<<index<<"]";
}

}; // namespace common
}; // namespace sight