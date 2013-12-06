#include "valSelector_structure.h"
#include <assert.h>

using namespace std;

namespace sight {
  
namespace structure {

/***********************
 ***** valSelector *****
 ***********************/

int valSelector::maxSelID=0;

valSelector::valSelector() {
  attrKeyKnown = false;
  selID=maxSelID;
  maxSelID++;
}

valSelector::valSelector(std::string attrKey) : attrKey(attrKey) { 
  attrKeyKnown = true;
  selID=maxSelID;
  maxSelID++;
}

int valSelector::getID() const
{ return selID; }

/*************************
 ***** colorSelector *****
 *************************/

colorSelector::colorSelector(properties* props) : valSelector()
{ init(0, 1, 1, .3, 0, .3); }

colorSelector::colorSelector(std::string attrKey, properties* props) : valSelector(attrKey)
{ init(0, 1, 1, .3, 0, .3); }

// Color selector with an explicit color gradient
colorSelector::colorSelector(float startR, float startG, float startB,
                             float endR,   float endG,   float endB, properties* props) : valSelector()
{ init(startR, startG, startB, endR, endG, endB); }

colorSelector::colorSelector(std::string attrKey,
                             float startR, float startG, float startB,
                             float endR,   float endG,   float endB, properties* props) : valSelector(attrKey)
{ init(startR, startG, startB, endR, endG, endB); }

void colorSelector::init(float startR, float startG, float startB,
                         float endR,   float endG,   float endB,
                         properties* props) {
  if(props==NULL) this->props = new properties();
  else            this->props = props;
  
  map<string, string> pMap;
  pMap["selID"]  = txt()<<selID;
  pMap["startR"] = txt()<<startR;
  pMap["endR"]   = txt()<<endR;
  pMap["startG"] = txt()<<startG;
  pMap["endG"]   = txt()<<endG;
  pMap["startB"] = txt()<<startB;
  pMap["endB"]   = txt()<<endB;
  
  this->props->add("colorSelector", pMap);
  
  //dbg.enter("colorSelector", properties, false);
  dbg.enter(this);
}

colorSelector::~colorSelector() {
  //dbg.exit("colorSelector");
  dbg.exit(this);
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

// instanceID: uniquely identifies the particular formatted region of output that is controlled 
// by the given value selector
string start_internal(valSelector& sel, const attrValue* val, string name) { 
  // Each text-colored region is given a different uniqueID to make it possible to apply formatting to each one 
  // independently. Region identities are assigned using the instanceID counter, which is continually incremented
  static int instanceID=0;
  
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  string valueStr;
  if(val) valueStr = sel.observeSelection(*val);
  else    valueStr = sel.observeSelection();
  
  properties p;
  map<string, string> pMap;
  pMap["selID"] = txt()<<sel.getID();
  pMap["instanceID"] = txt()<<instanceID++;
  pMap["value"] = valueStr;
  p.add(name, pMap);
  
  //dbg.enter(name, properties, false);
  return dbg.enterStr(p);
}

string end_internal(string name) {
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  properties p;
  map<string, string> pMap;
  p.add(name, pMap);
  
  //dbg.exit(name);
  return dbg.exitStr(p);
}

/*string textColor::start(valSelector& sel, const attrValue& val)
{ return start_internal(sel, &val, "textColor"); }

string textColor::start(valSelector& sel) 
{ return start_internal(sel, NULL, "textColor"); }
*/
std::ostream& textColor::operator<<(std::ostream& stream, const textColor::start& s) {
  dbgStream& ds = dynamic_cast<dbgStream&>(stream);
  ds.ownerAccessing();
  ds << start_internal(s.sel, s.val, "textColor");
  ds.userAccessing();  
  return ds;
}

/*string textColor::end()
{ return end_internal("textColor"); }*/
std::ostream& textColor::operator<<(std::ostream& stream, const textColor::end& e) {
  dbgStream& ds = dynamic_cast<dbgStream&>(stream);
  ds.ownerAccessing();
  ds << end_internal("textColor");
  ds.userAccessing();  
  return ds;
}

/*string bgColor::start(valSelector& sel, const attrValue& val)
{ return start_internal(sel, &val, "bgColor"); }

string bgColor::start(valSelector& sel) 
{ return start_internal(sel, NULL, "bgColor"); }*/

std::ostream& bgColor::operator<<(std::ostream& stream, const bgColor::start& s) {
  dbgStream& ds = dynamic_cast<dbgStream&>(stream);
  ds.ownerAccessing();
  ds << start_internal(s.sel, s.val, "bgColor");
  ds.userAccessing();  
  return ds;
}

/*string bgColor::end()
{ return end_internal("bgColor"); }*/
std::ostream& bgColor::operator<<(std::ostream& stream, const bgColor::end& e) {
  dbgStream& ds = dynamic_cast<dbgStream&>(stream);
  ds.ownerAccessing();
  ds << end_internal("bgColor");
  ds.userAccessing();  
  return ds;
}

/*string borderColor::start(valSelector& sel, const attrValue& val)
{ return start_internal(sel, &val, "borderColor"); }

string borderColor::start(valSelector& sel) 
{ return start_internal(sel, NULL, "borderColor"); }*/

std::ostream& borderColor::operator<<(std::ostream& stream, const borderColor::start& s) {
  dbgStream& ds = dynamic_cast<dbgStream&>(stream);
  ds.ownerAccessing();
  ds << start_internal(s.sel, s.val, "borderColor");
  ds.userAccessing();  
  return ds;
}

/*string borderColor::end()
{ return end_internal("borderColor"); }*/
std::ostream& borderColor::operator<<(std::ostream& stream, const borderColor::end& e) {
  dbgStream& ds = dynamic_cast<dbgStream&>(stream);
  ds.ownerAccessing();
  ds << end_internal("borderColor");
  ds.userAccessing();  
  return ds;
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
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  properties::iterator blockTag = tag;
    
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
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  properties::iterator blockTag = tag;
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // We only merge color annotations that correspond to the same colorSelector in the outgoing stream. 
    // ColorSelector IDs are merged when we enter them, which is guaranteed to have occured before
    // we process the color annotations associated with them.
    streamID inSID(properties::getInt(tag, "selID"), inStreamRecords["colorSelector"]->getVariantID());
    streamID outSID = ((ColorStreamRecord*)inStreamRecords["colorSelector"])->in2outID(inSID);
    key.push_back(txt()<<outSID.ID);
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
