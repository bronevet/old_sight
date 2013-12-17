#include "../../sight_common.h"
#include "../../sight_structure.h"
using namespace std;
using namespace sight::common;
  
namespace sight {
namespace structure {

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
  block(label, setProperties(showLoc, props))
{
  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; exit(-1); }
  
  init(label, contextAttrs, showLoc, viz, merge);
}

trace::trace(std::string label, std::string contextAttr, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(showLoc, props))
{
  init(label, context(contextAttr), showLoc, viz, merge);
}

trace::trace(std::string label, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(showLoc, props))
{
  init(label, context(), showLoc, viz, merge);
}

// Sets the properties of this object
properties* trace::setProperties(showLocT showLoc, properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> newProps;
  newProps["showLoc"] = txt()<<showLoc;
  props->add("trace", newProps);
  return props;
}

void trace::init(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz, mergeT merge) {
  //cout << "trace::init() label="<<label<<", this="<<this<<endl;
  
  // Record that this trace is active
  active[label] = this;
  
  // Create a stream for this trace and emit a tag that describes it
  stream = new traceStream(contextAttrs, viz, merge);
  /*properties streamProps;
  streamProps.add("trace_traceStream", stream->setProperties());
  dbg.tag(streamProps);*/
}

trace::~trace() {
  //cout << "trace::~trace() label="<<getLabel()<<", this="<<this<<endl; cout.flush();
  
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

/***********************
 ***** traceStream *****
 ***********************/

// Maximum ID assigned to any trace object
int traceStream::maxTraceID=0;
  
traceStream::traceStream(const std::list<std::string>& contextAttrs, vizT viz, mergeT merge, properties* props) : 
  sightObj(setProperties(contextAttrs, viz, merge, props), true),
  contextAttrs(contextAttrs), viz(viz), merge(merge)
{ init(); }

traceStream::traceStream(std::string contextAttr, vizT viz, mergeT merge, properties* props) : 
  sightObj(setProperties(structure::trace::context(contextAttr), viz, merge, props), true),
  contextAttrs(structure::trace::context(contextAttr)), viz(viz), merge(merge)
{ init(); }

traceStream::traceStream(vizT viz, mergeT merge, properties* props) : 
  sightObj(setProperties(structure::trace::context(), viz, merge, props), true),
  viz(viz), merge(merge)
{ init(); }

// Returns the properties of this object
properties* traceStream::setProperties(const std::list<std::string>& contextAttrs, vizT viz, mergeT merge, properties* props) {
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    map<string, string> pMap;
    pMap["traceID"] = txt()<<maxTraceID;
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

void traceStream::init() {
  traceID = maxTraceID;
  maxTraceID++;
  
  // Add this trace object as a change listener to all the context variables
  for(list<string>::iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++)
    attributes.addObs(*ca, this);
    
  //cout << "traceStream::init(), emitExitTag="<<emitExitTag<<endl;
}

traceStream::~traceStream() {
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
    contextAttrsMap[*a] = vals.begin()->getAsStr();
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
  
  // If we'll keep observations from different streams disjoint, record this stream's ID in each observation
  if(merge == disjMerge) pMap["outputStreamID"] = txt()<<outputStreamID;
  
  pMap["numTraceAttrs"] = txt()<<obs.size();
  // Emit the recently observed values and anchors of tracer attributes
  int i=0;
  for(map<string, pair<attrValue, anchor> >::const_iterator o=obs.begin(); o!=obs.end(); o++, i++) {
    pMap[txt()<<"tKey_"<<i] = o->first;
    pMap[txt()<<"tVal_"<<i] = o->second.first.getAsStr();
    pMap[txt()<<"tAnchorID_"<<i] = txt()<<o->second.second.getID();
  }
  
  // Emit the current values of the context attributes
  pMap["numCtxtAttrs"] = txt()<<contextAttrsMap.size();
  i=0;
  for(std::map<std::string, attrValue>::const_iterator a=contextAttrsMap.begin(); a!=contextAttrsMap.end(); a++, i++) {
    pMap[txt()<<"cKey_"<<i] = a->first;
    pMap[txt()<<"cVal_"<<i] = a->second.getAsStr();
  }
  
  props.add("traceObs", pMap);
  dbg.tag(props);
  
  // Reset the obs[] map since we've just emitted all these observations
  obs.clear();
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
  if(ended) { cerr << "measure::end() ERROR: measuring multiple times!"<<endl; exit(-1); }
 
  ended = true;
}

// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > measure::endGet(bool addToTrace)  {
  if(ended) { cerr << "measure::endGet() ERROR: measuring multiple times!"<<endl; exit(-1); }
 
  ended = true;
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

timeMeasure::~timeMeasure() {
}

// Common initialization code
void timeMeasure::init() {
  elapsed = 0.0;
}

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
    ts->traceFullObservation(fullMeasureCtxt, trace::observation(make_pair(valLabel, attrValue((double)elapsed))), anchor::noAnchor);
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
      ts->traceFullObservation(fullMeasureCtxt, trace::observation(make_pair(valLabel, attrValue((double)elapsed))), anchor::noAnchor);
    else
      ts->traceAttrObserved(valLabel, attrValue((double)elapsed), anchor::noAnchor);
  }
  
  std::list<std::pair<std::string, attrValue> > ret;
  ret.push_back(make_pair(valLabel, attrValue((double)elapsed)));
  return ret;
}

std::string timeMeasure::str() { 
  struct timeval cur;
  gettimeofday(&cur, NULL);

  return txt()<<"[timeMeasure: elapsed="<<(elapsed+((cur.tv_sec*1000000 + cur.tv_usec) - (lastStart.tv_sec*1000000 + lastStart.tv_usec)) / 1000000.0)<<"]";
}


/***********************
 ***** PAPIMeasure *****
 ***********************/

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

PAPIMeasure::~PAPIMeasure() {
}

// Common initialization code
void PAPIMeasure::init() {
  /*cout << "PAPIMeasure::init() events=";
  for(int i=0; i<events.size(); i++)
    cout << events[i] << " ";
  cout << endl;*/
  
  // Initialize the values array to all 0's
  values.resize(events.size(), 0);
}

// Start the measurement
void PAPIMeasure::start() {
  measure::start();
  
  // Start measurement
  //cout << "PAPIMeasure::start() "<<str()<<endl;
  
  /*float real_time, proc_time,mflips;
  long long flpins;
  float ireal_time, iproc_time, imflips;
  long long iflpins;
  int retval;
/*if((retval=PAPI_flips(&ireal_time,&iproc_time,&iflpins,&imflips)) < PAPI_OK)
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
  int eventsArray[] = {PAPI_TOT_INS};
  if(PAPI_start_counters(eventsArray, 1) != PAPI_OK) { cerr << "PAPIMeasure::start() ERROR starting PAPI counters!"<<endl; assert(0); }*/
  if(PAPI_start_counters((int*)&(events[0]), events.size()) != PAPI_OK) { cerr << "PAPIMeasure::start() ERROR starting PAPI counters!"<<endl; assert(0); }
}

// Pauses the measurement so that time elapsed between this call and resume() is not counted.
// Returns true if the measure is not currently paused and false if it is (i.e. the pause command has no effect)
bool PAPIMeasure::pause() {
  bool modified = measure::pause();

  // Add the counts since the last call to start or resume to values
  if(PAPI_accum_counters((long_long*)&(values[0]), values.size()) != PAPI_OK) { cerr << "PAPIMeasure::pause() ERROR accumulating PAPI counters!"<<endl; assert(0); }

  // Stop the counters, placing the current values into the dummy vector
  vector<long_long> dummy; dummy.resize(events.size());
  if(PAPI_stop_counters((long_long*)&(dummy[0]), dummy.size()) != PAPI_OK) { cerr << "PAPIMeasure::pause() ERROR stopping PAPI counters!"<<endl; assert(0); }

  return modified;  
}

// Restarts counting time. Time collection is restarted regardless of how many times pause() was called
// before the call to resume().
void PAPIMeasure::resume() {
  measure::resume();
  
  // Restart measurement
  if(PAPI_start_counters((int*)&events[0], events.size()) != PAPI_OK) { cerr << "PAPIMeasure::resume() ERROR starting PAPI counters!"<<endl; assert(0); }
}

// Complete the measurement
void PAPIMeasure::end() {
  measure::end();
  
  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  assert(ts);
  // Iterate over all the PAPI counters being measured
  for(int i=0; i<events.size(); i++) {
    char EventCodeStr[PAPI_MAX_STR_LEN];
    if (PAPI_event_code_to_name(events[i], EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::end() ERROR getting name of PAPI counter "<<events[i]<<"!"<<endl; assert(0); }
    
    if(fullMeasure)
      ts->traceFullObservation(fullMeasureCtxt, trace::observation(make_pair((string)(txt()<<valLabel<<":"<<EventCodeStr), attrValue((long)values[i]))), anchor::noAnchor);
    else
      ts->traceAttrObserved(txt()<<valLabel<<":"<<EventCodeStr, attrValue((long)values[i]), anchor::noAnchor);
  }
}

// Complete the measurement and return the observation.
// If addToTrace is true, the observation is addes to this measurement's trace and not, otherwise
std::list<std::pair<std::string, attrValue> > PAPIMeasure::endGet(bool addToTrace) {
  measure::end();
  
  // Call pause() to update elapsed with the time since the start of the measure or the last call to resume() 
  pause(); 
  
  if(addToTrace)
    assert(ts);
  
  // Iterate over all the PAPI counters being measured
  std::list<std::pair<std::string, attrValue> > ret;
  
  for(int i=0; i<events.size(); i++) {
    char EventCodeStr[PAPI_MAX_STR_LEN];
    if (PAPI_event_code_to_name(events[i], EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::end() ERROR getting name of PAPI counter "<<events[i]<<"!"<<endl; assert(0); }
    
    // If a value label was not provided, the label of the observation is just the name of the PAPI counter
    if(valLabel == "")
      ret.push_back(make_pair(string(EventCodeStr), attrValue((long)values[i])));
    // If a value label was provided, the label of the observation combines it and the name of the PAPI counter
    else 
      ret.push_back(make_pair((string)(txt()<<valLabel<<":"<<string(EventCodeStr)), attrValue((long)values[i])));
    
    if(addToTrace) {
      if(fullMeasure)
        ts->traceFullObservation(fullMeasureCtxt, trace::observation(make_pair((string)(txt()<<valLabel<<":"<<EventCodeStr), attrValue((long)values[i]))), anchor::noAnchor);
      else
        ts->traceAttrObserved(txt()<<valLabel<<":"<<EventCodeStr, attrValue((long)values[i]), anchor::noAnchor);
    }
  }
  
  return ret;
}

std::string PAPIMeasure::str() { 
  ostringstream s;
  s<<"[PAPIMeasure: ";
  for(int i=0; i<events.size(); i++) {
    if(i>0) s << ", ";
    
    char EventCodeStr[PAPI_MAX_STR_LEN];
    if (PAPI_event_code_to_name(events[i], EventCodeStr) != PAPI_OK) { cerr << "PAPIMeasure::end() ERROR getting name of PAPI counter "<<events[i]<<"!"<<endl; assert(0); }
    s << EventCodeStr;
  }
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
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Trace!"<<endl; exit(-1); }
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
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  BlockMerger::mergeKey(type, tag.next(), inStreamRecords, key);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
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
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Trace!"<<endl; exit(-1); }
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
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, key);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    key.push_back(properties::get(tag, "viz"));
    key.push_back(properties::get(tag, "merge"));
    
    key.push_back(properties::get(tag, "numCtxtAttrs"));
    int numCtxtAttrs = properties::getInt(tag, "numCtxtAttrs");
    for(int c=0; c<numCtxtAttrs; c++) key.push_back(properties::get(tag, txt()<<"ctxtAttr_"<<c));
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
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging TraceObs!"<<endl; exit(-1); }
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
        
        pMap[txt()<<"tVal_"<<t] = txt()<<aggr;
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
                              std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  Merger::mergeKey(type, tag.next(), inStreamRecords, key);
  
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Observations may only be merged if they correspond to traces that were merged in the outgoing stream
    streamID inSID(properties::getInt(tag, "traceID"), 
                   inStreamRecords["traceStream"]->getVariantID());
    key.push_back(txt()<<inStreamRecords["traceStream"]->in2outID(inSID).ID);
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

