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

class trace;
void traceAttr(std::string label, std::string key, const attrValue& val);
void traceAttr(std::string label, std::string key, const attrValue& val, anchor target);
void traceAttr(trace* t,          std::string key, const attrValue& val);
void traceAttr(trace* t,          std::string key, const attrValue& val, anchor target);
void traceAttr(std::string label, 
               const std::map<std::string, attrValue>& ctxt, 
               const std::list<std::pair<std::string, attrValue> >& obsList, 
               const anchor& target=anchor::noAnchor);
void traceAttr(trace* t, 
               const std::map<std::string, attrValue>& ctxt, 
               const std::list<std::pair<std::string, attrValue> >& obsList, 
               const anchor& target=anchor::noAnchor);
// Syntactic sugar for specifying anchors to observation sites
//typedef common::easylist<anchor> obsAnchors;

// Traces are organized as a two-level hierarchy. The traceStream class performs all the work of collecting
// trace data, emitting it to the output and laying the data out within some div in an HTML page. 
// traceStreams are contained in classes that derive from block and denote a specific location in the output.
// Such container point traceStream to the div in which it should render its results. The trace class is
// one such container but other classes may serve as containers for one or more traceStreams to make it possible
// for them to include data visualizations inside their other visualizations. One example, is the module
// widget, which shows data visualizations inside dot graphs.

class traceStream;

class trace: public block, public common::trace
{
  public:
  // Syntactic sugar for specifying contexts
  typedef common::easylist<std::string> context;
  
  // Syntactix sugar for specifying context name=>value mappings
  typedef common::easymap<std::string, attrValue> ctxtVals;
  
  // Syntactic sugar for specifying observations
  typedef common::easylist<std::pair<std::string, attrValue> > observation;
    
  private:
  traceStream* stream;
  
  // Maps the names of all the currently active traces to their trace objects
  static std::map<std::string, trace*> active;
  
  public:
  trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  trace(std::string label, std::string contextAttr,                    showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  trace(std::string label,                                             showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  ~trace();
  
  // Sets the properties of this object
  static properties* setProperties(showLocT showLoc, properties* props);
  
  void init(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz, mergeT merge);
  
  static trace*       getT (std::string label);
  static traceStream* getTS(std::string label);
  traceStream* getTS() const { return stream; }
 }; // class trace

class traceStream: public attrObserver, public common::trace, public sightObj
{    
  private:
  // Unique ID of this trace
  int traceID;
  
  // Maximum ID assigned to any trace object
  static int maxTraceID;
  
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
  // Records the context attributes that have already been initialized
  std::set<std::string> initializedCtxtAttrs;
  
  vizT viz;
  mergeT merge;
  
  public:
  traceStream(const std::list<std::string>& contextAttrs, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  traceStream(std::string contextAttr,                    vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  traceStream(                                            vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  
  // Sets the properties of this object
  properties* setProperties(const std::list<std::string>& contextAttrs, vizT viz, mergeT merge, properties* props);

  private:  
  void init();
  
  public:
  ~traceStream();
  
  private:
  
  // Records all the observations of trace variables since the last time variables in contextAttrs changed values
  std::map<std::string, std::pair<attrValue, anchor> > obs;
    
  public:
  int getTraceID() const { return traceID; }
  
  // Observe for changes to the values mapped to the given key
  void observePre(std::string key, attrObserver::attrObsAction action);
  void observePost(std::string key, attrObserver::attrObsAction action);
  
  // Called by traceAttr() to inform the trace that a new observation has been made
  void traceAttrObserved(std::string key, const attrValue& val, anchor target);
    
  // Records the full observation, including all the values of the context and observation values.
  // This observation is emitted immediately regardless of the current state of other observations
  // that have been recorded via traceAttrObserved.
  void traceFullObservation(const std::map<std::string, attrValue>& contextAttrsMap, 
                            const std::list<std::pair<std::string, attrValue> >& obsList, 
                            const anchor& target);
  
  private:
  
  // Emits the output record records the given context and observations pairing
  void emitObservations(const std::list<std::string>& contextAttrs, 
                        std::map<std::string, std::pair<attrValue, anchor> >& obs);
  
  // Emits the output record records the given context and observations pairing
  void emitObservations(const std::map<std::string, attrValue>& contextAttrsMap, 
                        std::map<std::string, std::pair<attrValue, anchor> >& obs);
}; // class traceStream

// Basic API for measuring the elapsed counts of events.
// The measure class starts the measurement when instances of this class are constructed and stops when they are deconstructed.
// When measurement is performed, an attribute named valLabel is added to a trace named traceLabel.
// Users who wish to get the measurement value back may perform the measurement manually by calling doMeasure().
// The startMeasure()/endMeasure() API provides this direct access to measurement.
class measure {
  traceStream* ts;
  std::string valLabel;
  // Counts the total time elapsed so far, accounting for any pauses and resumes
  double elapsed;
  // The time when we started or resumed this measure, whichever is most recent
  struct timeval lastStart;

  // Records whether time collection is currently paused
  bool paused;
  
  // Records whether we've already performed the measure
  bool measureDone;
  
  // Records whether the measurement will be a full measure, in the sense that when the measurement by itself
  // represents all the information that will ever be recorded for a given context rather than trickle in 
  // observation-by-observation.
  bool fullMeasure;
  // If we're doing a full measure, this is the context that will be used for this measurement
  std::map<std::string, attrValue> fullMeasureCtxt;
  
  public:
  // Non-full measure
  measure(std::string traceLabel, std::string valLabel);
  measure(trace* t,               std::string valLabel);
  measure(traceStream* ts,        std::string valLabel);
  // Full measure
  measure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  measure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  measure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  ~measure();
 
  // Common initialization code
  void init();
   
  // Pauses the measurement so that time elapsed between this call and resume() is not counted.
  // Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
  bool pause();

  // Restarts counting time. Time collection is restarted regardless of how many times pause() was called
  // before the call to resume().
  bool resume();
 
  double doMeasure();
}; // class measure

// Non-full measure
measure* startMeasure(std::string traceLabel, std::string valLabel);
measure* startMeasure(trace* t,               std::string valLabel);
measure* startMeasure(traceStream* ts,        std::string valLabel);
// Full measure
measure* startMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
measure* startMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
measure* startMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
double endMeasure(measure* m);

class TraceMerger : public BlockMerger {
  public:
  TraceMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
              
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key);
}; // class TraceMerger

class TraceStreamMerger : public Merger {
  public:
  TraceStreamMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
              
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key);
}; // class TraceStreamMerger

class TraceObsMerger : public Merger {
  public:
  TraceObsMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key);
}; // class TraceObsMerger

class TraceStreamRecord: public streamRecord {
  friend class TraceStreamMerger;
  friend class TraceObsMerger;
  
  // Records the maximum TraceID ever generated on a given outgoing stream
  //int maxTraceID;
  
  // Maps traceIDs to their merge types
  std::map<int, trace::mergeT> merge;
  
  // Maps the TraceIDs within an incoming stream to the TraceIDs on its corresponding outgoing stream
  //std::map<streamID, streamID> in2outTraceIDs;
  
  public:
  TraceStreamRecord(int vID)              : streamRecord(vID, "traceStream") { /*maxTraceID=0;*/ }
  TraceStreamRecord(const variantID& vID) : streamRecord(vID, "traceStream") { /*maxTraceID=0;*/ }
  TraceStreamRecord(const TraceStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
  
  // Marge the IDs of the next graph (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
  // updating each incoming stream's mappings from its IDs to the outgoing stream's IDs. Returns the traceID of the merged trace
  // in the outgoing stream.
  /*static int mergeIDs(std::map<std::string, std::string>& pMap, 
                       const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);*/
      
  std::string str(std::string indent="") const;
}; // class TraceStreamRecord

}; // namespace structure 
}; // namespace sight
