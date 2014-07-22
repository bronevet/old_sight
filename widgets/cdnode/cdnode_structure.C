// Licence information included in file LICENCE
#include "../../sight_structure.h"
#include "../../sight_common.h"
#include "cdnode_structure.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <functional>
#include <initializer_list>
using namespace std;
using namespace sight::common;
  
namespace sight {
namespace structure {

// normally label is cd id except for root
CDNode::CDNode(string label,                                      properties* props) 
    : scope(txt() << "CD : " << label, scope::medium,   setProperties(label, NULL, props))  {}

CDNode::CDNode(string label,               const attrOp& onoffOp, properties* props) 
    : scope(txt() << "CD : " << label, scope::medium, setProperties(label, &onoffOp, props))  {}

CDNode::CDNode(string label, string cd_id,                        properties* props) 
    : scope(txt() << "CD "<< cd_id <<" : " << label, scope::medium,   setProperties(cd_id, NULL, props))  {}

CDNode::CDNode(string label, string cd_id, const attrOp& onoffOp, properties* props) 
    : scope(txt() << "CD "<< cd_id <<" : " << label, scope::medium, setProperties(cd_id, &onoffOp, props))  {}

void CDNode::destroy() { if(destroyed) return; }

CDNode::~CDNode() {
  assert(!destroyed);
//  assert(stage.size()>0);
//  assert(stage.back() != NULL);
  //FIXME
/*  if( stage.find("preserve") != stage.end() ) {
    stage["preserve"]->destroy();
    delete stage["preserve"];
    stage.erase("preserve");
    cout << "there is a stage!!!" <<endl;
    getchar();
  }
  else {
    cout << "there is no stage!!!" <<endl;
    getchar();
  }*/
}

// Sets the properties of this object
properties* CDNode::setProperties(string cd_id, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    newProps["cd_id"] = txt() << cd_id;
    props->add("CDNode", newProps);
  }
  else {
    props->active = false;
  }
  return props;
}

// It only sets the key of the stage. 
// The value of that key is NULL and will be populated later (in CDHandle::Complete())
// Normal usage is, 
// XXX->addStageNode("preserve");
void CDNode::addStageNode(std::string name, properties* props) 
{ 
  if(name == "preserve") {
    stage[name] = new PreserveStageNode(props); 
  }
  else if(name == "detect") {
    cout << "detect is not supported yet" << endl;
    exit(-1);
  }
  else if(name == "recovery") {
    cout << "recovery is not supported yet" << endl;
    exit(-1);
  }
  else {
    cout << " error in addStageNode" << endl;
    exit(-1);
  }
}

// It populates the values of the stage.
// If there is no keys, then it does not new anything,
// which means that there is no "recovery" as a key,
// it does not new that.
// XXX->setStageNode("preserve", {data1, data2, ...} (order is important!)
void CDNode::setStageNode(string stage_name, string profile_name, unsigned long profile_data)  
{
  if(stage.find(stage_name) == stage.end()) {
    cout << "there is no stage yet" << endl;
    assert(0);
  }

  stage[stage_name]->setData(profile_name, txt() << profile_data);

}

// --------- StageNode -------------------------------------------------------------------------------------

// called by PreserveStageNode
StageNode::StageNode(string label, NodeType node_type, bool pop, properties* props) :
  scope(label, scope::minimum, props), node_t(node_type), is_populated(pop)  {}

// called directly
StageNode::StageNode(string label, properties* props) :
  scope(label, scope::minimum, props), node_t(kNull), is_populated(false)  {}

StageNode::~StageNode() { 
  assert(!destroyed); 
  cout<<"~StageNode"<<endl; 
}

void StageNode::destroy() { if(destroyed) return; }
// --------- PreserveNode ----------------------------------------------------------------------------------

PreserveStageNode::PreserveStageNode(std::string label, properties* props) :
  StageNode(txt()<<" "<<label, kPreserve, false, setProperties(NULL, props)) { }

PreserveStageNode::PreserveStageNode(properties* props) :
  StageNode(" ", kPreserve, false, setProperties(NULL, props)) { }

PreserveStageNode::~PreserveStageNode() 
{ 
  cout<< "~PreserveStageNode() is called" <<endl; 
}

void PreserveStageNode::destroy() {}

properties* PreserveStageNode::setProperties(const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    newProps["StageName"] = "preserve";
    props->add("StageNode", newProps);
  }
  else {
    props->active = false;
  }

  return props;
}


void PreserveStageNode::setData(string label, string data) 
{
  if(this->props == NULL) {
    cout << "props is null while setting data" << endl;
    assert(0);
  }
  is_populated = true;
  this->props->set("StageNode", label, data);
} 
// --------- DetectNode -----------------------------------------------


// --------- RecoveryNode -----------------------------------------------


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------ CDNodeMergeHandlerInstantiator ----------------------------------

CDNodeMergeHandlerInstantiator::CDNodeMergeHandlerInstantiator() { 
  (*MergeHandlers   )["CDNode"]       = CDNodeMerger::create;
  (*MergeKeyHandlers)["CDNode"]       = CDNodeMerger::mergeKey;
}
CDNodeMergeHandlerInstantiator CDNodeMergeHandlerInstance;

// ------------ CDNodeMerger -----------------------------------------------------

CDNodeMerger::CDNodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        ScopeMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* CDNodeMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Scope!"<<endl; exit(-1); }
  if(type==properties::enterTag) {

//    assert(tags.size()>0);
//
//    vector<string> names = getNames(tags); assert(allSame<string>(names));
//    assert(*names.begin() == "CDNode");
//    
//    vector<long> numRegionsV = str2int(getValues(tags, "numRegions"));
//    assert(allSame<long>(numRegionsV));
//    int numRegions = *numRegionsV.begin();
//    pMap["numRegions"] = txt()<<numRegions;
//    
//    for(int i=0; i<numRegions; i++) {
//      vector<string> cd_id = getValues(tags, txt()<<"cd_id_"<<i);
//      assert(allSame<string>(cd_id));
//      pMap[txt()<<"cd_id_"<<i] = *cd_id.begin();
//
//      vector<string> startLine = getValues(tags, txt()<<"prvData_"<<i);
//      assert(allSame<string>(startLine));
//      pMap[txt()<<"prvData_"<<i] = *startLine.begin();
//
//      vector<string> endLine = getValues(tags, txt()<<"cycleNum_"<<i);
//      assert(allSame<string>(endLine));
//      pMap[txt()<<"cycleNum_"<<i] = *endLine.begin();
//
//      vector<string> endLine = getValues(tags, txt()<<"errBitVector_"<<i);
//      assert(allSame<string>(endLine));
//      pMap[txt()<<"errBitVector_"<<i] = *endLine.begin();
//    }
    
    props->add("CDNode", pMap);

  } else {

    props->add("CDNode", pMap);

  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void CDNodeMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  ScopeMerger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
//    info.add(properties::get(tag, "XXXXXXXXXX"));
//    info.add(properties::get(tag, "XXXXXXXXXX"));

  }
}

// ------------------ StageNodeMerger ----------------------




// ---------------- StageNodeMergeHandlerInstantiator ---------------------------------

StageNodeMergeHandlerInstantiator::StageNodeMergeHandlerInstantiator() { 
  (*MergeHandlers   )["StageNode"]       = StageNodeMerger::create;
  (*MergeKeyHandlers)["StageNode"]       = StageNodeMerger::mergeKey;
}
StageNodeMergeHandlerInstantiator StageNodeMergeHandlerInstance;


// ---------------- StageNodeMerger ---------------------------------------------------

StageNodeMerger::StageNodeMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        ScopeMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* StageNodeMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Scope!"<<endl; exit(-1); }
  if(type==properties::enterTag) {

//    assert(tags.size()>0);
    
    props->add("StageNode", pMap);

  } else {

    props->add("StageNode", pMap);

  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void StageNodeMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  ScopeMerger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {

//    info.add(properties::get(tag, "XXXXXXXXXX"));
//    info.add(properties::get(tag, "XXXXXXXXXX"));
  }
}



}; // namespace structure
}; // namespace sight
