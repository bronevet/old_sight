#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "source_common.h"
#include "../../sight_common_internal.h"
#include "../../sight_structure_internal.h"

namespace sight {
namespace structure {

class source: public scope
{
  public:
  // Encapsulates code regions
  class reg {
    public:
    std::string fName;
    std::string startLine;
    std::string endLine;
    
    reg(std::string fName, std::string startLine, std::string endLine) :
      fName(fName), startLine(startLine), endLine(endLine)
    { }
  };
  
  typedef common::easylist<reg> regions;
  
  public:
  // label - the label associated with this scope.
  // pointsTo - the anchor(s) that terminate at this scope. Used to target links to scopes.
  // level - the type of visualization used, with higher levels associated with more amounts of debug output
  //    There are several features that are enabled by the levels:
  //       own_file: Text inside the scope is written to a separate file. Users must manually click a button to see it.
  //       own_color: The background color of the scope is different from its parent scope.
  //       label_shown: The label of this scope is shown in a larger font, along with controls to minimize the scope 
  //              by clicking on the label and open GDB to the point in the execution where the scope started
  //    Different levels
  //    high: own_file, own_color, label_shown
  //    medium: own_color, label_shown
  //    low: label_shown
  //    min: none of the above
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  source(std::string label, const reg& r,                            properties* props=NULL);
  source(std::string label, const reg& r,     const attrOp& onoffOp, properties* props=NULL);
  source(std::string label, const regions& r,                        properties* props=NULL);
  source(std::string label, const regions& r, const attrOp& onoffOp, properties* props=NULL);
  
  private:
  // Sets the properties of this object
  static properties* setProperties(std::string label, const regions& r, const attrOp* onoffOp, properties* props);
    
  public:
  ~source();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
}; // source

class SourceMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  SourceMergeHandlerInstantiator();
};
extern SourceMergeHandlerInstantiator SourceMergeHandlerInstance;

class SourceMerger : public ScopeMerger {
  public:
  SourceMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new SourceMerger(tags, outStreamRecords, inStreamRecords, props); }
  
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
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class ScopeMerger

} // namespace structure
} // namespace sight
