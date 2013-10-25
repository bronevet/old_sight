#include "../dbglog_common.h"
#include "../dbglog_structure.h"
using namespace std;
using namespace dbglog::common;
  
namespace dbglog {
namespace structure {

// Maximum ID assigned to any trace object
int trace::maxTraceID=0;

// Maps the names of all the currently active traces to their trace objects
std::map<std::string, trace*> trace::active;  

trace::trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz, properties* props) : 
  block(label, setProperties(maxTraceID, showLoc, viz, contextAttrs, props)), contextAttrs(contextAttrs)
{
  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; exit(-1); }
  
  init(label, showLoc, viz);
}

trace::trace(std::string label, std::string contextAttr, showLocT showLoc, vizT viz, properties* props) : 
  block(label, setProperties(maxTraceID, showLoc, viz, contextAttr, props))
{
  contextAttrs.push_back(contextAttr);
  
  init(label, showLoc, viz);
}

// Sets the properties of this object
properties* trace::setProperties(int traceID, showLocT showLoc, vizT viz, const std::list<std::string>& contextAttrs, properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> newProps;
  newProps["traceID"] = txt()<<traceID;
  newProps["showLoc"] = txt()<<showLoc;
  newProps["viz"]     = txt()<<viz;
  
  newProps["numCtxtAttrs"] = txt()<<contextAttrs.size();
  
  // Record the attributes this traces uses as context
  int i=0;
  for(list<string>::const_iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++, i++)
    newProps[txt()<<"ctxtAttr_"<<i] = *ca;
  
  props->add("trace", newProps);
  
  //dbg.enter("trace", properties, inheritedFrom);
  return props;
}

properties* trace::setProperties(int traceID, showLocT showLoc, vizT viz, std::string contextAttr, properties* props) {
  std::list<std::string> contextAttrs;
  contextAttrs.push_back(contextAttr);
  return setProperties(traceID, showLoc, viz, contextAttrs, props);
}

void trace::init(std::string label, showLocT showLoc, vizT viz) {
/*  map<string, string> properties;
  properties["showLoc"] = common::showLoc2Str(showLoc);
  properties["viz"] = common::viz2Str(viz);
  
  properties["numCtxtAttrs"] = txt()<<contextAttrs.size();
  
  // Record the attributes this traces uses as context
  int i=0;
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++, i++)
    properties[txt()<<"ctxtAttr_"<<i] = *ca;
  
  dbg.enter("trace", properties, inheritedFrom);*/
  
  traceID = maxTraceID;
  maxTraceID++;
  
  // Add this trace object as a change listener to all the context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.addObs(*ca, this);
  
  active[label] = this;
}

trace::~trace() {
  //dbg.exit("trace");
  //dbg.exit(this);
  
  assert(active.find(getLabel()) != active.end());
  active.erase(getLabel());
  
  // Stop this object's observations of changes in context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.remObs(*ca, this);
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
    
  dbglogObj *obj = new dbglogObj(new properties());
  
  map<string, string> newProps;
  
  //assert(trace::stack.size()>0);
  //trace* t = *(trace::stack.rbegin());
  assert(active.find(getLabel()) != active.end());
  trace* t = active[getLabel()];
  
  newProps["traceID"] = txt()<<t->traceID;
  
  newProps["numTraceAttrs"] = txt()<<obs.size();
  // Emit the recently observed values and anchors of tracer attributes
  int i=0;
  for(map<string, pair<attrValue, anchor> >::iterator o=obs.begin(); o!=obs.end(); o++, i++) {
    newProps[txt()<<"tKey_"<<i] = o->first;
    newProps[txt()<<"tVal_"<<i] = o->second.first.getAsStr();
    newProps[txt()<<"tAnchorID_"<<i] = txt()<<o->second.second.getID();
  }
  
  // Emit the current values of the context attributes
  newProps["numCtxtAttrs"] = txt()<<contextAttrs.size();
  i=0;
  for(std::list<std::string>::iterator a=t->contextAttrs.begin(); a!=t->contextAttrs.end(); a++, i++) {
    const std::set<attrValue>& vals = attributes.get(*a);
    assert(vals.size()>0);
    if(vals.size()>1) { cerr << "trace::traceAttr() ERROR: context attribute "<<*a<<" has multiple values!"; }
    
    newProps[txt()<<"cKey_"<<i] = *a;
    newProps[txt()<<"cVal_"<<i] = vals.begin()->getAsStr();
  }
  
  obj->props->add("traceObs", newProps);
  
  //dbg.tag("traceObs", properties, false);
  dbg.tag(obj);
  
  delete obj;
  
  // Reset the obs[] map since we've just emitted all these observations
  obs.clear();
}

void traceAttr(std::string label, std::string key, const attrValue& val) {
  // Find the tracer with the name label
  if(trace::active.find(label) == trace::active.end()) cerr << "traceAttr() ERROR: trace \""<<label<<"\" not active when observation \""<<key<<"\"=>\""<<val.str()<<"\" was observed!"; 
  assert(trace::active.find(label) != trace::active.end());
  trace* t = trace::active[label];
  
  // Inform the chosen tracer of the observation
  t->traceAttrObserved(key, val, anchor::noAnchor);
}

void traceAttr(std::string label, std::string key, const attrValue& val, anchor target) {
  // Find the tracer with the name label
  if(trace::active.find(label) == trace::active.end()) cerr << "traceAttr() ERROR: trace \""<<label<<"\" not active when observation \""<<key<<"\"=>\""<<val.str()<<"\" was observed!"; 
  assert(trace::active.find(label) != trace::active.end());
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

}; // namespace structure 
}; // namespace dbglog
