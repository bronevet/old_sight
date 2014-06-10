#include "valSelector_structure.h"
#include <assert.h>

using namespace std;

namespace sight {
  
namespace structure {

/***********************
 ***** valSelector *****
 ***********************/

int valSelector::maxSelID=0;

valSelector::valSelector(properties* props) : sightObj(props) {
  attrKeyKnown = false;
  selID=maxSelID;
  maxSelID++;
}

valSelector::valSelector(std::string attrKey, properties* props) : sightObj(props), attrKey(attrKey) { 
  attrKeyKnown = true;
  selID=maxSelID;
  maxSelID++;
}

int valSelector::getID() const
{ return selID; }


// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void valSelector::destroy() {
  this->~valSelector();
}

valSelector::~valSelector() {
  assert(!destroyed);
}

/*************************
 ***** colorSelector *****
 *************************/

colorSelector::colorSelector(properties* props) : valSelector(setProperties(0, 1, 1, .3, 0, .3, props))
{ }

colorSelector::colorSelector(std::string attrKey, properties* props) : valSelector(attrKey, setProperties(0, 1, 1, .3, 0, .3, props))
{  }

// Color selector with an explicit color gradient
colorSelector::colorSelector(float startR, float startG, float startB,
                             float endR,   float endG,   float endB, properties* props) : 
    valSelector(setProperties(startR, startG, startB, endR, endG, endB, props))
{ }

colorSelector::colorSelector(std::string attrKey,
                             float startR, float startG, float startB,
                             float endR,   float endG,   float endB, properties* props) : 
    valSelector(attrKey, setProperties(startR, startG, startB, endR, endG, endB, props))
{ }

properties* colorSelector::setProperties(float startR, float startG, float startB,
                                         float endR,   float endG,   float endB,
                                         properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> pMap;
  pMap["selID"]  = txt()<<maxSelID;
  pMap["startR"] = txt()<<startR;
  pMap["endR"]   = txt()<<endR;
  pMap["startG"] = txt()<<startG;
  pMap["endG"]   = txt()<<endG;
  pMap["startB"] = txt()<<startB;
  pMap["endB"]   = txt()<<endB;
  
  props->add("colorSelector", pMap);
  
  //dbg.enter("colorSelector", properties, false);
  //dbg.enter(this);
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void colorSelector::destroy() {
  this->~colorSelector();
}

colorSelector::~colorSelector() {
  if(destroyed) return;
}

// Informs the value selector that we have observed a new value that the selector needs to account for
// Returns the string reprentation of the current value.
string colorSelector::observeSelection(const attrValue& val) {
  return val.getAsStr();
}

// Informs the value selector that we have observed a new value that the selector needs to account for
// It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
// Returns the string reprentation of the current value.
string colorSelector::observeSelection() {
  if(!attrKeyKnown) { cerr << "colorSelector::observeSelection() ERROR: calling version of function that expects that the colorSelector knows the attribute to look up to choose the color but no attribute name was provided!"<<endl; exit(-1); }
  if(!attributes.exists(attrKey)) { cerr << "colorSelector::observeSelection() ERROR: attribute "<<attrKey<<" is not currently mapped to any values!!"<<endl; exit(-1); }
  
  const std::set<attrValue>& values = attributes.get(attrKey);
  assert(values.size()>0);
  return values.begin()->getAsStr();
}

/********************
 ***** cssColor *****
 ********************/

// Stack of sightObjects that correspond to currently live text formatting annotations
list<sightObj*> activeFormats;

// instanceID: uniquely identifies the particular formatted region of output that is controlled 
// by the given value selector
void start_internal(valSelector& sel, const attrValue* val, string name) { 
  // Each text-colored region is given a different uniqueID to make it possible to apply formatting to each one 
  // independently. Region identities are assigned using the instanceID counter, which is continually incremented
  static int instanceID=0;
  
  // Only bother if this text will be emitted
  if(!attributes.query()) return;
  
  string valueStr;
  if(val) valueStr = sel.observeSelection(*val);
  else    valueStr = sel.observeSelection();
  
  // Create a sightObj for the given formatting annotation and push it onto activeFormats
  properties* p = new properties();
  map<string, string> pMap;
  pMap["selID"] = txt()<<sel.getID();
  pMap["instanceID"] = txt()<<instanceID++;
  pMap["value"] = valueStr;
  p->add(name, pMap);
  activeFormats.push_back(new sightObj(p));
}

void end_internal(string name) {
  // Only bother if this text will be emitted
  if(!attributes.query()) return;
  
  // Pop the most recent formatting annotation from activeFormats
  assert(activeFormats.size()>0);
  delete activeFormats.back();
  activeFormats.pop_back();
}

std::ostream& textColor::operator<<(std::ostream& stream, const textColor::start& s) {
  start_internal(s.sel, s.val, "textColor");
  return stream;
}

std::ostream& textColor::operator<<(std::ostream& stream, const textColor::end& e) {
  end_internal("textColor");
  return stream;
}

std::ostream& bgColor::operator<<(std::ostream& stream, const bgColor::start& s) {
  start_internal(s.sel, s.val, "bgColor");
  return stream;
}

std::ostream& bgColor::operator<<(std::ostream& stream, const bgColor::end& e) {
  end_internal("bgColor");
  return stream;
}

std::ostream& borderColor::operator<<(std::ostream& stream, const borderColor::start& s) {
  start_internal(s.sel, s.val, "borderColor");
  return stream;
}

std::ostream& borderColor::operator<<(std::ostream& stream, const borderColor::end& e) {
  end_internal("borderColor");
  return stream;
}

/*************************************************
 ***** ColorSelectorMergeHandlerInstantiator *****
 *************************************************/

ColorSelectorMergeHandlerInstantiator::ColorSelectorMergeHandlerInstantiator() { 
  (*MergeHandlers   )["colorSelector"] = ColorSelectorMerger::create;
  (*MergeKeyHandlers)["colorSelector"] = ColorSelectorMerger::mergeKey;
  (*MergeHandlers   )["textColor"]     = ColorMerger::create;
  (*MergeKeyHandlers)["textColor"]     = ColorMerger::mergeKey;
  (*MergeHandlers   )["bgColor"]       = ColorMerger::create;
  (*MergeKeyHandlers)["bgColor"]       = ColorMerger::mergeKey;
  (*MergeHandlers   )["borderColor"]   = ColorMerger::create;
  (*MergeKeyHandlers)["borderColor"]   = ColorMerger::mergeKey;
  
  MergeGetStreamRecords->insert(&ColorSelectorGetMergeStreamRecord);
}
ColorSelectorMergeHandlerInstantiator ColorSelectorMergeHandlerInstance;

std::map<std::string, streamRecord*> ColorSelectorGetMergeStreamRecord(int streamID) {
  std::map<std::string, streamRecord*> mergeMap;
  mergeMap["colorSelector"] = new ColorSelectorStreamRecord(streamID);
  mergeMap["color"]         = new ColorStreamRecord(streamID);
  return mergeMap;
}


/*******************************
 ***** ColorSelectorMerger *****
 *******************************/
// Merger for colorSelector tag

ColorSelectorMerger::ColorSelectorMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) :
                    Merger(advance(tags), outStreamRecords, inStreamRecords, props)
{
  if(props==NULL) props = new properties();
  this->props = props;
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Trace!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    assert(tags.size()>0);
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "colorSelector");
    
    // Merge the selector IDs along all the streams
    int mergedSelID = streamRecord::mergeIDs("colorSelector", "selID", pMap, tags, outStreamRecords, inStreamRecords);
    pMap["selID"]  = txt()<<mergedSelID;
    
    pMap["startR"] = txt()<<vAvg(str2float(getValues(tags, "startR")));
    pMap["endR"]   = txt()<<vAvg(str2float(getValues(tags, "endR")));
    pMap["startG"] = txt()<<vAvg(str2float(getValues(tags, "startG")));
    pMap["endG"]   = txt()<<vAvg(str2float(getValues(tags, "endG")));
    pMap["startB"] = txt()<<vAvg(str2float(getValues(tags, "startB")));
    pMap["endB"]   = txt()<<vAvg(str2float(getValues(tags, "endB")));
  }
  props->add("colorSelector", pMap);
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void ColorSelectorMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
  }
}

/***********************
 ***** ColorMerger *****
 ***********************/
// Merger for tags textColor, bgColor, borderColor

ColorMerger::ColorMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) :
                    Merger(advance(tags), outStreamRecords, inStreamRecords, props)
{
  if(props==NULL) props = new properties();
  this->props = props;
  
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "textColor" ||
         *names.begin() == "bgColor"   ||
         *names.begin() == "borderColor");

  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Trace!"<<endl; exit(-1); }
  if(type==properties::enterTag) {    
    // Merge the selector IDs along all the streams (these should already have been merged when we processed
    // entry in this color annotation's associated colorSelector
    int mergedSelID = streamRecord::mergeIDs("colorSelector", "selID", pMap, tags, outStreamRecords, inStreamRecords);
    pMap["selID"]  = txt()<<mergedSelID;
    
    // Merge the trace IDs along all the streams
    int mergedInstID = streamRecord::mergeIDs("color", "instanceID", pMap, tags, outStreamRecords, inStreamRecords);
    pMap["instanceID"] = txt()<<mergedInstID;
    
    // We take an arbitrary value out of the set provided
    pMap["value"] = *getValues(tags, "value").begin();
  }
  
  props->add(*names.begin(), pMap);
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void ColorMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // We only merge color annotations that correspond to the same colorSelector in the outgoing stream. 
    // ColorSelector IDs are merged when we enter them, which is guaranteed to have occured before
    // we process the color annotations associated with them.
    streamID inSID(properties::getInt(tag, "selID"), inStreamRecords["colorSelector"]->getVariantID());
    streamID outSID = ((ColorStreamRecord*)inStreamRecords["colorSelector"])->in2outID(inSID);
    info.add(txt()<<outSID.ID);
  }
}

/*************************************
 ***** ColorSelectorStreamRecord *****
 *************************************/
// streamRecord for colorSelector tag

ColorSelectorStreamRecord::ColorSelectorStreamRecord(const ColorSelectorStreamRecord& that, int vSuffixID) :
  streamRecord(that, vSuffixID)
{}

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* ColorSelectorStreamRecord::copy(int vSuffixID) {
  return new ColorSelectorStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void ColorSelectorStreamRecord::resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams) {
  streamRecord::resumeFrom(streams);
}
  
std::string ColorSelectorStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[ColorSelectorStreamRecord: ";
  s << streamRecord::str(indent+"    ") << endl;  
  s << indent << "]";
  
  return s.str();
}

/*****************************
 ***** ColorStreamRecord *****
 *****************************/
// streamRecord for tags textColor, bgColor, borderColor

ColorStreamRecord::ColorStreamRecord(const ColorStreamRecord& that, int vSuffixID) :
  streamRecord(that, vSuffixID)
{}

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* ColorStreamRecord::copy(int vSuffixID) {
  return new ColorStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void ColorStreamRecord::resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams) {
  streamRecord::resumeFrom(streams);
}
  
std::string ColorStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[ColorStreamRecord: ";
  s << streamRecord::str(indent+"    ") << endl;  
  s << indent << "]";
  
  return s.str();
}


}; // namespace structure
}; // namespace sight
