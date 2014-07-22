#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdarg.h>
#include <initializer_list>
#include "cdnode_common.h"
#include "../../sight_common_internal.h"
#include "../../sight_structure_internal.h"
using namespace std;

namespace sight {
namespace structure {

class StageNode : public scope 
{
private:
// Sets the properties of this object
//  static properties* setProperties(string label, const attrOp* onoffOp, properties* props);
public:
  enum NodeType { kNull, kPreserve, kDetect, kRecover };

  NodeType node_t;
  bool is_populated;

  StageNode(string label, NodeType node_type, bool pop, properties* props=NULL);
  StageNode(string label, properties* props=NULL);
  virtual ~StageNode();
  virtual void destroy();
//  map<string, string> getProfile(void) { return profile; }
//  virtual string getNodeName(void) { }
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
//  void addNewInfo(std::string info, std::string val);
  virtual void setData(string label, string data) {}
};

// Preservation
class PreserveStageNode : public StageNode {
private:
  static properties* setProperties(const attrOp* onoffOp, properties* props);

public:
  PreserveStageNode(properties* props = NULL);
  PreserveStageNode(std::string label, properties* props=NULL);
  virtual  ~PreserveStageNode();
  virtual void destroy();

  virtual void setData(string label, string data);
};

// Detection

// Recovery

// ----------------- class CDNode --------------------------

class CDNode: public scope
{
public:
  
//  using StageCreator = std::function<StageNode*(void)>; 
//  typedef std::map<std::string, StageCreator> StageNodeCreatorTable; 

  map<string, StageNode*> stage;

  // label    - the label associated with this scope.
  // cd_id    - 
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  CDNode(string label,                                      properties* props=NULL); 
  CDNode(string label,               const attrOp& onoffOp, properties* props=NULL); 
  CDNode(string label, string cd_id,                        properties* props=NULL); 
  CDNode(string label, string cd_id, const attrOp& onoffOp, properties* props=NULL); 

  void addStageNode(string name, properties* props = NULL);
  void setStageNode(string stage_name, string profile_name, unsigned long profile_data);
//  void setStageNode(string name, initializer_list<unsigned long> data_il, properties* props = NULL) ; 
private:
  // Sets the properties of this object
  static properties* setProperties(string label, const attrOp* onoffOp, properties* props);

public:
  ~CDNode();

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
}; // CDNode



// ------ CD Node Merge Handler ------------------------------------------------

class CDNodeMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  CDNodeMergeHandlerInstantiator();
};
extern CDNodeMergeHandlerInstantiator CDNodeMergeHandlerInstance;

class CDNodeMerger : public ScopeMerger {
  public:
  CDNodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new CDNodeMerger(tags, outStreamRecords, inStreamRecords, props); }
  
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

// ------ Stage Node Merge Handler ------------------------------------------------

class StageNodeMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  StageNodeMergeHandlerInstantiator();
};
extern StageNodeMergeHandlerInstantiator StageNodeMergeHandlerInstance;

class StageNodeMerger : public ScopeMerger {
  public:
  StageNodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new StageNodeMerger(tags, outStreamRecords, inStreamRecords, props); }
  
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
