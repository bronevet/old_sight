#include "../../sight_common.h"
#include "../../sight_structure.h"
using namespace std;
using namespace sight::common;
  
namespace sight {
namespace structure {

// -------------------------
// ----- Configuration -----
// -------------------------

// Record the configuration handlers in this file
traceConfHandlerInstantiator::traceConfHandlerInstantiator() {
#ifdef RAPL
  (*enterHandlers)["rapl"]          = &RAPLMeasure::configure;
  (*exitHandlers )["rapl"]          = &traceConfHandlerInstantiator::defaultExitFunc;
#endif // RAPL
}
traceConfHandlerInstantiator traceConfHandlerInstance;

/*********************
 ***** traceAttr *****
 *********************/

void traceAttr(std::string label, std::string key, const attrValue& val) {
  // Inform the chosen tracer of the observation
  trace::getTS(label)->traceAttrObserved(key, val, anchor::noAnchor);
}

void traceAttr(std::string label, std::string key, const attrValue& val, anchor target) {
  // Inform the chosen tracer of the observation
  trace::getTS(label)->traceAttrObserved(key, val, target);
}

void traceAttr(trace* t, std::string key, const attrValue& val) {
  // Inform the chosen tracer of the observation
  t->getTS()->traceAttrObserved(key, val, anchor::noAnchor);
}

void traceAttr(std::string label, 
               const std::map<std::string, attrValue>& ctxt, 
               const std::list<std::pair<std::string, attrValue> >& obsList, 
               const anchor& target) {
  // Inform the chosen tracer of the observation
  trace::getTS(label)->traceFullObservation(ctxt, obsList, target);
}

void traceAttr(trace* t, 
               const std::map<std::string, attrValue>& ctxt, 
               const std::list<std::pair<std::string, attrValue> >& obsList, 
               const anchor& target) {
  // Inform the chosen tracer of the observation
  t->getTS()->traceFullObservation(ctxt, obsList, target);
}

/*****************
 ***** trace *****
 *****************/

// Maps the names of all the currently active traces to their trace objects
std::map<std::string, trace*> trace::active;  

trace::trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(NULL, showLoc, props))
{
//  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; assert(0);; }
  
  init(label, contextAttrs, showLoc, viz, merge, props);
}

trace::trace(std::string label, std::string contextAttr, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(NULL, showLoc, props))
{
  init(label, context(contextAttr), showLoc, viz, merge, props);
}

trace::trace(std::string label, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(NULL, showLoc, props))
{
  init(label, context(), showLoc, viz, merge, props);
}

trace::trace(std::string label, const std::list<std::string>& contextAttrs, const attrOp& onoffOp, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(&onoffOp, showLoc, props))
{
//  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; assert(0);; }
  
  init(label, contextAttrs, showLoc, viz, merge, props);
}

trace::trace(std::string label, std::string contextAttr, const attrOp& onoffOp, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(&onoffOp, showLoc, props))
{
  init(label, context(contextAttr), showLoc, viz, merge, props);
}

trace::trace(std::string label, const attrOp& onoffOp, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(&onoffOp, showLoc, props))
{
  init(label, context(), showLoc, viz, merge, props);
}

// Sets the properties of this object
properties* trace::setProperties(const attrOp* onoffOp, showLocT showLoc, properties* props) {
  if(props==NULL) props = new properties();
  
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> pMap;
    pMap["showLoc"] = txt()<<showLoc;
    props->add("trace", pMap);
  } else
    props->active = false;
  
  return props;
}

void trace::init(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz, mergeT merge, properties* props) {
  if(getProps().active) {
    // Determine whether this object is an instance of a class that derives from trace (props!=NULL) or trace itself (props==NULL)
    bool isDerived = (props != NULL);
    
    // Record that this trace is active
    active[label] = this;

    // If this object is an instance of trace
    if(!isDerived)
      // Create a stream for this trace and emit a tag that describes it
      stream = new traceStream(contextAttrs, viz, merge);
    // Otherwise, we expect the object that derives from trace to create its own traceStream
  }
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void trace::destroy() {
  this->~trace();
}

trace::~trace() {
  assert(!destroyed);
  assert(active.find(getLabel()) != active.end());
  active.erase(getLabel());
  
  assert(stream);
  delete(stream);
}

trace* trace::getT(string label) {
  map<string, trace*>::iterator t=active.find(label);
  // Find the tracer with the name label
  if(t == active.end()) cerr << "ERROR: trace \""<<label<<"\" not active!"<<endl; 
  assert(t != trace::active.end());
  return t->second;
}

traceStream* trace::getTS(string label) {
  return getT(label)->stream;
}

/**************************
 ***** processedTrace *****
 **************************/

processedTrace::processedTrace(std::string label, const std::list<std::string>& contextAttrs, const std::list<std::string>& processorCommands,                        showLocT showLoc, vizT viz, mergeT merge, properties* props) :
    trace(label, contextAttrs,          showLoc, viz, merge, setProperties(NULL, processorCommands, props))
{ init(label, contextAttrs,         processorCommands, viz, merge, props); }
processedTrace::processedTrace(std::string label, std::string contextAttr,                    const std::list<std::string>& processorCommands,                        showLocT showLoc, vizT viz, mergeT merge, properties* props) :
    trace(label, contextAttr,           showLoc, viz, merge, setProperties(NULL, processorCommands, props))
{ init(label, context(contextAttr), processorCommands, viz, merge, props); }
processedTrace::processedTrace(std::string label,                                             const std::list<std::string>& processorCommands,                        showLocT showLoc, vizT viz, mergeT merge, properties* props) :
    trace(label, context(),             showLoc, viz, merge, setProperties(NULL, processorCommands, props))
{ init(label, context(),            processorCommands, viz, merge, props); }
processedTrace::processedTrace(std::string label, const std::list<std::string>& contextAttrs, const std::list<std::string>& processorCommands, const attrOp& onoffOp, showLocT showLoc, vizT viz, mergeT merge, properties* props) :
    trace(label, contextAttrs, onoffOp, showLoc, viz, merge, setProperties(&onoffOp, processorCommands, props))
{ init(label, contextAttrs,         processorCommands, viz, merge, props); }
processedTrace::processedTrace(std::string label, std::string contextAttr,                    const std::list<std::string>& processorCommands, const attrOp& onoffOp, showLocT showLoc, vizT viz, mergeT merge, properties* props) :
    trace(label, context(contextAttr),  onoffOp, showLoc, viz, merge, setProperties(&onoffOp, processorCommands, props))
{ init(label, context(contextAttr), processorCommands, viz, merge, props); }
processedTrace::processedTrace(std::string label,                                             const std::list<std::string>& processorCommands, const attrOp& onoffOp, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
    trace(label,               onoffOp, showLoc, viz, merge, setProperties(&onoffOp, processorCommands, props))
{ init(label, context(),            processorCommands, viz, merge, props); }

properties* processedTrace::setProperties(const attrOp* onoffOp, const std::list<std::string>& processorCommands, properties* props) {
  if(props==NULL) props = new properties();
  
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    // Don't add anything to the properties. processedTraces behave just like normal traces
    // but will use processedTraceStreams instead of regular traceStreams
  } else
    props->active = false;
  
  return props;
}

void processedTrace::init(std::string label, const std::list<std::string>& contextAttrs, const std::list<std::string>& processorCommands, vizT viz, mergeT merge, properties* props) {
  if(getProps().active) {
    // Determine whether this processedTrace object is an instance of a class that derives from 
    // processedTrace (props!=NULL) or trace itself (props==NULL)
    bool isDerived = (props != NULL);

    // If this object is an instance of processedTrace
    if(!isDerived) 
      // Create a stream for this trace and emit a tag that describes it
      stream = new processedTraceStream(contextAttrs, processorCommands, viz, merge);
    // Otherwise, we expect the object that derives from processedTrace to create its own traceStream
  }
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void processedTrace::destroy() {
  this->~processedTrace();
}

processedTrace::~processedTrace() {
  assert(!destroyed);
}

/***********************
 ***** traceStream *****
 ***********************/

// Maximum ID assigned to any trace object
int traceStream::maxTraceID=0;
  
// Callers can optionally provide a traceID that this traceStream will use. This is useful for cases where 
// the ID of the trace used within a given host object needs to be known before the traceStream is actually
// created. In this case the host calls genTraceID(), and then ultimately passes it into the traceStream constructor.
// It can be set to a negative value (or omitted) to indicate that the traceStream should generate an ID on its own.

traceStream::traceStream(const std::list<std::string>& contextAttrs, vizT viz, mergeT merge, int traceID, properties* props) : 
  sightObj(setProperties(contextAttrs, viz, merge, traceID, props), /*isTag:*/ true),
  contextAttrs(contextAttrs), viz(viz), merge(merge)
{ init(traceID); }

traceStream::traceStream(std::string contextAttr, vizT viz, mergeT merge, int traceID, properties* props) : 
  sightObj(setProperties(structure::trace::context(contextAttr), viz, merge, traceID, props), /*isTag:*/ true),
  contextAttrs(structure::trace::context(contextAttr)), viz(viz), merge(merge)
{ init(traceID); }

traceStream::traceStream(vizT viz, mergeT merge, int traceID, properties* props) : 
  sightObj(setProperties(structure::trace::context(), viz, merge, traceID, props), /*isTag:*/ true),
  viz(viz), merge(merge)
{ init(traceID); }

// Returns the properties of this object
properties* traceStream::setProperties(const std::list<std::string>& contextAttrs, vizT viz, mergeT merge, int traceID, properties* props) {
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    map<string, string> pMap;
    pMap["traceID"] = txt()<<(traceID<0? maxTraceID: traceID);
    pMap["viz"]     = txt()<<viz;
    pMap["merge"]   = txt()<<merge;
    
    pMap["numCtxtAttrs"] = txt()<<contextAttrs.size();
    
    // Record the attributes this traces uses as context
    int i=0;
    for(list<string>::const_iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++, i++)
      pMap[txt()<<"ctxtAttr_"<<i] = *ca;
  
    props->add("traceStream", pMap);
  }
  //cout << "Emitting trace "<<traceID<<endl;
  
  return props;
}

// Generates a fresh traceID and returns it
int traceStream::genTraceID() {
  return maxTraceID++;
}

void traceStream::init(int traceID) {
  // If we need to generate a traceID on our own
  if(traceID < 0) {
    this->traceID = genTraceID();
  } else
    this->traceID = traceID;
  
  // Add this trace object as a change listener to all the context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.addObs(*ca, this);
    
  //cout << "traceStream::init(), emitExitTag="<<emitExitTag<<endl;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void traceStream::destroy() {
  this->~traceStream();
}

traceStream::~traceStream() {
  assert(!destroyed);
  
  //cout << "traceStream::~traceStream()"<<endl;
  //cout << "#active="<<active.size()<<", #contextAttrs="<<contextAttrs.size()<<endl;//", #tracerKeys="<<tracerKeys.size()<<endl;
  
  // Stop this object's observations of changes in context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++) {
    //cout << "    *ca="<<*ca<<endl;
    attributes.remObs(*ca, this);
  }
}

// Observe for changes to the values mapped to the given key
void traceStream::observePre(std::string key, attrObserver::attrObsAction action)
{
  // If an attribute is about to be removed, emit the current observation, if one is available
  if(action==attrObserver::attrRemove) {
    // If values for all context keys are still available
    if(initializedCtxtAttrs.size() == contextAttrs.size())
      // Emit any observations performed since the last change of any of the context variables
      emitObservations(contextAttrs, obs);
  
    initializedCtxtAttrs.erase(key);
  }
}

// Observe for changes to the values mapped to the given key
void traceStream::observePost(std::string key, attrObserver::attrObsAction action)
{
  //cout << "traceStream::observePost("<<key<<", "<<attrObserver::attrObsAction2Str(action)<<")"<<endl;
  // If we just got a new attribute value 
  if(action==attrObserver::attrAdd || action==attrObserver::attrReplace) {
    // If values for all context keys are still available
    if(initializedCtxtAttrs.size() == contextAttrs.size())
      // Emit any observations performed since the last change of any of the context variables
      emitObservations(contextAttrs, obs);
    if(action==attrObserver::attrAdd) initializedCtxtAttrs.insert(key);
  }
}

// Called by traceAttr() to inform the trace that a new observation has been made
void traceStream::traceAttrObserved(std::string key, const attrValue& val, anchor target) {
  //cout << "trace::traceAttrObserved("<<key<<", "<<val.str()<<")"<<endl;
  obs[key] = make_pair(val, target);
  //tracerKeys.insert(key);
}

// Records the full observation, including all the values of the context and observation values.
// This observation is emitted immediately regardless of the current state of other observations
// that have been recorded via traceAttrObserved.
void traceStream::traceFullObservation(const std::map<std::string, attrValue>& contextAttrsMap, 
                                 const std::list<std::pair<std::string, attrValue> >& obsList, 
                                 const anchor& target) {
  // Temporary observation map for just this observation
  std::map<std::string, std::pair<attrValue, anchor> > curObs;
  for(list<pair<string, attrValue> >::const_iterator o=obsList.begin(); o!=obsList.end(); o++)
    curObs[o->first] = make_pair(o->second, target);

//cout << "traceStream::traceFullObservation() #contextAttrsMap="<<contextAttrsMap.size()<<", #obsList="<<obsList.size()<<", #obs="<<obs.size()<<endl;  
  // Call emitObservations with the temporary context and observation records, which are separate from the
  // ones that are maintained by this trace and record multiple separate observations for the same context
  emitObservations(contextAttrsMap, curObs);
}

// Emits the output record records the given context and observations pairing
void traceStream::emitObservations(const std::list<std::string>& contextAttrs, 
                             std::map<std::string, std::pair<attrValue, anchor> >& obs) {
  // Read out the current values of the context attributes and store them in a map
  std::map<std::string, attrValue> contextAttrsMap;
  for(std::list<std::string>::const_iterator a=contextAttrs.begin(); a!=contextAttrs.end(); a++) {
    const std::set<attrValue>& vals = attributes.get(*a);
    assert(vals.size()>0);
    if(vals.size()>1) { cerr << "traceStream::traceAttr() ERROR: context attribute "<<*a<<" has multiple values!"; }
    contextAttrsMap[*a] = *vals.begin();
  }
  
  emitObservations(contextAttrsMap, obs);
}

// Emits the output record records the given context and observations pairing
void traceStream::emitObservations(const std::map<std::string, attrValue>& contextAttrsMap, 
                                   std::map<std::string, std::pair<attrValue, anchor> >& obs) {
  // Only emit observations of the trace variables if we have made any observations since the last change in the context variables
  if(obs.size()==0) return;
    
  properties props;
  map<string, string> pMap;
  
  pMap["traceID"] = txt()<<traceID;
  /*ostringstream traceIDStr; traceIDStr << traceID;
  pMap["traceID"] = traceIDStr.str();* /
  char traceIDStr[10000]; snprintf(traceIDStr, 10000, "%d", traceID);
  pMap["traceID"] = string(traceIDStr);*/
  
  // If we'll keep observations from different streams disjoint, record this stream's ID in each observation
  if(merge == disjMerge) pMap["outputStreamID"] = txt()<<outputStreamID;
  
  pMap["numTraceAttrs"] = txt()<<obs.size();
  // Emit the recently observed values and anchors of tracer attributes
  int i=0;
  for(map<string, pair<attrValue, anchor> >::const_iterator o=obs.begin(); o!=obs.end(); o++, i++) {
    pMap[txt()<<"tKey_"<<i] = o->first;
    pMap[txt()<<"tVal_"<<i] = o->second.first.serialize();
    pMap[txt()<<"tAnchorID_"<<i] = txt()<<o->second.second.getID();
  }
  
  // Emit the current values of the context attributes
  pMap["numCtxtAttrs"] = txt()<<contextAttrsMap.size();
  i=0;
  for(std::map<std::string, attrValue>::const_iterator a=contextAttrsMap.begin(); a!=contextAttrsMap.end(); a++, i++) {
    pMap[txt()<<"cKey_"<<i] = a->first;
    pMap[txt()<<"cVal_"<<i] = a->second.serialize();
  }
  
  props.add("traceObs", pMap);
  dbg.tag(props);
  
  // Reset the obs[] map since we've just emitted all these observations
  obs.clear();
}

/********************************
 ***** processedTraceStream *****
 ********************************/

// Callers can optionally provide a traceID that this processedTraceStream will use. This is useful for cases where 
// the ID of the trace used within a given host object needs to be known before the processedTraceStream is actually
// created. In this case the host calls genTraceID(), and then ultimately passes it into the processedTraceStream constructor.
// It can be set to a negative value (or omitted) to indicate that the processedTraceStream should generate an ID on its own.

processedTraceStream::processedTraceStream(const std::list<std::string>& contextAttrs, const std::list<std::string>& processorCommands, vizT viz, mergeT merge, int traceID, properties* props) : 
  traceStream(contextAttrs, viz, merge, traceID, setProperties(processorCommands, props))
{}

processedTraceStream::processedTraceStream(std::string contextAttr,                    const std::list<std::string>& processorCommands, vizT viz, mergeT merge, int traceID, properties* props) : 
  traceStream(contextAttr,  viz, merge, traceID, setProperties(processorCommands, props))
{}

processedTraceStream::processedTraceStream(                                            const std::list<std::string>& processorCommands, vizT viz, mergeT merge, int traceID, properties* props) : 
  traceStream(              viz, merge, traceID, setProperties(processorCommands, props))
{}

// Returns the properties of this object
properties* processedTraceStream::setProperties(const std::list<std::string>& processorCommands, properties* props) {
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    map<string, string> pMap;
    pMap["numCmds"] = txt()<<processorCommands.size();
    int i=0;
    for(list<string>::const_iterator c=processorCommands.begin(); c!=processorCommands.end(); c++)
      pMap[txt()<<"cmd"<<i] = *c;
    
    props->add("processedTraceStream", pMap);
  }
  //cout << "Emitting trace "<<traceID<<endl;
  
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void processedTraceStream::destroy() {
  this->~processedTraceStream();
}

processedTraceStream::~processedTraceStream() {
  assert(!destroyed);
}

/*******************
 ***** measure *****
 *******************/

// Non-full measure
measure::measure(): ts(NULL)
{
  fullMeasure = false;
  init();
}


measure::measure(std::string traceLabel): ts(trace::getTS(traceLabel))
{
  fullMeasure = false;
  init();
}

measure::measure(trace* t): ts(t->getTS())
{
  fullMeasure = false;
  init();
}

measure::measure(traceStream* ts): ts(ts)
{
  fullMeasure = false;
  init();
}


// Full measure
measure::measure(                        const std::map<std::string, attrValue>& fullMeasureCtxt) :
     ts(NULL), fullMeasureCtxt(fullMeasureCtxt)
{
  fullMeasure = true;
  init();
}

measure::measure(std::string traceLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     ts(trace::getTS(traceLabel)), fullMeasureCtxt(fullMeasureCtxt)
{
  fullMeasure = true;
  init();
}

measure::measure(trace* t,               const std::map<std::string, attrValue>& fullMeasureCtxt) :
     ts(t->getTS()), fullMeasureCtxt(fullMeasureCtxt)
{
  fullMeasure = true;
  init();
}

measure::measure(traceStream* ts,        const std::map<std::string, attrValue>& fullMeasureCtxt) :
     ts(ts), fullMeasureCtxt(fullMeasureCtxt)
{
  fullMeasure = true;
  init();
}

measure::measure(const measure& that) : 
  ts(that.ts), paused(that.paused), ended(that.ended), fullMeasure(that.fullMeasure), fullMeasureCtxt(that.fullMeasureCtxt)
{}

measure::~measure()
{}

// Specify the trace that is associated with this measure object
void measure::setTrace(std::string traceLabel)
{ ts = trace::getTS(traceLabel); }

void measure::setTrace(trace* t)
{ ts = t->getTS(); }

void measure::setTrace(traceStream* ts)
{ this->ts = ts; }

// Specify the label of the value measured by this measure object
//void measure::setValLabel(std::string valLabel)
//{ this->valLabel = valLabel; }

// Specify the full context of this object's measurement
void measure::setCtxt(const std::map<std::string, attrValue>& fullMeasureCtxt) { 
  this->fullMeasureCtxt = fullMeasureCtxt;
  fullMeasure = true;
}

// Common initialization code
void measure::init() {
  ended = false;
  paused = true;
}

// Start the measurement
void measure::start() {
  paused = false;
}

// Pauses the measurement so that time elapsed between this call and resume() is not counted.
// Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
bool measure::pause() {
  bool modified = paused==false;
  paused = true;
  return modified;
}

// Restarts counting time. Time collection is restarted regardless of how many times pause() was called
// before the call to resume().
void measure::resume() {
  paused = false;
}

// Complete the measurement and add the observation to the trace associated with this measurement
void measure::end() {
  if(ended) { cerr << "measure::end() ERROR: measuring multiple times!"<<endl; assert(0);; }
 
  ended = true;
}

// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > measure::endGet(bool addToTrace)  {
  if(ended) { cerr << "measure::endGet() ERROR: measuring multiple times!"<<endl; assert(0);; }
 
  ended = true;

  return std::list<std::pair<std::string, attrValue> >();
}

std::string measure::str() const {
  return txt()<<"[measure: paused="<<paused<<", ended="<<ended<<"]";
}

/***********************
 ***** timeMeasure *****
 ***********************/

// Non-full measure
timeMeasure::timeMeasure(                        std::string valLabel): measure(), valLabel(valLabel)
{ init(); }

timeMeasure::timeMeasure(std::string traceLabel, std::string valLabel): measure(traceLabel), valLabel(valLabel)
{ init(); }

timeMeasure::timeMeasure(trace* t,               std::string valLabel): measure(t), valLabel(valLabel)
{ init(); }

timeMeasure::timeMeasure(traceStream* ts,        std::string valLabel): measure(ts), valLabel(valLabel)
{ init(); }

// Full measure
timeMeasure::timeMeasure(                        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(fullMeasureCtxt), valLabel(valLabel)
{ init(); }

timeMeasure::timeMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(traceLabel, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

timeMeasure::timeMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(t, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

timeMeasure::timeMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(ts, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

timeMeasure::timeMeasure(const timeMeasure& that) : measure(that), elapsed(that.elapsed), lastStart(that.lastStart), valLabel(that.valLabel)
{ }

timeMeasure::~timeMeasure() {
}

// Common initialization code
void timeMeasure::init() {
  elapsed = 0.0;
}

// Returns a copy of this measure object, including its current measurement state, if any. The returned
// object is connected to the same traceStream, if any, as the original object.
measure* timeMeasure::copy() const
{ return new timeMeasure(*this); }

// Start the measurement
void timeMeasure::start() {
  measure::start();
  gettimeofday(&lastStart, NULL);  
}

// Pauses the measurement so that time elapsed between this call and resume() is not counted.
// Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
bool timeMeasure::pause() {
  bool modified = measure::pause();

  struct timeval end;
  gettimeofday(&end, NULL);

  elapsed += ((end.tv_sec*1000000 + end.tv_usec) - (lastStart.tv_sec*1000000 + lastStart.tv_usec)) / 1000000.0;

  return modified;  
}

// Restarts counting time. Time collection is restarted regardless of how many times pause() was called
// before the call to resume().
void timeMeasure::resume() {
  measure::resume();
  gettimeofday(&lastStart, NULL);
}

// Complete the measurement
void timeMeasure::end() {
  measure::end();
  
  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  assert(ts);
  if(fullMeasure)
    ts->traceFullObservation(fullMeasureCtxt, trace::observation(valLabel, attrValue((double)elapsed)), anchor::noAnchor);
  else
    ts->traceAttrObserved(valLabel, attrValue((double)elapsed), anchor::noAnchor);
}

// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > timeMeasure::endGet(bool addToTrace) {
  measure::end();
  
  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  if(addToTrace) {
    assert(ts);
    if(fullMeasure)
      ts->traceFullObservation(fullMeasureCtxt, trace::observation(valLabel, attrValue((double)elapsed)), anchor::noAnchor);
    else
      ts->traceAttrObserved(valLabel, attrValue((double)elapsed), anchor::noAnchor);
  }
  
  std::list<std::pair<std::string, attrValue> > ret;
  ret.push_back(make_pair(valLabel, attrValue((double)elapsed)));
  return ret;
}

std::string timeMeasure::str() const { 
  struct timeval cur;
  gettimeofday(&cur, NULL);

  return txt()<<"[timeMeasure: elapsed="<<(elapsed+((cur.tv_sec*1000000 + cur.tv_usec) - (lastStart.tv_sec*1000000 + lastStart.tv_usec)) / 1000000.0)<<" "<<
                measure::str()<<"]";
}

/****************************
 ***** timeStampMeasure *****
 ****************************/

// Non-full measure
timeStampMeasure::timeStampMeasure(                        std::string valLabel): measure(), valLabel(valLabel)
{ init(); }

timeStampMeasure::timeStampMeasure(std::string traceLabel, std::string valLabel): measure(traceLabel), valLabel(valLabel)
{ init(); }

timeStampMeasure::timeStampMeasure(trace* t,               std::string valLabel): measure(t), valLabel(valLabel)
{ init(); }

timeStampMeasure::timeStampMeasure(traceStream* ts,        std::string valLabel): measure(ts), valLabel(valLabel)
{ init(); }

// Full measure
timeStampMeasure::timeStampMeasure(                        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(fullMeasureCtxt), valLabel(valLabel)
{ init(); }

timeStampMeasure::timeStampMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(traceLabel, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

timeStampMeasure::timeStampMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(t, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

timeStampMeasure::timeStampMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(ts, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

timeStampMeasure::timeStampMeasure(const timeStampMeasure& that) : measure(that), startTime(that.startTime), endTime(that.endTime), valLabel(that.valLabel)
{ }

timeStampMeasure::~timeStampMeasure() {
}

// Common initialization code
void timeStampMeasure::init() {
  startTime = 0.0;
  endTime = 0.0;
}

// Returns a copy of this measure object, including its current measurement state, if any. The returned
// object is connected to the same traceStream, if any, as the original object.
measure* timeStampMeasure::copy() const
{ return new timeStampMeasure(*this); }

// Start the measurement
void timeStampMeasure::start() {
  measure::start();
  struct timeval startTV;
  gettimeofday(&startTV, NULL);  
  startTime = double(startTV.tv_sec*1000000 + startTV.tv_usec)/1000000.0;
}

// Pauses the measurement so that time elapsed between this call and resume() is not counted.
// Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
bool timeStampMeasure::pause() {
  return measure::pause();
}

// Restarts counting time. Time collection is restarted regardless of how many times pause() was called
// before the call to resume().
void timeStampMeasure::resume() {
  measure::resume();
}

// Complete the measurement
void timeStampMeasure::end() {
  measure::end();
  
  struct timeval endTV;
  gettimeofday(&endTV, NULL);  
  endTime = double(endTV.tv_sec*1000000 + endTV.tv_usec)/1000000.0;
  
  assert(ts);
  if(fullMeasure) {
    ts->traceFullObservation(fullMeasureCtxt, 
                             trace::observation(txt()<<valLabel<<"_Start", startTime),
                             anchor::noAnchor);
    ts->traceFullObservation(fullMeasureCtxt, 
                             trace::observation(txt()<<valLabel<<"_End", endTime),
                             anchor::noAnchor);
  } else {
    ts->traceAttrObserved(txt()<<valLabel<<"_Start", attrValue(startTime), anchor::noAnchor);
    ts->traceAttrObserved(txt()<<valLabel<<"_End",   attrValue(endTime),   anchor::noAnchor);
  }
}

// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > timeStampMeasure::endGet(bool addToTrace) {
  measure::end();
  
  struct timeval endTV;
  gettimeofday(&endTV, NULL);  
  endTime = double(endTV.tv_sec*1000000 + endTV.tv_usec)/1000000.0;
  
  if(addToTrace) {
    if(fullMeasure) {
      ts->traceFullObservation(fullMeasureCtxt, 
                               trace::observation(txt()<<valLabel<<"_Start", startTime),
                               anchor::noAnchor);
      ts->traceFullObservation(fullMeasureCtxt, 
                               trace::observation(txt()<<valLabel<<"_End", endTime),
                               anchor::noAnchor);
    } else {
      ts->traceAttrObserved(txt()<<valLabel<<"_Start", attrValue(startTime), anchor::noAnchor);
      ts->traceAttrObserved(txt()<<valLabel<<"_End",   attrValue(endTime),   anchor::noAnchor);
    }
  }
  
  std::list<std::pair<std::string, attrValue> > ret;
  ret.push_back(make_pair("Start", attrValue(startTime)));
  ret.push_back(make_pair("End",   attrValue(endTime)));
  return ret;
}

std::string timeStampMeasure::str() const { 
  struct timeval cur;
  gettimeofday(&cur, NULL);

  return txt()<<"[timeStampMeasure: startTime="<<startTime<<", endTime="<<endTime<<" "<<
                measure::str()<<"]";
}

/***********************
 ***** PAPIMeasure *****
 ***********************/

// Indicates the number of PAPIMeasure objects that are currently measuring the counters
int PAPIMeasure::numMeasurers=0;

// Records the set of PAPI counters currently being measured (non-empty iff numMeasurers>0)
std::vector<int> PAPIMeasure::curMeasuredEvents;

PAPIMeasure::PAPIMeasure(const papiEvents& events) : measure(), events(events)
{ init(); }

// Non-full measure
PAPIMeasure::PAPIMeasure(                        std::string valLabel, const papiEvents& events): measure(), events(events), valLabel(valLabel)
{ init(); }

PAPIMeasure::PAPIMeasure(std::string traceLabel, std::string valLabel, const papiEvents& events): measure(traceLabel), events(events), valLabel(valLabel)
{ init(); }

PAPIMeasure::PAPIMeasure(trace* t,               std::string valLabel, const papiEvents& events): measure(t), events(events), valLabel(valLabel)
{ init(); }

PAPIMeasure::PAPIMeasure(traceStream* ts,        std::string valLabel, const papiEvents& events): measure(ts), events(events), valLabel(valLabel)
{ init(); }

// Full measure
PAPIMeasure::PAPIMeasure(                        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt, const papiEvents& events) :
     measure(fullMeasureCtxt), events(events), valLabel(valLabel)
{ init(); }

PAPIMeasure::PAPIMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt, const papiEvents& events) :
     measure(traceLabel, fullMeasureCtxt), events(events), valLabel(valLabel)
{ init(); }

PAPIMeasure::PAPIMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt, const papiEvents& events) :
     measure(t, fullMeasureCtxt), events(events), valLabel(valLabel)
{ init(); }

PAPIMeasure::PAPIMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt, const papiEvents& events) :
     measure(ts, fullMeasureCtxt), events(events), valLabel(valLabel)
{ init(); }

PAPIMeasure::PAPIMeasure(const PAPIMeasure& that) : 
  measure(that), accumValues(that.accumValues), lastValues(that.lastValues), readValues(that.readValues), events(that.events), eventsToMeasure(that.eventsToMeasure), events2Idx(that.events2Idx), valLabel(that.valLabel), PAPIOperational(that.PAPIOperational)
{ }

PAPIMeasure::~PAPIMeasure() {
}

// Common initialization code
void PAPIMeasure::init() {
  /*cout << "PAPIMeasure::init() eventsToMeasure=";
  for(int i=0; i<eventsToMeasure.size(); i++)
    cout << eventsToMeasure[i] << " ";
  cout << endl;*/

  // events may contain both raw and derived PAPI counters. We now initialize eventsToMeasure to contain
  // the exact raw counters that we need to measure.
  //
  // First we collect the set of raw events that need to be measured
  set<int> rawEvents; // Set of all the raw events that need to be placed in eventsToMeasure
  for(std::vector<int>::const_iterator e=events.begin(); e!=events.end(); e++) {
    //cout << "PAPIMeasure::init() event="<<*e<<endl;
    // If the current counter is a raw PAPI counter, add it directly to eventsToMeasure
    if(*e<PAPI_MIN_DERIVED || *e>PAPI_MAX_DERIVED)
      rawEvents.insert(*e);
    // Else, if it is a derived counter, add the raw counters that need to be measured to eventsToMeasure
    else {
      switch(*e) {
        case PAPI_L1_TC_MR: // L1 Total cache miss rate
          rawEvents.insert(PAPI_L1_TCA);
          rawEvents.insert(PAPI_L1_TCM);
          break;
        case PAPI_L2_TC_MR: // L2 Total cache miss rate
          rawEvents.insert(PAPI_L2_TCA);
          rawEvents.insert(PAPI_L2_TCM);
          break;
        case PAPI_L3_TC_MR: // L3 Total cache miss rate
          rawEvents.insert(PAPI_L3_TCA);
          rawEvents.insert(PAPI_L3_TCM);
          break;
        case PAPI_L1_DC_MR: // L1 Data cache miss rate
          rawEvents.insert(PAPI_L1_DCA);
          rawEvents.insert(PAPI_L1_DCM);
          break;
        case PAPI_L2_DC_MR: // L2 Data cache miss rate
          rawEvents.insert(PAPI_L1_DCA);
          rawEvents.insert(PAPI_L1_DCM);
          break;
        case PAPI_L3_DC_MR: // L3 Data cache miss rate
          rawEvents.insert(PAPI_L1_DCA);
          rawEvents.insert(PAPI_L1_DCM);
          break;
        case PAPI_L1_IC_MR: // L1 Instruction cache miss rate
          rawEvents.insert(PAPI_L1_ICA);
          rawEvents.insert(PAPI_L1_ICM);
          break;
        case PAPI_L2_IC_MR: // L2 Instruction cache miss rate
          rawEvents.insert(PAPI_L2_ICA);
          rawEvents.insert(PAPI_L2_ICM);
          break;
        case PAPI_L3_IC_MR: // L3 Instruction cache miss rate
          rawEvents.insert(PAPI_L3_ICA);
          rawEvents.insert(PAPI_L3_ICM);
          break;
        default:
          cerr << "ERROR: Unknown derived PAPI event "<<*e<<endl;
          assert(0);
      }
    }
  }

  //cout << "PAPIMeasure::init() #rawEvents="<<rawEvents.size()<<endl;
  // Next we add all the events in rawEvents into eventsToMeasure, recording the 
  // mapping from event ID to its index in events2Idx
  eventsToMeasure.resize(rawEvents.size());
  int i=0;
  for(set<int>::const_iterator e=rawEvents.begin(); e!=rawEvents.end(); e++, i++) {
    eventsToMeasure[i] = *e;
    events2Idx[*e] = i;
  }
  //cout << "PAPIMeasure::init() measuring counters=(#"<<eventsToMeasure.size()<<")="<<list2str(getMeasuredCounterNames())<<endl;

  // Initialize the values arrays to contain one counter for each event, 
  accumValues.resize(eventsToMeasure.size(), 0); // Initialized to all 0's
  lastValues.resize(eventsToMeasure.size());
  readValues.resize(eventsToMeasure.size());

  PAPIOperational = false;
}

// Returns a copy of this measure object, including its current measurement state, if any. The returned
// object is connected to the same traceStream, if any, as the original object.
measure* PAPIMeasure::copy() const
{ return new PAPIMeasure(*this); }

// Start the measurement
void PAPIMeasure::start() {
  measure::start();
  
  // If no other PAPIMeasure objects are currently trying to measure counters, 
  if(numMeasurers==0) {
    // Start measuring them
    if(PAPI_start_counters((int*)&(eventsToMeasure[0]), eventsToMeasure.size()) != PAPI_OK) { 
      cerr << "PAPIMeasure::start() ERROR starting PAPI counters! counters="<<list2str(getMeasuredCounterNames())<<endl;
      PAPIOperational=false;
      return;
    }
    PAPIOperational=true;
    
    // Record the eventsToMeasure that are currently being measured
    curMeasuredEvents = eventsToMeasure;
  } else {
    // If we're currently measuring counters, confirm that the currently measured counter set is the
    // same as the set we wish to measure
    if(eventsToMeasure != curMeasuredEvents) {
      cerr << "ERROR: PAPIMeasure is asked to simultaneously measure different counter sets!"<<endl;
      cerr << "    Currently measuring: [";
      for(int i=0; i<curMeasuredEvents.size(); i++) {
        if(i>0) cerr << ", ";
        char EventCodeStr[PAPI_MAX_STR_LEN];
        if (PAPI_event_code_to_name(curMeasuredEvents[i], EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::start() ERROR getting name of PAPI counter "<<curMeasuredEvents[i]<<"!"<<endl; assert(0); }
        cerr << EventCodeStr;
      }
      cerr << "]"<<endl;

      cerr << "   New measurement request: [";
      for(int i=0; i<eventsToMeasure.size(); i++) {
        if(i>0) cerr << ", ";
        char EventCodeStr[PAPI_MAX_STR_LEN];
        if (PAPI_event_code_to_name(eventsToMeasure[i], EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::start() ERROR getting name of PAPI counter "<<eventsToMeasure[i]<<"!"<<endl; assert(0); }
        cerr << EventCodeStr;
      }
      cerr << "]"<<endl;
      assert(0);
    } else
      PAPIOperational=true;
  }
  numMeasurers++;
  
  // Read in the initial values of the counters
  if(PAPI_read_counters((long_long*)&(lastValues[0]), lastValues.size()) != PAPI_OK) { cerr << "PAPIMeasure::start() ERROR reading PAPI counters!"<<endl; assert(0); }
  
  if(PAPI_read_counters((long_long*)&(readValues[0]), readValues.size()) != PAPI_OK) { cerr << "PAPIMeasure::pause() ERROR accumulating PAPI counters!"<<endl; assert(0); }
  // Start measurement
  //cout << "PAPIMeasure::start() "<<str()<<endl;
  
  /*float real_time, proc_time,mflips;
  long long flpins;
  float ireal_time, iproc_time, imflips;
  long long iflpins;
  int retval;
/ *if((retval=PAPI_flips(&ireal_time,&iproc_time,&iflpins,&imflips)) < PAPI_OK)
  {
    printf("Could not initialise PAPI_flips \n");
    printf("Your platform may not support floating point instruction event.\n");    printf("retval: %d\n", retval);
    exit(1);
  }* /
  if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
   {
      printf("Library initialization error! \n");
      exit(1);
   }

  long long s = PAPI_get_real_cyc();
  for(int i=0; i<1000000; i++) {}
  long long e = PAPI_get_real_cyc();
/cout << "cycles = "<<(e-s)<<endl;
  int eventsToMeasureArray[] = {PAPI_TOT_INS};
  if(PAPI_start_counters(eventsToMeasureArray, 1) != PAPI_OK) { cerr << "PAPIMeasure::start() ERROR starting PAPI counters!"<<endl; assert(0); }*/
  //if(PAPI_start_counters((int*)&(eventsToMeasure[0]), eventsToMeasure.size()) != PAPI_OK) { cerr << "PAPIMeasure::start() ERROR starting PAPI counters!"<<endl; assert(0); }
}

// Pauses the measurement so that time elapsed between this call and resume() is not counted.
// Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
bool PAPIMeasure::pause() {
  bool modified = measure::pause();

  // Return if PAPI measurement is not operational
  if(!PAPIOperational) return  modified;

  // Add the counts since the last call to start or resume to values
  if(PAPI_read_counters((long_long*)&(readValues[0]), readValues.size()) != PAPI_OK) { cerr << "PAPIMeasure::pause() ERROR accumulating PAPI counters!"<<endl; assert(0); }

  for(int i=0; i<eventsToMeasure.size(); i++) {
    if(readValues[i] > lastValues[i])
      accumValues[i] += readValues[i] - lastValues[i];
  }
  
  // Stop the counters, placing the current values into the dummy vector
  //vector<long_long> dummy; dummy.resize(eventsToMeasure.size());
  //if(PAPI_stop_counters((long_long*)&(dummy[0]), dummy.size()) != PAPI_OK) { cerr << "PAPIMeasure::pause() ERROR stopping PAPI counters!"<<endl; assert(0); }

  return modified;  
}

// Restarts counting time. Time collection is restarted regardless of how many times pause() was called
// before the call to resume().
void PAPIMeasure::resume() {
  measure::resume();

  // Return if PAPI measurement is not operational
  if(!PAPIOperational) return;

  
  // Restart measurement
  //if(PAPI_start_counters((int*)&eventsToMeasure[0], eventsToMeasure.size()) != PAPI_OK) { cerr << "PAPIMeasure::resume() ERROR starting PAPI counters!"<<endl; assert(0); }

  // Read in the initial values of the counters
  if(PAPI_read_counters((long_long*)&(lastValues[0]), lastValues.size()) != PAPI_OK) { cerr << "PAPIMeasure::resume() ERROR reading PAPI counters!"<<endl; assert(0); }
}

// Returns the string names of the counters currently being measured
std::list<std::string> PAPIMeasure::getMeasuredCounterNames() {
  std::list<std::string> ret;
  int i=0;
  for(std::vector<int>::const_iterator e=eventsToMeasure.begin(); e!=eventsToMeasure.end(); e++) {
    char EventCodeStr[PAPI_MAX_STR_LEN];
    if (PAPI_event_code_to_name(*e, EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::getMeasuredCounterNames() ERROR getting name of PAPI counter "<<*e<<"!"<<endl; assert(0); }
     ret.push_back(EventCodeStr);
  }
  return ret;
}

// Returns a list that maps all the raw and derived counters in events to their observed values,
// using the raw counter values in accumValues.
std::list<std::pair<std::string, attrValue> > PAPIMeasure::getAccumValues() {
  std::list<std::pair<std::string, attrValue> > ret;
  int i=0;
 for(std::vector<int>::const_iterator e=events.begin(); e!=events.end(); e++, i++) {
    // If the current counter is a raw PAPI counter, add it directly to ret
    if(*e<PAPI_MIN_DERIVED || *e>PAPI_MAX_DERIVED) {
      char EventCodeStr[PAPI_MAX_STR_LEN];
      if (PAPI_event_code_to_name(*e, EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::getAccumValues() ERROR getting name of PAPI counter "<<*e<<"!"<<endl; assert(0); }
      ret.push_back(make_pair(string(EventCodeStr), attrValue(long(accumValues[events2Idx[*e]]))));
    // Else, if it is a derived counter, compute the derived metric based on the values of the raw counters
    } else {
      switch(*e) {
        case PAPI_L1_TC_MR: // L1 Total cache miss rate
          ret.push_back(make_pair("PAPI_L1_TC_MR", attrValue(accumValues[events2Idx[PAPI_L1_TCA]]==0? 0:
                                                                double(accumValues[events2Idx[PAPI_L1_TCM]])/
                                                                  accumValues[events2Idx[PAPI_L1_TCA]])));
          break;
        case PAPI_L2_TC_MR: // L2 Total cache miss rate
          ret.push_back(make_pair("PAPI_L2_TC_MR", attrValue(accumValues[events2Idx[PAPI_L2_TCA]]==0? 0:
                                                                double(accumValues[events2Idx[PAPI_L2_TCM]])/
                                                                   accumValues[events2Idx[PAPI_L2_TCA]])));
          break;
        case PAPI_L3_TC_MR: // L3 Total cache miss rate
          ret.push_back(make_pair("PAPI_L3_TC_MR", attrValue(accumValues[events2Idx[PAPI_L3_TCA]]==0? 0:
                                                                double(accumValues[events2Idx[PAPI_L3_TCM]])/
                                                                   accumValues[events2Idx[PAPI_L3_TCA]])));
          break;
        case PAPI_L1_DC_MR: // L1 Data cache miss rate
          ret.push_back(make_pair("PAPI_L1_DC_MR", attrValue(accumValues[events2Idx[PAPI_L1_DCA]]==0? 0:
                                                                double(accumValues[events2Idx[PAPI_L1_DCM]])/
                                                                   accumValues[events2Idx[PAPI_L1_DCA]])));
          break;
        case PAPI_L2_DC_MR: // L2 Data cache miss rate
          ret.push_back(make_pair("PAPI_L2_DC_MR", attrValue(accumValues[events2Idx[PAPI_L2_DCA]]==0? 0: 
                                                                double(accumValues[events2Idx[PAPI_L2_DCM]])/
                                                                   accumValues[events2Idx[PAPI_L2_DCA]])));
          break;
        case PAPI_L3_DC_MR: // L3 Data cache miss rate
          ret.push_back(make_pair("PAPI_L3_DC_MR", attrValue(accumValues[events2Idx[PAPI_L3_DCA]]==0? 0: 
                                                                double(accumValues[events2Idx[PAPI_L3_DCM]])/
                                                                   accumValues[events2Idx[PAPI_L3_DCA]])));
          break;
        case PAPI_L1_IC_MR: // L1 Instruction cache miss rate
          ret.push_back(make_pair("PAPI_L1_IC_MR", attrValue(accumValues[events2Idx[PAPI_L1_ICA]]==0? 0:
                                                                double(accumValues[events2Idx[PAPI_L1_ICM]])/
                                                                   accumValues[events2Idx[PAPI_L1_ICA]])));
          break;
        case PAPI_L2_IC_MR: // L2 Instruction cache miss rate
          ret.push_back(make_pair("PAPI_L2_IC_MR", attrValue(accumValues[events2Idx[PAPI_L2_ICA]]==0? 0:
                                                                double(accumValues[events2Idx[PAPI_L2_ICM]])/
                                                                   accumValues[events2Idx[PAPI_L2_ICA]])));
          break;
        case PAPI_L3_IC_MR: // L3 Instruction cache miss rate
          ret.push_back(make_pair("PAPI_L3_IC_MR", attrValue(accumValues[events2Idx[PAPI_L3_ICA]]==0? 0:
                                                                double(accumValues[events2Idx[PAPI_L3_ICM]])/
                                                                   accumValues[events2Idx[PAPI_L3_ICA]])));
          break;
        default:
          cerr << "ERROR: Unknown derived PAPI event "<<*e<<endl;
          assert(0);
      }
    }
  }

/*  cout << "Raw:"<<endl;
  for(std::map<int, int>::iterator i=events2Idx.begin(); i!=events2Idx.end(); i++) {
      char EventCodeStr[PAPI_MAX_STR_LEN];
      if (PAPI_event_code_to_name(i->first, EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::getAccumValues() ERROR getting name of PAPI counter "<<i->first<<"!"<<endl; assert(0); }
      cout << EventCodeStr << " => "<<accumValues[i->second]<<endl;
  }
 
  cout << "Final:"<<endl;
  for(std::list<std::pair<std::string, attrValue> >::iterator i=ret.begin(); i!=ret.end(); i++)
    cout << "    "<<i->first<<" => "<<i->second.getAsStr()<<endl;*/

  return ret;
}

// Complete the measurement
void PAPIMeasure::end() {
  measure::end();
  
  // Return if PAPI measurement is not operational
  if(!PAPIOperational) return;

  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  // One fewer instance of PAPIMeasure is actively reading the performance counters
  numMeasurers--;
  
  // If no instance of PAPIMeasure needs to read performance counters, 
  if(numMeasurers==0) {
    // Stop the counters, placing the current values into the dummy vector
    vector<long_long> dummy; dummy.resize(eventsToMeasure.size());
    if(PAPI_stop_counters((long_long*)&(dummy[0]), dummy.size()) != PAPI_OK) { cerr << "PAPIMeasure::end() ERROR stopping PAPI counters!"<<endl; assert(0); }
    
    curMeasuredEvents.clear();
  }
  
  assert(ts);
  // Compute the counter measurements and emit them to the trace
  list<pair<string, attrValue> > values = getAccumValues();
  for(list<pair<string, attrValue> >::iterator v=values.begin(); v!=values.end(); v++) {
    if(fullMeasure)
      ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<valLabel<<":"<<v->first), v->second), anchor::noAnchor);
    else
      ts->traceAttrObserved(txt()<<valLabel<<":"<<v->first, v->second, anchor::noAnchor);
  }
}

// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > PAPIMeasure::endGet(bool addToTrace) {
  measure::end();
  
  // Return if PAPI measurement is not operational
  if(!PAPIOperational) return std::list<std::pair<std::string, attrValue> >();
  
  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  // One fewer instance of PAPIMeasure is actively reading the performance counters
  numMeasurers--;
  
  // If no instance of PAPIMeasure needs to read performance counters, 
  if(numMeasurers==0) {
    // Stop the counters, placing the current values into the dummy vector
    vector<long_long> dummy; dummy.resize(eventsToMeasure.size());
    if(PAPI_stop_counters((long_long*)&(dummy[0]), dummy.size()) != PAPI_OK) { cerr << "PAPIMeasure::end() ERROR stopping PAPI counters!"<<endl; assert(0); }
    
    curMeasuredEvents.clear();
  }
  
  // Iterate over all the PAPI counters being measured
  list<pair<string, attrValue> > values = getAccumValues();
  if(addToTrace) {
    assert(ts);
    for(list<pair<string, attrValue> >::iterator v=values.begin(); v!=values.end(); v++) {
      if(fullMeasure)
        ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<valLabel<<":"<<v->first), v->second), anchor::noAnchor);
      else
        ts->traceAttrObserved(txt()<<valLabel<<":"<<v->first, v->second, anchor::noAnchor);
    }
  }
  return values;
}

std::string PAPIMeasure::str() const { 
  ostringstream s;
  s<<"[PAPIMeasure: ";
  for(int i=0; i<eventsToMeasure.size(); i++) {
    if(i>0) s << ", ";
    
    char EventCodeStr[PAPI_MAX_STR_LEN];
    if (PAPI_event_code_to_name(eventsToMeasure[i], EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::end() ERROR getting name of PAPI counter "<<eventsToMeasure[i]<<"!"<<endl; assert(0); }
    s << EventCodeStr;
  }
  s<<" "<<measure::str();
  s<<"]";
  return s.str();
}

void endMeasure(measure* m) {
  m->end();
}

// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > endGetMeasure(measure* m, bool addToTrace) {
  assert(m);
  std::list<std::pair<std::string, attrValue> > result = m->endGet(addToTrace);
  delete m;
  return result;
}


#ifdef RAPL

/**********************
 ***** MSRMeasure *****
 **********************/

int MSRMeasure::numMeasurers=0;

MSRMeasure::MSRMeasure() {
  // If the MSRs are not currently being measured, initialize the MSR library
  if(numMeasurers==0) init_msr();

  numMeasurers++;
}

MSRMeasure::~MSRMeasure() {
  numMeasurers--;

  // If the MSRs are no longer being measured, finalize the MSR library
//  if(numMeasurers==0) finalize_msr();
}

/***********************
 ***** RAPLMeasure *****
 ***********************/

// Non-full measure
RAPLMeasure::RAPLMeasure(): measure(), valLabel("")
{ init(); }

RAPLMeasure::RAPLMeasure(                        std::string valLabel): measure(), valLabel(valLabel)
{ init(); }

RAPLMeasure::RAPLMeasure(std::string traceLabel, std::string valLabel): measure(traceLabel), valLabel(valLabel)
{ init(); }

RAPLMeasure::RAPLMeasure(trace* t,               std::string valLabel): measure(t), valLabel(valLabel)
{ init(); }

RAPLMeasure::RAPLMeasure(traceStream* ts,        std::string valLabel): measure(ts), valLabel(valLabel)
{ init(); }

// Full measure
RAPLMeasure::RAPLMeasure(                        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(fullMeasureCtxt), valLabel(valLabel)
{ init(); }

RAPLMeasure::RAPLMeasure(std::string traceLabel, std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(traceLabel, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

RAPLMeasure::RAPLMeasure(trace* t,               std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(t, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

RAPLMeasure::RAPLMeasure(traceStream* ts,        std::string valLabel, const std::map<std::string, attrValue>& fullMeasureCtxt) :
     measure(ts, fullMeasureCtxt), valLabel(valLabel)
{ init(); }

RAPLMeasure::RAPLMeasure(const RAPLMeasure& that) : 
  measure(that), valLabel(that.valLabel) 
{
  for(int socket=0; socket<NUM_SOCKETS; socket++) {
    accumCpuE[socket]  = that.accumCpuE[socket];
    accumDramE[socket] = that.accumDramE[socket];
    rapl[socket]       = that.rapl[socket];
  }
}

RAPLMeasure::~RAPLMeasure() {
}

// Common initialization code
void RAPLMeasure::init() {
  /*for(int socket=0; socket<NUM_SOCKETS; socket++) {
    rapl[socket].flags = RDF_REENTRANT | RDF_INIT;
    read_rapl_data(socket, &(rapl[socket]));
    cout << "socket "<<socket<<" initialized"<<endl;
  }*/
}

// Returns a copy of this measure object, including its current measurement state, if any. The returned
// object is connected to the same traceStream, if any, as the original object.
measure* RAPLMeasure::copy() const
{ return new RAPLMeasure(*this); }

// Start the measurement
void RAPLMeasure::start() {
  measure::start();
  
  // Start measuring the RAPL counters
  for(int socket=0; socket<NUM_SOCKETS; socket++) {
    //cout << "socket "<<socket<<" started &rapl[socket]="<<&rapl[socket]<<endl;
    // Initialize the rapl struct
    rapl[socket].flags = RDF_REENTRANT | RDF_INIT;
    read_rapl_data(socket, &rapl[socket]);

    // Set the flags so that subsequentcalls to read_rapl_data don't re-initialize rapl[socket]
    rapl[socket].flags = RDF_REENTRANT;
  }
}

// Pauses the measurement so that time elapsed between this call and resume() is not counted.
// Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
bool RAPLMeasure::pause() {
  bool modified = measure::pause();

  // Read the energy used on each socket since the last measurement and accumulate it into accumCpuE and accumDramE
  for(int socket=0; socket<NUM_SOCKETS; socket++) {
    read_rapl_data(socket, &rapl[socket]);
    // Eliminate noisy almost-0 energy measurements
    if(rapl[socket].pkg_delta_joules<1e-10)  rapl[socket].pkg_delta_joules = 0;
    if(rapl[socket].dram_delta_joules<1e-10) rapl[socket].dram_delta_joules = 0;

    // Accumulare the measurement into accumCpuE and accumDramE
    /*cout << "socket "<<socket<<", CPU pkg_delta_joules="<<rapl[socket].pkg_delta_joules<<" elapsed="<<rapl[socket].elapsed<<endl;
    cout << "socket "<<socket<<", DRAM pkg_delta_joules="<<rapl[socket].dram_delta_joules<<" elapsed="<<rapl[socket].elapsed<<endl;*/
    accumCpuE[socket].add(rapl[socket].pkg_delta_joules,   rapl[socket].elapsed);
    accumDramE[socket].add(rapl[socket].dram_delta_joules, rapl[socket].elapsed);
  }

  return modified;  
}

// Restarts counting time. Time collection is restarted regardless of how many times pause() was called
// before the call to resume().
void RAPLMeasure::resume() {
  measure::resume();
  
  // Restart measurement
  for(int socket=0; socket<NUM_SOCKETS; socket++) {
    read_rapl_data(socket, &(rapl[socket]));
  }
}

// Complete the measurement
void RAPLMeasure::end() {
  measure::end();
  
  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  assert(ts);

  string labelStr="";
  if(valLabel!="") labelStr = valLabel + ":";

  // Emit the measurement
  for(int socket=0; socket<NUM_SOCKETS; socket++) {
    if(fullMeasure) {
      ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Energy_CPU_S"<<socket),  accumCpuE[socket].E),           anchor::noAnchor);
      ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Energy_DRAM_S"<<socket), accumDramE[socket].E),          anchor::noAnchor);
      ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Power_CPU_S"<<socket),   accumCpuE[socket].getPower()),  anchor::noAnchor);
      ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Power_DRAM_S"<<socket),  accumDramE[socket].getPower()), anchor::noAnchor);
      ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"EDP_S"<<socket),         (accumCpuE[socket].E+accumDramE[socket].E)/accumCpuE[socket].T), anchor::noAnchor);
    } else {
      ts->traceAttrObserved(txt()<<labelStr<<"Energy_CPU_S"<<socket,  accumCpuE[socket].E,           anchor::noAnchor);
      ts->traceAttrObserved(txt()<<labelStr<<"Energy_DRAM_S"<<socket, accumDramE[socket].E,          anchor::noAnchor);
      ts->traceAttrObserved(txt()<<labelStr<<"Power_CPU_S"<<socket,   accumCpuE[socket].getPower(),  anchor::noAnchor);
      ts->traceAttrObserved(txt()<<labelStr<<"Power_DRAM_S"<<socket,  accumDramE[socket].getPower(), anchor::noAnchor);
      ts->traceAttrObserved(txt()<<labelStr<<"EDP_S"<<socket,         (accumCpuE[socket].E+accumDramE[socket].E)/accumCpuE[socket].T, anchor::noAnchor);
    }
  }
}

// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > RAPLMeasure::endGet(bool addToTrace) {
  measure::end();
  
  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  if(addToTrace)
    assert(ts);
  
  string labelStr="";
  if(valLabel!="") labelStr = valLabel + ":";

  // Emit the measurement and record it in ret;
  std::list<std::pair<std::string, attrValue> > ret;
 
  if(addToTrace)
    ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Elapsed"),  accumCpuE[0].T),  anchor::noAnchor);
  ret.push_back(make_pair((string)(txt()<<labelStr<<"Elapsed"),  accumCpuE[0].T));
  
  for(int socket=0; socket<NUM_SOCKETS; socket++) {
    ret.push_back(make_pair((string)(txt()<<labelStr<<"Energy_CPU_S"<<socket),  accumCpuE[socket].E));
    ret.push_back(make_pair((string)(txt()<<labelStr<<"Energy_DRAM_S"<<socket), accumDramE[socket].E));
    ret.push_back(make_pair((string)(txt()<<labelStr<<"Power_CPU_S"<<socket),   accumCpuE[socket].getPower()));
    ret.push_back(make_pair((string)(txt()<<labelStr<<"Power_DRAM_S"<<socket),  accumDramE[socket].getPower()));
    ret.push_back(make_pair((string)(txt()<<labelStr<<"EDP_S"<<socket),         (accumCpuE[socket].E+accumDramE[socket].E)/accumCpuE[socket].T));
    //cout << "elapsed="<<accumCpuE[socket].T<<", CPU E="<<accumCpuE[socket].E<<", CPU P="<<accumCpuE[socket].getPower()<<endl;
   
    if(addToTrace) { 
      if(fullMeasure) {
        ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Energy_CPU_S"<<socket),  accumCpuE[socket].E),  anchor::noAnchor);
        ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Energy_DRAM_S"<<socket), accumDramE[socket].E), anchor::noAnchor);
        ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Power_CPU_S"<<socket),   accumCpuE[socket].getPower()),  anchor::noAnchor);
        ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"Power_DRAM_S"<<socket),  accumDramE[socket].getPower()), anchor::noAnchor);
        ts->traceFullObservation(fullMeasureCtxt, trace::observation((string)(txt()<<labelStr<<"EDP_S"<<socket),         (accumCpuE[socket].E+accumDramE[socket].E)/accumCpuE[socket].T), anchor::noAnchor);
      } else {
        ts->traceAttrObserved(txt()<<labelStr<<"Energy_CPU_S"<<socket,  accumCpuE[socket].E,           anchor::noAnchor);
        ts->traceAttrObserved(txt()<<labelStr<<"Energy_DRAM_S"<<socket, accumDramE[socket].E,          anchor::noAnchor);
        ts->traceAttrObserved(txt()<<labelStr<<"Power_CPU_S"<<socket,   accumCpuE[socket].getPower(),  anchor::noAnchor);
        ts->traceAttrObserved(txt()<<labelStr<<"Power_DRAM_S"<<socket,  accumDramE[socket].getPower(), anchor::noAnchor);
        ts->traceAttrObserved(txt()<<labelStr<<"EDP_S"<<socket,         (accumCpuE[socket].E+accumDramE[socket].E)/accumCpuE[socket].T, anchor::noAnchor);
      }
    }
  }
  
  return ret;
}

std::string RAPLMeasure::str() const { 
  ostringstream s;
  s<<"[RAPLMeasure: ";
  s<<" "<<measure::str();
  s<<"]";
  return s.str();
}

common::Configuration* RAPLMeasure::configure(properties::iterator props) {
  // Set the current power cap based on the specification in props
  struct rapl_limit CPULimit, DRAMLimit;
  bool CPUSpecified=false;
  if(props.exists("CPUWatts") && props.exists("CPUSeconds")) {
    CPULimit.watts   = props.getFloat("CPUWatts");
    CPULimit.seconds = props.getFloat("CPUSeconds");
    CPULimit.bits    = 0;
    CPUSpecified=true;
  }

  bool DRAMSpecified=false;
  if(props.exists("DRAMWatts") && props.exists("DRAMSeconds")) {
    DRAMLimit.watts   = props.getFloat("DRAMWatts");
    DRAMLimit.seconds = props.getFloat("DRAMSeconds");
    DRAMLimit.bits    = 0;
    DRAMSpecified=true;
  }
  
  if(CPUSpecified || DRAMSpecified) {
    init_msr();

   /*for(int s=0; s<NUM_SOCKETS; s++) {
      struct rapl_limit socketCPULimit, socketDRAMLimit;
      get_rapl_limit(s, (CPUSpecified? &socketCPULimit: NULL), NULL, DRAMSpecified? &socketDRAMLimit: NULL);
      if(CPUSpecified)  std::cout << "Initial CPU on socket "<<s<<" to "<<socketCPULimit.watts<<"W * "<<socketCPULimit.seconds<<"s "<<std::endl;
      if(DRAMSpecified) std::cout << "Initial DRAM on socket "<<s<<" to "<<socketDRAMLimit.watts<<"W * "<<socketDRAMLimit.seconds<<"s"<<std::endl;
    }*/

    for(int s=0; s<NUM_SOCKETS; s++) {
      /*if(CPUSpecified)  std::cout << "Setting CPU on socket "<<s<<" to "<<CPULimit.watts<<"W * "<<CPULimit.seconds<<"s "<<std::endl;
      if(DRAMSpecified) std::cout << "Setting DRAM on socket "<<s<<" to "<<DRAMLimit.watts<<"W * "<<DRAMLimit.seconds<<"s"<<std::endl;*/
      set_rapl_limit(s, (CPUSpecified? &CPULimit: NULL), NULL, DRAMSpecified? &DRAMLimit: NULL);
    }

    /*for(int s=0; s<NUM_SOCKETS; s++) {
      struct rapl_limit socketCPULimit, socketDRAMLimit;
      get_rapl_limit(s, (CPUSpecified? &socketCPULimit: NULL), NULL, DRAMSpecified? &socketDRAMLimit: NULL);
      if(CPUSpecified)  std::cout << "Set CPU on socket "<<s<<" to "<<socketCPULimit.watts<<"W * "<<socketCPULimit.seconds<<"s "<<std::endl;
      if(DRAMSpecified) std::cout << "Set DRAM on socket "<<s<<" to "<<socketDRAMLimit.watts<<"W * "<<socketDRAMLimit.seconds<<"s"<<std::endl;
    }*/
  }

  return NULL;
}

#endif // RAPL

/*****************************************
 ***** TraceMergeHandlerInstantiator *****
 *****************************************/

TraceMergeHandlerInstantiator::TraceMergeHandlerInstantiator() { 
  (*MergeHandlers   )["trace"]       = TraceMerger::create;
  (*MergeKeyHandlers)["trace"]       = TraceMerger::mergeKey;
  (*MergeHandlers   )["traceStream"] = TraceStreamMerger::create;
  (*MergeKeyHandlers)["traceStream"] = TraceStreamMerger::mergeKey;
  (*MergeHandlers   )["traceObs"]    = TraceObsMerger::create;
  (*MergeKeyHandlers)["traceObs"]    = TraceObsMerger::mergeKey;
  MergeGetStreamRecords->insert(&TraceGetMergeStreamRecord);
}
TraceMergeHandlerInstantiator TraceMergeHandlerInstance;

std::map<std::string, streamRecord*> TraceGetMergeStreamRecord(int streamID) {
  std::map<std::string, streamRecord*> mergeMap;
  mergeMap["traceStream"] = new TraceStreamRecord(streamID);
  return mergeMap;
}

/***********************
 ***** TraceMerger *****
 ***********************/

TraceMerger::TraceMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        BlockMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props))
{ }


// Sets the properties of the merged object
properties* TraceMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "trace");
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Trace!"<<endl; assert(0);; }
  if(type==properties::enterTag) {
    // Default to showing before the trace, unless showLoc==showEnd on all incoming streams
    vector<long> showLocs = str2int(getValues(tags, "showLoc"));
    trace::showLocT showLoc;
    for(vector<long>::iterator i=showLocs.begin(); i!=showLocs.end(); i++) {
      if(i==showLocs.begin()) showLoc = (trace::showLocT)(*i);
      else if(showLoc != (trace::showLocT)(*i)) showLoc = trace::showBegin;
    }
    pMap["showLoc"] = txt()<<showLoc;
  }
  props->add("trace", pMap);
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void TraceMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  BlockMerger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; assert(0);; }
  if(type==properties::enterTag) {
    // We don't specify a key for traces since they're mostly shells for traceStreams and there isn't much to differentiate them
  }
}

/*****************************
 ***** TraceStreamMerger *****
 *****************************/

TraceStreamMerger::TraceStreamMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) :
                    Merger(advance(tags), outStreamRecords, inStreamRecords, props)
{
  if(props==NULL) props = new properties();
  this->props = props;
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Trace!"<<endl; assert(0);; }
  if(type==properties::enterTag) {
    assert(tags.size()>0);
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "traceStream");
    
    // Merge the trace IDs along all the streams
    int mergedTraceID = streamRecord::mergeIDs("traceStream", "traceID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // If the visualization values disagree then we have an error. We differentiate traces that 
    // use different visualizations in mergeKey().
    vector<string> viz = getValues(tags, "viz");
    assert(allSame<string>(viz));
    pMap["viz"] = *viz.begin();
    
    // If the merge values disagree then we have an error. We differentiate traces that 
    // use different merge types in mergeKey().
    vector<long> merge = str2int(getValues(tags, "merge"));
    assert(allSame<long>(merge));
    pMap["merge"] = txt()<<*merge.begin();
    
    // Set the merge type of the merged trace in the outgoing stream
    ((TraceStreamRecord*)outStreamRecords["traceStream"])->merge[mergedTraceID] = (trace::mergeT)*merge.begin();

    // If the set of context variables used by the different traces disagree then we have an error. 
    // In the future we'll need to reject merges of traces that use different visualizations.
    // Let the context variables of the merged trace be the union of the context variables     
    vector<long> numCtxtAttrsVec = str2int(getValues(tags, "numCtxtAttrs"));
    assert(allSame<long>(numCtxtAttrsVec));
    int numCtxtAttrs = *numCtxtAttrsVec.begin();
    pMap["numCtxtAttrs"] = txt()<<numCtxtAttrs;
    
    set<string> contextAttrs;
    for(int t=0; t<tags.size(); t++) {
      set<string> curContextAttrs;
      for(int c=0; c<numCtxtAttrs; c++) {
        curContextAttrs.insert(properties::get(tags[t].second, txt()<<"ctxtAttr_"<<c));
        if(t==0) pMap[txt()<<"ctxtAttr_"<<c] = properties::get(tags[t].second, txt()<<"ctxtAttr_"<<c);
      }
      
      if(t==0) contextAttrs = curContextAttrs;
      else     assert(contextAttrs == curContextAttrs);
    }
  }
  props->add("traceStream", pMap);
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void TraceStreamMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; assert(0);; }
  if(type==properties::enterTag) {
    info.add(properties::get(tag, "viz"));
    info.add(properties::get(tag, "merge"));
    
    info.add(properties::get(tag, "numCtxtAttrs"));
    int numCtxtAttrs = properties::getInt(tag, "numCtxtAttrs");
    for(int c=0; c<numCtxtAttrs; c++) info.add(properties::get(tag, txt()<<"ctxtAttr_"<<c));
  }
}

/**************************************
 ***** ProcessedTraceStreamMerger *****
 **************************************/

ProcessedTraceStreamMerger::ProcessedTraceStreamMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        TraceMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props))
{ }


// Sets the properties of the merged object
properties* ProcessedTraceStreamMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "processedTS");
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Processed Trace!"<<endl; assert(0);; }
  if(type==properties::enterTag) {
    // All the  process commands must be identical
    pMap["numCmds"] = getSameValue(tags, "numCmds");
    long numCmds = attrValue::parseInt(pMap["numCmds"]);
    
    for(int c=0; c<numCmds; c++)
      pMap[txt()<<"cmd"<<c] = getSameValue(tags, txt()<<"cmd"<<c);
  }
  props->add("processedTrace", pMap);
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void ProcessedTraceStreamMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  BlockMerger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; assert(0);; }
  if(type==properties::enterTag) {
    // All the  process commands must be identical
    info.add(tag.get("numCmds"));
    int numCmds = tag.getInt("numCmds");
    for(int c=0; c<numCmds; c++) {
      info.add(tag.get(txt()<<"cmd"<<c));
    }
  }
}

/**************************
 ***** TraceObsMerger *****
 **************************/

TraceObsMerger::TraceObsMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                       properties* props) : 
                                Merger(advance(tags), outStreamRecords, inStreamRecords, props) {
  if(props==NULL) props = new properties();
  this->props = props;

  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "traceObs");
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging TraceObs!"<<endl; assert(0);; }
  if(type==properties::enterTag) {
    // Merge the trace IDs along all the streams
    //int mergedTraceID = TraceStreamRecord::mergeIDs(pMap, tags, outStreamRecords, inStreamRecords);
    int mergedTraceID = streamRecord::sameID("traceStream", "traceID", pMap, tags, outStreamRecords, inStreamRecords);
        
    //cout << "TraceObsMerger::TraceObsMerger() mergedTraceID="<<mergedTraceID<<" outStreamRecords[trace]="<<outStreamRecords["traceStream"]->str()<<endl;
      
    // Get the trace merging policy
    trace::mergeT merge = (trace::mergeT)((TraceStreamRecord*)outStreamRecords["traceStream"])->merge[mergedTraceID];
    
    // Create a separate tag for each observation on each stream (we'll add aggregation code in the future)
    // The last stream's entry tag will be written to pMap, while the other streams' entry and exit tags will be 
    // placed in moreTagsBefore. Thus, the merged exit tag will end up corresponding to the last stream's entry tag.
    properties obsExitProps("traceObs");

    // If we need to keep separate observations for each stream
    if(merge == trace::disjMerge) {
      for(int t=0; t<tags.size(); t++) {
        TraceStreamRecord* ts = (TraceStreamRecord*)inStreamRecords[t]["traceStream"];
        
        map<string, string> traceObsMap = pMap;
        map<string, string>& curMap = (t<tags.size()-1? traceObsMap: pMap);
        
        curMap["traceID"] = txt()<<mergedTraceID;
        
        curMap["outputStreamID"] = properties::get(tags[t].second, "outputStreamID");
        
        curMap["numTraceAttrs"] = properties::get(tags[t].second, "numTraceAttrs");
        int numTraceAttrs = properties::getInt(tags[t].second, "numTraceAttrs");
        for(int i=0; i<numTraceAttrs; i++) {
          curMap[txt()<<"tKey_"<<i] = properties::get(tags[t].second, txt()<<"tKey_"<<i);
          curMap[txt()<<"tVal_"<<i] = properties::get(tags[t].second, txt()<<"tVal_"<<i);
          
          // If the anchorID is noAnchor, leave it as it is
          int tAnchorID = properties::getInt(tags[t].second, txt()<<"tAnchorID_"<<i);
          if(tAnchorID==-1) curMap[txt()<<"tAnchorID_"<<i] = "-1";
          // Otherwise, convert it from its ID in the incoming stream to its ID in the outgoing stream
          else {
            streamID inSID(properties::getInt(tags[t].second, txt()<<"tAnchorID_"<<i), 
                           inStreamRecords[t]["traceStream"]->getVariantID());
            streamID outSID = inStreamRecords[t]["anchor"]->in2outID(inSID);
            curMap[txt()<<"tAnchorID_"<<i] = txt()<<outSID.ID;
          }
        }
        
        curMap["numCtxtAttrs"] = properties::get(tags[t].second, "numCtxtAttrs");
        int numCtxtAttrs = properties::getInt(tags[t].second, "numCtxtAttrs");
        for(int i=0; i<numCtxtAttrs; i++) {
          curMap[txt()<<"cKey_"<<i] = properties::get(tags[t].second, txt()<<"cKey_"<<i);
          curMap[txt()<<"cVal_"<<i] = properties::get(tags[t].second, txt()<<"cVal_"<<i);
        }
        if(t<tags.size()-1) {
          properties enterProps;
          enterProps.add("traceObs", traceObsMap);
          moreTagsBefore.push_back(make_pair(properties::enterTag, enterProps));
          moreTagsBefore.push_back(make_pair(properties::exitTag,  obsExitProps));
        }
      }
    
    // If we aggregate observations from multiple streams
    } else {
      vector<long> numCtxtAttrsVec = str2int(getValues(tags, "numCtxtAttrs"));
      assert(allSame<long>(numCtxtAttrsVec));
      long numCtxtAttrs = *numCtxtAttrsVec.begin();
      
      // Read the values of the context attributes, ensuring that they are the same across the observations across all streams
      map<string, attrValue> context;
      for(int t=0; t<tags.size(); t++) {
        map<string, attrValue> curContext;
        for(int c=0; c<numCtxtAttrs; c++) {
          curContext[properties::get(tags[t].second, txt()<<"cKey_"<<c)] = 
                            attrValue(properties::get(tags[t].second, txt()<<"cVal_"<<c));
                  
          if(t==0) {
            pMap[txt()<<"cKey_"<<c] = properties::get(tags[t].second, txt()<<"cKey_"<<c);
            pMap[txt()<<"cVal_"<<c] = properties::get(tags[t].second, txt()<<"cVal_"<<c);
          }
        }
        
        if(t==0) context = curContext;
        else     assert(context == curContext);
      }
      
      vector<long> numTraceAttrsVec = str2int(getValues(tags, "numTraceAttrs"));
      assert(allSame<long>(numTraceAttrsVec));
      long numTraceAttrs = *numTraceAttrsVec.begin();
    
      for(int t=0; t<numTraceAttrs; t++) {
        vector<string> tKeyVec = getValues(tags, txt()<<"tKey_"<<t);
        assert(allSame<string>(tKeyVec));
        pMap[txt()<<"tKey_"<<t] = *tKeyVec.begin();
        
        double aggr;
        switch(merge) {
          case trace::avgMerge: aggr=0;      break;
          case trace::minMerge: aggr=1e100;  break;
          case trace::maxMerge: aggr=-1e100; break;
          default: assert(0);
        }
        
        vector<double> tValVec = str2float(getValues(tags, txt()<<"tVal_"<<t));
        for(vector<double>::iterator v=tValVec.begin(); v!=tValVec.end(); v++) {
          switch(merge) {
            case trace::avgMerge: aggr+=*v;      break;
            case trace::minMerge: aggr=(*v<aggr? *v: aggr); break;
            case trace::maxMerge: aggr=(*v>aggr? *v: aggr); break;
            default: assert(0);
          }
        }
        
        switch(merge) {
          case trace::avgMerge: aggr/=tValVec.size();     break;
          case trace::minMerge:                           break;
          case trace::maxMerge:                           break;
          default: assert(0);
        }
        
        attrValue aggrVal(aggr);
        pMap[txt()<<"tVal_"<<t] = txt()<<aggrVal.serialize();
      }
    }
  }
  
  props->add("traceObs", pMap);
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void TraceObsMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                              std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  static long maxObsID=0;
  
  Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; assert(0);; }
  if(type==properties::enterTag) {
    /* // Observations may only be merged if they correspond to traces that were merged in the outgoing stream
    streamID inSID(properties::getInt(tag, "traceID"), 
                   inStreamRecords["traceStream"]->getVariantID());
    info.add(txt()<<inStreamRecords["traceStream"]->in2outID(inSID).ID);*/
    
    // Observations may never be merged. Therefore, each observation gets a unique key.
    info.add(txt()<<(maxObsID++));
  }
}

/*****************************
 ***** TraceStreamRecord *****
 *****************************/

TraceStreamRecord::TraceStreamRecord(const TraceStreamRecord& that, int vSuffixID) :
  streamRecord(that, vSuffixID)//, maxTraceID(that.maxTraceID), in2outTraceIDs(that.in2outTraceIDs)
{}

// Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
// which is appended to the new stream's variant list.
streamRecord* TraceStreamRecord::copy(int vSuffixID) {
  return new TraceStreamRecord(*this, vSuffixID);
}

// Given multiple streamRecords from several variants of the same stream, update this streamRecord object
// to contain the state that succeeds them all, making it possible to resume processing
void TraceStreamRecord::resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams) {
  // Compute the maximum maxTraceID among all the streams
  /*maxTraceID = -1;
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++)
    maxTraceID = (((TraceStreamRecord*)(*s)["traceStream"])->maxTraceID > maxTraceID? 
                   ((TraceStreamRecord*)(*s)["traceStream"])->maxTraceID: 
                   maxTraceID);*/
  streamRecord::resumeFrom(streams);
  
  // Set edges and in2outTraceIDs to be the union of its counterparts in streams
  /*in2outTraceIDs.clear();
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    TraceStreamRecord* ns = (TraceStreamRecord*)(*s)["traceStream"];
    for(map<streamID, streamID>::const_iterator i=ns->in2outTraceIDs.begin(); i!=ns->in2outTraceIDs.end(); i++)
      in2outTraceIDs.insert(*i);
  }*/
}

/*
// Marge the IDs of the next graph (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
// updating each incoming stream's mappings from its IDs to the outgoing stream's IDs. Returns the traceID of the merged trace
// in the outgoing stream.
int TraceStreamRecord::mergeIDs(std::map<std::string, std::string>& pMap, 
                     const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                     std::map<std::string, streamRecord*>& outStreamRecords,
                     std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  // Assign to the merged trace the next ID for this output stream
  int mergedTraceID = ((TraceStreamRecord*)outStreamRecords["traceStream"])->maxTraceID;
  pMap["traceID"] = txt()<<mergedTraceID;
  
  // The trace's ID within the outgoing stream    
  streamID outSID(((TraceStreamRecord*)outStreamRecords["traceStream"])->maxTraceID,
                  outStreamRecords["traceStream"]->getVariantID());
  
  // Update inStreamRecords to map the trace's ID within each incoming stream to the assigned ID in the outgoing stream
  for(int t=0; t<tags.size(); t++) {
    TraceStreamRecord* ts = (TraceStreamRecord*)inStreamRecords[t]["traceStream"];
    
    // The graph's ID within the current incoming stream
    streamID inSID(properties::getInt(tags[t].second, "traceID"), 
                   inStreamRecords[t]["traceStream"]->getVariantID());
    
    if(ts->in2outTraceIDs.find(inSID) == ts->in2outTraceIDs.end())
      ((TraceStreamRecord*)inStreamRecords[t]["traceStream"])->in2outTraceIDs[inSID] = outSID;
  }
  
  // Advance maxTraceID
  ((TraceStreamRecord*)outStreamRecords["traceStream"])->maxTraceID++;
  
  return mergedTraceID;
}*/
  
std::string TraceStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[TraceStreamRecord: ";
  s << streamRecord::str(indent+"    ") << endl;  
  s << indent << "]";
  
  return s.str();
}

}; // namespace structure 
}; // namespace sight

