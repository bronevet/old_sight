#include "valSelector.h"
#include <assert.h>

using namespace std;

namespace dbglog {
/*************************
 ***** colorSelector *****
 *************************/

int colorSelector::maxSelID=0;

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
  static bool initialized=false;
  if(!initialized) {
    dbg.includeFile("valSelector.js");
    dbg.includeWidgetScript("valSelector.js", "text/javascript");
    initialized=true;
  }
  
  selID=maxSelID;
  dbg.widgetScriptCommand(txt()<<"colorSelectorInit("<<selID<<", ["<<startR<<","<<startG<<","<<startB<<"], ["<<endR<<","<<endG<<","<<endB<<"]);\n");
  maxSelID++;
}

// Returns a string that contains a call to a JavaScipt function that at log view time will return a value
void colorSelector::getSelFunction(const attrValue& val, std::string arg) {
  dbg.widgetScriptCommand(txt()<<"colorSelector("<<selID<<", \""<<val.getAsStr()<<"\", "<<arg<<");");
}

// Returns a string that contains a call to a JavaScipt function that at log view time will return a value
// It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
void colorSelector::getSelFunction(std::string arg) {
  if(!attrKeyKnown) { cerr << "colorSelector::getSelFunction() ERROR: calling version of function that expects that the colorSelector knows the attribute to look up to choose the color but no attribute name was provided!"<<endl; exit(-1); }
  if(!attributes.exists(attrKey)) { cerr << "colorSelector::getSelFunction() ERROR: attribute "<<attrKey<<" is not currently mapped to any values!!"<<endl; exit(-1); }
  
  const std::set<attrValue>& values = attributes.get(attrKey);
  assert(values.size()>0);
  
  dbg.widgetScriptCommand(txt()<<"colorSelector("<<selID<<", \""<<values.begin()->getAsStr()<<"\", "<<arg<<");");
}

/********************
 ***** cssColor *****
 ********************/

int cssColorID=0;
map<string, string> emptyFieldSettings;
string start_internal(valSelector& sel, const attrValue* val, string fieldName, const map<string, string>& fieldSettings) { 
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  // Add to this file's epilogue a call to colorSelectorDisplay(), which will apply all the colors
  // chosen by the selector
  static bool initialized=false;
  if(!initialized) {
    dbg.widgetScriptEpilogCommand("colorSelectorDisplay();\n");
    initialized = true;
  }
  
  ostringstream spanID; spanID << "cssColorSpan_"<<cssColorID;
  
  // Emit a command to assign whichever color to this span the selector chooses
  ostringstream cmd;
  cmd << "function(value){ ";
  // Set all key keys of fieldSettings to their values for this span
  for(map<string, string>::const_iterator i=fieldSettings.begin(); i!=fieldSettings.end(); i++)
    cmd << "document.getElementById('"<<spanID.str()<<"').style."<<i->first<<" = '"<<i->second<<"'; ";
  cmd << "document.getElementById('"<<spanID.str()<<"').style."<<fieldName<<" = value;";
  cmd << "}";
  
  if(val) sel.getSelFunction(*val, cmd.str());
  else    sel.getSelFunction(      cmd.str());
  
  cssColorID++;
  
  // Emit the start of a span with a unique name
  ostringstream ret; ret << "<span id=\""<<spanID.str()<<"\">";
  return ret.str();
}

string textColor::start(valSelector& sel, const attrValue& val) { 
  return start_internal(sel, &val, "color", emptyFieldSettings);
}

string textColor::start(valSelector& sel) { 
  return start_internal(sel, NULL, "color", emptyFieldSettings);
}

string textColor::end() {
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  return "</span>";
}

string bgColor::start(valSelector& sel, const attrValue& val) { 
  return start_internal(sel, &val, "backgroundColor", emptyFieldSettings);
}

string bgColor::start(valSelector& sel) { 
  return start_internal(sel, NULL, "backgroundColor", emptyFieldSettings);
}

string bgColor::end() {
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  return "</span>";
}

map<string, string> borderFieldSettings;
string borderColor::start(valSelector& sel, const attrValue& val) { 
  if(borderFieldSettings.size()==0) borderFieldSettings["border"]="solid";
  return start_internal(sel, &val, "borderColor", borderFieldSettings);
}

string borderColor::start(valSelector& sel) { 
  if(borderFieldSettings.size()==0) borderFieldSettings["border"]="solid";
  return start_internal(sel, NULL, "borderColor", borderFieldSettings);
}

string borderColor::end() {
  // Only bother if this text will be emitted
  if(!attributes.query()) return "";
  
  return "</span>";
}

}; // namespace dbglog