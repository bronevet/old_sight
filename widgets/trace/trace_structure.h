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
#include "../../sight_common.h"
#include "../../sight_structure_internal.h"
#include <sys/time.h>
#include <papi.h>
#ifdef RAPL
#include "msr_core.h"
#endif

namespace sight {
namespace structure {

class trace;
// Syntactic sugar for specifying anchors to observation sites
//typedef common::easylist<anchor> obsAnchors;

class traceConfHandlerInstantiator : common::confHandlerInstantiator {
  public:
  traceConfHandlerInstantiator();
};
extern traceConfHandlerInstantiator traceConfHandlerInstance;

class traceTraceStream;

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
  //typedef common::easylist<std::pair<std::string, attrValue> > observation;
  
  // Syntactic sugar for specifying observations
  class observation : public std::list<std::pair<std::string, attrValue> > {
    public:
    observation() {}

    observation(const std::string& key0, const attrValue& val0)
    { this->push_back(make_pair(key0, val0)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); this->push_back(make_pair(key2, val2)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); this->push_back(make_pair(key2, val2)); this->push_back(make_pair(key3, val3)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); this->push_back(make_pair(key2, val2)); this->push_back(make_pair(key3, val3)); this->push_back(make_pair(key4, val4)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); this->push_back(make_pair(key2, val2)); this->push_back(make_pair(key3, val3)); this->push_back(make_pair(key4, val4)); this->push_back(make_pair(key5, val5)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); this->push_back(make_pair(key2, val2)); this->push_back(make_pair(key3, val3)); this->push_back(make_pair(key4, val4)); this->push_back(make_pair(key5, val5)); this->push_back(make_pair(key6, val6)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); this->push_back(make_pair(key2, val2)); this->push_back(make_pair(key3, val3)); this->push_back(make_pair(key4, val4)); this->push_back(make_pair(key5, val5)); this->push_back(make_pair(key6, val6)); this->push_back(make_pair(key7, val7)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7, const std::string& key8, const attrValue& val8)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); this->push_back(make_pair(key2, val2)); this->push_back(make_pair(key3, val3)); this->push_back(make_pair(key4, val4)); this->push_back(make_pair(key5, val5)); this->push_back(make_pair(key6, val6)); this->push_back(make_pair(key7, val7)); this->push_back(make_pair(key8, val8)); }

    observation(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7, const std::string& key8, const attrValue& val8, const std::string& key9, const attrValue& val9)
    { this->push_back(make_pair(key0, val0)); this->push_back(make_pair(key1, val1)); this->push_back(make_pair(key2, val2)); this->push_back(make_pair(key3, val3)); this->push_back(make_pair(key4, val4)); this->push_back(make_pair(key5, val5)); this->push_back(make_pair(key6, val6)); this->push_back(make_pair(key7, val7)); this->push_back(make_pair(key8, val8)); this->push_back(make_pair(key9, val9)); }
  }; // class observation
    
  protected:
  traceStream* stream;
  
  // Maps the names of all the currently active traces to their trace objects
  static std::map<std::string, trace*> active;
  
  public:
  trace(std::string label, const std::list<std::string>& contextAttrs,                        showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  trace(std::string label, std::string contextAttr,                                           showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  trace(std::string label,                                                                    showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  trace(std::string label, const std::list<std::string>& contextAttrs, const attrOp& onoffOp, showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  trace(std::string label, std::string contextAttr,                    const attrOp& onoffOp, showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  trace(std::string label,                                             const attrOp& onoffOp, showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  
  ~trace();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  // Sets the properties of this object
  static properties* setProperties(const attrOp* onoffOp, showLocT showLoc, properties* props);
  
  void init(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz, mergeT merge, properties* props);
  
  static trace*       getT (std::string label);
  static traceStream* getTS(std::string label);
  traceStream* getTS() const { return stream; }
}; // class trace

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

// Traces where observations are filtered through a sequence of one or more external processing tools,
// the paths to which are provided by the user
class processedTrace: public trace
{
  public:
  typedef common::easylist<std::string> commands;
  
  processedTrace(std::string label, const std::list<std::string>& contextAttrs, const std::list<std::string>& processorCommands,                        showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  processedTrace(std::string label, std::string contextAttr,                    const std::list<std::string>& processorCommands,                        showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  processedTrace(std::string label,                                             const std::list<std::string>& processorCommands,                        showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  processedTrace(std::string label, const std::list<std::string>& contextAttrs, const std::list<std::string>& processorCommands, const attrOp& onoffOp, showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  processedTrace(std::string label, std::string contextAttr,                    const std::list<std::string>& processorCommands, const attrOp& onoffOp, showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);
  processedTrace(std::string label,                                             const std::list<std::string>& processorCommands, const attrOp& onoffOp, showLocT showLoc=showBegin, vizT viz=table, mergeT merge=disjMerge, properties* props=NULL);  
  
  properties* setProperties(const attrOp* onoffOp, const std::list<std::string>& processorCommands, properties* props);
  
  void init(std::string label, const std::list<std::string>& contextAttrs, const std::list<std::string>& processorCommands, vizT viz, mergeT merge, properties* props);

  ~processedTrace();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
}; // class processedTrace


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
  // Callers can optionally provide a traceID that this traceStream will use. This is useful for cases where 
  // the ID of the trace used within a given host object needs to be known before the traceStream is actually
  // created. In this case the host calls genTraceID(), and then ultimately passes it into the traceStream constructor.
  // It can be set to a negative value (or omitted) to indicate that the traceStream should generate an ID on its own.
  traceStream(const std::list<std::string>& contextAttrs, vizT viz=table, mergeT merge=disjMerge, int traceID=-1, properties* props=NULL);
  traceStream(std::string contextAttr,                    vizT viz=table, mergeT merge=disjMerge, int traceID=-1, properties* props=NULL);
  traceStream(                                            vizT viz=table, mergeT merge=disjMerge, int traceID=-1, properties* props=NULL);
  
  // Sets the properties of this object
  properties* setProperties(const std::list<std::string>& contextAttrs, vizT viz, mergeT merge, int traceID, properties* props);

  // Generates a fresh traceID and returns it
  static int genTraceID();
  
  private:  
  void init(int traceID);
  
  public:
  ~traceStream();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
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
  
//  private:
  
  // Emits the output record records the given context and observations pairing
  void emitObservations(const std::list<std::string>& contextAttrs, 
                        std::map<std::string, std::pair<attrValue, anchor> >& obs);
  
  // Emits the output record records the given context and observations pairing
  void emitObservations(const std::map<std::string, attrValue>& contextAttrsMap, 
                        std::map<std::string, std::pair<attrValue, anchor> >& obs);
}; // class traceStream

class processedTraceStream: public traceStream
{    
  private:
  // The paths of the executables that will be run on the trace observations
  std::list<std::string> processorCommands;
  
  public:
  // Callers can optionally provide a traceID that this processedTraceStream will use. This is useful for cases where 
  // the ID of the trace used within a given host object needs to be known before the processedTraceStream is actually
  // created. In this case the host calls genTraceID(), and then ultimately passes it into the processedTraceStream constructor.
  // It can be set to a negative value (or omitted) to indicate that the processedTraceStream should generate an ID on its own.
  processedTraceStream(const std::list<std::string>& contextAttrs, const std::list<std::string>& processorCommands, vizT viz=table, mergeT merge=disjMerge, int traceID=-1, properties* props=NULL);
  processedTraceStream(std::string contextAttr,                    const std::list<std::string>& processorCommands, vizT viz=table, mergeT merge=disjMerge, int traceID=-1, properties* props=NULL);
  processedTraceStream(                                            const std::list<std::string>& processorCommands, vizT viz=table, mergeT merge=disjMerge, int traceID=-1, properties* props=NULL);
  
  // Sets the properties of this object
  properties* setProperties(const std::list<std::string>& processorCommands, properties* props);

  ~processedTraceStream();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
}; // class processedTraceStream

// Basic API for measuring the elapsed counts of events.
// After an instance of the measure class is constructed, the measurement may be started by calling start() 
//    and completed by calling end() or endGet(). The measurement can also be paused and resumed. When a measurement is 
//    completed via end(), the observation is added to the trace associated with the measurement using the specified
//    context, and associated with the label(s) specified for the measurement (each class derived from measure may use
//    a separate labeling scheme since a single measurement may produce multiple observations). Alternately, the 
//    measurement can also be completed using endGet(), which returns the measurement as a map from value names 
//    to the attrValues that hold the measured values, and optionally adds the observations to the trace if the caller
//    specifically asks for this. If the caller does not wish the measure to add the observations to its trace, it does 
//    not need to specify the trace and context in the measurement's constructor (the value label is needed) and the 
//    caller is responsible for adding the measurement to a trace if it needs to.
// More specifics for cases where the measurement automatically adds the observation to its trace:
//    This trace can be specified by providing its string label, a pointer to the trace object, or a pointer to the 
//    traceStream object inside it (traceStreams may be used inside traces as well as other widgets). Then, when the 
//    measurement completes the measure automatically adds an observation to the given trace under the label valLabel, 
//    which must also be provided when creating the measure. The context for this measurement will be the current 
//    values of the context attributes of the trace. If the user wishes to specify context manually, they may optionally 
//    provide them as a map from their string lavels to their attrValues. All of the above may be specified in the 
//    measure constructor or afterwards by calling setTrace(), setValLabel() and setFullMeasureCtxt(), as long as 
//    these are called before the call to end().
// The startMeasure()/endMeasure() API makes it easy to make measurements:
//    measurement* m = startMeasure(...);
//    ...
//    endMeasure(m);

class measure {
  protected:
  traceStream* ts;
  
  // Records whether time collection is currently paused
  bool paused;
  
  // Records whether we've already performed the measure
  bool ended;
  
  // Records whether the measurement will be a full measure, in the sense that when the measurement by itself
  // represents all the information that will ever be recorded for a given context rather than trickle in 
  // observation-by-observation.
  bool fullMeasure;
  // If we're doing a full measure, this is the context that will be used for this measurement
  std::map<std::string, attrValue> fullMeasureCtxt;
  
  public:
  measure();

  // Non-full measure                                                                                            
  //measure(                      );
  measure(std::string traceLabel);
  measure(trace* t);
  measure(traceStream* ts);
  // Full measure                                                                                                
  measure(                        const std::map<std::string, attrValue>& fullMeasureCtxt);
  measure(std::string traceLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  measure(trace* t,               const std::map<std::string, attrValue>& fullMeasureCtxt);
  measure(traceStream* ts,        const std::map<std::string, attrValue>& fullMeasureCtxt);

  measure(const measure& that);
  
  virtual ~measure();
  
  // Returns a copy of this measure object, including its current measurement state, if any. The returned
  // object is connected to the same traceStream, if any, as the original object.
  virtual measure* copy() const=0;
  
  // Specify the trace that is associated with this measure object
  void setTrace(std::string traceLabel);
  void setTrace(trace* t);
  void setTrace(traceStream* ts);
  
  // Specify the label of the value measured by this measure object
  //void setValLabel(std::string valLabel);
  
  // Specify the full context of this object's measurement
  void setCtxt(const std::map<std::string, attrValue>& fullMeasureCtxt);
  
  private:
  // Common initialization code
  void init();
  
  public:
  // Start the measurement
  virtual void start();
   
  // Pauses the measurement so that time elapsed between this call and resume() is not counted.
  // Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
  virtual bool pause();

  // Restarts counting time. Time collection is restarted regardless of how many times pause() was called
  // before the call to resume().
  virtual void resume();

  // Complete the measurement and add the observation to the trace associated with this measurement
  virtual void end();
  
  // Complete the measurement and return the observation.
  // If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
  virtual std::list<std::pair<std::string, attrValue> > endGet(bool addToTrace=false);
  
  virtual std::string str() const;
}; // class measure

// Syntactic sugar for specifying measurements
typedef common::easylist<measure*> measures;
typedef common::easymap<std::string, measure*> namedMeasures;

class timeMeasure : public measure {
  // Counts the total time elapsed so far, accounting for any pauses and resumes
  double elapsed;
  // The time when we started or resumed this measure, whichever is most recent
  struct timeval lastStart;
  
  // The label associated with this measurement
  std::string valLabel;
  
  public:
  // Non-full measure
  timeMeasure(                        std::string valLabel="time");
  timeMeasure(std::string traceLabel, std::string valLabel);
  timeMeasure(trace* t,               std::string valLabel="time");
  timeMeasure(traceStream* ts,        std::string valLabel="time");
  // Full measure
  timeMeasure(                        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  timeMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  timeMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  timeMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  
  timeMeasure(const timeMeasure& that);
  
  ~timeMeasure();
 
  private:
  // Common initialization code
  void init();   
          
  public:
  
  // Returns a copy of this measure object, including its current measurement state, if any. The returned
  // object is connected to the same traceStream, if any, as the original object.
  measure* copy() const;
  
  // Start the measurement
  void start();
   
  // Pauses the measurement so that time elapsed between this call and resume() is not counted.
  // Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
  bool pause();

  // Restarts counting time. Time collection is restarted regardless of how many times pause() was called
  // before the call to resume().
  void resume();
 
  // Complete the measurement and add the observation to the trace associated with this measurement
  void end();
  
  // Complete the measurement and return the observation.
  // If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
  std::list<std::pair<std::string, attrValue> > endGet(bool addToTrace=false);
  
  std::string str() const;
}; // class timeMeasure

// Measures the absolute time when measurement started and stopped, ignoring
// any pauses in measurement
class timeStampMeasure : public measure {
  // The time when we started and ended this measure since the Epoch, seconds
  double startTime;
  double endTime;
  
  // The label associated with this measurement
  std::string valLabel;
  
  public:
  // Non-full measure
  timeStampMeasure(                        std::string valLabel="timestamp");
  timeStampMeasure(std::string traceLabel, std::string valLabel);
  timeStampMeasure(trace* t,               std::string valLabel="timestamp");
  timeStampMeasure(traceStream* ts,        std::string valLabel="timestamp");
  // Full measure
  timeStampMeasure(                        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  timeStampMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  timeStampMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  timeStampMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  
  timeStampMeasure(const timeStampMeasure& that);
  
  ~timeStampMeasure();
 
  private:
  // Common initialization code
  void init();   
          
  public:
  
  // Returns a copy of this measure object, including its current measurement state, if any. The returned
  // object is connected to the same traceStream, if any, as the original object.
  measure* copy() const;
  
  // Start the measurement
  void start();
   
  // Pauses the measurement so that time elapsed between this call and resume() is not counted.
  // Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
  bool pause();

  // Restarts counting time. Time collection is restarted regardless of how many times pause() was called
  // before the call to resume().
  void resume();
 
  // Complete the measurement and add the observation to the trace associated with this measurement
  void end();
  
  // Complete the measurement and return the observation.
  // If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
  std::list<std::pair<std::string, attrValue> > endGet(bool addToTrace=false);
  
  std::string str() const;
}; // class timeStampMeasure

// Syntactic sugar for specifying measurements
typedef common::easyvector<int> papiEvents;

// List of standard derived counters the user may request. Since PAPI numbers its counters
// from 2^31-1 down, we set these #defines to count from 0 up
#define PAPI_MIN_DERIVED 0 // The minimum value of any derived counter
#define PAPI_L1_TC_MR 0 // L1 Total cache miss rate
#define PAPI_L2_TC_MR 1 // L2 Total cache miss rate
#define PAPI_L3_TC_MR 2 // L3 Total cache miss rate
#define PAPI_L1_DC_MR 3 // L1 Data cache miss rate
#define PAPI_L2_DC_MR 4 // L2 Data cache miss rate
#define PAPI_L3_DC_MR 5 // L3 Data cache miss rate
#define PAPI_L1_IC_MR 6 // L1 Instruction cache miss rate
#define PAPI_L2_IC_MR 7 // L2 Instruction cache miss rate
#define PAPI_L3_IC_MR 8 // L3 Instruction cache miss rate
#define PAPI_MAX_DERIVED 8 // The maximum value of any derived counter

class PAPIMeasure : public measure {
  // Counts the total number of counter events observed so far, accounting for any pauses and resumes
  std::vector<long_long> accumValues;
  
  // The values of the counters recorded when measurement last started or restarted
  std::vector<long_long> lastValues;
  
  // Buffer into which we'll read counters
  std::vector<long_long> readValues;
  
  // The events that the user wishes to measure. These may include both raw PAPI counters and derived
  // counters.
  papiEvents events;

  // The raw PAPI events that are actually being measured.
  std::vector<int> eventsToMeasure;

  // Maps from the PAPI IDs of the raw counters in eventsToMeasure, to their indexes in eventsToMeasure
  std::map<int, int> events2Idx;

  // The label associated with this measurement
  std::string valLabel;
  
  // Indicates the number of PAPIMeasure objects that are currently measuring the counters
  static int numMeasurers;
  
  // Records the set of PAPI counters currently being measured (non-empty iff numMeasurers>0)
  static std::vector<int> curMeasuredEvents;

  // Records whether we were able to successfully start PAPI counting of the selected counters
  bool PAPIOperational;
  
  public:
  PAPIMeasure(const papiEvents& events);
  
  // Non-full measure
  PAPIMeasure(                        std::string valLabel, const papiEvents& events);
  PAPIMeasure(std::string traceLabel, std::string valLabel, const papiEvents& events);
  PAPIMeasure(trace* t,               std::string valLabel, const papiEvents& events);
  PAPIMeasure(traceStream* ts,        std::string valLabel, const papiEvents& events);
  // Full measure
  PAPIMeasure(                        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt, const papiEvents& events);
  PAPIMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt, const papiEvents& events);
  PAPIMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt, const papiEvents& events);
  PAPIMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt, const papiEvents& events);
  
  PAPIMeasure(const PAPIMeasure& that);
  
  ~PAPIMeasure();
 
  private:
  // Common initialization code
  void init();   
          
  public:
  // Returns a copy of this measure object, including its current measurement state, if any
  measure* copy() const;
    
  // Start the measurement
  void start();
   
  // Pauses the measurement so that time elapsed between this call and resume() is not counted.
  // Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
  bool pause();

  // Restarts counting time. Time collection is restarted regardless of how many times pause() was called
  // before the call to resume().
  void resume();
 
  // Returns the string names of the counters currently being measured
  std::list<std::string> getMeasuredCounterNames();
  
  // Returns a list that maps all the raw and derived counters in events to their observed values,
  // using the raw counter values in accumValues.
  std::list<std::pair<std::string, attrValue> > getAccumValues();
  
  // Complete the measurement and add the observation to the trace associated with this measurement
  void end();
  
  // Complete the measurement and return the observation.
  // If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
  std::list<std::pair<std::string, attrValue> > endGet(bool addToTrace=false);
  
  std::string str() const;
}; // class PAPIMeasure

#ifdef RAPL
#include "msr_rapl.h"

// Base class of measurement objects that measure MSRs 
class MSRMeasure {
  protected:
  static int numMeasurers;
  MSRMeasure();
  ~MSRMeasure();
};

// Measurement class for RAPL counters, which report power draw of the CPU and other sub-systems
class RAPLMeasure : public measure, public MSRMeasure {
  // Records the accumulated energy use and the amount of time over which it was used
  class Euse {
    public:
    double E;
    double T;

    Euse() : E(0), T(0) {}
    Euse(const Euse& that) : E(that.E), T(that.T) {}
    void add(double moreE, double moreT) {
      E+=moreE;
      T+=moreT;
    }
    double getPower() {
      if(T==0) return 0;
      else return E/T;
    }
  };

  // The internal state of the RAPL measurement library, specific to this RAPLMeasure instance
  //struct rapl_int_state raplS;

  // Records the accumulated energy use in the CPU and DRAM
  Euse accumCpuE[NUM_SOCKETS];
  Euse accumDramE[NUM_SOCKETS];

  // The data structure that maintains the current state of the RAPL counters
  struct rapl_data rapl[NUM_SOCKETS];
  
  // The label associated with this measurement
  std::string valLabel;
  
  
  public:
  RAPLMeasure();
  
  // Non-full measure
  RAPLMeasure(                        std::string valLabel);
  RAPLMeasure(std::string traceLabel, std::string valLabel);
  RAPLMeasure(trace* t,               std::string valLabel);
  RAPLMeasure(traceStream* ts,        std::string valLabel);
  // Full measure
  RAPLMeasure(                        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  RAPLMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  RAPLMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  RAPLMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt);
  
  RAPLMeasure(const RAPLMeasure& that);
  
  ~RAPLMeasure();
 
  private:
  // Common initialization code
  void init();   
          
  public:
  // Returns a copy of this measure object, including its current measurement state, if any
  measure* copy() const;
    
  // Start the measurement
  void start();
   
  // Pauses the measurement so that time elapsed between this call and resume() is not counted.
  // Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
  bool pause();

  // Restarts counting time. Time collection is restarted regardless of how many times pause() was called
  // before the call to resume().
  void resume();
 
  // Complete the measurement and add the observation to the trace associated with this measurement
  void end();
  
  // Complete the measurement and return the observation.
  // If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
  std::list<std::pair<std::string, attrValue> > endGet(bool addToTrace=false);
  
  std::string str() const;

  // -------------------------
  // ----- Configuration -----
  // -------------------------
  public:
  static common::Configuration* configure(properties::iterator props);
}; // class RAPLMeasure
#endif // RAPL

// Non-full measure
template<class MT>
MT* startMeasure(std::string traceLabel, std::string valLabel) {
  MT* m = new MT(traceLabel, valLabel);
  m->start();
  return m;
}

template<class MT>
MT* startMeasure(trace* t,               std::string valLabel) {
  MT* m = new MT(t, valLabel);
  m->start();
  return m;
}

template<class MT>
MT* startMeasure(traceStream* ts,        std::string valLabel) {
  MT* m = new MT(ts, valLabel);
  m->start();
  return m;
}

template<class MT>
MT* startMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) {
  MT* m = new MT(traceLabel, valLabel, fullMeasureCtxt);
  m->start();
  return m;
}

template<class MT>
MT* startMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) {
  MT* m = new MT(t, valLabel, fullMeasureCtxt);
  m->start();
  return m;
}

template<class MT>
MT* startMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) {
  MT* m = new MT(ts, valLabel, fullMeasureCtxt);
  m->start();
  return m;
}

void endMeasure(measure* m);

//template<class MT, class RET>
// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > endGetMeasure(measure* m, bool addToTrace=false);/* {
  MT* mt = dynamic_cast<MT*>(m);
  assert(mt);
  RET result = mt->endGet();
  delete m;
  return result;
}

template<class MT, class RET>
RET endGetMeasure(MT* m) {
  RET result = m->endGet();
  delete m;
  return result;
}*/


class TraceMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  TraceMergeHandlerInstantiator();
};
extern TraceMergeHandlerInstantiator TraceMergeHandlerInstance;

std::map<std::string, streamRecord*> TraceGetMergeStreamRecord(int streamID);


class TraceMerger : public BlockMerger {
  public:
  TraceMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
            
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new TraceMerger(tags, outStreamRecords, inStreamRecords, props); }
              
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
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class TraceMerger

class TraceStreamMerger : public Merger {
  public:
  TraceStreamMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
       
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new TraceStreamMerger(tags, outStreamRecords, inStreamRecords, props); }
              
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
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class TraceStreamMerger

class ProcessedTraceStreamMerger : public TraceMerger {
  public:
  ProcessedTraceStreamMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
            
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ProcessedTraceStreamMerger(tags, outStreamRecords, inStreamRecords, props); }
              
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
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class ProcessedTraceStreamMerger

class TraceObsMerger : public Merger {
  public:
  TraceObsMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new TraceObsMerger(tags, outStreamRecords, inStreamRecords, props); }

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
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

} // namespace structure 
} // namespace sight
