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
#include "../dbglog.h"

namespace dbglog {

void traceAttr(std::string key, const attrValue& val);

class trace: public block, public attrObserver
{
  friend void traceAttr(std::string key, const attrValue& val);
    
  public:
  // Indicates whether the trace visualization should be shown at the beginning or the end of its visual block
  typedef enum {showBegin, showEnd} showLocT;
  private:
  showLocT showLoc;
  
  public:
  // Identifies the type of visualization used to show the trace
  typedef enum {table, decTree} vizT;
  vizT viz;
  
  // Stack of currently active traces
  static std::list<trace*> stack;
    
  // Records whether the tracer infrastructure has been initialized
  static bool initialized;
  
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
  
  public:
  trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc=showBegin, vizT viz=table);
  trace(std::string label, std::string contextAttr, showLocT showLoc=showBegin, vizT viz=table);
  
  private:
  void init();
  
  public:
  ~trace();
  
  // Returns a string representation of a vizT object
  static std::string viz2Str(vizT viz);
  
  private:
  // Place the code to show the visualization
  void showViz();
  
  // Records all the observations of trace variables since the last time variables in contextAttrs changed values
  std::map<std::string, attrValue> obs;
    
  // The keys of all the tracer attributes ever observed
  std::set<std::string> tracerKeys;
  
  public:
  // Observe for changes to the values mapped to the given key
  void observePre(std::string key);
  
  // Called by traceAttr() to inform the trace that a new observation has been made
  void traceAttrObserved(std::string key, const attrValue& val);
  
  private:
  // Emits the JavaScript command that encodes the observations made since the last time a context attribute changed
  void emitObservations();
};

}; // namespace dbglog