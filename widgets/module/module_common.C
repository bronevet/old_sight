#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include "module_common.h"

using namespace std;

namespace sight {
namespace common {

/*******************
 ***** context *****
 *******************/

// Loads this context from the given properties map. The names of all the fields are assumed to be prefixed
// with the given string.
module::context::context(properties::iterator props, std::string prefix) {
  
  int numCfgKeys = properties::getInt(props, txt()<<prefix<<"numCfgKeys");
  for(int i=0; i<numCfgKeys; i++) {
    configuration[properties::get(props, txt()<<"key_"<<i)] = 
                              attrValue(properties::get(props, txt()<<"val_"<<i),
                                        (attrValue::valueType)properties::getInt(props, txt()<<"type_"<<i));
  }
}

// Returns the properties map that describes this context object.
// The names of all the fields in the map are prefixed with the given string.
map<string, string> module::context::getProperties(std::string prefix) const {
  map<string, string> pMap;
  int i=0;
  pMap[txt()<<prefix<<"numCfgKeys"] = txt()<<configuration.size();
  for(std::map<std::string, attrValue>::const_iterator cfg=configuration.begin(); cfg!=configuration.end(); cfg++, i++) {
    pMap[txt()<<prefix<<"key_"<<i]  = cfg->first;
    pMap[txt()<<prefix<<"val_"<<i]  = cfg->second.getAsStr();
    pMap[txt()<<prefix<<"type_"<<i] = txt()<<cfg->second.getType();
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

}; // namespace common
}; // namespace sight