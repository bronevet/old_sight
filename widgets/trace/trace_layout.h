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
#include "../../sight_layout.h"
#include <sys/time.h>

namespace sight {
namespace layout {

class traceLayoutHandlerInstantiator : layoutHandlerInstantiator {
  public:
  traceLayoutHandlerInstantiator();
};
extern traceLayoutHandlerInstantiator traceLayoutHandlerInstance;

class traceStream;

class trace: public block, public common::trace
{
  friend class processedTrace;
  showLocT showLoc;
  traceStream* stream;
  
  // Stack of currently active traces
  static std::list<trace*> tStack;
  
  public:
  trace(properties::iterator props);
  
  private:
  void init(std::string label);
  
  public:
  ~trace();
  
  static void *enterTraceStream(properties::iterator props);
}; // class trace


class processedTrace: public trace
{
  public:
  static void *enterProcessedTraceStream(properties::iterator props);
}; // class processedTrace



// Interface implemented by objects that listen for observations a traceStream reads. traceObservers may be arranged
// as arbitrary graphs of filters, consuming observations from some traceStreams or traceObservers and forwarding 
// the same or other observations (e.g. the sum of multiple observations) to other traceObservers. For simplicity
// traceStreams also implement the traceObserver interface so that they can leverage the common mechanism for
// forwarding observations.
class traceObserver {
  //protected:
public:
  // Set of objects this traceObserver informs of any observations it reads. We map each traceObserver to the number
  // of times it was registered to allow a given observer to get registered multiple times.
  std::map<traceObserver*, int> observers;
  //std::set<traceObserver*> observersS;
  
  // Set of objects being observed by this observer
  std::map<traceObserver*, int> observing;
  
  // Records whether we've notified observers of this trace that it has finished
  bool finishNotified;
  
  public:
    
  traceObserver() { finishNotified=false; }
  virtual ~traceObserver();
  // Called on each observation from the traceObserver this object is observing
  // traceID - unique ID of the trace from which the observation came
  // ctxt - maps the names of the observation's context attributes to string representations of their values
  // obs - maps the names of the trace observation attributes to string representations of their values
  // obsAnchor - maps the names of the trace observation attributes to the anchor that identifies where they were observed
  // follower - if non-NULL points to the traceObserver object to which this observe call should pass on 
  //    the observations it emits.
  virtual void observe(int traceID, 
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor)=0;
/*  { observe(traceID, ctxt, obs, obsAnchor, std::set<traceObserver*>()); }

  virtual void observe(int traceID, 
                       const std::map<std::string, std::string>& ctxt, 
                       const std::map<std::string, std::string>& obs,
                       const std::map<std::string, anchor>&      obsAnchor,
                       const std::set<traceObserver*>&           observers)=0;*/
  
  // Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
  // This method is optional.
  virtual void obsFinished();
  
  // Notifies all observers of this trace that it is finished
  void notifyObsFinish();
  
  // Called from inside observe() to emit an observation that this traceObserver makes.
  void emitObservation(int traceID, 
                       const std::map<std::string, std::string>& ctxt, 
                       const std::map<std::string, std::string>& obs,
                       const std::map<std::string, anchor>&      obsAnchor);/* {
    emitObservation(traceID, ctxt, obs, obsAnchor, observersS);
  }
  
  // Emits an empty observation for cases where the caller wishes that observers are informed that
  // an observation was made but has not information to provide
  / *void emitEmptyObservation(int traceID, const std::set<traceObserver*>& observers) {
    emitObservation(traceID, 
                    std::map<std::string, std::string>(), 
                    std::map<std::string, std::string>(), 
                    std::map<std::string, anchor>(), 
                    observersS);
  }* /
  
  void emitObservation(int traceID, 
                       const std::map<std::string, std::string>& ctxt, 
                       const std::map<std::string, std::string>& obs,
                       const std::map<std::string, anchor>&      obsAnchor,
                       const std::set<traceObserver*>&           observers);*/
  
  // Registers/unregisters a given object as an observer of this traceStream
  virtual void registerObserver(traceObserver* obs);
  virtual void unregisterObserver(traceObserver* obs);

  // Places the given set of traceObservers immediately after this observer and before all of the observers that watch it
  virtual void prependObservers(const std::set<traceObserver*>& newObservers);
  
  // Returns the total number of observers for this object
  int numObservers() const { return observers.size(); }
}; // traceObserver

typedef common::easylist<traceObserver*> traceObservers;

// Maintains a chain of observers, each of which receives observations, filters them and then passes them on to
// observers that follow it in the chain. An observer is not required to pass each observation it receives onwards
// and may send out multiple observations in a single observe() call.
class traceObserverQueue: public traceObserver {
  protected:
  // The first traceObserver in the observers queue
  traceObserver* firstO;
  
  // The last traceObserver in the observers queue
  traceObserver* lastO;
  //std::list<traceObserver*> queue;
  
  public:
  traceObserverQueue();
  traceObserverQueue(const std::list<traceObserver*>& observersL);
  
  // Push a new observer to the back of the observers queue
  void push_back(traceObserver* obs);
  
  // Push a new observer to the front of the observers queue
  void push_front(traceObserver* obs);
  
  // Override the registration methods from traceObserver to add the observers to the back of the queue
  // Registers/unregisters a given object as an observer of this traceStream
  void registerObserver(traceObserver* obs);
  void unregisterObserver(traceObserver* obs);
  
  // traceID - unique ID of the trace from which the observation came
  // ctxt - maps the names of the observation's context attributes to string representations of their values
  // obs - maps the names of the trace observation attributes to string representations of their values
  // obsAnchor - maps the names of the trace observation attributes to the anchor that identifies where they were observed
  void observe(int traceID, 
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor);
  
  // Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
  // This method is optional.
  void obsFinished();
}; // traceObserverQueue

// This is a trace observer that processes incoming observations by writing them into a file, running some
// application on this file and emitting the observations produced by the application.
class externalTraceProcessor_File : public traceObserver {
  // Path of the executable that will be executed on the observations
  std::string processorFName;
  
  // Path name that can be used to store the file that contains the observations.
  std::string obsFName;
  
  // Parameters to pass to the command before and after the path of the observation file
  std::list<std::string> beforeParams;
  std::list<std::string> afterParams;
  
  
  // The file stream to which we write observations. The processor application will read this file.
  std::ofstream traceFile;
  
  // Records whether this trace has finished
  bool finished;
  
  public:  
  externalTraceProcessor_File(std::string processorFName, std::string obsFName);
  externalTraceProcessor_File(std::string processorFName, 
                              const std::list<std::string>& beforeParams, std::string obsFName, const std::list<std::string>& afterParams);
  
  // Interface implemented by objects that listen for observations a traceStream reads. Such objects
  // call traceStream::registerObserver() to inform a given traceStream that it should observations.
  void observe(int traceID,
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor);
  
  // Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
  // This method is optional.
  void obsFinished();
}; // class externalTraceProcessor_File

// This is a trace observer that processes incoming observations by writing them into the given file in
// tab-separated format
class traceFileWriterTSV : public traceObserver {
  // The keys of the context and trace observations. All observations must have identical keys.
  std::set<std::string> ctxtKeys;
  std::set<std::string> traceKeys;
  
  // The file that the observations will be emitted to
  std::ofstream out;
  
  // The number of observations that have been seen
  int numObservations;
  
  public:  
  traceFileWriterTSV(std::string outFName);
  ~traceFileWriterTSV();
  
  // Interface implemented by objects that listen for observations a traceStream reads. Such objects
  // call traceStream::registerObserver() to inform a given traceStream that it should observations.
  void observe(int traceID,
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor);
  
  // Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
  // This method is optional.
  void obsFinished();
}; // class traceFileWriterTSV

class traceStream: public attrObserver, public common::trace, public traceObserver
{
  public:
  typedef common::easylist<std::string> attrNames;
  private:
  // Unique ID of this trace
  int traceID;
  
  // The ID of the block into which the trace visualization will be written
  //std::string tgtBlockID;
  
  public:
  vizT viz;
  
  // Maps the traceIDs of all the currently active traces to their trace objects
  static std::map<int, traceStream*> active;

  private:
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
  
  // Set that contains the same attributes as contextAttrs. It is used for quick lookups to ensure that
  // all observations have the same set of context attributes, even for cases where the context is not
  // specified up-front for the entire trace but separately for each observation.
  std::set<std::string> contextAttrsSet;
  
  // Flag that indicates whether this trace's context has already been initialized (it may have 0 keys)
  bool contextAttrsInitialized;
  
  // The keys of all the tracer attributes ever observed
  std::list<std::string> traceAttrs;
  
  // Set that contains the same attributes as traceAttrs. It is used for quick lookups to ensure that
  // all observations have the same set of Trace attributes, even for cases where they are not
  // specified up-front for the entire trace but separately for each observation.
  std::set<std::string> traceAttrsSet;
  
  // Flag that indicates whether this trace's trace has already been initialized (it may have 0 keys)
  bool traceAttrsInitialized;
  
  // The div specified by the object that hosts this traceStream. The stream's data visualization will be placed
  // in this div.
  std::string hostDiv;
  
  // Indicates whether the trace should be shown by default (true) or whether the host will control
  // when it is shown.
  bool showTrace;
  
  public:
  // hostDiv - the div where the trace data should be displayed
  // showTrace - indicates whether the trace should be shown by default (true) or whether the host will control
  //             when it is shown
  traceStream(properties::iterator props, std::string hostDiv, bool showTrace=true);
  
  private:
  void init(std::string label);
  
  public:
  ~traceStream();
  
  // Given a set of context and trace attributes to visualize, a target div and a visualization type, returns the command 
  // to do this visualization.
  // showFresh: boolean that indicates whether we should overwrite the prior contents of hostDiv (true) or whether we should append
  //      to them (false)
  // showLabels: boolean that indicates whether we should show a label that annotates a data plot (true) or whether
  //      we should just show the plot (false)
  std::string getDisplayJSCmd(const std::list<std::string>& contextAttrs, const std::list<std::string>& traceAttrs,
                              std::string hostDiv="", vizT viz=unknown, 
                              bool showFresh=true, bool showLabels=false, bool refreshView=false);
  
  int getID() const { return traceID; }
  private:
  // Place the code to show the visualization
  void showViz();
  
   // [{ctxt:{key0:..., val0:..., key1:..., val1:..., ...}, div:divID}, ...]
  std::list<std::pair<std::list<std::pair<std::string, std::string> >, std::string> > splitCtxtHostDivs;
  public:
  void setSplitCtxtHostDivs(const std::list<std::pair<std::list<std::pair<std::string, std::string> >, std::string> >& splitCtxtHostDivs);
  
  // Records all the observations of trace variables since the last time variables in contextAttrs changed values
  std::map<std::string, std::pair<attrValue, anchor> > obs;
  
  public:
  // Record an observation
  static void* observe(properties::iterator props);
  
  // Called on each observation from the traceObserver this object is observing
  // traceID - unique ID of the trace from which the observation came
  // ctxt - maps the names of the observation's context attributes to string representations of their values
  // obs - maps the names of the trace observation attributes to string representations of their values
  // obsAnchor - maps the names of the trace observation attributes to the anchor that identifies where they were observed
  
  // Called by any observers of the stream, which may have filtered the raw observation, to inform the traceStream
  // that the given observation should actually be emitted to the output
  void observe(int traceID, 
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor/*,
               const std::set<traceObserver*>&           observers*/);
    
  // Given a traceID returns a pointer to the corresponding trace object
  static traceStream* get(int traceID);
}; // class traceStream

class processedTraceStream: public traceStream
{
  // The directory that is used for storing intermediate files
  static std::string workDir;
  // The maximum unique ID assigned to any file that was used as input to a processor
  static int maxFileID;
  
  // The queue of externalTraceProcessors that filter this traceStream
  traceObserverQueue* queue;
  
  // Pointers to the actual externalTraceProcessors objects in the queue
  std::list<externalTraceProcessor_File*> commandProcessors;
  
  public:
  // hostDiv - the div where the trace data should be displayed
  // showTrace - indicates whether the trace should be shown by default (true) or whether the host will control
  //             when it is shown
  processedTraceStream(properties::iterator props, std::string hostDiv, bool showTrace=true);
  
  private:
  void init(std::string label);
  
  public:
  ~processedTraceStream();
}; // class processedTraceStream

}; // namespace layout
}; // namespace sight
