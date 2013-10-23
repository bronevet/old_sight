#pragma once

namespace dbglog {
namespace common {

//namespace trace {
  // Identifies the type of visualization used to show the trace
  typedef enum {table, lines, decTree} vizT;

  // Indicates whether the trace visualization should be shown at the beginning or the end of its visual block
  typedef enum {showBegin, showEnd} showLocT;
  
  // Returns a string representation of a showLocT object
  /*static */std::string showLoc2Str(showLocT showLoc);
  
  // Returns a string representation of a vizT object
  /*static */std::string viz2Str(vizT viz);
//};

}; // namespace common
}; // namespace dbglog
