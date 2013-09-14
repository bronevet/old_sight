#include "trace_structure.h"
using namespace std;

namespace dbglog {
namespace structure {

// Maps the names of all the currently active traces to their trace objects
std::map<std::string, trace*> trace::active;  

trace::trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz) : block(label), contextAttrs(contextAttrs) {
  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; exit(-1); }
  
  init(label, showLoc, viz);
}

trace::trace(std::string label, std::string contextAttr, showLocT showLoc, vizT viz) : block(label) {
  contextAttrs.push_back(contextAttr);
  
  init(label, showLoc, viz);
}

void trace::init(std::string label, showLocT showLoc, vizT viz) {
  map<string, string> properties;
  properties["showLoc"] = showLoc2Str(showLoc);
  properties["viz"] = viz2Str(viz);
  
  properties["numCtxtAttrs"] = txt()<<contextAttrs.size();
  
  // Record the attributes this traces uses as context
  int i=0;
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++, i++)
    properties[txt()<<"ctxtAttr_"<<i] = *ca;
  
  dbg.enter("trace", properties);
  
  // Add this trace object as a change listener to all the context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.addObs(*ca, this);
  
  active[label] = this;
}

trace::~trace() {
  dbg.exit("trace");
  
  assert(active.find(getLabel()) != active.end());
  active.erase(getLabel());
  
  // Stop this object's observations of changes in context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.remObs(*ca, this);
}

// Returns a string representation of a showLocT object
std::string trace::showLoc2Str(showLocT showLoc) 
{ return (showLoc==showBegin? "showBegin": (showLoc==showEnd? "showEnd": "???")); }

// Returns a string representation of a vizT object
string trace::viz2Str(vizT viz) {
       if(viz == table)   return "table";
  else if(viz == lines)   return "lines";
  else if(viz == decTree) return "decTree";
  else                    return "???";
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
  
  map<string, string> properties;
  
  //assert(trace::stack.size()>0);
  //trace* t = *(trace::stack.rbegin());
  assert(active.find(getLabel()) != active.end());
  trace* t = active[getLabel()];
  
  properties["traceID"] = t->getBlockID();
  
  properties["numTraceAttrs"] = txt()<<obs.size();
  // Emit the recently observed values of tracer attributes
  int i=0;
  for(map<string, attrValue>::iterator o=obs.begin(); o!=obs.end(); o++, i++) {
    properties[txt()<<"tKey_"<<i] = o->first;
    properties[txt()<<"tVal_"<<i] = o->second.getAsStr();
  }
  
  // Emit the current values of the context attributes
  properties["numCtxtAttrs"] = txt()<<obs.size();
  for(std::list<std::string>::iterator a=t->contextAttrs.begin(); a!=t->contextAttrs.end(); a++) {
    const std::set<attrValue>& vals = attributes.get(*a);
    assert(vals.size()>0);
    if(vals.size()>1) { cerr << "trace::traceAttr() ERROR: context attribute "<<*a<<" has multiple values!"; }
    
    properties[txt()<<"tKey_"<<i] = *a;
    properties[txt()<<"tVal_"<<i] = vals.begin()->getAsStr();
  }
  
  dbg.tag("traceObs", properties);
  
  // Reset the obs[] map since we've just emitted all these observations
  obs.clear();
}

void traceAttr(std::string label, std::string key, const attrValue& val) {
  /*assert(trace::stack.size()>0);
  trace* t = *(trace::stack.rbegin());*/
  assert(trace::active.find(label) != trace::active.end());
  trace* t = trace::active[label];
  
  // Inform the inner-most tracer of the observation
  t->traceAttrObserved(key, val);
}

/*******************
 ***** measure *****
 *******************/
measure::measure(std::string traceLabel, std::string valLabel): traceLabel(traceLabel), valLabel(valLabel)
{
  measureDone = false;
  gettimeofday(&start, NULL);
}

measure::~measure() {
  if(!measureDone) doMeasure();
}

double measure::doMeasure() {
  if(measureDone) { cerr << "measure::doMeasure() ERROR: measuring variable \""<<valLabel<<"\" in trace \""<<traceLabel<<"\" multiple times!"<<endl; exit(-1); }
  measureDone = true;
  
  struct timeval end;
  gettimeofday(&end, NULL);
  
  double elapsed = ((end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec)) / 1000000.0;
  traceAttr(traceLabel, valLabel, attrValue((double)elapsed));
  return elapsed;
}

measure* startMeasure(std::string traceLabel, std::string valLabel) {
  return new measure(traceLabel, valLabel);
}

double endMeasure(measure* m) {
  double result = m->doMeasure();
  delete m;
  return result;
}

}; // namespace structure 
}; // namespace dbglog
