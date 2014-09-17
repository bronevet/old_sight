#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../sight_structure_internal.h"
#include "../../attributes/attributes_structure.h"

namespace sight {
namespace structure {

class commSend: public uniqueMark, public common::scope
{
  std::string recvID;
  public:
  commSend(const std::string& label, const std::string& sendID, const std::string& recvID,                        properties* props=NULL);
  commSend(const std::string& label, const std::string& sendID, const std::string& recvID, const attrOp& onoffOp, properties* props=NULL);

  private:
  // Sets the properties of this object
  static properties* setProperties(const std::string& recvID, const attrOp* onoffOp, properties* props);
  
  public:
  ~commSend();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
}; // commSend

class commRecv: public uniqueMark, public common::scope
{
  std::string sendID;
  public:
  commRecv(const std::string& label, const std::string& sendID, const std::string& recvID,                        properties* props=NULL);
  commRecv(const std::string& label, const std::string& sendID, const std::string& recvID, const attrOp& onoffOp, properties* props=NULL);

  private:
  // Sets the properties of this object
  static properties* setProperties(const std::string& sendID, const attrOp* onoffOp, properties* props);
  
  public:
  ~commRecv();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
}; // commRecv

class commBar: public uniqueMark, public common::scope
{
  std::string sendID;
  public:
  commBar(const std::string& label, const std::string& barID,                        properties* props=NULL);
  commBar(const std::string& label, const std::string& barID, const attrOp& onoffOp, properties* props=NULL);

  private:
  // Sets the properties of this object
  static properties* setProperties(const attrOp* onoffOp, properties* props);
  
  public:
  ~commBar();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
}; // commBar

class ParallelMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  ParallelMergeHandlerInstantiator();
};
extern ParallelMergeHandlerInstantiator ParallelMergeHandlerInstance;

class CommSendMerger : public UniqueMarkMerger {
  public:
  CommSendMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new CommSendMerger(tags, outStreamRecords, inStreamRecords, props); }
  
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
}; // class CommSendMerger

class CommRecvMerger : public UniqueMarkMerger {
  public:
  CommRecvMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new CommRecvMerger(tags, outStreamRecords, inStreamRecords, props); }
  
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
}; // class CommRecvMerger

class CommBarMerger : public UniqueMarkMerger {
  public:
  CommBarMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new CommBarMerger(tags, outStreamRecords, inStreamRecords, props); }
  
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
}; // class CommBarMerger

} // namespace structure
} // namespace sight
