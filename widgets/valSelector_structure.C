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
  
  map<string, string> newProps;
  newProps["selID"]  = txt()<<selID;
  newProps["startR"] = txt()<<startR;
  newProps["endR"]   = txt()<<endR;
  newProps["startG"] = txt()<<startG;
  newProps["endG"]   = txt()<<endG;
  newProps["startB"] = txt()<<startB;
  newProps["endB"]   = txt()<<endB;
  
  this->props->add("colorSelector", newProps);
  
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
  
  common::sightObj obj(new properties());
  map<string, string> newProps;
  newProps["selID"] = txt()<<sel.getID();
  newProps["instanceID"] = txt()<<instanceID++;
  newProps["value"] = valueStr;
  obj.props->add(name, newProps);
  
  //dbg.enter(name, properties, false);
  return dbg.enterStr(*obj.props);
}

string end_internal(string name) {
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  common::sightObj obj(new properties());
  map<string, string> newProps;
  obj.props->add(name, newProps);
  
  //dbg.exit(name);
  return dbg.exitStr(*obj.props);
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

}; // namespace structure
}; // namespace sight
