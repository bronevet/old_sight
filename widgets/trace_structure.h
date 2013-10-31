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
#include "../sight_structure_internal.h"
#include <sys/time.h>

namespace sight {
namespace structure {

void traceAttr(std::string label, std::string key, const attrValue& val);
void traceAttr(std::string label, std::string key, const attrValue& val, anchor target);

class trace: public block, public attrObserver, public common::trace
{
  friend void traceAttr(std::string label, std::string key, const attrValue& val);
  friend void traceAttr(std::string label, std::string key, const attrValue& val, anchor target);
  
  private:
  // Unique ID of this trace
  int traceID;
  
  // Maximum ID assigned to any trace object
  static int maxTraceID;
    
  // Maps the names of all the currently active traces to their trace objects
  static std::map<std::string, trace*> active;
  
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
  
  public:
  trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc=showBegin, vizT viz=table, properties* props=NULL);
  trace(std::string label, std::string contextAttr,                    showLocT showLoc=showBegin, vizT viz=table, properties* props=NULL);
  
  private:
  // Sets the properties of this object
  static properties* setProperties(int traceID, showLocT showLoc, vizT viz, const std::list<std::string>& contextAttrs, properties* props);
  static properties* setProperties(int traceID, showLocT showLoc, vizT viz, std::string contextAttr,                    properties* props);
  
  void init(std::string label, showLocT showLoc, vizT viz);
  
  public:
  ~trace();
  
  private:
  
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

class TraceMerger : public BlockMerger {
  static int maxTraceID;
  public:
  TraceMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags);
}; // class TraceMerger

/*class TraceObsMerger : public BlockMerger {
  public:
  TraceObsMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags);
}; // class TraceObsMerger
*/


// Basic API for measuring the elapsed counts of events.
// The measure class starts the measurement when instances of this class are constructed and stops when they are deconstructed.
// When measurement is performed, an attribute named valLabel is added to a trace named traceLabel.
// Users who wish to get the measurement value back may perform the measurement manually by calling doMeasure().
// The startMeasure()/endMeasure() API provides this direct access to measurement.
class measure {
  std::string traceLabel;
  std::string valLabel;
  // Counts the total time elapsed so far, accounting for any pauses and resumes
  double elapsed;
  // The time when we started or resumed this measure, whichever is most recent
  struct timeval lastStart;

  // Records whether time collection is currently paused
  bool paused;
  
  // Records whether we've already performed the measure
  bool measureDone;
  
  public:
  measure(std::string traceLabel, std::string valLabel);
  ~measure();
 
  // Pauses the measurement so that time elapsed between this call and resume() is not counted.
  // Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
  bool pause();

  // Restarts counting time. Time collection is restarted regardless of how many times pause() was called
  // before the call to resume().
  bool resume();
 
  double doMeasure();
}; // class measure

measure* startMeasure(std::string traceLabel, std::string valLabel);
double endMeasure(measure* m);

}; // namespace structure 
}; // namespace sight
