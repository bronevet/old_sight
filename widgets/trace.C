#include "trace.h"
using namespace std;

namespace dbglog {

// Stack of currently active traces
std::list<trace*> trace::stack;
  
// Records whether the tracer infrastructure has been initialized
bool trace::initialized=false;

trace::trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc) : block(label), contextAttrs(contextAttrs), showLoc(showLoc) {
  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; exit(-1); }
  
  init();
}

trace::trace(std::string label, std::string contextAttr, showLocT showLoc) : block(label), showLoc(showLoc) {
  contextAttrs.push_back(contextAttr);
  
  init();
}


void trace::init() {
  if(!initialized) {
    dbg.includeFile("trace.js");
    //dbg.includeScript("https://www.google.com/jsapi", "text/javascript");
    dbg.includeScript("http://yui.yahooapis.com/3.11.0/build/yui/yui-min.js", "text/javascript");
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

// Place the code to show the visualization
void trace::showViz() {
  dbg << "<div class=\"example yui3-skin-sam\">\n";
  dbg.enterBlock(this, false, true);
  
  //dbg.widgetScriptPrologCommand(txt()<<"loadGoogleAPI();");
  dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<getBlockID()<<"');");
  
  dbg.exitBlock();
  dbg << "</div>\n";
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
  cmd << "});";
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
