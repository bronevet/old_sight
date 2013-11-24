#pragma once

namespace sight {

namespace common {
class trace {
  public:
  // Identifies the type of visualization used to show the trace
  typedef enum {table, lines, decTree, heatmap, boxplot, unknown} vizT;

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

}; // namespace common
}; // namespace sight
