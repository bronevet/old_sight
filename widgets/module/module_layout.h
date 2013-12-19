#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../sight_common.h"
#include "../../sight_layout.h"
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

// Records the information of a given module when the module is entered so that we have it available 
// when the module is exited
class module {
  public:
  std::string moduleName;
  int moduleID;
  int numInputs;
  int numOutputs;
  int count;
  
  module(const std::string& moduleName, int moduleID, int numInputs, int numOutputs, int count) :
    moduleName(moduleName), moduleID(moduleID), numInputs(numInputs), numOutputs(numOutputs), count(count)
  {}
};

class modularApp: public block, public common::module, public traceObserver
{
  friend class moduleNodeTraceStream;
  protected:
 
  // Points to the currently active instance of modularApp. There can be only one.
  static modularApp* activeMA;
    
  // The path the directory where output files of the graph widget are stored
  // Relative to current path
  static std::string outDir;
  // Relative to root of HTML document
  static std::string htmlOutDir;
    
  // Name of this modular app
  std::string appName;
  
  // Unique ID of this modular app
  int appID;
  
  // Stack of the modules that are currently in scope within this modularApp
  std::list<sight::layout::module> mStack;
  
  // Maps each module group's ID to the trace that holds the observations performed within it
  std::map<int, traceStream*> moduleTraces;
    
  // Maps each traceStream's ID to the ID of its corresponding module graph node
  std::map<int, int> trace2moduleID;
  
  // The dot file that will hold the representation of the module interaction graph
  std::ofstream dotFile;
  
  public:
  
  modularApp(properties::iterator props);
  ~modularApp();
  
  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();
  
  static void *enterTraceStream(properties::iterator props);
  
  // Registers the ID of the trace that is associated with the current module
  //void registerTraceID(int traceID);
  //static void* registerTraceID(properties::iterator props);
  
  // Do a multi-variate polynomial fit of the data observed for the given nodeID and return for each trace attribute 
  // a string that describes the function that best fits its values
  std::vector<std::string> polyFit(int moduleID);
  
  // Emits to the dot file the buttons used to select the combination of input property and trace attribute
  // that should be shown in the data panel of a given module node.
  // numInputs/numOutputs - the number of inputs/outputs of this module node
  // ID - the unique ID of this module node
  // prefix - We measure both the observations of measurements during the execution of modules and the 
  //    properties of module outputs. Both are included in the trace of the module but the names of measurements
  //    are prefixed with "measure:" and the names of outputs are prefixed with "output:#:", where the # is the
  //    index of the output. The prefix argument identifies the types of attributs we'll be making buttons for and
  //    it should be either "module" or "output".
  // bgColor - The background color of the buttons
  void showButtons(int numInputs, int numOutputs, int ID, std::string prefix, std::string bgColor);
  
  // Enter a new module within the current modularApp
  // numInputs/numOutputs - the number of inputs/outputs of this module node
  // ID - the unique ID of this module node
  void enterModule(std::string node, int moduleID, int numInputs, int numOutputs, int count);
  // Static version of enterModule() that pulls the from/to anchor IDs from the properties iterator and calls 
  // enterModule() in the currently active modularApp
  static void* enterModule(properties::iterator props);
  
  // Exit a module within the current modularApp
  void exitModule();
  // Static version of enterModule() that calls exitModule() in the currently active modularApp
  static void exitModule(void* obj);
  
  // Add a directed edge from the location of the from anchor to the location of the to anchor
  void addEdge(int fromC, common::module::ioT fromT, int fromP, 
               int toC,   common::module::ioT toT,   int toP,
               double prob);
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
    
  // The number of observations for which we've allocated space in polyfitCtxt and polyfitObs (the rows)
  std::map<int, int> numAllocObs;
  
  // The number of trace attributes for which we've allocated space in newObs (the columns)
  std::map<int, int> numAllocTraceAttrs;
  
  // Maps the names of trace attributes to their columns in polyfitCtxt
  std::map<int, std::map<std::string, int > > traceAttrName2Col;
  
  // Records the number of observations we've made of each trace attribute (indexed according to the column numbers in traceAttrName2Col)
  std::map<int, std::vector<int> > traceAttrName2Count;
  
  // The number of numeric context attributes of each node. Should be the same for all observations for the node
  std::map<int, int> numNumericCtxt;
  std::map<int, std::list<std::string> > numericCtxtNames;
    
  // For each node, for each input, the names of its context attributes
  std::map<int, std::map<int, std::list<std::string> > > ctxtNames;
    
  // The names of the observation trace attributes
  std::map<int, std::set<std::string> > traceAttrNames;
  
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
