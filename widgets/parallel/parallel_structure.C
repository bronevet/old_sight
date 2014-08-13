// Licence information included in file LICENCE
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include "parallel_structure.h"

using namespace std;
using namespace sight::common;
  
namespace sight {
namespace structure {

/********************
 ***** commSend *****
 ********************/

commSend::commSend(const std::string& label, const std::string& sendID, const std::string& recvID,                        properties* props) : 
  uniqueMark(label, sendID, setProperties(recvID, NULL,     props))
{}

commSend::commSend(const std::string& label, const std::string& sendID, const std::string& recvID, const attrOp& onoffOp, properties* props) : 
  uniqueMark(label, sendID, setProperties(recvID, &onoffOp, props))
{}

// Sets the properties of this object
properties* commSend::setProperties(const std::string& recvID, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes->query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    if(recvID != "") {
      newProps["numRecvIDs"] = "1";
      newProps["ID0"] = recvID;
    } else
      newProps["numRecvIDs"] = "0";
    props->add("commSend", newProps);
  }
  else
    props->active = false;
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void commSend::destroy() {
  this->~commSend();
}

commSend::~commSend() {
  assert(!destroyed);
}

/********************
 ***** commRecv *****
 ********************/

commRecv::commRecv(const std::string& label, const std::string& sendID, const std::string& recvID,                        properties* props) : 
  uniqueMark(label, recvID, setProperties(sendID, NULL,     props))
{}

commRecv::commRecv(const std::string& label, const std::string& sendID, const std::string& recvID, const attrOp& onoffOp, properties* props) : 
  uniqueMark(label, recvID, setProperties(sendID, &onoffOp, props))
{}

// Sets the properties of this object
properties* commRecv::setProperties(const std::string& sendID, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes->query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    if(sendID != "") {
      newProps["numSendIDs"] = "1";
      newProps["ID0"] = sendID;
    } else
      newProps["numSendIDs"] = "0";
    props->add("commRecv", newProps);
  }
  else
    props->active = false;
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void commRecv::destroy() {
  this->~commRecv();
}

commRecv::~commRecv() {
  assert(!destroyed);
}

/********************
 ***** commBar *****
 ********************/

commBar::commBar(const std::string& label, const std::string& barID,                        properties* props) : 
  uniqueMark(label, barID, setProperties(NULL,     props))
{}

commBar::commBar(const std::string& label, const std::string& barID, const attrOp& onoffOp, properties* props) : 
  uniqueMark(label, barID, setProperties(&onoffOp, props))
{}

// Sets the properties of this object
properties* commBar::setProperties(const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes->query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    props->add("commBar", newProps);
  }
  else
    props->active = false;
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void commBar::destroy() {
  this->~commBar();
}

commBar::~commBar() {
  assert(!destroyed);
}


/**********************************************
 ***** ParallelMergeHandlerInstantiator *****
 **********************************************/

ParallelMergeHandlerInstantiator::ParallelMergeHandlerInstantiator() { 
  (*MergeHandlers   )["commSend"]  = CommSendMerger::create;
  (*MergeKeyHandlers)["commSend"]  = CommSendMerger::mergeKey;
  (*MergeHandlers   )["commRecv"]  = CommRecvMerger::create;
  (*MergeKeyHandlers)["commRecv"]  = CommRecvMerger::mergeKey;
  (*MergeHandlers   )["commBar"]   = CommBarMerger::create;
  (*MergeKeyHandlers)["commBar"]   = CommBarMerger::mergeKey;
}
ParallelMergeHandlerInstantiator ParallelMergeHandlerInstance;
                                                    
/**************************
 ***** CommSendMerger *****
 **************************/

CommSendMerger::CommSendMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        UniqueMarkMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* CommSendMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging CommSend!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    assert(tags.size()>0);
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "commSend");

    // Collect all the IDs from all the incoming streams, while using the allIDs
    // set to remove duplicates.
    set<string> allIDs;
    for(std::vector<std::pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin();
        t!=tags.end(); t++) {
      int numIDs = t->second.getInt("numRecvIDs");
      for(int i=0; i<numIDs; i++)
        allIDs.insert(t->second.get(txt()<<"ID"<<i));
    }

    // Add allIDs to pMap
    pMap["numRecvIDs"] = txt()<<allIDs.size();
    int j=0;
    for(set<string>::const_iterator i=allIDs.begin(); i!=allIDs.end(); i++, j++)
      pMap[txt()<<"ID"<<j] = *i;
    
    props->add("commSend", pMap);
  } else {
    props->add("commSend", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void CommSendMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  UniqueMarkMerger::mergeKey(type, tag.next(), inStreamRecords, info);
}

/**************************
 ***** CommRecvMerger *****
 **************************/

CommRecvMerger::CommRecvMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        UniqueMarkMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* CommRecvMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging CommRecv!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    assert(tags.size()>0);

    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "commRecv");

    // Collect all the IDs from all the incoming streams, while using the allIDs
    // set to remove duplicates.
    set<string> allIDs;
    for(std::vector<std::pair<properties::tagType, properties::iterator> >::const_iterator t=tags.begin();
        t!=tags.end(); t++) {
      int numIDs = t->second.getInt("numSendIDs");
      for(int i=0; i<numIDs; i++)
        allIDs.insert(t->second.get(txt()<<"ID"<<i));
    }

    // Add allIDs to pMap
    pMap["numSendIDs"] = txt()<<allIDs.size();
    int j=0;
    for(set<string>::const_iterator i=allIDs.begin(); i!=allIDs.end(); i++, j++)
      pMap[txt()<<"ID"<<j] = *i;

     props->add("commRecv", pMap);
  } else {
    props->add("commRecv", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void CommRecvMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  UniqueMarkMerger::mergeKey(type, tag.next(), inStreamRecords, info);
}

/**************************
 ***** CommBarMerger *****
 **************************/

CommBarMerger::CommBarMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        UniqueMarkMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* CommBarMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging CommRecv!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    assert(tags.size()>0);

    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "commBar");

    props->add("commBar", pMap);
  } else {
    props->add("commBar", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void CommBarMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  UniqueMarkMerger::mergeKey(type, tag.next(), inStreamRecords, info);
}

}; // namespace structure
}
