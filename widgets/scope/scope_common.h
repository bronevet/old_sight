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

namespace sight {
namespace common {

class scope {
  public:
  // Records whether this scope is included in the emitted output (true) or not (false)
  //bool active;
  typedef enum {high, medium, low, minimum} scopeLevel;

  static std::string level2Str(scopeLevel level)
  { return (level==high? "high": (level==medium? "medium": (level==low? "high": (level==minimum? "minimum": "???")))); }
};

} // namespace common
} // namespace sight
