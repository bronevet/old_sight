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
#include <sys/time.h>

namespace dbglog {

void traceAttr(std::string label, std::string key, const attrValue& val);
void traceAttr(std::string label, std::string key, const attrValue& val, anchor target);

class trace: public block, public attrObserver
{
  friend void traceAttr(std::string label, std::string key, const attrValue& val);
  friend void traceAttr(std::string label, std::string key, const attrValue& val, anchor target);
    
  public:
  // Indicates whether the trace visualization should be shown at the beginning or the end of its visual block
  typedef enum {showBegin, showEnd} showLocT;
  private:
  showLocT showLoc;
  
  // The ID of the block into which the trace visualization will be written
  std::string tgtBlockID;
  
  public:
  // Identifies the type of visualization used to show the trace
  typedef enum {table, lines, decTree, heatmap} vizT;
  vizT viz;
  
  // Stack of currently active traces
  //static std::list<trace*> stack;
  static std::map<std::string, trace*> active;
    
  // Records whether the tracer infrastructure has been initialized
  static bool initialized;
  
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
  
  public:
  trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc=showBegin, vizT viz=table);
  trace(std::string label, std::string contextAttr, showLocT showLoc=showBegin, vizT viz=table);
  
  private:
  void init(std::string label);
  
  public:
  ~trace();
  
  // Returns a string representation of a vizT object
  static std::string viz2Str(vizT viz);
  
  private:
  // Place the code to show the visualization
  void showViz();
  
  // Records all the observations of trace variables since the last time variables in contextAttrs changed values
  std::map<std::string, std::pair<attrValue, anchor> > obs;
    
  // The keys of all the tracer attributes ever observed
  std::set<std::string> tracerKeys;
  
  public:
  // Observe for changes to the values mapped to the given key
  void observePre(std::string key);
  
  // Called by traceAttr() to inform the trace that a new observation has been made
  void traceAttrObserved(std::string key, const attrValue& val, anchor target);
  
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

}; // namespace dbglog
