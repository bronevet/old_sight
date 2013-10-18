#include "trace.h"
using namespace std;

namespace dbglog {

// Stack of currently active traces
//std::list<trace*> trace::stack;
std::map<std::string, trace*> trace::active;  

// Records whether the tracer infrastructure has been initialized
bool trace::initialized=false;

trace::trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz) : block(label), contextAttrs(contextAttrs), showLoc(showLoc), viz(viz) {
  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; exit(-1); }
  
  init(label);
}

trace::trace(std::string label, std::string contextAttr, showLocT showLoc, vizT viz) : block(label), showLoc(showLoc), viz(viz) {
  contextAttrs.push_back(contextAttr);
  
  init(label);
}


void trace::init(std::string label) {
  if(viz==heatmap && contextAttrs.size()!=2) { cerr << "ERROR heatmap tables require 2 context attributes!"; exit(-1); }
  
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
    
    dbg.includeWidgetScript("gradient.js",         "text/javascript"); dbg.includeFile("gradient.js");
    
    dbg.includeWidgetScript("ID3-Decision-Tree/js/id3.js", "text/javascript");
    //dbg.includeScript("https://www.google.com/jsapi?autoload={\"modules\":[{\"name\":\"visualization\",\"version\":\"1\",\"packages\":[\"orgchart\"]}]}", "text/javascript");
    dbg.includeFile("ID3-Decision-Tree");

    // D3 Widgets
    dbg.includeScript("http://d3js.org/d3.v3.min.js", "text/javascript");
    dbg.includeWidgetScript("boxplot.js", "text/javascript"); dbg.includeFile("boxplot.js");


    dbg.includeFile("trace.js"); dbg.includeWidgetScript("trace.js", "text/javascript");
    
    initialized = true;
  }
  
  // Add this trace object as a change listener to all the context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.addObs(*ca, this);
  
  //stack.push_back(this);
  active[label] = this;
  
  dbg.enterBlock(this, false, true);
  
  tgtBlockID = getBlockID();
  
  // If we should show the visualization at the beginning of the block
  if(showLoc == showBegin) showViz();
}

trace::~trace() {
  // Emit any observations performed since the last change of any of the context variables
  emitObservations();
  
  // If we should show the visualization at the end of the block
  if(showLoc == showEnd) showViz();
  
  // String that contains the names of all the context attributes 
  ostringstream contextAttrsStr;
  if(viz==table || viz==decTree || viz==heatmap || viz==boxplot) {
    contextAttrsStr << "[";
    for(std::list<std::string>::iterator a=contextAttrs.begin(); a!=contextAttrs.end(); a++) {
      if(a!=contextAttrs.begin()) contextAttrsStr << ", ";
      contextAttrsStr << "'"<<*a<<"'";
    }
    contextAttrsStr << "]";
  }
  
  // String that contains the names of all the trace attributes
  ostringstream tracerAttrsStr;
  if(viz==table || viz==lines || viz==heatmap || viz==boxplot) {
    tracerAttrsStr << "[";
    for(set<string>::iterator a=tracerKeys.begin(); a!=tracerKeys.end(); a++) {
      if(a!=tracerKeys.begin()) tracerAttrsStr << ", ";
      tracerAttrsStr << "'"<<*a<<"'";
    }
    tracerAttrsStr << "]";
  }
  
  // Now that we know all the trace variables that are included in this trace, emit the trace
  if(viz==table || viz==heatmap) {
    //dbg.widgetScriptPrologCommand(txt()<<"loadGoogleAPI();");
    ostringstream cmd; 
    cmd<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"-"<<(viz==table?"Table": (viz==heatmap?"Heatmap": "???"))<<"', "<<
                       contextAttrsStr.str()<<", " << 
                       tracerAttrsStr.str()<<", "<<  
                       "'"<<viz2Str(viz)<<"', "<<
                       "'"<<showLoc2Str(showLoc)<<"');";
    dbg.widgetScriptEpilogCommand(cmd.str());
  } else if(viz==lines) {
    // Create a separate decision tree for each context attribute
    for(std::list<std::string>::iterator c=contextAttrs.begin(); c!=contextAttrs.end(); c++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"', ['"<<*c<<"'], "<<tracerAttrsStr.str()<<", "<<
                                    "'"<<viz2Str(viz)<<"', "<<
                                    "'"<<showLoc2Str(showLoc)<<"');");
    }
  } else if(viz==decTree) {
    // Create a separate decision tree for each tracer attribute
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"', "<<contextAttrsStr.str()<<", ['"<<*t<<"'], "<<
                                    "'"<<viz2Str(viz)<<"', "<<
                                    "'"<<showLoc2Str(showLoc)<<"');");
    }
  } else if(viz==boxplot) {
    // Create a separate set of box plots for each combination of context and tracer attributes
    /*for(std::list<std::string>::iterator c=contextAttrs.begin(); c!=contextAttrs.end(); c++) {
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"', ['"<<*c<<"'], ['"<<*t<<"'], '"<<viz2Str(viz)<<"');");
    } }*/
    dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<tgtBlockID<<"', "<<
                                  contextAttrsStr.str()<<", "<<
                                  tracerAttrsStr.str()<<", "<<
                                  "'"<<viz2Str(viz)<<"', "<<
                                  "'"<<showLoc2Str(showLoc)<<"');");
  }
  
  /*assert(stack.size()>0);
  stack.pop_back();*/
  assert(active.find(getLabel()) != active.end());
  active.erase(getLabel());
  
  // Stop this object's observations of changes in context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.remObs(*ca, this);

  dbg.exitBlock();
}

// Returns a string representation of a vizT object
string trace::viz2Str(vizT viz) {
       if(viz == table)   return "table";
  else if(viz == lines)   return "lines";
  else if(viz == decTree) return "decTree";
  else if(viz == heatmap) return "heatmap";
  else if(viz == boxplot) return "boxplot";
  else                    return "???";
}

// Returns a string representation of a showLocT object
std::string trace::showLoc2Str(showLocT showLoc) {
       if(showLoc == showBegin) return "showBegin";
  else if(showLoc == showEnd)   return "showEnd";
  else                          return "???";  
}

// Place the code to show the visualization
void trace::showViz() {
  dbg.enterBlock(this, false, true);
  
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
  } else if(viz==heatmap) {
    dbg << "<div id=\"div"<<tgtBlockID<<"-Heatmap\"></div>";
  } /*else if(viz==boxplot) {
cout << "#contextAttrs="<<contextAttrs.size()<<", #tracerKeys="<<tracerKeys.size()<<endl;
    // Create a separate set of box plots for each combination of context and tracer attributes
    for(std::list<std::string>::iterator c=contextAttrs.begin(); c!=contextAttrs.end(); c++) {
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg << "Context="<<*c<<", Trace="<<*t<<endl;
      dbg << "<div id=\"div"<<tgtBlockID<<"_"<<*c<<"_"<<*t<<"\"></div>\n";
    } }
  }*/
  
  dbg.exitBlock();
}

// Observe for changes to the values mapped to the given key
void trace::observePre(std::string key)
{
  // Emit any observations performed since the last change of any of the context variables
  emitObservations();
}

// Called by traceAttr() to inform the trace that a new observation has been made
void trace::traceAttrObserved(std::string key, const attrValue& val, anchor target) {
  obs[key] = make_pair(val, target);
  tracerKeys.insert(key);
}

// Emits the JavaScript command that encodes the observations made since the last time a context attribute changed
void trace::emitObservations() {
  // Only emit observations of the trace variables if we have made any observations since the last change in the context variables
  if(obs.size()==0) return;
    
  //assert(trace::stack.size()>0);
  //trace* t = *(trace::stack.rbegin());
  assert(active.find(getLabel()) != active.end());
  trace* t = active[getLabel()];
  
  ostringstream cmd;
  cmd << "traceRecord('"<<getLabel()<<"', ";
  
  // Emit the recently observed values of tracer attributes
  cmd << "{";
  for(map<string, pair<attrValue, anchor> >::iterator i=obs.begin(); i!=obs.end(); i++) {
    if(i!=obs.begin()) cmd << ", ";
    cmd << "'"<<i->first << "': '" << i->second.first.getAsStr()<<"'";
  }
  
  cmd << "}, {";
  
  // Emit the anchors observed tracer values
  for(map<string, pair<attrValue, anchor> >::iterator i=obs.begin(); i!=obs.end(); i++) {
    if(i!=obs.begin()) cmd << ", ";
    cmd << "'"<<i->first << "': \"javascript:" << (i->second.second==anchor::noAnchor? "": i->second.second.getLinkJS())<<"\"";
  }
  
  cmd << "}, {";
  
  // Emit the current values of the context attributes
  for(std::list<std::string>::iterator a=t->contextAttrs.begin(); a!=t->contextAttrs.end(); a++) {
    if(a!=t->contextAttrs.begin()) cmd << ", ";
    const std::set<attrValue>& vals = attributes.get(*a);
    assert(vals.size()>0);
    if(vals.size()>1) { cerr << "trace::traceAttr() ERROR: context attribute "<<*a<<" has multiple values!"; }
    cmd << "'" << *a << "': '" << vals.begin()->getAsStr() << "'";
  }
  cmd << "}, '"<<viz2Str(viz)<<"');";
  widgetScriptCommand(cmd.str());
  
  // Reset the obs[] map since we've just emitted all these observations
  obs.clear();
}

void traceAttr(std::string label, std::string key, const attrValue& val) {
  /*assert(trace::stack.size()>0);
  trace* t = *(trace::stack.rbegin());*/
  // Find the tracer with the name label
  if(trace::active.find(label) == trace::active.end()) { cerr << "traceAttr() ERROR: trace \""<<label<<"\" not active when observation \""<<key<<"\"=>\""<<val.str()<<"\" was observed!"; assert(0); }
  trace* t = trace::active[label];
  
  // Inform the chosen tracer of the observation
  t->traceAttrObserved(key, val, anchor::noAnchor);
}

void traceAttr(std::string label, std::string key, const attrValue& val, anchor target) {
  // Find the tracer with the name label
  if(trace::active.find(label) == trace::active.end()) { cerr << "traceAttr() ERROR: trace \""<<label<<"\" not active when observation \""<<key<<"\"=>\""<<val.str()<<"\" was observed!"; assert(0); }
  trace* t = trace::active[label];
  
  // Inform the chosen tracer of the observation
  t->traceAttrObserved(key, val, target);
}

/*******************
 ***** measure *****
 *******************/
measure::measure(std::string traceLabel, std::string valLabel): traceLabel(traceLabel), valLabel(valLabel)
{
  elapsed = 0.0;
  measureDone = false;
  paused = false;
  gettimeofday(&lastStart, NULL);
}

measure::~measure() {
  if(!measureDone) doMeasure();
}

double measure::doMeasure() {
  if(measureDone) { cerr << "measure::doMeasure() ERROR: measuring variable \""<<valLabel<<"\" in trace \""<<traceLabel<<"\" multiple times!"<<endl; exit(-1); }
  measureDone = true;
 
  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  traceAttr(traceLabel, valLabel, attrValue((double)elapsed));
  return elapsed;
}

// Pauses the measurement so that time elapsed between this call and resume() is not counted.
// Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
bool measure::pause() {
  bool modified = paused==false;
  paused = true;

  struct timeval end;
  gettimeofday(&end, NULL);

  elapsed += ((end.tv_sec*1000000 + end.tv_usec) - (lastStart.tv_sec*1000000 + lastStart.tv_usec)) / 1000000.0;

  return modified;  
}

// Restarts counting time. Time collection is restarted regardless of how many times pause() was called
// before the call to resume().
bool measure::resume() {
  gettimeofday(&lastStart, NULL);
  paused = false;
}

measure* startMeasure(std::string traceLabel, std::string valLabel) {
  return new measure(traceLabel, valLabel);
}

double endMeasure(measure* m) {
  double result = m->doMeasure();
  delete m;
  return result;
}


}; // namespace dbglog
