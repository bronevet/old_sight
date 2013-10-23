#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include "trace_common.h"

using namespace std;

namespace dbglog {
namespace common {


// Returns a string representation of a common::trace::showLocT object
string showLoc2Str(common::showLocT showLoc) 
{ return (showLoc==showBegin? "showBegin": (showLoc==showEnd? "showEnd": "???")); }

// Returns a string representation of a vizT object
string viz2Str(common::vizT viz) {
       if(viz == table)   return "table";
  else if(viz == lines)   return "lines";
  else if(viz == decTree) return "decTree";
  else                    return "???";
}

}; // namespace common
}; // namespace dbglog