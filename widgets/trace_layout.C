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
  (*layoutEnterHandlers)["trace_traceStream"] = &trace::enterTraceStream;
  (*layoutExitHandlers )["trace_traceStream"] = &defaultExitHandler;
  (*layoutEnterHandlers)["traceObs"] = &traceStream::observe;
  (*layoutExitHandlers )["traceObs"] = &defaultExitHandler;
}
traceLayoutHandlerInstantiator traceLayoutHandlerInstance;

/*****************
 ***** trace *****
 *****************/

// Stack of currently active traces
std::list<trace*> trace::tStack;

trace::trace(properties::iterator props) : block(properties::next(props)) {
  showLoc = (showLocT)properties::getInt(props, "showLoc");
  
  dbg.enterBlock(this, false, true);
  
  // If we should show the visualization at the beginning of the block
  if(showLoc == showBegin) dbg << "<div id=\"'div"<<getBlockID()<<"-Trace\"></div>";
  
  stream = NULL;
    
  tStack.push_back(this);
}

trace::~trace() {
  if(showLoc == showEnd) dbg << "<div id=\"div"<<getBlockID()<<"-Trace\"></div>";
  
  assert(stream);
  delete(stream);
  
  assert(tStack.size()>0);
  assert(tStack.back()==this);
  tStack.pop_back();
}

void *trace::enterTraceStream(properties::iterator props) {
  // Get the currently active trace that this traceStream belongs to
  assert(tStack.size()>0);
  trace* t = tStack.back();
  t->stream = new traceStream(props, txt()<<"div"<<t->getBlockID()<<"-Trace");
  return NULL;
}

/***********************
 ***** traceStream *****
 ***********************/


// Maps the traceIDs of all the currently active traces to their trace objects
std::map<int, traceStream*> traceStream::active;

traceStream::traceStream(properties::iterator props, std::string hostDiv) : hostDiv(hostDiv) {
  static bool initialized = false;
  
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
  viz     = (vizT)properties::getInt(props, "viz");
  
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
  cout << "New Trace "<<traceID<<", this="<<this<<endl;
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

traceStream::~traceStream() {
  cout << "traceStream::~traceStream() this="<<this<<", traceID="<<traceID<<endl;
  // String that contains the names of all the context attributes 
  string ctxtAttrsStr;
  if(viz==table || viz==decTree || viz==heatmap || viz==boxplot)
    ctxtAttrsStr = JSArray<list<string> >(contextAttrs);
  
  // String that contains the names of all the trace attributes
  string tracerAttrsStr;
  if(viz==table || viz==lines || viz==heatmap || viz==boxplot)
    tracerAttrsStr = JSArray<set<string> >(tracerKeys);
  
  assert(hostDiv != "");
  
  // Now that we know all the trace variables that are included in this trace, emit the trace
  if(viz==table || viz==heatmap) {
    //dbg.widgetScriptPrologCommand(txt()<<"loadGoogleAPI();");
    ostringstream cmd; 
    cmd<<"displayTrace('"<<traceID<<"', "<<
                       "'"<<hostDiv<<"', "<<
                       ctxtAttrsStr<<", " << 
                       tracerAttrsStr<<", "<<  
                       "'"<<viz2Str(viz)<<"');";
    dbg.widgetScriptEpilogCommand(cmd.str());
  } else if(viz==lines) {
    // Create a separate line graph for each context attribute
    for(std::set<std::string>::iterator c=contextAttrsSet.begin(); c!=contextAttrsSet.end(); c++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<traceID<<"', "<<
                                    "'"<<hostDiv<<"', "<<
                                    "['"<<*c<<"'], "<<
                                    tracerAttrsStr<<", "<<
                                    "'"<<viz2Str(viz)<<"');");
    }
  } else if(viz==decTree) {
    // Create a separate decision tree for each tracer attribute
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<traceID<<"', "<<
                                    "'"<<hostDiv<<"', "<<
                                    ctxtAttrsStr<<", "<<
                                    "['"<<*t<<"'], "<<
                                    "'"<<viz2Str(viz)<<"');");
    }
  } else if(viz==boxplot) {
    dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<traceID<<"', "<<
                                  "'"<<hostDiv<<"', "<<
                                  ctxtAttrsStr<<", "<<
                                  tracerAttrsStr<<", "<<
                                  "'"<<viz2Str(viz)<<"');");
  }
  
  /*assert(stack.size()>0);
  stack.pop_back();*/
  assert(active.find(traceID) != active.end());
  active.erase(traceID);
}

// Record an observation
void* traceStream::observe(properties::iterator props)
{
  //cout << "trace::observe() props="<<properties::str(props)<<endl;
  long traceID = properties::getInt(props, "traceID");
  assert(active.find(traceID) != active.end());
  traceStream* ts = active[traceID];
  //cout << "    t="<<t<<endl;
  //cout << "    #ts->tracerKeys="<<ts->tracerKeys.size()<<endl;
  
  long numTraceAttrs = properties::getInt(props, "numTraceAttrs");
  long numCtxtAttrs = properties::getInt(props, "numCtxtAttrs");
    
  // Read all the context attributes. If contextAttrs is empty, it is filled with the context attributes of 
  // this observation. Otherwise, we verify that this observation's context is identical to prior observations.

  if(ts->contextAttrsInitialized) assert(ts->contextAttrs.size() == numCtxtAttrs);
  for(long i=0; i<numCtxtAttrs; i++) {
    string ctxtName = properties::get(props, txt()<<"cKey_"<<i);
    if(!ts->contextAttrsInitialized) {
      ts->contextAttrs.push_back(ctxtName);
      // Context attributes cannot be repeated
      assert(ts->contextAttrsSet.find(ctxtName) == ts->contextAttrsSet.end());
      ts->contextAttrsSet.insert(ctxtName);
    } else
      assert(ts->contextAttrsSet.find(ctxtName) != ts->contextAttrsSet.end());
  }
  // The context attributes of this trace are now definitely initialized
  ts->contextAttrsInitialized = true;
  
  // Update the tracer with the keys of this observation's traced values
  for(long i=0; i<numTraceAttrs; i++)
    ts->tracerKeys.insert(properties::get(props, txt()<<"tKey_"<<i));
    
  ostringstream cmd;
  cmd << "traceRecord(\""<<ts->traceID<<"\", ";
  
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
  cmd << "}, \""<<viz2Str(ts->viz)<<"\");";
  
  dbg.widgetScriptCommand(cmd.str());
  
  return NULL;
}

// Given a traceID returns a pointer to the corresponding trace object
traceStream* traceStream::get(int traceID) {
  std::map<int, traceStream*>::iterator it = active.find(traceID);
  assert(it != active.end());
  return it->second;
}

}; // namespace layout
}; // namespace sight
