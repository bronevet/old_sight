#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "scopeOMP_common.h"
#include "../../sight_common_internal.h"
#include "../../sight_structure_internal.h"
#include "scope/scope_structure.h"

namespace sight {
namespace structure {

class scopeOMP: public scope
{
  /*
  public:
  class reg {
    public:
    std::string forID;
    std::string blockID;
    
    reg(std::string forID, std::string blockID) :
      forID(forID), blockID(blockID)
    { }
  };
  
  typedef common::easylist<reg> regions;
  */
  
  public:
  
  scopeOMP(std::string label, int forID, int numIter, int iterID, properties* props=NULL);
  scopeOMP(std::string label, int forID, int numIter, int iterID, const attrOp& onoffOp, properties* props=NULL);
  
  private:
  // Sets the properties of this object
  static properties* setProperties(std::string label, int forID, int numIter, int iterID, const attrOp* onoffOp, properties* props);
    
  public:
  ~scopeOMP();

  virtual void destroy();
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
}; // scopeOMP

class scopeOMPMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  scopeOMPMergeHandlerInstantiator();
};
extern scopeOMPMergeHandlerInstantiator scopeOMPMergeHandlerInstance;

class scopeOMPMerger : public ScopeMerger {
  public:
  scopeOMPMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new scopeOMPMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class ScopeMerger
} // namespace structure
} // namespace sight
