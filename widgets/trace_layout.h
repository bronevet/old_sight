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

class traceStream: public attrObserver, public common::trace
{

  private:
  // Unique ID of this trace
  int traceID;
  
  // The ID of the block into which the trace visualization will be written
  //std::string tgtBlockID;
  
  public:
  vizT viz;
  
  // Maps the traceIDs of all the currently active traces to their trace objects
  static std::map<int, traceStream*> active;
  
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
 
  private:
  // Set that contains the same attributes as contextAttrs. It is used for quick lookups to ensure that
  // all observations have the same set of context attributes, even for cases where the context is not
  // specified up-front for the entire trace but separately for each observation.
  std::set<std::string> contextAttrsSet;
  
  // Flag that indicates whether this trace's context has already been initialized (it may have 0 keys)
  bool contextAttrsInitialized;
  
  // The div specified by the object that hosts this traceStream. The stream's data visualization will be placed
  // in this div.
  std::string hostDiv;
  
  public:
  traceStream(properties::iterator props, std::string hostDiv);
  
  private:
  void init(std::string label);
  
  public:
  ~traceStream();
  
  private:
  // Place the code to show the visualization
  void showViz();
  
   // [{ctxt:{key0:..., val0:..., key1:..., val1:..., ...}, div:divID}, ...]
  std::list<std::pair<std::list<std::pair<std::string, std::string> >, std::string> > splitCtxtHostDivs;
  public:
  void setSplitCtxtHostDivs(const std::list<std::pair<std::list<std::pair<std::string, std::string> >, std::string> >& splitCtxtHostDivs);
  
  // Records all the observations of trace variables since the last time variables in contextAttrs changed values
  std::map<std::string, std::pair<attrValue, anchor> > obs;
  
  // The keys of all the tracer attributes ever observed
  std::set<std::string> tracerKeys;
  
  public:
  // Record an observation
  static void* observe(properties::iterator props);
    
  // Given a traceID returns a pointer to the corresponding trace object
  static traceStream* get(int traceID);
}; // class streamTrace

}; // namespace layout
}; // namespace sight
