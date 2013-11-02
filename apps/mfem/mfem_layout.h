#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "sight_layout.h"

namespace sight {
namespace layout {

class mfemLayoutHandlerInstantiator : layoutHandlerInstantiator {
  public:
  mfemLayoutHandlerInstantiator();
};
extern mfemLayoutHandlerInstantiator mfemLayoutHandlerInstance;

class mfem: public block
{
  public:
  
  mfem(properties::iterator props);
  ~mfem();

  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock);
  bool subBlockExitNotify (block* subBlock);
};
} // namespace layout
} // namespace sight
