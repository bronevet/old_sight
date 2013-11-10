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
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  cout << "#tags="<<tags.size()<<" type="<<type<<endl;
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Scope!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    set<string> names = getNameSet(tags);
    assert(names.size()==1);
    assert(*names.begin() == "scope");
    
    pMap["level"] = txt()<<setAvg(str2intSet(getValueSet(tags, "level")));
    props->add("scope", pMap);
  } else {
    props->add("scope", pMap);
  }
  
  return props;
}

}; // namespace structure
}; // namespace sight
