#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../sight_common.h"
#include "../sight_layout.h"
#include "module_common.h"
#include <gsl/gsl_multifit.h>

namespace sight {
namespace layout {

class moduleLayoutHandlerInstantiator : layoutHandlerInstantiator {
  public:
  moduleLayoutHandlerInstantiator();
};
extern moduleLayoutHandlerInstantiator moduleLayoutHandlerInstance;

class moduleNodeTraceStream;

class module: public block, public common::module, public traceObserver
{
  friend class moduleNodeTraceStream;
  protected:
  
  // The path the directory where output files of the graph widget are stored
  // Relative to current path
  static std::string outDir;
  // Relative to root of HTML document
  static std::string htmlOutDir;
    
  // Unique ID of this module object
  int moduleID;
  
  // The names of all the modules
  std::set<std::string> moduleNames;
  
  // Stack of the modules that are currently in scope
  static std::list<module*> mStack;
  
  // Maps each module group's ID to the trace that holds the observations performed within it
  std::map<int, traceStream*> moduleTraces;
    
  // Maps each traceStream's ID to the ID of its corresponding module graph node
  std::map<int, int> trace2nodeID;
  
  // The dot file that will hold the representation of the module interaction graph
  std::ofstream dotFile;
  
  public:
  
  module(properties::iterator props);
  ~module();
  
  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();
  
  static void *enterTraceStream(properties::iterator props);
  
  // Registers the ID of the trace that is associated with the current module
  //void registerTraceID(int traceID);
  //static void* registerTraceID(properties::iterator props);
  
  // Do a multi-variate polynomial fit of the data observed for the given nodeID and return for each trace attribute 
  // a string that describes the function that best fits its values
  std::vector<std::string> polyFit(int nodeID);
  
  // Add a node to the modules graph
  void addNode(std::string node, int numInputs, int numOutputs, int ID, int count/*, const std::set<std::string>& nodeContexts*/);
  static void* addNode(properties::iterator props);
  
  // Add a directed edge from the location of the from anchor to the location of the to anchor
  void addEdge(int fromC, common::module::ioT fromT, int fromP, 
               int toC,   common::module::ioT toT,   int toP);
  static void* addEdge(properties::iterator props);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return false; }
  bool subBlockExitNotify (block* subBlock) { return false; }
  
  protected:
  // Maps each nodeID to the data needed to compute a polynomial approximation of the relationship
  // between its input context and its observations
  
  // Matrix of polynomial terms composed of context values, 1 row per observation, 1 column for each combination of terms
  std::map<int, gsl_matrix*> polyfitCtxt;
  
  // For each value that is observed, a vector of the values actually observed, one entry per observation
  //std::map<int, std::vector<gsl_vector*> >  polyfitObs;
  std::map<int, gsl_matrix*> polyfitObs;
    
  // The number of observations made for each node
  std::map<int, int> numObs;
    
  // The number of observations for which we've allocated space in polyfitCtxt and polyfitObs
  std::map<int, int> numAllocObs;
  
  // The number of numeric context attributes of each node. Should be the same for all observations for the node
  std::map<int, int> numNumericCtxt;
  std::map<int, std::list<std::string> > numericCtxtNames;
    
  // For each node, for each input, the names of its context attributes
  std::map<int, std::map<int, std::list<std::string> > > ctxtNames;
    
  // The names of the observation trace attributes
  std::map<int, std::vector<std::string> > traceAttrNames;
  
  public:
  // Interface implemented by objects that listen for observations a traceStream reads. Such objects
  // call traceStream::registerObserver() to inform a given traceStream that it should observations.
  void observe(int traceID,
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor);
}; // class module

// Specialization of traceStreams for the case where they are hosted by a module node
class moduleNodeTraceStream: public traceStream
{
  public:
  moduleNodeTraceStream(properties::iterator props);
};


}; // namespace layout
}; // namespace sight
