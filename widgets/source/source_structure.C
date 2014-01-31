// Licence information included in file LICENCE
#include "../../sight_structure.h"
#include "../../sight_common.h"
#include "source_structure.h"
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

source::source(std::string label, const source::reg& r,                            properties* props) :
  scope(label, scope::minimum, setProperties(label, source::regions(r), NULL, props))
{}

source::source(std::string label, const source::reg& r,     const attrOp& onoffOp, properties* props) :
  scope(label, scope::minimum, setProperties(label, source::regions(r), &onoffOp, props))
{}

source::source(std::string label, const source::regions& r,                        properties* props) :
  scope(label, scope::minimum, setProperties(label, r, NULL, props))
{}

source::source(std::string label, const source::regions& r, const attrOp& onoffOp, properties* props) :
  scope(label, scope::minimum, setProperties(label, r, &onoffOp, props))
{}

// Sets the properties of this object
properties* source::setProperties(std::string label, const source::regions& r, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> pMap;
    pMap["numRegions"] = txt() << r.size();
    int i=0;
    for(regions::const_iterator rj=r.begin(); rj!=r.end(); rj++, i++) {
      pMap[txt()<<"fName_"<<i]     = rj->fName;
      pMap[txt()<<"startLine_"<<i] = rj->startLine;
      pMap[txt()<<"endLine_"<<i]   = rj->endLine;
    }
    props->add("source", pMap);
  }
  else
    props->active = false;
  return props;
}

source::~source() { if(!destroyed) destroy(); }

// Contains the code to destroy this object. This method is called to clean up application state due to an
// abnormal termination instead of using delete because some objects may be allocated on the stack. Classes
// that implement destroy should call the destroy method of their parent object.
void source::destroy() {
  scope::destroy();
}

/*****************************************
 ***** SourceMergeHandlerInstantiator *****
 *****************************************/

SourceMergeHandlerInstantiator::SourceMergeHandlerInstantiator() { 
  (*MergeHandlers   )["source"]       = SourceMerger::create;
  (*MergeKeyHandlers)["source"]       = SourceMerger::mergeKey;
}
SourceMergeHandlerInstantiator SourceMergeHandlerInstance;


/***********************
 ***** SourceMerger ****
 ***********************/

SourceMerger::SourceMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        ScopeMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* SourceMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Scope!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    assert(tags.size()>0);

    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "source");
    
    vector<long> numRegionsV = str2int(getValues(tags, "numRegions"));
    assert(allSame<long>(numRegionsV));
    int numRegions = *numRegionsV.begin();
    pMap["numRegions"] = txt()<<numRegions;
    
    for(int i=0; i<numRegions; i++) {
      vector<string> fName = getValues(tags, txt()<<"fName_"<<i);
      assert(allSame<string>(fName));
      pMap[txt()<<"fName_"<<i] = *fName.begin();

      vector<string> startLine = getValues(tags, txt()<<"startLine_"<<i);
      assert(allSame<string>(startLine));
      pMap[txt()<<"startLine_"<<i] = *startLine.begin();

      vector<string> endLine = getValues(tags, txt()<<"endLine_"<<i);
      assert(allSame<string>(endLine));
      pMap[txt()<<"endLine_"<<i] = *endLine.begin();
    }
    
    props->add("source", pMap);
  } else {
    props->add("source", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void SourceMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  properties::iterator blockTag = tag;
  ScopeMerger::mergeKey(type, ++blockTag, inStreamRecords, key);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    key.push_back(properties::get(tag, "numRegions"));
    int numRegions = properties::getInt(tag, "numRegions");
    
    for(int i=0; i<numRegions; i++) {
      key.push_back(properties::get(tag, txt()<<"fName_"<<i));
      key.push_back(properties::get(tag, txt()<<"startLine_"<<i));
      key.push_back(properties::get(tag, txt()<<"endLine_"<<i));
    }
  }
}

}; // namespace structure
}; // namespace sight
