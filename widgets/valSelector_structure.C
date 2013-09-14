#include "valSelector_structure.h"
#include <assert.h>

using namespace std;

namespace dbglog {
  
namespace structure {

/*************************
 ***** valSelector *****
 *************************/

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

colorSelector::colorSelector() : valSelector()
{ init(0, 1, 1, .3, 0, .3); }

colorSelector::colorSelector(std::string attrKey) : valSelector(attrKey)
{ init(0, 1, 1, .3, 0, .3); }

// Color selector with an explicit color gradient
colorSelector::colorSelector(float startR, float startG, float startB,
                             float endR,   float endG,   float endB) : valSelector()
{ init(startR, startG, startB, endR, endG, endB); }

colorSelector::colorSelector(std::string attrKey,
                             float startR, float startG, float startB,
                             float endR,   float endG,   float endB) : valSelector(attrKey)
{ init(startR, startG, startB, endR, endG, endB); }

void colorSelector::init(float startR, float startG, float startB,
                         float endR,   float endG,   float endB) {
  map<string, string> properties;
  properties["ID"] = txt()<<selID;
  properties["startR"] = txt()<<startR;
  properties["endR"]   = txt()<<endR;
  properties["startG"] = txt()<<startG;
  properties["endG"]   = txt()<<endG;
  properties["startB"] = txt()<<startB;
  properties["endB"]   = txt()<<endB;
  dbg.enter("colorSelector", properties);
}

colorSelector::~colorSelector() {
  dbg.exit("colorSelector");
}

// Informs the value selector that we have observed a new value that the selector needs to account for
void colorSelector::observeSelection(const attrValue& val) {
  map<string, string> properties;
  properties["ID"] = txt()<<selID;
  properties["value"] = val.getAsStr();
  dbg.tag("colSelObs", properties);
}

// Informs the value selector that we have observed a new value that the selector needs to account for
// It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
void colorSelector::observeSelection() {
  if(!attrKeyKnown) { cerr << "colorSelector::observeSelection() ERROR: calling version of function that expects that the colorSelector knows the attribute to look up to choose the color but no attribute name was provided!"<<endl; exit(-1); }
  if(!attributes.exists(attrKey)) { cerr << "colorSelector::observeSelection() ERROR: attribute "<<attrKey<<" is not currently mapped to any values!!"<<endl; exit(-1); }
  
  const std::set<attrValue>& values = attributes.get(attrKey);
  assert(values.size()>0);
  
  map<string, string> properties;
  properties["ID"] = txt()<<selID;
  properties["value"] = values.begin()->getAsStr();
  dbg.tag("colSelObs", properties);
}

/********************
 ***** cssColor *****
 ********************/

string start_internal(valSelector& sel, const attrValue* val, string name) { 
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  if(val) sel.observeSelection(*val);
  else    sel.observeSelection();
    
  map<string, string> properties;
  properties["selID"] = txt()<<sel.getID();
  dbg.enter(name, properties);
  return "";
}

string end_internal(string name) {
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  dbg.exit(name);
  return "";
}

string textColor::start(valSelector& sel, const attrValue& val)
{ return start_internal(sel, &val, "textColor"); }

string textColor::start(valSelector& sel) 
{ return start_internal(sel, NULL, "textColor"); }

string textColor::end()
{ return end_internal("textColor"); }

string bgColor::start(valSelector& sel, const attrValue& val)
{ return start_internal(sel, &val, "bgColor"); }

string bgColor::start(valSelector& sel) 
{ return start_internal(sel, NULL, "bgColor"); }

string bgColor::end()
{ return end_internal("bgColor"); }

string borderColor::start(valSelector& sel, const attrValue& val)
{ return start_internal(sel, &val, "borderColor"); }

string borderColor::start(valSelector& sel) 
{ return start_internal(sel, NULL, "borderColor"); }

string borderColor::end()
{ return end_internal("borderColor"); }

}; // namespace structure
}; // namespace dbglog