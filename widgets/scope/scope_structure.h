// Copyright (c) 203 Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory
// Written by Greg Bronevetsky <bronevetsky1@llnl.gov>
//  
// LLNL-CODE-642002.
// All rights reserved.
//  
// This file is part of Sight. For details, see https://github.com/bronevet/sight. 
// Please read the COPYRIGHT file for Our Notice and
// for the BSD License.
#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../attributes/attributes_structure.h"
#include "scope_common.h"
#include "../../sight_structure_internal.h"
#include "Callpath.h"

namespace sight {
namespace structure {

class scope: public block, public common::scope
{
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
  scope(std::string label,                             scopeLevel level, const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label, anchor& pointsTo,           scopeLevel level, const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label, std::set<anchor>& pointsTo, scopeLevel level, const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label,                                               const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label, anchor& pointsTo,                             const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label, std::set<anchor>& pointsTo,                   const attrOp& onoffOp, properties* props=NULL);
  scope(std::string label,                             scopeLevel level=medium,                 properties* props=NULL);
  scope(std::string label, anchor& pointsTo,           scopeLevel level=medium,                 properties* props=NULL);
  scope(std::string label, std::set<anchor>& pointsTo, scopeLevel level=medium,                 properties* props=NULL);
  
  private:
  // Sets the properties of this object
  static properties* setProperties(scopeLevel level, const attrOp* onoffOp, properties* props);
    
  // Common initialization code
  //void init(scopeLevel level, const attrOp* onoffOp);
  
  public:
  ~scope();

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
}; // scope

class ScopeMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  ScopeMergeHandlerInstantiator();
};
extern ScopeMergeHandlerInstantiator ScopeMergeHandlerInstance;

class ScopeMerger : public BlockMerger {
  public:
  ScopeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ScopeMerger(tags, outStreamRecords, inStreamRecords, props); }
  
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
