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

// Interface implemented by objects that listen for observations a traceStream reads. Such objects
// call traceStream::registerObserver() to inform a given traceStream that it should observations.
class traceObserver {
  public:
  virtual void observe(int traceID, 
                       const std::map<std::string, std::string>& ctxt, 
                       const std::map<std::string, std::string>& obs,
                       const std::map<std::string, anchor>&      obsAnchor)=0;
}; 

class traceStream: public attrObserver, public common::trace
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
    
  // Set of objects this traceStream informs of any observations it reads
  std::set<traceObserver*> observers;
  
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
                              std::string hostDiv="", vizT viz=unknown, bool showFresh=true, bool showLabels=false);
  
  int getID() const { return traceID; }
  private:
  // Place the code to show the visualization
  void showViz();
  
   // [{ctxt:{key0:..., val0:..., key1:..., val1:..., ...}, div:divID}, ...]
  std::list<std::pair<std::list<std::pair<std::string, std::string> >, std::string> > splitCtxtHostDivs;
  public:
  void setSplitCtxtHostDivs(const std::list<std::pair<std::list<std::pair<std::string, std::string> >, std::string> >& splitCtxtHostDivs);
  
  // Registers/unregisters a given object as an observer of this traceStream
  void registerObserver(traceObserver* obs) { observers.insert(obs); }
  void unregisterObserver(traceObserver* obs) { assert(observers.find(obs) != observers.end()); observers.erase(obs); }
  
  // Records all the observations of trace variables since the last time variables in contextAttrs changed values
  std::map<std::string, std::pair<attrValue, anchor> > obs;
  
  public:
  // Record an observation
  static void* observe(properties::iterator props);
    
  // Given a traceID returns a pointer to the corresponding trace object
  static traceStream* get(int traceID);
}; // class streamTrace

}; // namespace layout
}; // namespace sight
