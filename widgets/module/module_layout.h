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

class moduleConfHandlerInstantiator : common::confHandlerInstantiator {
  public:
  moduleConfHandlerInstantiator();
};
extern moduleConfHandlerInstantiator moduleConfHandlerInstance;

class moduleTraceStream;

// Records the information of a given module when the module is entered so that we have it available 
// when the module is exited
class moduleInfo {
  public:
  std::string moduleName;
  int moduleID;
  int numInputs;
  int numOutputs;
  int count;
  
  moduleInfo(const std::string& moduleName, int moduleID, int numInputs, int numOutputs, int count) :
    moduleName(moduleName), moduleID(moduleID), numInputs(numInputs), numOutputs(numOutputs), count(count)
  {}
};

// Records the information of a given module when the module is entered so that we have it available 
// when the module is exited
class module : public common::module, public traceObserver {
  friend class modularApp;
  // Maps each moduleID to the data needed to compute a polynomial approximation of the relationship
  // between its input context and its observations
  /*
  // Matrix of polynomial terms composed of context values, 1 row per observation, 1 column for each combination of terms
  gsl_matrix* polyfitCtxt;
  
  // For each value that is observed, a vector of the values actually observed, one entry per observation
  //std::map<int, std::vector<gsl_vector*> >  polyfitObs;
  gsl_matrix* polyfitObs;*/
    
  // The number of observations made for each node
  int numObs;
    
  /* // The number of observations for which we've allocated space in polyfitCtxt and polyfitObs (the rows)
  int numAllocObs;
  
  // The number of trace attributes for which we've allocated space in newObs (the columns)
  int numAllocTraceAttrs;
  
  // Maps the names of trace attributes to their columns in polyfitCtxt
  std::map<std::string, int > traceAttrName2Col;
  
  // Records the number of observations we've made of each trace attribute (indexed according to the column numbers in traceAttrName2Col)
  std::vector<int> traceAttrName2Count;
  
  // The number of numeric context attributes of each node. Should be the same for all observations for the node
  int numNumericCtxt;
  std::list<std::string> numericCtxtNames;*/
    
  /* // For each node, for each input, the names of its context attributes
  std::map<int, std::map<int, std::list<std::string> > > ctxtNames;*/
  // For each node, for each grouping of context attributes, the names of all the attributes within the grouping
  std::map<std::string, std::list<std::string> > ctxtNames;
  //std::map<std::string, std::set<std::string> > ctxtNames;
    
  // The names of the observation trace attributes
  std::set<std::string> traceAttrNames;

  int moduleID;
  
  public:
  module(int moduleID);
  
  ~module();
  
  // Do a multi-variate polynomial fit of the data observed for the given moduleID and return for each trace attribute 
  // a string that describes the function that best fits its values
  //std::vector<std::string> polyFit();
  
  // Interface implemented by objects that listen for observations a traceStream reads. Such objects
  // call traceStream::registerObserver() to inform a given traceStream that it should observations.
  void observe(int traceID,
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor/*,
               const std::set<traceObserver*>&           observers*/);
}; // class module

// Observation filter that finds a polynomial fit of the numeric features of the observed data
class polyFitFilter : public traceObserver {
  private:
  // The total number of polyFitFilter instances that have been created so far. 
  // Used to set unique names to files output by polyFitFilters.
  static int maxFileNum;
  int fileNum;
  
  // Records whether the working directory for this class has been initialized
  static bool workDirInitialized;
  
  // The directory that is used for storing intermediate files
  static std::string workDir;
    
  // Maps each numeric trace attribute's name to the file that holds its observations
  //std::map<std::string, std::ofstream*> outFiles;
  
  // Stores the configuration info for the fit algorithm
  std::ofstream cfgFile;
    
  // Maps each numeric trace attribute's name to the processor that will analyze it
  //std::map<std::string, externalTraceProcessor_File*> outProcessors;
  
  // The numeric context attributes of each node.
  // This is the intersection of the numeric attributes of the individual observations.
  std::set<std::string> numericCtxtNames;
  
  // The numeric trace attributes of each node.
  // This is the intersection of the numeric attributes of the individual observations.
  std::set<std::string> numericTraceNames;
  
  // Maps the names of all the contexts for which only one value has been observed to that value.
  // Used to identify and filter out contexts that are constant since we'll have a single
  // term dedicated to const-ness
  std::map<std::string, std::string> ctxtConstVals;
  
  // Records the context and trace data of all the observations observed by this filter.
  // This data is passed to observers at the end when we've identified the context and
  // trace attributes that are constant or not always numeric.
  std::list<std::map<std::string, std::string> > dataCtxt;
  std::list<std::map<std::string, std::string> > dataTrace;
  
  
  // The total number of observations that have passed through this filter
  int numObservations; 

  public:
  polyFitFilter();
  
  // Iterates over all combinations of keys in numericCtxt upto maxDegree in size and computes the products of
  // their values. Adds each such product to the given vector termVals.
  void addPolyTerms(const std::map<std::string, double>& numericCtxt, int termCnt, int maxDegree, 
                    std::vector<double>& termVals, double product=1);
  
  // Determines which context or trace attributes are numeric and return a mapping of their names to their numeric values
  //    (if numObservations>0, only the attributes that are currently in numericAttrNames)
  // Updates numericAttrNames to be the the intersection of itself and the numeric 
  //    attributes of this observation (keys of data)
  // data - maps context/trace attribute name to their observed values
  // numNumeric - refers to the count of numeric context/trace attributes
  // numericAttrNames - refers to the list of names of numeric context/trace attributes
  // label - identifies this as context or trace (for error messages)
  std::map<std::string, std::string/*double*/> getNumericAttrs(const std::map<std::string, std::string>& data, 
                                                std::set<std::string>& numericAttrNames, 
                                                std::string label);
  
  // Interface implemented by objects that listen for observations a traceStream reads. Such objects
  // call traceStream::registerObserver() to inform a given traceStream that it should observations.
  void observe(int traceID,
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor);

  // Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
  // This method is optional.
  void obsFinished();
}; // polyFitFilter

// Class that observes the polynomial fits that are produced by polyFitFilter. modularApp reads these fits
// from this object.
class polyFitObserver: public traceObserver
{
  // Maps the names of each trace attribute for which a fit was computed to the list of terms
  // in this fit
  std::map<std::string, std::list<std::string> > fits;
  
  public:
  // Interface implemented by objects that listen for observations a traceStream reads. Such objects
  // call traceStream::registerObserver() to inform a given traceStream that it should observations.
  void observe(int traceID,
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor);
  
  // Returns the number of trace attributes for which fits were computed
  int numFits() const { return fits.size(); }
  
  // Returns the formatted text representation of the fits, to be included in the HTML table 
  // that encodes each module's graph node
  std::string getFitText() const;
}; // polyFitObserver

class SynopticModuleObsLogger;

class modularApp: public block, public common::module
{
  friend class module;
  friend class moduleTraceStream;
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
  
  // Stack of the module markers that are currently in scope within this modularApp
  static std::list<sight::layout::moduleInfo> mMarkerStack;
  
  // Stack of the modules that are currently in scope within this modularApp
  static std::list<sight::layout::moduleInfo> mStack;
  
  // Maps each module group's ID to the trace that holds the observations performed within it
  std::map<int, traceStream*> moduleTraces;
  
  // Maps each module group's ID to the module object that processes its data
  std::map<int, sight::layout::module*> modules;
  
  // Maps each module group's ID to the polyFitObserver object that records the polynomial fit of the module's trace attributes
  std::map<int, sight::layout::polyFitObserver*> polyFits;
    
  // Maps each traceStream's ID to the ID of its corresponding module graph node
  std::map<int, int> trace2moduleID;
  
  // The dot file that will hold the representation of the module interaction graph
  std::ofstream dotFile;
  
  public:
  
  modularApp(properties::iterator props);
  ~modularApp();
  
  // Returns the unique instance of modularApp currently active
  static modularApp* getInstance() { assert(activeMA); return activeMA; }
  const std::string& getAppName() { return appName; }
  static const std::string getOutDir() { return outDir; }
  static const std::string getHtmlOutDir() { return htmlOutDir; }
  static const std::list<sight::layout::moduleInfo> getMMarkerStack() { return mMarkerStack; }
  static const std::list<sight::layout::moduleInfo> getMStack() { return mStack; }
  
  // Returns a string that uniquely identifies all the module markers currently on the stack, using the 
  // given string to separate the strings of different module instances.
  std::string getModuleMarkerStackName(std::string separator="_");
  
  // Returns a string that uniquely identifies all the modules currently on the stack, using the 
  // given string to separate the strings of different module instances.
  std::string getModuleStackName(std::string separator="_");
  
  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();
    
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
  //void showButtons(int numInputs, int numOutputs, int ID, std::string prefix, std::string bgColor);
 
  // Enter a new moduleMarker within the current modularApp
  // numInputs/numOutputs - the number of inputs/outputs of this module node
  // ID - the unique ID of this module node
  void enterModuleMarker(std::string moduleName, int numInputs, int numOutputs);

  // Static version of enterModuleMarker() that pulls the from/to anchor IDs from the properties iterator and calls
  // enterModule() in the currently active modularApp
  static void* enterModuleMarker(properties::iterator props);

  // Exit a module within the current modularApp
  void exitModuleMarker();

  // Static version of exitModuleMarker() that calls exitModuleMarker() in the currently active modularApp
  static void exitModuleMarker(void* obj);

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
  
  // Register the given module object (keeps data on the raw observations) and polyFitObserver object 
  // (keeps data on the polynomial fits that summarize these observations) with the currently active modularApp
  static void registerModule(int moduleID, sight::layout::module* m, polyFitObserver* pf);
  
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
  
  // Records whether we should emit the observations of each module into a separate table for use by external tools
  static bool emitObsIndividualDataTable;
  // Records whether we should emit the observations of all modular into a single table for use by external tools
  static bool emitObsCommonDataTable;

  // If emitObsCommonDataTable, points to the instance of SynopticModuleObsLogger that monitors and 
  // records all module observations.
  SynopticModuleObsLogger* commonDataTableLogger;
  
  // -------------------------
  // ----- Configuration -----
  // -------------------------
  // Currently there isn't anything that can be configured but in the future we may wish to
  // add measurements that will be taken on all modules
  public:
  class ModularAppConfiguration : public common::Configuration{
    public:
    ModularAppConfiguration(properties::iterator props) : common::Configuration(props.next()) {
      emitObsIndividualDataTable = props.exists("emitObsIndividualDataTable");
      emitObsCommonDataTable     = props.exists("emitObsCommonDataTable");
    }
  };

  static common::Configuration* configure(properties::iterator props) {
    // Create a ModuleConfiguration object, using the invocation of the constructor hierarchy to
    // record the configuration details with the respective widgets from which modules inherit
    ModularAppConfiguration* c = new ModularAppConfiguration(props);
    delete c;
    return NULL;
  }
}; // class modularApp

// Specialization of traceStreams for the case where they are hosted by a module
class moduleTraceStream: public traceStream
{
  friend class modularApp;
  protected:
  int moduleID;
  std::string name;
  int numInputs;
  int numOutputs;
  
  // The observers that processes observations of this object
  module* mFilter;
  polyFitFilter* polyFitter;
  polyFitObserver* polyFitCollector;
  traceFileWriterTSV* fileWriter;
  
  // The queue that passes all incoming observations through cmFilter and then forwards them to modularApp.
  //traceObserverQueue* queue;
  
  public:
  moduleTraceStream(properties::iterator props, traceObserver* observer=NULL);
  ~moduleTraceStream();
  
  // Called when we observe the entry tag of a moduleTraceStream
  static void *enterTraceStream(properties::iterator props);
};

// This class analyzes the observations from a compModuleTraceStream. It relates all observations that share
// the same values for the subset of context attributes that are not in the set options to a single one reference 
// observation (one reference for each value of non-option attributes) and emits these comparisons to the 
// traceObservers that listen to it.
class compModule : public common::module, public traceObserver {
  friend class compModuleTraceStream;
  // Records whether this is the reference configuration of the module
  //bool isReference;
  
  // The context that describes the configuration options of this module
  //context options;
  
  // Maps each configuration of input context values for which compContexts were not provided (this identifies
  // an equivalence class of observations) to the mapping of context and trace attributes to their values 
  // for observations within that equivalence class.
  std::map<std::map<std::string, std::string>, std::map<std::string, attrValue> > referenceCtxt;
  std::map<std::map<std::string, std::string>, std::map<std::string, attrValue> > referenceObs;
  
  // Records for each configuration of input context values all the the mappings of trace and context attributes to
  // their values within non-reference configurations of the compModule. We keep these around until we 
  // find the reference configuration for the given configuration of input context values and once we find 
  // it, we relate these to the reference, emit them to this compModuleTraceStream's observer and empty them out.
  std::map<std::map<std::string, std::string>, std::list<std::map<std::string, attrValue> > > comparisonObs;
  std::map<std::map<std::string, std::string>, std::list<std::map<std::string, attrValue> > > comparisonCtxt;
  
  // For each input and name of a context of the input for which a comparator was specified, records a pointer 
  // to the comparator to be used for this input context.
  std::vector<std::map<std::string, comparator*> > inComparators;
  
  // For each output and name of a context of the output, records a pointer to the comparator to be used
  // for this output context.
  std::vector<std::map<std::string, comparator*> > outComparators;
  
  // Maps the name of each measurement to the pointer to the comparator to be used for this measurement
  std::map<std::string, comparator*> measComparators;

  public:  
  compModule() { }
  
  // Compare the value of each trace attribute value ctxtobs (a given context or observation) to the corresponding value 
  // in ref (the corresponding reference observation) and return a mapping of each trace attribute to the serialized 
  // representation of their relationship. Where a comparator is not provided add the raw value from ctxtobs to the 
  // returned map.
  // The set of trace attributes in ref must contain that in ctxtobs.
  std::map<std::string, std::string> compareObservations(
                                         const std::map<std::string, attrValue>& ctxtobs,
                                         const std::map<std::string, attrValue>& ref);
  
  // Given a mapping of trace attribute names to the serialized representations of their attrValues, returns
  // the same mapping but with the attrValues deserialized as attrValues
  static std::map<std::string, attrValue> deserializeObs(const std::map<std::string, std::string>& obs);
  
  // Interface implemented by objects that listen for observations a traceStream reads. Such objects
  // call traceStream::registerObserver() to inform a given traceStream that it should observations.
  void observe(int traceID,
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor);
}; // class compModule

// Specialization of traceStreams for the case where they are hosted by a compModule 
class compModuleTraceStream: public moduleTraceStream
{
  // The object that filters observations to compare non-reference observations to their reference values
  compModule* cmFilter;
  
  // The object that filters comparison observations that come out of cmFilter
  //module* mFilter;
  
  public:
  compModuleTraceStream(properties::iterator props, traceObserver* observer=NULL);
  ~compModuleTraceStream();
  
  // Called when we observe the entry tag of a compModuleTraceStream
  static void *enterTraceStream(properties::iterator props);
}; // class compModuleTraceStream

// Specialization of traceStreams for the case where they are hosted by a processModule 
class processedModuleTraceStream: public moduleTraceStream
{ 
  // The directory that is used for storing intermediate files
  static std::string workDir;
  // The maximum unique ID assigned to any file that was used as input to a processor
  static int maxFileID;
  
  // Pointers to the actual externalTraceProcessors objects in the queue
  std::list<externalTraceProcessor_File*> commandProcessors;
  
  traceObserverQueue* filterQueue;
  
  public:
  processedModuleTraceStream(properties::iterator props, traceObserver* observer=NULL);
  ~processedModuleTraceStream();
  
  // Called when we observe the entry tag of a processedModuleTraceStream
  static void *enterTraceStream(properties::iterator props);
}; // class processedModuleTraceStream

// This is a trace observer that processes incoming module by writing them into the given file 
// Synoptic format, writing for each module observation two lines, one with the entry timestamp
// and one with the exit timestamp. SynopticModuleObsLogger requires the use of timeStampMeasures.
// Only records with such timestamps are emitted into the log.
class SynopticModuleObsLogger: public traceObserver {
  // Map the IDs of the traces this logger is observing to their string labels
  std::map<int, std::string> traceID2Label;
  
  // The file that the observations will be emitted to
  std::ofstream out;
  
  // The root file name that this instance will use to write output to
  std::string outFName;

  // The number of observations seen
  int numObservations;
  
  public: 
  SynopticModuleObsLogger(std::string outFName);
  ~SynopticModuleObsLogger();
  
  // Records the mapping from the given traceID to the given label
  void recordTraceLabel(int traceID, std::string label) { traceID2Label[traceID] = label; }
  
  // Interface implemented by objects that listen for observations a traceStream reads. Such objects
  // call traceStream::registerObserver() to inform a given traceStream that it should observations.
  void observe(int traceID,
               const std::map<std::string, std::string>& ctxt, 
               const std::map<std::string, std::string>& obs,
               const std::map<std::string, anchor>&      obsAnchor);
  
  // Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
  // This method is optional.
  void obsFinished();
}; // class SynopticModuleObsLogger
  
}; // namespace layout
}; // namespace sight
