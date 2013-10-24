#include "../dbglog_layout.h"
using namespace std;

namespace dbglog {
namespace layout {

// Record the layout handlers in this file
void* traceEnterHandler(properties::iterator props) { return new trace(props); }
void  traceExitHandler(void* obj) { trace* t = static_cast<trace*>(obj); delete t; }
  
traceLayoutHandlerInstantiator::traceLayoutHandlerInstantiator() { 
  layoutEnterHandlers["trace"] = &traceEnterHandler;
  layoutExitHandlers ["trace"] = &traceExitHandler;
  layoutEnterHandlers["traceObs"] = &trace::observe;
  layoutExitHandlers ["traceObs"] = &defaultExitHandler;
}

// Maps the traceIDs of all the currently active traces to their trace objects
std::map<int, trace*> trace::active;

// Records whether the tracer infrastructure has been initialized
bool trace::initialized=false;

trace::trace(properties::iterator props) : block(properties::next(props)) {
  if(!initialized) {
    // Table/Lines visualization
    //dbg.includeScript("https://www.google.com/jsapi", "text/javascript");
    //dbg.includeScript("http://yui.yahooapis.com/3.11.0/build/yui/yui-min.js", "text/javascript");
    dbg.includeWidgetScript("yui-min.js", "text/javascript"); dbg.includeFile("yui-min.js");
    
    // Decision Tree
    //dbg.includeScript("http://code.jquery.com/jquery-1.8.1.min.js",                                  "text/javascript");
    //dbg.includeScript("http://cdnjs.cloudflare.com/ajax/libs/underscore.js/1.3.3/underscore-min.js", "text/javascript");
    //dbg.includeScript("http://d3js.org/d3.v2.js", "text/javascript");
    
    dbg.includeWidgetScript("jquery-1.8.1.min.js", "text/javascript"); dbg.includeFile("jquery-1.8.1.min.js");
    dbg.includeWidgetScript("underscore-min.js",   "text/javascript"); dbg.includeFile("underscore-min.js");
    dbg.includeWidgetScript("d3.v2.js",            "text/javascript"); dbg.includeFile("d3.v2.js");
    
    dbg.includeWidgetScript("ID3-Decision-Tree/js/id3.js", "text/javascript");
    //dbg.includeScript("https://www.google.com/jsapi?autoload={\"modules\":[{\"name\":\"visualization\",\"version\":\"1\",\"packages\":[\"orgchart\"]}]}", "text/javascript");
    dbg.includeFile("ID3-Decision-Tree");

    dbg.includeFile("trace.js");
    dbg.includeWidgetScript("trace.js", "text/javascript");
    
    initialized = true;
  }
  
  traceID = properties::getInt(props, "traceID");
  showLoc = (showLocT)properties::getInt(props, "showLoc");
  viz     = (vizT)    properties::getInt(props, "viz");
  
  // Add this trace object as a change listener to all the context variables
  long numCtxtAttrs = properties::getInt(props, "numCtxtAttrs");
  for(long i=0; i<numCtxtAttrs; i++) {
    contextAttrs.push_back(properties::get(props, txt()<<"ctxtAttr_"<<i));
    attributes.addObs(properties::get(props, txt()<<"ctxtAttr_"<<i), this);
  }
  active[traceID] = this;
  
  // If we should show the visualization at the beginning of the block
  if(showLoc == showBegin) showViz();
}

trace::~trace() {
  // If we should show the visualization at the end of the block
  if(showLoc == showEnd) showViz();
  
  ostringstream contextAttrsStr;
  if(viz==table || viz == decTree) {
    contextAttrsStr << "[";
    for(std::list<std::string>::iterator a=contextAttrs.begin(); a!=contextAttrs.end(); a++) {
      if(a!=contextAttrs.begin()) contextAttrsStr << ", ";
      contextAttrsStr << "'"<<*a<<"'";
    }
    contextAttrsStr << "]";
  }
  
  ostringstream tracerAttrsStr;
  if(viz==table || viz==lines) {
    tracerAttrsStr << "[";
    for(set<string>::iterator a=tracerKeys.begin(); a!=tracerKeys.end(); a++) {
      if(a!=tracerKeys.begin()) tracerAttrsStr << ", ";
      tracerAttrsStr << "'"<<*a<<"'";
    }
    tracerAttrsStr << "]";
  }
  
  // Now that we know all the trace variables that are included in this trace, emit the trace
  if(viz==table) {
    //dbg.widgetScriptPrologCommand(txt()<<"loadGoogleAPI();");
    ostringstream cmd; 
    cmd<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"-Table', "<<
                       contextAttrsStr.str()<<", " << 
                       tracerAttrsStr.str()<<", "<<  
                       "'"<<common::viz2Str(viz)<<"');";
    dbg.widgetScriptEpilogCommand(cmd.str());
  } else if(viz==lines) {
    // Create a separate decision tree for each context attribute
    for(std::list<std::string>::iterator c=contextAttrs.begin(); c!=contextAttrs.end(); c++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"', ['"<<*c<<"'], "<<tracerAttrsStr.str()<<", '"<<common::viz2Str(viz)<<"');");
    }
  } else if(viz==decTree) {
    // Create a separate decision tree for each tracer attribute
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"', "<<contextAttrsStr.str()<<", ['"<<*t<<"'], '"<<common::viz2Str(viz)<<"');");
    }
  }
  
  /*assert(stack.size()>0);
  stack.pop_back();*/
  assert(active.find(traceID) != active.end());
  active.erase(traceID);
  
  // Stop this object's observations of changes in context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.remObs(*ca, this);
}

// Place the code to show the visualization
void trace::showViz() {
  dbg.enterBlock(this, false, true);
  
  tgtBlockID = getBlockID();
  if(viz==table) {
    dbg << "<div class=\"example yui3-skin-sam\"><div id=\"div"<<tgtBlockID<<"-Table\"></div></div>";
  } else if(viz==lines) {
    // Create a separate line graph for each context attribute
    for(std::list<std::string>::iterator c=contextAttrs.begin(); c!=contextAttrs.end(); c++) {
      dbg << *c << endl;
      dbg << "<div id=\"div"<<tgtBlockID<<"_"<<*c<<"\" style=\"height:300\"></div>\n";
    }
  } else if(viz==decTree) {
    // Create a separate decision tree for each tracer attribute
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg << *t << endl;
      dbg << "<div id=\"div"<<tgtBlockID<<"_"<<*t<<"\"></div>\n";
    }
  }
  
  dbg.exitBlock();
}

// Record an observation
void* trace::observe(properties::iterator props)
{
  long traceID = properties::getInt(props, "traceID");
  assert(active.find(traceID) != active.end());
  trace* t = active[traceID];
  
  long numTraceAttrs = properties::getInt(props, "numTraceAttrs");
  long numCtxtAttrs = properties::getInt(props, "numCtxtAttrs");
  
  // Update the tracer with the keys of this observation's traced values
  for(long i=0; i<numTraceAttrs; i++)
    t->tracerKeys.insert(properties::get(props, txt()<<"tKey_"<<i));
    
  ostringstream cmd;
  cmd << "traceRecord('"<<t->getLabel()<<"', ";
  
  // Emit the observed values of tracer attributes
  cmd << "{";
  for(long i=0; i<numTraceAttrs; i++) {
    if(i!=0) cmd << ", ";
    string tKey = properties::get(props, txt()<<"tKey_"<<i);
    string tVal = properties::get(props, txt()<<"tVal_"<<i);
    cmd << "'"<< tKey << "': '" << tVal <<"'";
  }
  cmd << "}, {";
  
  // Emit the current values of the context attributes
  for(long i=0; i<numCtxtAttrs; i++) {
    if(i!=0) cmd << ", ";
    string cKey = properties::get(props, txt()<<"cKey_"<<i);
    string cVal = properties::get(props, txt()<<"cVal_"<<i);
    cmd << "'" << cKey << "': '" << cVal << "'";
  }
  cmd << "}, '"<<common::viz2Str(t->viz)<<"');";
  
  dbg.widgetScriptCommand(cmd.str());
  
  return NULL;
}

}; // namespace layout
}; // namespace dbglog
