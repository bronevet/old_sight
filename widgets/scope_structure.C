// Licence information included in file LICENCE
#include "../dbglog_structure_internal.h"
#include "scope_structure.h"
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

namespace dbglog {
namespace structure {

/*****************
 ***** scope *****
 *****************/
scope::scope(std::string label,                                    scopeLevel level, const attrOp& onoffOp) : 
  block(label)
{ init(level, &onoffOp); }

scope::scope(string label, const anchor& pointsTo,                 scopeLevel level, const attrOp& onoffOp): 
  block(label, pointsTo)
{ init(level, &onoffOp); }

scope::scope(string label, const set<anchor>& pointsTo,            scopeLevel level, const attrOp& onoffOp) :
  block(label, pointsTo)
{ init(level, &onoffOp); }

scope::scope(std::string label,                                                      const attrOp& onoffOp) : 
  block(label)
{ init(scope::medium, &onoffOp); }

scope::scope(string label, const anchor& pointsTo,                                   const attrOp& onoffOp): 
  block(label, pointsTo)
{ init(scope::medium, &onoffOp); }

scope::scope(string label, const set<anchor>& pointsTo,                             const attrOp& onoffOp) :
  block(label, pointsTo)
{ init(scope::medium, &onoffOp); }

scope::scope(std::string label,                                   scopeLevel level) :
  block(label)
{ init(level, NULL); }

scope::scope(std::string label, const anchor& pointsTo,           scopeLevel level) :
  block(label, pointsTo)
{ init(level, NULL); }
  
scope::scope(std::string label, const std::set<anchor>& pointsTo, scopeLevel level) :
  block(label, pointsTo)
{ init(level, NULL); }

// Common initialization code
void scope::init(scopeLevel level, const attrOp* onoffOp)
{
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    active = true;
    map<string, string> properties;
    properties["level"] = level2Str(level);
    dbg.enter("scope", properties);
  }
  else
    active = false;
}

scope::~scope()
{ 
  if(active) {
    dbg.exit("scope");
  }
}

}; // namespace structure
}; // namespace dbglog
