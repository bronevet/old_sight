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
string trace::showLoc2Str(trace::showLocT showLoc) 
{ return (showLoc==trace::showBegin? "showBegin": (showLoc==trace::showEnd? "showEnd": "???")); }

// Returns a string representation of a vizT object
string trace::viz2Str(trace::vizT viz) {
       if(viz == trace::table)   return "table";
  else if(viz == trace::lines)   return "lines";
  else if(viz == trace::decTree) return "decTree";
  else if(viz == trace::heatmap) return "heatmap";
  else if(viz == trace::boxplot) return "boxplot";
  else                    return "???";
}

}; // namespace common
}; // namespace dbglog
