// Copyright (c) 203 Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory
// Written by Greg Bronevetsky <bronevetsky1@llnl.gov>
//  
// LLNL-CODE-642002.
// All rights reserved.
//  
// This file is part of Sight. For details, see https://github.com/bronevet/sight. 
// Please read the COPYRIGHT file for Our Notice and
// for the BSD License.
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
