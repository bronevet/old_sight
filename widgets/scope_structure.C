// Licence information included in file LICENCE
#include "../sight_structure.h"
#include "../sight_common.h"
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

scope::scope(std::string label,                                    scopeLevel level, const attrOp& onoffOp, properties* props) : 
  block(label, setProperties(level, &onoffOp, props))
//{ init(level, &onoffOp); }
{}

scope::scope(string label, anchor& pointsTo,                 scopeLevel level, const attrOp& onoffOp, properties* props): 
  block(label, pointsTo, setProperties(level, &onoffOp, props))
//{ init(level, &onoffOp); }
{}

scope::scope(string label, set<anchor>& pointsTo,            scopeLevel level, const attrOp& onoffOp, properties* props) :
  block(label, pointsTo, setProperties(level, &onoffOp, props))
//{ init(level, &onoffOp); }
{}

scope::scope(std::string label,                                                                     const attrOp& onoffOp, properties* props) : 
  block(label, setProperties(medium, &onoffOp, props))
//{ init(medium, &onoffOp); }
{}

scope::scope(string label, anchor& pointsTo,                                                  const attrOp& onoffOp, properties* props): 
  block(label, pointsTo, setProperties(medium, &onoffOp, props))
//{ init(medium, &onoffOp); }
{}

scope::scope(string label, set<anchor>& pointsTo,                                             const attrOp& onoffOp, properties* props) :
  block(label, pointsTo, setProperties(medium, &onoffOp, props))
//{ init(medium, &onoffOp); }
{}

scope::scope(std::string label,                                   scopeLevel level,                         properties* props) :
  block(label, setProperties(level, NULL, props))
//{ init(level, NULL); }
{}

scope::scope(std::string label, anchor& pointsTo,           scopeLevel level,                         properties* props) :
  block(label, pointsTo, setProperties(level, NULL, props))
//{ init(level, NULL); }
{}

scope::scope(std::string label, std::set<anchor>& pointsTo, scopeLevel level,                         properties* props) :
  block(label, pointsTo, setProperties(level, NULL, props))
//{ init(level, NULL); }
{}

// Sets the properties of this object
properties* scope::setProperties(scopeLevel level, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    newProps["level"] = txt()<<level;
    //cout << "scope: "<<cp2str(CPRuntime.doStackwalk())<<endl;
    newProps["callPath"] = cp2str(CPRuntime.doStackwalk());
    //dbg.enter("scope", properties, inheritedFrom);
    props->add("scope", newProps);
  }
  else
    props->active = false;
  return props;
}

// Common initialization code
/*void scope::init(scopeLevel level, const attrOp* onoffOp)
{
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    active = true;
    map<string, string> properties;
    properties["level"] = txt()<<level;
    dbg.enter("scope", properties, inheritedFrom);
  }
  else
    active = false;
}*/

scope::~scope()
{ 
  
  /*if(props->active) {
    dbg.exit(this);
  }*/
}

ScopeMerger::ScopeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        BlockMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* ScopeMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);

  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "scope");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Scope!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    pMap["level"] = txt()<<vAvg(str2int(getValues(tags, "level")));
    
    vector<string> cpValues = getValues(tags, "callPath");
    assert(allSame<string>(cpValues));
    pMap["callPath"] = *cpValues.begin();
    
    props->add("scope", pMap);
  } else {
    props->add("scope", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void ScopeMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  properties::iterator blockTag = tag;
  BlockMerger::mergeKey(type, ++blockTag, inStreamRecords, key);
}

}; // namespace structure
}; // namespace sight
