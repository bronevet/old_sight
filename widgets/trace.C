#include "trace.h"
using namespace std;

namespace dbglog {

// Stack of currently active traces
std::list<trace*> trace::stack;
  
// Records whether the tracer infrastructure has been initialized
bool trace::initialized=false;

trace::trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz) : block(label), contextAttrs(contextAttrs), showLoc(showLoc), viz(viz) {
  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; exit(-1); }
  
  init();
}

trace::trace(std::string label, std::string contextAttr, showLocT showLoc, vizT viz) : block(label), showLoc(showLoc), viz(viz) {
  contextAttrs.push_back(contextAttr);
  
  init();
}


void trace::init() {
  if(!initialized) {
    // Table visualization
    //dbg.includeScript("https://www.google.com/jsapi", "text/javascript");
    dbg.includeScript("http://yui.yahooapis.com/3.11.0/build/yui/yui-min.js", "text/javascript");
    
    // Decision Tree
    dbg.includeScript("http://code.jquery.com/jquery-1.8.1.min.js",                                  "text/javascript");
    dbg.includeScript("http://cdnjs.cloudflare.com/ajax/libs/underscore.js/1.3.3/underscore-min.js", "text/javascript");
    dbg.includeScript("http://d3js.org/d3.v2.js", "text/javascript");
    dbg.includeWidgetScript("ID3-Decision-Tree/js/id3.js", "text/javascript");
    //dbg.includeScript("https://www.google.com/jsapi?autoload={\"modules\":[{\"name\":\"visualization\",\"version\":\"1\",\"packages\":[\"orgchart\"]}]}", "text/javascript");
    dbg.includeFile("ID3-Decision-Tree");

    dbg.includeFile("trace.js");
    dbg.includeWidgetScript("trace.js", "text/javascript");
    
    initialized = true;
  }
  
  // Add this trace object as a change listener to all the context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.addObs(*ca, this);
  
  stack.push_back(this);
  
  // If we should show the visualization at the beginning of the block
  if(showLoc == showBegin) showViz();
}

trace::~trace() {
  // Emit any observations performed since the last change of any of the context variables
  emitObservations();
  
  // If we should show the visualization at the end of the block
  if(showLoc == showEnd) showViz();
  
  assert(stack.size()>0);
  stack.pop_back();
  
  // Stop this object's observations of changes in context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.remObs(*ca, this);
}

// Returns a string representation of a vizT object
string trace::viz2Str(vizT viz) {
       if(viz == table)   return "table";
  else if(viz == decTree) return "decTree";
  else                    return "???";
}
// Place the code to show the visualization
void trace::showViz() {
  dbg.enterBlock(this, false, true);
  if(viz==table) {
    //dbg.widgetScriptPrologCommand(txt()<<"loadGoogleAPI();");
    dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<getBlockID()<<"-Table', '', '"<<viz2Str(viz)<<"');");
    dbg << "<div class=\"example yui3-skin-sam\"><div id=\"div"<<getBlockID()<<"-Table\"></div></div>";
  } else if(viz==decTree) {
    // Create a separate decision tree for each tracer attribute
    for(set<string>::iterator t=tracerKeys.begin(); t!=tracerKeys.end(); t++) {
      dbg << *t << endl;
      dbg << "<div id=\"div"<<getBlockID()<<":"<<*t<<"\"></div>\n";
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<getBlockID()<<"', '"<<*t<<"', '"<<viz2Str(viz)<<"');");
    }
  }
  dbg.exitBlock();
}

// Observe for changes to the values mapped to the given key
void trace::observePre(std::string key)
{
  // Emit any observations performed since the last change of any of the context variables
  emitObservations();
}

// Called by traceAttr() to inform the trace that a new observation has been made
void trace::traceAttrObserved(std::string key, const attrValue& val) {
  obs[key] = val;
  tracerKeys.insert(key);
}

// Emits the JavaScript command that encodes the observations made since the last time a context attribute changed
void trace::emitObservations() {
  // Only emit observations of the trace variables if we have made any observations since the last change in the context variables
  if(obs.size()==0) return;
    
  assert(trace::stack.size()>0);
  trace* t = *(trace::stack.rbegin());
  
  ostringstream cmd;
  cmd << "traceRecord(";
  
  // Emit the recently observed values of tracer attributes
  cmd << "{";
  for(map<string, attrValue>::iterator i=obs.begin(); i!=obs.end(); i++) {
    if(i!=obs.begin()) cmd << ", ";
    cmd << i->first << ": '" << i->second.getAsStr()<<"'";
  }
  
  cmd << "}, {";
  
  // Emit the current values of the context attributes
  for(std::list<std::string>::iterator a=t->contextAttrs.begin(); a!=t->contextAttrs.end(); a++) {
    if(a!=t->contextAttrs.begin()) cmd << ", ";
    const std::set<attrValue>& vals = attributes.get(*a);
    assert(vals.size()>0);
    if(vals.size()>1) { cerr << "trace::traceAttr() ERROR: context attribute "<<*a<<" has multiple values!"; }
    cmd << *a << ": '" << vals.begin()->getAsStr() << "'";
  }
  cmd << "}, '"<<viz2Str(viz)<<"');";
  dbg.widgetScriptCommand(cmd.str());
  
  // Reset the obs[] map since we've just emitted all these observations
  obs.clear();
}

void traceAttr(std::string key, const attrValue& val) {
  assert(trace::stack.size()>0);
  trace* t = *(trace::stack.rbegin());
  
  // Inform the inner-most tracer of the observation
  t->traceAttrObserved(key, val);
}

}; // namespace dbglog
