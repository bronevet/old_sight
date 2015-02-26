// Licence information included in file LICENCE
#include "../../sight_structure.h"
#include "../../sight_common.h"
#include "scopeOMP_structure.h"
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


using namespace std;
using namespace sight::common;
  
namespace sight {
namespace structure {

scopeOMP::scopeOMP(std::string label, int forID, int numIter, int iterID, properties* props) :
  scope(label, scope::minimum, setProperties(label, forID, numIter, iterID, NULL, props))
{}

scopeOMP::scopeOMP(std::string label, int forID, int numIter, int iterID, const attrOp& onoffOp, properties* props) :
  scope(label, scope::minimum, setProperties(label, forID, numIter, iterID, &onoffOp, props))
{}

// Sets the properties of this object
properties* scopeOMP::setProperties(std::string label, int forID, int numIter, int iterID, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes->query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> pMap;
    
    pMap["forID"] = txt()<< forID;    
    pMap["numIter"] = txt() << numIter; 
    pMap["iterID"] = txt()<< iterID;   
    
    props->add("scopeOMP", pMap);
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
void scopeOMP::destroy() {
  if(destroyed) return;
}

scopeOMP::~scopeOMP() {
  assert(!destroyed);
}


/*****************************************
 ***** scopeOMPMergeHandlerInstantiator *****
 *****************************************/

scopeOMPMergeHandlerInstantiator::scopeOMPMergeHandlerInstantiator() { 
  (*MergeHandlers   )["scopeOMP"]       = scopeOMPMerger::create;
  (*MergeKeyHandlers)["scopeOMP"]       = scopeOMPMerger::mergeKey;
}
scopeOMPMergeHandlerInstantiator scopeOMPMergeHandlerInstance;

/***********************
 ***** scopeOMPMerger ****
 ***********************/
scopeOMPMerger::scopeOMPMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        ScopeMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* scopeOMPMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Scope!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    assert(tags.size()>0);

    
    vector<long> forIDV = str2int(getValues(tags, "forID"));
    assert(allSame<long>(forIDV));
    int forID = *forIDV.begin();
    pMap["forID"] = txt()<<forID;
    
    vector<long> numIterV = str2int(getValues(tags, "numIter"));
    assert(allSame<long>(numIterV));
    int numIter = *numIterV.begin();
    pMap["numIter"] = txt()<<numIter;

    vector<long> iterIDV = str2int(getValues(tags, "iterID"));
    assert(allSame<long>(iterIDV));
    int iterID = *iterIDV.begin();
    pMap["iterID"] = txt()<<iterID;

    props->add("scopeOMP", pMap);
  } else {
    props->add("scopeOMP", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void scopeOMPMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  ScopeMerger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
      info.add(properties::get(tag, txt()<<"forID"));
      info.add(properties::get(tag, txt()<<"numIter"));
      info.add(properties::get(tag, txt()<<"iterID"));    
  }
}

}; // namespace structure
}; // namespace sight
