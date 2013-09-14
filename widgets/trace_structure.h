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
#include "../dbglog_structure.h"
#include <sys/time.h>

namespace dbglog {
namespace structure {

void traceAttr(std::string label, std::string key, const attrValue& val);

class trace: public block, public attrObserver
{
  friend void traceAttr(std::string label, std::string key, const attrValue& val);
    
  public:
  // Indicates whether the trace visualization should be shown at the beginning or the end of its visual block
  typedef enum {showBegin, showEnd} showLocT;
  
  public:
  // Identifies the type of visualization used to show the trace
  typedef enum {table, lines, decTree} vizT;
  
  private:
  // Maps the names of all the currently active traces to their trace objects
  static std::map<std::string, trace*> active;
  
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
  
  public:
  trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc=showBegin, vizT viz=table);
  trace(std::string label, std::string contextAttr, showLocT showLoc=showBegin, vizT viz=table);
  
  private:
  void init(std::string label, showLocT showLoc, vizT viz);
  
  public:
  ~trace();
  
  // Returns a string representation of a showLocT object
  static std::string showLoc2Str(showLocT showLoc);
  
  // Returns a string representation of a vizT object
  static std::string viz2Str(vizT viz);
  
  private:
  
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
}; // class trace

// Basic API for measuring the elapsed counts of events.
// The measure class starts the measurement when instances of this class are constructed and stops when they are deconstructed.
// When measurement is performed, an attribute named valLabel is added to a trace named traceLabel.
// Users who wish to get the measurement value back may perform the measurement manually by calling doMeasure().
// The startMeasure()/endMeasure() API provides this direct access to measurement.
class measure {
  std::string traceLabel;
  std::string valLabel;
  struct timeval start;
  
  // Records whether we've already performed the measure
  bool measureDone;
  
  public:
  measure(std::string traceLabel, std::string valLabel);
  ~measure();
  
  double doMeasure();
}; // class measure

measure* startMeasure(std::string traceLabel, std::string valLabel);
double endMeasure(measure* m);

}; // namespace structure 
}; // namespace dbglog