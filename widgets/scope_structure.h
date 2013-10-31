#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../attributes_structure.h"
#include "scope_common.h"
#include "../sight_structure_internal.h"

namespace sight {
namespace structure {

class scope: public block, public common::scope
{
  public:
  // label - the label associated with this scope.
  // pointsTo - the anchor(s) that terminate at this scope. Used to target links to scopes.
  // level - the type of visualization used, with higher levels associated with more amounts of debug output
  //    There are several features that are enabled by the levels:
  //       own_file: Text inside the scope is written to a separate file. Users must manually click a button to see it.
  //       own_color: The background color of the scope is different from its parent scope.
  //       label_shown: The label of this scope is shown in a larger font, along with controls to minimize the scope 
  //              by clicking on the label and open GDB to the point in the execution where the scope started
  //    Different levels
  //    high: own_file, own_color, label_shown
  //    medium: own_color, label_shown
  //    low: label_shown
  //    min: none of the above
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  scope(std::string label,                             scopeLevel level, const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label, anchor& pointsTo,           scopeLevel level, const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label, std::set<anchor>& pointsTo, scopeLevel level, const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label,                                               const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label, anchor& pointsTo,                             const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label, std::set<anchor>& pointsTo,                   const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label,                             scopeLevel level=medium,                 properties* props=NULL);
  scope(std::string label, anchor& pointsTo,           scopeLevel level=medium,                 properties* props=NULL);
  scope(std::string label, std::set<anchor>& pointsTo, scopeLevel level=medium,                 properties* props=NULL);
  
  private:
  // Sets the properties of this object
  static properties* setProperties(scopeLevel level, const attrOp* onoffOp, properties* props);
    
  // Common initialization code
  //void init(scopeLevel level, const attrOp* onoffOp);
  
  public:
  ~scope();
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
}; // scope

}; // namespace structure
}; // namespace sight
