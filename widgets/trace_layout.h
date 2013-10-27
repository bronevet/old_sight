#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <assert.h>
#include "../sight_common.h"
#include "../sight_layout.h"
#include <sys/time.h>

namespace sight {
namespace layout {

class traceLayoutHandlerInstantiator {
  public:
  traceLayoutHandlerInstantiator();
};

void traceAttr(std::string label, std::string key, const attrValue& val);

class trace: public block, public attrObserver, public common::trace
{
  friend void traceAttr(std::string key, const attrValue& val);
    
  private:
  // Unique ID of this trace
  int traceID;
  
  showLocT showLoc;
  
  // The ID of the block into which the trace visualization will be written
  std::string tgtBlockID;
  
  public:
  vizT viz;
  
  // Maps the traceIDs of all the currently active traces to their trace objects
  static std::map<int, trace*> active;
    
  // Records whether the tracer infrastructure has been initialized
  static bool initialized;
  
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
  
  public:
  trace(properties::iterator props);
  
  private:
  void init(std::string label);
  
  public:
  ~trace();
  
  private:
  // Place the code to show the visualization
  void showViz();
  
  // Records all the observations of trace variables since the last time variables in contextAttrs changed values
  std::map<std::string, std::pair<attrValue, anchor> > obs;
  
  // The keys of all the tracer attributes ever observed
  std::set<std::string> tracerKeys;
  
  public:
  // Record an observation
  static void* observe(properties::iterator props);
}; // class trace

}; // namespace layout
}; // namespace sight
