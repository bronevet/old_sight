#include "../sight_layout_internal.h"
#include "trace_layout.h"

using namespace std;

namespace sight {
namespace layout {

// Record the layout handlers in this file
void* traceEnterHandler(properties::iterator props) { return new trace(props); }
void  traceExitHandler(void* obj) { trace* t = static_cast<trace*>(obj); delete t; }
  
traceLayoutHandlerInstantiator::traceLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["trace"] = &traceEnterHandler;
  (*layoutExitHandlers )["trace"] = &traceExitHandler;
  (*layoutEnterHandlers)["traceObs"] = &trace::observe;
  (*layoutExitHandlers )["traceObs"] = &defaultExitHandler;
}
traceLayoutHandlerInstantiator traceLayoutHandlerInstance;

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
    
    // JQuery must be loaded before prototype.js. Because we don't have a clean way to guaranteed this, we'll load jquery in sight_layout.C
    //dbg.includeWidgetScript("jquery-1.8.1.min.js", "text/javascript"); dbg.includeFile("jquery-1.8.1.min.js");
    dbg.includeWidgetScript("underscore-min.js",   "text/javascript"); dbg.includeFile("underscore-min.js");
    
    dbg.includeWidgetScript("ID3-Decision-Tree/js/id3.js", "text/javascript");
    //dbg.includeScript("https://www.google.com/jsapi?autoload={\"modules\":[{\"name\":\"visualization\",\"version\":\"1\",\"packages\":[\"orgchart\"]}]}", "text/javascript");
    dbg.includeFile("ID3-Decision-Tree");
    
    // D3 Widgets
    dbg.includeWidgetScript("d3.v3.min.js", "text/javascript"); dbg.includeFile("d3.v3.min.js");
    dbg.includeWidgetScript("boxplot.js",   "text/javascript"); dbg.includeFile("boxplot.js");
    dbg.includeWidgetScript("gradient.js",  "text/javascript"); dbg.includeFile("gradient.js");

    dbg.includeFile("trace.js"); dbg.includeWidgetScript("trace.js", "text/javascript");
    
    initialized = true;
  }
  
  traceID = properties::getInt(props, "traceID");
  showLoc = (showLocT)properties::getInt(props, "showLoc");
  viz     = (vizT)    properties::getInt(props, "viz");
  
  // Add this trace object as a change listener to all the context variables
  long numCtxtAttrs = properties::getInt(props, "numCtxtAttrs");
  if(numCtxtAttrs > 0) {
    for(long i=0; i<numCtxtAttrs; i++) {
      string ctxtName = properties::get(props, txt()<<"ctxtAttr_"<<i);
      contextAttrs.push_back(ctxtName);
      // Context attributes cannot be repeated
      assert(contextAttrsSet.find(ctxtName) == contextAttrsSet.end());
      contextAttrsSet.insert(ctxtName);
      //attributes.addObs(properties::get(props, txt()<<"ctxtAttr_"<<i), this);
    }
    contextAttrsInitialized = true;
  } else
    contextAttrsInitialized = false;
  
  active[traceID] = this;
  cout << "New Trace "<<traceID<<", this="<<this<<", label="<<getLabel()<<endl;
  
  dbg.enterBlock(this, false, true);
  
  // If we should show the visualization at the beginning of the block
  if(showLoc == showBegin) showViz();
}

// Returns the representation of the given list as a JavaScript array
template<class aggr>
string JSArray(const aggr& l) {
  ostringstream s;
  s << "[";
  for(typename aggr::const_iterator a=l.begin(); a!=l.end(); a++) {
    if(a!=l.begin()) s << ", ";
    s << "'"<<*a<<"'";
  }
  s << "]";
  return s.str();
}

trace::~trace() {
  cout << "trace::~trace() this="<<this<<", traceID="<<traceID<<endl;
  // If we should show the visualization at the end of the block
  if(showLoc == showEnd) showViz();
  
  string splitCtxtStr = JSArray<list<string> >(splitContextAttrs);
  
  // String that contains the names of all the context attributes 
  string projectCtxtStr;
  if(viz==table || viz==decTree || viz==heatmap || viz==boxplot)
    projectCtxtStr = JSArray<list<string> >(projectContextAttrs);
  
  // String that contains the names of all the trace attributes
  string tracerAttrsStr;
  if(viz==table || viz==lines || viz==heatmap || viz==boxplot)
    tracerAttrsStr = JSArray<set<string> >(tracerKeys);
  
  string tgtBlockID = getBlockID();
  
  // Now that we know all the trace variables that are included in this trace, emit the trace
  if(viz==table || viz==heatmap) {
    //dbg.widgetScriptPrologCommand(txt()<<"loadGoogleAPI();");
    ostringstream cmd; 
    cmd<<"displayTrace('"<<getLabel()<<"', "<<
                       splitCtxtStr<<", "<<
                       projectCtxtStr<<", " << 
                       tracerAttrsStr<<", "<<  
                       "'div"<<tgtBlockID<<"', undefined, "<<
                       "'"<<viz2Str(viz)<<"', "<<
                       "'"<<showLoc2Str(showLoc)<<"');";
    dbg.widgetScriptEpilogCommand(cmd.str());
  } else if(viz==lines) {
    // Create a separate line graph for each context attribute
    for(std::list<std::string>::iterator c=projectContextAttrs.begin(); c!=projectContextAttrs.end(); c++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', "<<
                                    splitCtxtStr<<", "<<
                                    "['"<<*c<<"'], "<<
                                    tracerAttrsStr<<", "<<
                                    "'div"<<tgtBlockID<<"', undefined, "<<
                                    "'"<<viz2Str(viz)<<"', "<<
                                    "'"<<showLoc2Str(showLoc)<<"');");
    }
  } else if(viz==decTree) {
    // Create a separate decision tree for each tracer attribute
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', "<<
                                    splitCtxtStr<<", "<<
                                    projectCtxtStr<<", "<<
                                    "['"<<*t<<"'], "<<
                                    "'div"<<tgtBlockID<<"', undefined, "<<
                                    "'"<<viz2Str(viz)<<"', "<<
                                    "'"<<showLoc2Str(showLoc)<<"');");
    }
  } else if(viz==boxplot) {
    // Create a separate set of box plots for each combination of context and tracer attributes
    /*for(std::list<std::string>::iterator c=contextAttrs.begin(); c!=contextAttrs.end(); c++) {
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"', ['"<<*c<<"'], ['"<<*t<<"'], '"<<viz2Str(viz)<<"');");
    } }*/
    dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', "<<
                                  splitCtxtStr<<", "<<
                                  projectCtxtStr<<", "<<
                                  tracerAttrsStr<<", "<<
                                  "'div"<<tgtBlockID<<"', undefined, "<<
                                  "'"<<viz2Str(viz)<<"', "<<
                                  "'"<<showLoc2Str(showLoc)<<"');");
  }
  
  /*assert(stack.size()>0);
  stack.pop_back();*/
  assert(active.find(traceID) != active.end());
  active.erase(traceID);
  
  /* // Stop this object's observations of changes in context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.remObs(*ca, this);*/
  
  dbg.exitBlock();
}

// Place the code to show the visualization
void trace::showViz() {
  /*tgtBlockID = getBlockID();
  if(viz==table) {
    dbg << "<div class=\"example yui3-skin-sam\"><div id=\"'div"<<tgtBlockID<<"'-Table\"></div></div>";
  } else if(viz==lines) {
    // Create a separate line graph for each context attribute
    for(std::list<std::string>::iterator c=contextAttrs.begin(); c!=contextAttrs.end(); c++) {
      dbg << *c << endl;
      dbg << "<div id=\"'div"<<tgtBlockID<<"'_"<<*c<<"\" style=\"height:300\"></div>\n";
    }
  } else if(viz==decTree) {
    // Create a separate decision tree for each tracer attribute
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg << *t << endl;
      dbg << "<div id=\"'div"<<tgtBlockID<<"'_"<<*t<<"\"></div>\n";
    }
  } else if(viz==heatmap) {
    dbg << "<div id=\"'div"<<tgtBlockID<<"'-Heatmap\"></div>";
  }*/
}

void module::setSplitCtxtHostDivs(const std::list<std::pair<std::list<std::pair<std::string, std::string> >, std::string> >& splitCtxtHostDivs)
{ 
  this->splitCtxtHostDivs = splitCtxtHostDivs;
  
  // Move all the attribute names in splitCtxtHostDivs from projectContextAttrs to splitContextAttrs
  for(list<pair<list<pair<string, string> >, string> >::iterator i=splitCtxtHostDivs.begin(); 
      i!=splitCtxtHostDivs.end(); i++) {
    if(i==splitCtxtHostDivs.begin()) {
      for(list<pair<string, string> >::iterator j=i->first.begin(); j!=i->first.end(); j++) {
        // Move the current key from projectContextAttrs to splitContextAttrs
        projectContextAttrs.remove(j->first);
        splitContextAttrs.push_back(j->first);
      }
    } else {
      assert(i->first.size() == projectContextAttrs.size());
      list<string>::iterator p=projectContextAttrs;
      for(list<pair<string, string> >::iterator j=i->first.begin(); j!=i->first.end(); j++, p++)
      { assert(j->first == *p); }
    }
  }
}

// Record an observation
void* trace::observe(properties::iterator props)
{
  //cout << "trace::observe() props="<<properties::str(props)<<endl;
  long traceID = properties::getInt(props, "traceID");
  assert(active.find(traceID) != active.end());
  trace* t = active[traceID];
  //cout << "    t="<<t<<endl;
  //cout << "    #t->tracerKeys="<<t->tracerKeys.size()<<endl;
  
  long numTraceAttrs = properties::getInt(props, "numTraceAttrs");
  long numCtxtAttrs = properties::getInt(props, "numCtxtAttrs");
    
  // Read all the context attributes. If contextAttrs is empty, it is filled with the context attributes of 
  // this observation. Otherwise, we verify that this observation's context is identical to prior observations.
  
/*  // If this trace has no explicit context attributes
  if(numCtxtAttrs==0) {
    if(t->contextAttrsInitialized) assert(t->contextAttrs.size() == 1);
    // Add a single attribute that contains the instance ID*/

  if(t->contextAttrsInitialized) assert(t->contextAttrs.size() == numCtxtAttrs);
  for(long i=0; i<numCtxtAttrs; i++) {
    string ctxtName = properties::get(props, txt()<<"cKey_"<<i);
    if(!t->contextAttrsInitialized) {
      t->contextAttrs.push_back(ctxtName);
      // Context attributes cannot be repeated
      assert(t->contextAttrsSet.find(ctxtName) == t->contextAttrsSet.end());
      t->contextAttrsSet.insert(ctxtName);
    } else
      assert(t->contextAttrsSet.find(ctxtName) != t->contextAttrsSet.end());
  }
  if(!t->contextAttrsInitialized) {
    // Initialize so that we project on all the context attributes. This can be adjusted on request.
    t->projectContextAttrs = t->contextAttrs;
    // The context attributes of this trace are now definitely initialized
    t->contextAttrsInitialized = true;
  }
  
  // Update the tracer with the keys of this observation's traced values
  for(long i=0; i<numTraceAttrs; i++)
    t->tracerKeys.insert(properties::get(props, txt()<<"tKey_"<<i));
    
  ostringstream cmd;
  cmd << "traceRecord(\""<<t->getLabel()<<"\", ";
  
  // Emit the observed values of tracer attributes
  cmd << "{";
  for(long i=0; i<numTraceAttrs; i++) {
    if(i!=0) cmd << ", ";
    string tKey = properties::get(props, txt()<<"tKey_"<<i);
    string tVal = properties::get(props, txt()<<"tVal_"<<i);
    cmd << "\""<< tKey << "\": \"" << tVal <<"\"";
  }
  cmd << "}, {";
  
  // Emit the observed anchors of tracer attributes
  for(long i=0; i<numTraceAttrs; i++) {
    if(i!=0) cmd << ", ";
    string tKey = properties::get(props, txt()<<"tKey_"<<i);
    anchor tAnchor(properties::getInt(props, txt()<<"tAnchorID_"<<i));
    cmd << "\""<< tKey << "\": \"" << (tAnchor==anchor::noAnchor? "": tAnchor.getLinkJS()) <<"\"";
  }
  cmd << "}, {";
  
  // Emit the current values of the context attributes
  for(long i=0; i<numCtxtAttrs; i++) {
    if(i!=0) cmd << ", ";
    string cKey = properties::get(props, txt()<<"cKey_"<<i);
    string cVal = properties::get(props, txt()<<"cVal_"<<i);
    cmd << "\"" << cKey << "\": \"" << cVal << "\"";
  }
  cmd << "}, \""<<viz2Str(t->viz)<<"\");";
  
  dbg.widgetScriptCommand(cmd.str());
  
  return NULL;
}

// Given a traceID returns a pointer to the corresponding trace object
trace* trace::getTrace(int traceID) {
  std::map<int, trace*>::iterator it = active.find(traceID);
  assert(it != active.end());
  return it->second;
}

}; // namespace layout
}; // namespace sight
