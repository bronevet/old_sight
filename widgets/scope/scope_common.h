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
