#include "../sight_common.h"
#include "../sight_structure.h"
using namespace std;
using namespace sight::common;
  
namespace sight {
namespace structure {

// Maximum ID assigned to any trace object
int trace::maxTraceID=0;

// Maps the names of all the currently active traces to their trace objects
std::map<std::string, trace*> trace::active;  

trace::trace(std::string label, const std::list<std::string>& contextAttrs, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(maxTraceID, showLoc, viz, merge, contextAttrs, props)), contextAttrs(contextAttrs), merge(merge)
{
  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; exit(-1); }
  
  init(label, showLoc, viz, merge);
}

trace::trace(std::string label, std::string contextAttr, showLocT showLoc, vizT viz, mergeT merge, properties* props) : 
  block(label, setProperties(maxTraceID, showLoc, viz, merge, contextAttr, props)), merge(merge)
{
  contextAttrs.push_back(contextAttr);
  
  init(label, showLoc, viz, merge);
}

// Sets the properties of this object
properties* trace::setProperties(int traceID, showLocT showLoc, vizT viz, mergeT merge, const std::list<std::string>& contextAttrs, properties* props) {
  if(props==NULL) props = new properties();
  
  map<string, string> newProps;
  newProps["traceID"] = txt()<<traceID;
  newProps["showLoc"] = txt()<<showLoc;
  newProps["viz"]     = txt()<<viz;
  newProps["merge"]   = txt()<<merge;
  
  newProps["numCtxtAttrs"] = txt()<<contextAttrs.size();
  
  // Record the attributes this traces uses as context
  int i=0;
  for(list<string>::const_iterator ca=contextAttrs.begin(); ca!=contextAttrs.end(); ca++, i++)
    newProps[txt()<<"ctxtAttr_"<<i] = *ca;
  
  props->add("trace", newProps);
  
  //dbg.enter("trace", properties, inheritedFrom);
  return props;
}

properties* trace::setProperties(int traceID, showLocT showLoc, vizT viz, mergeT merge, std::string contextAttr, properties* props) {
  std::list<std::string> contextAttrs;
  contextAttrs.push_back(contextAttr);
  return setProperties(traceID, showLoc, viz, merge, contextAttrs, props);
}

void trace::init(std::string label, showLocT showLoc, vizT viz, mergeT merge) {
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
    
  sightObj *obj = new sightObj(new properties());
  
  map<string, string> newProps;
  
  //assert(trace::stack.size()>0);
  //trace* t = *(trace::stack.rbegin());
  assert(active.find(getLabel()) != active.end());
  trace* t = active[getLabel()];
  
  newProps["traceID"] = txt()<<t->traceID;
  
  // If we'll keep observations from different streams disjoint, record this stream's ID in each observation
  if(merge == disjMerge) newProps["outputStreamID"] = txt()<<outputStreamID;
  
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
    // Merge the trace IDs along all the streams
    //int mergedTraceID = mergeIDs("trace", "traceID", pMap, tags, outStreamRecords, inStreamRecords);
    int mergedTraceID = streamRecord::mergeIDs("trace", "traceID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // Default to showing before the trace, unless showLoc==showEnd on all incoming streams
    vector<long> showLocs = str2int(getValues(tags, "showLoc"));
    trace::showLocT showLoc;
    for(vector<long>::iterator i=showLocs.begin(); i!=showLocs.end(); i++) {
      if(i==showLocs.begin()) showLoc = (trace::showLocT)(*i);
      else if(showLoc != (trace::showLocT)(*i)) showLoc = trace::showBegin;
    }
    pMap["showLoc"] = txt()<<showLoc;
    
    // If the visualization values disagree then we have an error. In the future we'll need to
    // reject merges of traces that use different visualizations.
    vector<string> viz = getValues(tags, "viz");
    assert(allSame<string>(viz));
    pMap["viz"] = *viz.begin();
    
    // If the merge values disagree then we have an error. In the future we'll need to
    // reject merges of traces that use different merge types.
    vector<long> merge = str2int(getValues(tags, "merge"));
    assert(allSame<long>(merge));
    pMap["merge"] = txt()<<*merge.begin();
    
    // Set the merge type of the merged trace in the outgoing stream
    ((TraceStreamRecord*)outStreamRecords["trace"])->merge[mergedTraceID] = (trace::mergeT)*merge.begin();

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
  props->add("trace", pMap);
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void TraceMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, std::list<std::string>& key) {
  properties::iterator blockTag = tag;
  BlockMerger::mergeKey(type, ++blockTag, inStreamRecords, key);
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    key.push_back(properties::get(tag, "viz"));
    key.push_back(properties::get(tag, "merge"));
    
    key.push_back(properties::get(tag, "numCtxtAttrs"));
    int numCtxtAttrs = properties::getInt(tag, "numCtxtAttrs");
    for(int c=0; c<numCtxtAttrs; c++) key.push_back(properties::get(tag, txt()<<"ctxtAttr_"<<c));
  }
}

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
    int mergedTraceID = streamRecord::sameID("trace", "traceID", pMap, tags, outStreamRecords, inStreamRecords);
        
    cout << "TraceObsMerger::TraceObsMerger() mergedTraceID="<<mergedTraceID<<" outStreamRecords[trace]="<<outStreamRecords["trace"]->str()<<endl;
      
    // Get the trace merging policy
    trace::mergeT merge = (trace::mergeT)((TraceStreamRecord*)outStreamRecords["trace"])->merge[mergedTraceID];
    
    // Create a separate tag for each observation on each stream (we'll add aggregation code in the future)
    // The last stream's entry tag will be written to pMap, while the other streams' entry and exit tags will be 
    // placed in moreTagsBefore. Thus, the merged exit tag will end up corresponding to the last stream's entry tag.
    properties obsExitProps("traceObs");

    // If we need to keep separate observations for each stream
    if(merge == trace::disjMerge) {
      for(int t=0; t<tags.size(); t++) {
        TraceStreamRecord* ts = (TraceStreamRecord*)inStreamRecords[t]["trace"];
        
        map<string, string> traceObsMap = pMap;
        map<string, string>& curMap = (t<tags.size()-1? traceObsMap: pMap);
        
        curMap["traceID"] = txt()<<mergedTraceID;
        
        curMap["outputStreamID"] = properties::get(tags[t].second, "outputStreamID");
        
        curMap["numTraceAttrs"] = properties::get(tags[t].second, "numTraceAttrs");
        int numTraceAttrs = properties::getInt(tags[t].second, "numTraceAttrs");
        for(int i=0; i<numTraceAttrs; i++) {
          curMap[txt()<<"tKey_"<<i] = properties::get(tags[t].second, txt()<<"tKey_"<<i);
          curMap[txt()<<"tVal_"<<i] = properties::get(tags[t].second, txt()<<"tVal_"<<i);
          streamID inSID(properties::getInt(tags[t].second, txt()<<"tAnchorID_"<<i), 
                         inStreamRecords[t]["trace"]->getVariantID());
          streamID outSID = inStreamRecords[t]["anchor"]->in2outID(inSID);
          curMap[txt()<<"tAnchorID_"<<i] = txt()<<outSID.ID;
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
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Observations may only be merged if they correspond to traces that were merged in the outgoing stream
    streamID inSID(properties::getInt(tag, "traceID"), 
                   inStreamRecords["trace"]->getVariantID());
    key.push_back(txt()<<inStreamRecords["trace"]->in2outID(inSID).ID);
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
  maxTraceID = -1;
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++)
    maxTraceID = (((TraceStreamRecord*)(*s)["trace"])->maxTraceID > maxTraceID? 
                   ((TraceStreamRecord*)(*s)["trace"])->maxTraceID: 
                   maxTraceID);
  
  // Set edges and in2outTraceIDs to be the union of its counterparts in streams
  in2outTraceIDs.clear();
  for(vector<map<string, streamRecord*> >::iterator s=streams.begin(); s!=streams.end(); s++) {
    TraceStreamRecord* ns = (TraceStreamRecord*)(*s)["trace"];
    for(map<streamID, streamID>::const_iterator i=ns->in2outTraceIDs.begin(); i!=ns->in2outTraceIDs.end(); i++)
      in2outTraceIDs.insert(*i);
  }
}

// Marge the IDs of the next graph (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
// updating each incoming stream's mappings from its IDs to the outgoing stream's IDs. Returns the traceID of the merged trace
// in the outgoing stream.
int TraceStreamRecord::mergeIDs(std::map<std::string, std::string>& pMap, 
                     const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                     std::map<std::string, streamRecord*>& outStreamRecords,
                     std::vector<std::map<std::string, streamRecord*> >& inStreamRecords) {
  // Assign to the merged trace the next ID for this output stream
  int mergedTraceID = ((TraceStreamRecord*)outStreamRecords["trace"])->maxTraceID;
  pMap["traceID"] = txt()<<mergedTraceID;
  
  // The trace's ID within the outgoing stream    
  streamID outSID(((TraceStreamRecord*)outStreamRecords["trace"])->maxTraceID,
                  outStreamRecords["trace"]->getVariantID());
  
  // Update inStreamRecords to map the trace's ID within each incoming stream to the assigned ID in the outgoing stream
  for(int t=0; t<tags.size(); t++) {
    TraceStreamRecord* ts = (TraceStreamRecord*)inStreamRecords[t]["trace"];
    
    // The graph's ID within the current incoming stream
    streamID inSID(properties::getInt(tags[t].second, "traceID"), 
                   inStreamRecords[t]["trace"]->getVariantID());
    
    if(ts->in2outTraceIDs.find(inSID) == ts->in2outTraceIDs.end())
      ((TraceStreamRecord*)inStreamRecords[t]["trace"])->in2outTraceIDs[inSID] = outSID;
  }
  
  // Advance maxTraceID
  ((TraceStreamRecord*)outStreamRecords["trace"])->maxTraceID++;
  
  return mergedTraceID;
}
  
std::string TraceStreamRecord::str(std::string indent) const {
  ostringstream s;
  s << "[TraceStreamRecord: maxTraceID="<<maxTraceID<<endl;
  
  s << indent << "in2outTraceIDs="<<endl;
  for(map<streamID, streamID>::const_iterator i=in2outTraceIDs.begin(); i!=in2outTraceIDs.end(); i++)
    s << indent << "    "<<i->first.str()<<" =&gt; "<<i->second.str()<<endl;
  
  s << indent << "]";
  
  return s.str();
}

}; // namespace structure 
}; // namespace sight

