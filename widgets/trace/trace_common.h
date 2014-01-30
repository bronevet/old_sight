#pragma once

#include <map>

namespace sight {

namespace common {
class trace {
  public:
  // Identifies the type of visualization used to show the trace
  typedef enum {table, lines, scatter3d, decTree, heatmap, boxplot, unknown} vizT;

  // Indicates whether the trace visualization should be shown at the beginning or the end of its visual block
  typedef enum {showBegin, showEnd} showLocT;
  
  // Returns a string representation of a showLocT object
  static std::string showLoc2Str(showLocT showLoc);
    
  // The way trace observations from multiple stream are combined together
  typedef enum {disjMerge, // Observations with the same context from different streams are differentiated by the stream's ID
                // Observations with the same context from different streams are
                avgMerge,  // averaged
                maxMerge,  // max-ed
                minMerge}  // min-ed
            mergeT;
  
  // Returns a string representation of a mergeT object
  static std::string mergeT2Str(mergeT merge);
  
  // Returns a string representation of a vizT object
  static std::string viz2Str(vizT viz);
};

/*******************************************
 ***** Support for parsing trace files *****
 *******************************************/

// Type of the user-provided functor that takes as input a description of the observation data read from traces.
// readData: Maps field group (e.g. "ctxt", "obs", "anchor") to the mapping of attribute names to their string representations
// lineNum: The current line in the file, which is useful for generating error messages.
class traceFileReader {
  public:
  virtual void operator()(const std::map<std::string, attrValue>& ctxt,
                          const std::map<std::string, attrValue>& obs,
                          const std::map<std::string, int>& anchor, 
                          int lineNum)=0;
}; 

// Reads the given file of trace observations, calling the provided functor on each instance.
// Each line is a white-space separated sequence of mappings in the format group:key:value, where group
// may be ctxt, obs or anchor. Each call to functor f is provided with a description of a single
// trace file line, with three different maps for each group. For ctxt and obs we map keys to attrValues,
// while for anchor we map keys to integer anchor IDs.
void readTraceFile(std::string fName, traceFileReader& f);

// Given the context, observables and anchor information for an observation, returns its serialized representation
std::string serializeTraceObservation(const std::map<std::string, attrValue>& ctxt,
                                      const std::map<std::string, attrValue>& obs,
                                      const std::map<std::string, int>& anchor);

} // namespace common
} // namespace sight
