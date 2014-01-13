#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include "trace_common.h"

using namespace std;

namespace sight {
namespace common {


// Returns a string representation of a common::trace::showLocT object
string trace::showLoc2Str(trace::showLocT showLoc) 
{ return (showLoc==trace::showBegin? "showBegin": (showLoc==trace::showEnd? "showEnd": "???")); }

// Returns a string representation of a vizT object
string trace::viz2Str(trace::vizT viz) {
  switch(viz) {
    case trace::table:     return "table";
    case trace::lines:     return "lines";
    case trace::scatter3d: return "scatter3d";
    case trace::decTree:   return "decTree";
    case trace::heatmap:   return "heatmap";
    case trace::boxplot:   return "boxplot";
    default:               return "???";
  }
}

// Returns a string representation of a mergeT object
string trace::mergeT2Str(mergeT merge) {
  switch(merge) {
    case trace::disjMerge: return "disjMerge";
    case trace::avgMerge:  return "avgMerge";
    case trace::maxMerge:  return "maxMerge";
    case trace::minMerge:  return "minMerge";
    default:               return "???";
  }
}

}; // namespace common
}; // namespace sight
