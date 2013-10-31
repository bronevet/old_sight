// Licence information included in file LICENCE
#include "../sight_structure.h"
#include "../sight_common.h"
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

scope::scope(std::string label,                                    scopeLevel level, const attrOp& onoffOp, properties* props) : 
  block(label, setProperties(level, &onoffOp, props))
//{ init(level, &onoffOp); }
{}

scope::scope(string label, anchor& pointsTo,                 scopeLevel level, const attrOp& onoffOp, properties* props): 
  block(label, pointsTo, setProperties(level, &onoffOp, props))
//{ init(level, &onoffOp); }
{}

scope::scope(string label, set<anchor>& pointsTo,            scopeLevel level, const attrOp& onoffOp, properties* props) :
  block(label, pointsTo, setProperties(level, &onoffOp, props))
//{ init(level, &onoffOp); }
{}

scope::scope(std::string label,                                                                     const attrOp& onoffOp, properties* props) : 
  block(label, setProperties(medium, &onoffOp, props))
//{ init(medium, &onoffOp); }
{}

scope::scope(string label, anchor& pointsTo,                                                  const attrOp& onoffOp, properties* props): 
  block(label, pointsTo, setProperties(medium, &onoffOp, props))
//{ init(medium, &onoffOp); }
{}

scope::scope(string label, set<anchor>& pointsTo,                                             const attrOp& onoffOp, properties* props) :
  block(label, pointsTo, setProperties(medium, &onoffOp, props))
//{ init(medium, &onoffOp); }
{}

scope::scope(std::string label,                                   scopeLevel level,                         properties* props) :
  block(label, setProperties(level, NULL, props))
//{ init(level, NULL); }
{}

scope::scope(std::string label, anchor& pointsTo,           scopeLevel level,                         properties* props) :
  block(label, pointsTo, setProperties(level, NULL, props))
//{ init(level, NULL); }
{}

scope::scope(std::string label, std::set<anchor>& pointsTo, scopeLevel level,                         properties* props) :
  block(label, pointsTo, setProperties(level, NULL, props))
//{ init(level, NULL); }
{}

// Sets the properties of this object
properties* scope::setProperties(scopeLevel level, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    newProps["level"] = txt()<<level;
    //dbg.enter("scope", properties, inheritedFrom);
    props->add("scope", newProps);
  }
  else
    props->active = false;
  return props;
}

// Common initialization code
/*void scope::init(scopeLevel level, const attrOp* onoffOp)
{
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    active = true;
    map<string, string> properties;
    properties["level"] = txt()<<level;
    dbg.enter("scope", properties, inheritedFrom);
  }
  else
    active = false;
}*/

scope::~scope()
{ 
  /*if(props->active) {
    dbg.exit(this);
  }*/
}

}; // namespace structure
}; // namespace sight
