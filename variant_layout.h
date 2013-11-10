#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "sight_common.h"
#include "sight_layout.h"

namespace sight {
namespace layout {

class variantLayoutHandlerInstantiator  : layoutHandlerInstantiator{
  public:
  variantLayoutHandlerInstantiator();
};
extern variantLayoutHandlerInstantiator variantLayoutHandlerInstance;

class variant: public scope
{
  int numVariants;
  public:

  // properties: maps property names to their values
  variant(properties::iterator props);
  
  private:
  // Common initialization code
  void init();
  
  public:
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
  
  ~variant();
}; // variant

}; // namespace layout
}; // namespace sight
