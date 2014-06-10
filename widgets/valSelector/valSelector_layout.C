#include "../../sight_layout.h"
#include <assert.h>

using namespace std;

namespace sight {
namespace layout {

// Record the layout handlers in this file
void* colorSelectorEnterHandler(properties::iterator props) { return new colorSelector(props); }
void  colorSelectorExitHandler(void* obj) { colorSelector* t = static_cast<colorSelector*>(obj); delete t; }
  
valSelectorLayoutHandlerInstantiator::valSelectorLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["colorSelector"] = &colorSelectorEnterHandler;
  (*layoutExitHandlers )["colorSelector"] = &colorSelectorExitHandler;
  (*layoutEnterHandlers)["textColor"]     = &textColor::start;
  (*layoutExitHandlers )["textColor"]     = &textColor::end;
  (*layoutEnterHandlers)["bgColor"]       = &bgColor::start;
  (*layoutExitHandlers )["bgColor"]       = &bgColor::end;
  (*layoutEnterHandlers)["borderColor"]   = &borderColor::start;
  (*layoutExitHandlers )["borderColor"]   = &borderColor::end;
}
valSelectorLayoutHandlerInstantiator valSelectorLayoutHandlerInstance;

/***********************
 ***** valSelector *****
 ***********************/

// Maps the selIDs of all the active valSelectors to their objects
std::map<int, valSelector*> valSelector::active;

map<string, string> emptyFieldSettings;
void valSelector::observeSelection(properties::iterator props, string fieldName, const map<string, string>& fieldSettings) {
  // Add to this file's epilogue a call to colorSelectorDisplay(), which will apply all the colors
  // chosen by the selector
  static bool initialized=false;
  if(!initialized) {
    dbg.widgetScriptEpilogCommand("colorSelectorDisplay();\n");
    initialized = true;
  }
  
  long selID = properties::getInt(props, "selID");
  assert(active.find(selID) != active.end());
  valSelector* sel = active[selID];
  
  ostringstream spanID; spanID << "valueSelectSpan_"<<properties::getInt(props, "instanceID");
  
  // Emit a command to assign whichever color to this span the selector chooses
  ostringstream cmd;
  cmd << "function(value){ ";
  // Set all key keys of fieldSettings to their values for this span
  for(map<string, string>::const_iterator i=fieldSettings.begin(); i!=fieldSettings.end(); i++)
    cmd << "document.getElementById('"<<spanID.str()<<"').style."<<i->first<<" = '"<<i->second<<"'; ";
  cmd << "document.getElementById('"<<spanID.str()<<"').style."<<fieldName<<" = value;";
  cmd << "}";
  
  
  //dbg.widgetScriptCommand(txt()<<"colorSelector("<<sel->selID<<", \""<<properties::get(props, "value")<<"\", "<<cmd.str()<<");");
  sel->getSelFunction(properties::get(props, "value"), cmd.str());
    
  // Emit the start of a span with a unique name
  dbg << "<span id=\""<<spanID.str()<<"\">";
}

/*************************
 ***** colorSelector *****
 *************************/

colorSelector::colorSelector(properties::iterator props) : valSelector()
{
  static bool initialized=false;
  if(!initialized) {
    // Create the directory that holds the module-specific scripts
    pair<string, string> paths = dbg.createWidgetDir("valSelector");
    
    dbg.includeWidgetScript("valSelector/valSelector.js", "text/javascript"); dbg.includeFile("valSelector/valSelector.js"); 
    initialized=true;
  }

  selID=properties::getInt(props, "selID");
  assert(active.find(selID) == active.end());
  active[selID] = this;
  dbg.widgetScriptCommand(txt()<<"colorSelectorInit("<<selID<<", "<<
                                     "["<<properties::getFloat(props, "startR")<<","<<
                                          properties::getFloat(props, "startG")<<","<<
                                          properties::getFloat(props, "startB")<<
                                     "], ["<<
                                          properties::getFloat(props, "endR")<<","<<
                                          properties::getFloat(props, "endG")<<","<<
                                          properties::getFloat(props, "endB")<<
                                      "]);\n");
}

colorSelector::~colorSelector()
{
  assert(active.find(selID) != active.end());
  active.erase(selID);
}

// Returns a string that contains a call to a JavaScipt function that at log view time will return a value
// It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
void colorSelector::getSelFunction(string val, string arg) {
  //if(!attrKeyKnown) { cerr << "colorSelector::getSelFunction() ERROR: calling version of function that expects that the colorSelector knows the attribute to look up to choose the color but no attribute name was provided!"<<endl; exit(-1); }
  //if(!attributes.exists(attrKey)) { cerr << "colorSelector::getSelFunction() ERROR: attribute "<<attrKey<<" is not currently mapped to any values!!"<<endl; exit(-1); }
  
  //const std::set<attrValue>& values = attributes.get(attrKey);
  //assert(values.size()>0);
  
  //dbg.widgetScriptCommand(txt()<<"colorSelector("<<selID<<", \""<<values.begin()->getAsStr()<<"\", "<<arg<<");");
  dbg.widgetScriptCommand(txt()<<"colorSelector("<<selID<<", \""<<val<<"\", "<<arg<<");");
}

/********************
 ***** cssColor *****
 ********************/

void* textColor::start(properties::iterator props) { 
  valSelector::observeSelection(props, "color", emptyFieldSettings);
  return NULL;
}

void textColor::end(void* obj) {
  dbg << "</span>";
}

void* bgColor::start(properties::iterator props) { 
  valSelector::observeSelection(props, "backgroundColor", emptyFieldSettings);
  return NULL;
}

void bgColor::end(void* obj) {
  dbg << "</span>";
}

map<string, string> borderFieldSettings;
void* borderColor::start(properties::iterator props) { 
  if(borderFieldSettings.size()==0) borderFieldSettings["border"]="solid";
  valSelector::observeSelection(props, "borderColor", borderFieldSettings);
  return NULL;
}

void borderColor::end(void* obj) {
  dbg << "</span>";
}

}; // namespace layout
}; // namespace sight
