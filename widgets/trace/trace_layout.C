#include "../../sight_layout_internal.h"
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
  (*layoutEnterHandlers)["traceStream"] = &trace::enterTraceStream;
  (*layoutExitHandlers )["traceStream"] = &defaultExitHandler;
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
  if(showLoc == showBegin) dbg << "<div id=\"div"<<getBlockID()<<"-Trace\"></div>";
  
  stream = NULL;
    
  tStack.push_back(this);
}

trace::~trace() {
  if(showLoc == showEnd) dbg << "<div id=\"div"<<getBlockID()<<"-Trace\"></div>";
  
  assert(stream);
  delete(stream);
  
  dbg.exitBlock();
  
  assert(tStack.size()>0);
  assert(tStack.back()==this);
  tStack.pop_back();
}

void *trace::enterTraceStream(properties::iterator props) {
  // Get the currently active trace that this traceStream belongs to
  assert(tStack.size()>0);
  trace* t = tStack.back();
  t->stream = new traceStream(props, txt()<<"div"<<t->getBlockID()<<"-Trace");
  
  // Register the traceStream to listen directly to all of the events it decodes. In the vanilla version of traces
  // we don't do any filtering so all the raw observations that are recorded in the log are shown in the trace
  // visualization.
  t->stream->registerObserver(t->stream);
  
  return NULL;
}

/*************************
 ***** traceObserver *****
 *************************/

// Records whether we've notified observers of this trace that it has finished
bool traceObserver::finishNotified=false;

traceObserver::~traceObserver() {
  notifyObsFinish();
}  

// Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
// This method is optional.
void traceObserver::obsFinished() {
  notifyObsFinish();
}

// Notifies all observers of this trace that it is finished
void traceObserver::notifyObsFinish() {
  // If we have not yet notified all observers that this trace is finished, do so now
  if(!finishNotified) {
    finishNotified=true;
    for(std::map<traceObserver*, int>::iterator o=observers.begin(); o!=observers.end(); o++)
      o->first->obsFinished();
  }
}
  
// Called from inside observe() to emit an observation that this traceObserver makes.
void traceObserver::emitObservation(int traceID, 
                                    const std::map<std::string, std::string>& ctxt, 
                                    const std::map<std::string, std::string>& obs,
                                    const std::map<std::string, anchor>&      obsAnchor/*,
                                    const std::set<traceObserver*>&           observers*/) {
  for(std::map<traceObserver*, int>::iterator o=observers.begin(); o!=observers.end(); o++)
  //for(std::set<traceObserver*>::iterator o=observers.begin(); o!=observers.end(); o++)
    o->first->observe(traceID, ctxt, obs, obsAnchor);
}

// Registers/unregisters a given object as an observer of this traceStream
void traceObserver::registerObserver(traceObserver* obs) { 
  // If this observer has not been registered until now, initialize it to a count of 1
  if(observers.find(obs) == observers.end()) {
    observers[obs] = 1;
    //observersS.insert(obs);
  // Otherwise, increment its count
  } else
    observers[obs]++;
}

void traceObserver::unregisterObserver(traceObserver* obs) {
  assert(observers.find(obs) != observers.end());
  observers[obs]--;
  // If this observer's counter has dropped to 0, erase it
  if(observers[obs] == 0) {
    observers.erase(obs);
    //observersS.erase(obs);
  }
} 

/******************************
 ***** traceObserverQueue *****
 ******************************/

traceObserverQueue::traceObserverQueue() {
  firstO = NULL;
  lastO = NULL;
}

traceObserverQueue::traceObserverQueue(const std::list<traceObserver*>& observersL)/* : queue(observersL)*/ {
  // If observersL is non-empty
  if(observersL.size()>0) {
    firstO = observersL.front();
    lastO  = observersL.back();
    
    // Iterate through the observersL list, adding each traceObserver in the list as an observer of the traceObserver
    // that precedes it.

    list<traceObserver*>::const_iterator o=observersL.begin();

    // The traceObserver most recently encountered by the loop
    traceObserver* recentO=*o;

    for(o++; o!=observersL.end(); o++) {
      recentO->registerObserver(*o);
      recentO = *o;
    }
  // If observersL is empty
  } else {
    firstO = NULL;
    lastO = NULL;
  }
}

// Push a new observer to the back of the observers queue
void traceObserverQueue::push_back(traceObserver* obs)
{
  //queue.push_back(obs);
  // If this is the first element in the observers queue
  if(firstO == NULL) {
    firstO = obs;
    lastO  = obs;
    
    // Take any observers currently registered with this queue and register them with the queue's last element
    for(map<traceObserver*, int>::iterator o=observers.begin(); o!=observers.end(); o++)
      lastO->registerObserver(o->first);

  // If observers already has elements
  } else {
    // Move over any observers currently registered with this queue to be registered with the queue's new last element
    for(map<traceObserver*, int>::iterator o=observers.begin(); o!=observers.end(); o++) {
      lastO->unregisterObserver(o->first);
      obs->registerObserver(o->first);
    }
    
    // Place obs immediately after lastO
    lastO->registerObserver(obs);
    
    lastO = obs;
  }
}

// Push a new observer to the front of the observers queue
void traceObserverQueue::push_front(traceObserver* obs)
{
  //queue.push_front(obs);
  // If this is the first element in observers
  if(lastO==NULL) {
    firstO = obs;
    lastO  = obs;
    
    // Take any observers currently registered with this queue and register them with the queue's last element
    for(map<traceObserver*, int>::iterator o=observers.begin(); o!=observers.end(); o++)
      lastO->registerObserver(o->first);
  // If observers already has elements
  } else {
    // Place obs immediately before firstO
    obs->registerObserver(firstO);
    
    firstO = obs;
  }
}

// Override the registration methods from traceObserver to add the observers to the back of the queue
// Registers/unregisters a given object as an observer of this traceStream
void traceObserverQueue::registerObserver(traceObserver* obs) {
  // Register this observer within this object. We'll use traceObserver::observers as the record of which 
  // traceObservers are registered and we'll make sure that they're registered with the last element in the queue.
  traceObserver::registerObserver(obs);
  
  // If there are currently elements in the observers queue
  if(lastO!=NULL) {
    // Also register this observer within the last element. It must be that the set of observers registered with
    // this traceObserverQueue is identical to the set of observers registered with the queue's last element.
    lastO->registerObserver(obs);
  }
}

void traceObserverQueue::unregisterObserver(traceObserver* obs) {
  // Unregister this observer within this object. We'll use traceObserver::observers as the record of which 
  // traceObservers are registered and we'll make sure that they're registered with the last element in the queue.
  traceObserver::unregisterObserver(obs);
  
  // If there are currently elements in the observers queue
  if(lastO!=NULL) {
    // Also unregister this observer within the last element. It must be that the set of observers registered with
    // this traceObserverQueue is identical to the set of observers registered with the queue's last element.
    lastO->unregisterObserver(obs);
  }
}

// traceID - unique ID of the trace from which the observation came
// ctxt - maps the names of the observation's context attributes to string representations of their values
// obs - maps the names of the trace observation attributes to string representations of their values
// obsAnchor - maps the names of the trace observation attributes to the anchor that identifies where they were observed
void traceObserverQueue::observe(int traceID, 
                                 const std::map<std::string, std::string>& ctxt, 
                                 const std::map<std::string, std::string>& obs,
                                 const std::map<std::string, anchor>&      obsAnchor/*,
                                 const std::set<traceObserver*>&           observers*/) {
  if(firstO!=NULL)
    // Forward the observation to the first traceObserver in the queue and let it forward it to its
    // successors in the queue as it needs to.
    firstO->observe(traceID, ctxt, obs, obsAnchor);
  
  /*
  static bool observing=false;
  static list<traceObserver*>::iterator curObserver;
  static int obsIdx;
  // Points
  //static const std::set<traceObserver*>* startObservers;
  
  // If this is the start of an observation
  traceObserver* o=NULL;
  if(observing==false) {
    curObserver = queue.begin();
    o = *curObserver;
    observing = true;
    obsIdx=0;
    //cout << "traceObserverQueue::observe("<<traceID<<") start obsIdx="<<obsIdx<<endl;
  // We're in the middle of observing
  } else {
    o = *curObserver;
    
    obsIdx++;
    
    //cout << "traceObserverQueue::observe("<<traceID<<") continue obsIdx="<<obsIdx<<endl;
  }
  
  curObserver++;
  // If this is the last traceObserver in observers, reset state
  if(curObserver == queue.end()) {
    observing = false;
    //cout << "traceObserverQueue::observe("<<traceID<<") end obsIdx="<<obsIdx<<endl;
  }
    
  std::set<traceObserver*> nextObs; nextObs.insert(this);
  o->observe(traceID, ctxt, obs, obsAnchor, nextObs);
  //emitObservation(traceID, ctxt, obs, obsAnchor, observers);*/
}

/***************************************
 ***** externalTraceProcessor_File *****
 ***************************************/

externalTraceProcessor_File::externalTraceProcessor_File(std::string processorFName, std::string obsFName) : 
  processorFName(processorFName), obsFName(obsFName) 
{
  traceFile.open(obsFName.c_str());
}
  
// Interface implemented by objects that listen for observations a traceStream reads. Such objects
// call traceStream::registerObserver() to inform a given traceStream that it should observations.
void externalTraceProcessor_File::observe(int traceID,
             const std::map<std::string, std::string>& ctxt, 
             const std::map<std::string, std::string>& obs,
             const std::map<std::string, anchor>&      obsAnchor) {

  // Serialize the current observation into the trace file
  for(std::map<std::string, std::string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    sight::common::escapedStr key(c->first,  " :", sight::common::escapedStr::escaped);
    sight::common::escapedStr val(c->second, " :", sight::common::escapedStr::escaped);
    traceFile << "ctxt:"<<key.escape()<<":"<<val.escape()<<" ";
  }

  for(std::map<std::string, std::string>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
    sight::common::escapedStr key(o->first,  " :", sight::common::escapedStr::escaped);
    sight::common::escapedStr val(o->second, " :", sight::common::escapedStr::escaped);
    traceFile << "obs:"<<key.escape()<<":"<<val.escape()<<" ";
  }

  for(std::map<std::string, anchor>::const_iterator a=obsAnchor.begin(); a!=obsAnchor.end(); a++) {
    sight::common::escapedStr key(a->first,  " :", sight::common::escapedStr::escaped);
    traceFile << "anchor:"<<key.escape()<<":"<<a->second.getID()<<" ";
  }

  traceFile << endl;
}

// Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
// This method is optional.
void externalTraceProcessor_File::obsFinished() {
  // We're done writing the trace file, so close the stream
  traceFile.close();
  
  // Execute the processing application
  ostringstream s; s<< processorFName << " "<<obsFName;
  FILE* out = popen(s.str().c_str(), "r");

  // Read out the trace produced by the processor
  char line[10000];
  while(fgets(line, 10000, out)) {
    // Split the line into its individual terms
    sight::common::escapedStr es(string(line), " ", sight::common::escapedStr::escaped);
    vector<string> terms = es.unescapeSplit(" ");

    map<string, string> ctxt, obs;
    map<string, anchor> obsAnchor;

    // Iterate over each term of this observation, adding each one into this observation's context, trace observation or anchors
    for(vector<string>::iterator t=terms.begin(); t!=terms.end(); t++) {
      // Split the term into the key and the value
      sight::common::escapedStr es(string(*t), ":", sight::common::escapedStr::escaped);
      vector<string> kv = es.unescapeSplit(":");
      assert(kv.size()==2);
      string key=kv[0];
      string val=kv[1];

      // Identify the type of this term
      int colonLoc = key.find(":");
      string type = key.substr(0, colonLoc);
      string keyName = key.substr(colonLoc+1);
      if(type == "ctxt") ctxt[keyName] = val;
      else if(type=="obs") obs[keyName] = val;
      else obsAnchor[keyName] = anchor(attrValue::parseInt(val));
    }

    // Emit the observation
    emitObservation(-1, ctxt, obs, obsAnchor);
  }
  
  // Inform this trace's observers that it has finished
  traceObserver::obsFinished();;
}

/***********************
 ***** traceStream *****
 ***********************/

// Maps the traceIDs of all the currently active traces to their trace objects
std::map<int, traceStream*> traceStream::active;

// hostDiv - the div where the trace data should be displayed
  // showTrace - indicates whether the trace should be shown by default (true) or whether the host will control
  //             when it is shown
traceStream::traceStream(properties::iterator props, std::string hostDiv, bool showTrace) : 
  hostDiv(hostDiv), showTrace(showTrace)
{

  static bool initialized = false;
  
  if(!initialized) {
    // Create the directory that holds the trace-specific scripts
    dbg.createWidgetDir("trace");
    
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

    // Three.js
    dbg.includeWidgetScript("Three.js",               "text/javascript"); dbg.includeFile("Three.js");
    dbg.includeWidgetScript("Detector.js",            "text/javascript"); dbg.includeFile("Detector.js");
    dbg.includeWidgetScript("OrbitControls.js",       "text/javascript"); dbg.includeFile("OrbitControls.js");
    dbg.includeWidgetScript("THREEx.WindowResize.js", "text/javascript"); dbg.includeFile("THREEx.WindowResize.js");
    //dbg.includeWidgetScript("Stats.js",   "text/javascript"); dbg.includeFile("Stats.js");
    
    // D3 Widgets
    dbg.includeWidgetScript("d3.v3.min.js", "text/javascript"); dbg.includeFile("d3.v3.min.js");
    //dbg.includeWidgetScript("d3.v3.js", "text/javascript"); dbg.includeFile("d3.v3.js");
    dbg.includeWidgetScript("trace/table.js",     "text/javascript"); dbg.includeFile("trace/table.js");
    dbg.includeWidgetScript("trace/boxplot.js",   "text/javascript"); dbg.includeFile("trace/boxplot.js");
    dbg.includeWidgetScript("trace/gradient.js",  "text/javascript"); dbg.includeFile("trace/gradient.js");
    dbg.includeWidgetScript("trace/scatter.js",   "text/javascript"); dbg.includeFile("trace/scatter.js");
    dbg.includeWidgetScript("trace/scatter3d.js", "text/javascript"); dbg.includeFile("trace/scatter3d.js");

    dbg.includeWidgetScript("trace/trace.js", "text/javascript"); dbg.includeFile("trace/trace.js"); 
    
    initialized = true;
  }
  
  traceID = properties::getInt(props, "traceID");
  viz     = (vizT)properties::getInt(props, "viz");

//cout << "ts::ts this="<<this<<" props="<<props.str()<<endl<<"viz="<<viz<<endl;
  
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
  
  //traceAttrsInitialized = false;

  active[traceID] = this;
  //cout << "New Trace "<<traceID<<", this="<<this<<endl;
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
  // If the trace is shown by default
  if(showTrace) { 
    // String that contains the names of all the context attributes 
    string ctxtAttrsStr;
    if(viz==table || viz==decTree || viz==scatter3d || viz==heatmap || viz==boxplot)
      ctxtAttrsStr = JSArray<list<string> >(contextAttrs);
    
    // String that contains the names of all the trace attributes
    string tracerAttrsStr;
    if(viz==table || viz==lines || viz==heatmap || viz==scatter3d || viz==boxplot)
      tracerAttrsStr = JSArray<list<string> >(traceAttrs);
    
    assert(hostDiv != "");
    // Now that we know all the trace variables that are included in this trace, emit the trace
    if(viz==table || viz==heatmap || viz==scatter3d) {
      //dbg.widgetScriptPrologCommand(txt()<<"loadGoogleAPI();");
      ostringstream cmd; 
      cmd<<"displayTrace('"<<traceID<<"', "<<
                         "'"<<hostDiv<<"', "<<
                         ctxtAttrsStr<<", " << 
                         tracerAttrsStr<<", "<<  
                         "'"<<viz2Str(viz)<<"', true, true);";
      dbg.widgetScriptEpilogCommand(cmd.str());
    } else if(viz==lines) {
      // Create a separate line graph for each context attribute
      for(std::set<std::string>::iterator c=contextAttrsSet.begin(); c!=contextAttrsSet.end(); c++) {
        dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<traceID<<"', "<<
                                      "'"<<hostDiv<<"', "<<
                                      "['"<<*c<<"'], "<<
                                      tracerAttrsStr<<", "<<
                                      "'"<<viz2Str(viz)<<"', false, true);");
      }
    } else if(viz==decTree) {
      // Create a separate decision tree for each tracer attribute
      for(list<string>::iterator t=traceAttrs.begin(); t!=traceAttrs.end(); t++) {
        dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<traceID<<"', "<<
                                      "'"<<hostDiv<<"', "<<
                                      ctxtAttrsStr<<", "<<
                                      "['"<<*t<<"'], "<<
                                      "'"<<viz2Str(viz)<<"', false, true);");
      }
    } else if(viz==boxplot) {
      dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<traceID<<"', "<<
                                    "'"<<hostDiv<<"', "<<
                                    ctxtAttrsStr<<", "<<
                                    tracerAttrsStr<<", "<<
                                    "'"<<viz2Str(viz)<<"', false, true);");
    }
  }
  
  /*assert(stack.size()>0);
  stack.pop_back();*/
  assert(active.find(traceID) != active.end());
  active.erase(traceID);
}

// Given a set of context and trace attributes to visualize, a target div and a visualization type, returns the command 
// to do this visualization.
// showFresh: boolean that indicates whether we should overwrite the prior contents of hostDiv (true) or whether we should append
//      to them (false)
// showLabels: boolean that indicates whether we should show a label that annotates a data plot (true) or whether
//      we should just show the plot (false)
std::string traceStream::getDisplayJSCmd(const std::list<std::string>& contextAttrs, const std::list<std::string>& traceAttrs,
                                         std::string hostDiv, vizT viz, bool showFresh, bool showLabels) {
  // Default contextAttrs, traceAttrs, hostDiv and viz to be the values set within this traceStream
  const std::list<std::string> *localContextAttrs = &contextAttrs,
                               *localTraceAttrs   = &traceAttrs;
  if(contextAttrs.size()==0) localContextAttrs = &(this->contextAttrs);
  if(traceAttrs.size()==0)   localTraceAttrs = &(this->traceAttrs);
  if(hostDiv=="")            hostDiv = this->hostDiv;
  if(viz == unknown)         viz = this->viz;
    
  string ctxtAttrsStr = JSArray<list<string> >(*localContextAttrs);
  string tracerAttrsStr = JSArray<list<string> >(*localTraceAttrs);
  
  return txt()<<"displayTrace('"<<traceID<<"', "<<
                             "'"<<hostDiv<<"', "<<
                             ctxtAttrsStr<<", " << 
                             tracerAttrsStr<<", "<<  
                             "'"<<viz2Str(viz)<<"', "<<
                             (showFresh?"true":"false")<<", "<<
                             (showLabels?"true":"false")<<");";
}

// Record an observation
void* traceStream::observe(properties::iterator props)
{
  //cout << "trace::observe() props="<<properties::str(props)<<endl;
  long traceID = properties::getInt(props, "traceID");
  assert(active.find(traceID) != active.end());
  traceStream* ts = active[traceID];
  //cout << "    t="<<t<<endl;
  //cout << "    #ts->traceAttrs="<<ts->traceAttrs.size()<<endl;
  
  long numTraceAttrs = properties::getInt(props, "numTraceAttrs");
  long numCtxtAttrs  = properties::getInt(props, "numCtxtAttrs");
  
  // Maps that record the observation to be forwarded to any observers listening on this traceStream
  map<string, string> ctxtToObs, traceToObs;
  map<std::string, anchor> traceAnchorToObs;

  // If some observers are listening on this traceStream, record the current observation so they can look at it
  if(ts->numObservers()>0) {
    for(long i=0; i<numTraceAttrs; i++) {
      string tKey = properties::get(props, txt()<<"tKey_"<<i);
      traceToObs[tKey] = properties::get(props, txt()<<"tVal_"<<i);
      traceAnchorToObs[tKey] = anchor(properties::getInt(props, txt()<<"tAnchorID_"<<i));
    }
    for(long i=0; i<numCtxtAttrs; i++)
      ctxtToObs[properties::get(props, txt()<<"cKey_"<<i)] = properties::get(props, txt()<<"cVal_"<<i);
  
    // Inform any observers listening on this traceStream of the new observation
    ts->emitObservation(traceID, ctxtToObs, traceToObs, traceAnchorToObs);
  }
  
  return NULL;
}

// Called on each observation from the traceObserver this object is observing
// traceID - unique ID of the trace from which the observation came
// ctxt - maps the names of the observation's context attributes to string representations of their values
// obs - maps the names of the trace observation attributes to string representations of their values
// obsAnchor - maps the names of the trace observation attributes to the anchor that identifies where they were observed

// Called by any observers of the stream, which may have filtered the raw observation, to inform the traceStream
// that the given observation should actually be emitted to the output
void traceStream::observe(int traceID,
                          const map<string, string>& ctxt, 
                          const map<string, string>& obs,
                          const map<string, anchor>& obsAnchor/*,
                          const set<traceObserver*>& observers*/)
{
  //cout << "traceStream::observe("<<traceID<<") this="<<this<<", this->traceID="<<this->traceID<<", #contextAttrs="<<contextAttrs.size()<<" #ctxt="<<ctxt.size()<<endl;
  // Read all the context attributes. If contextAttrs is empty, it is filled with the context attributes of 
  // this observation. Otherwise, we verify that this observation's context is identical to prior observations.
  if(contextAttrsInitialized) assert(contextAttrs.size() == ctxt.size());
  //for(long i=0; i<numCtxtAttrs; i++) {
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    //string ctxtName = properties::get(props, txt()<<"cKey_"<<i);
    string ctxtName = c->first;
    //cout << traceID<<": "<<ctxtName<<", ts->contextAttrsInitialized="<<ts->contextAttrsInitialized<<endl;
    if(!contextAttrsInitialized) {
      contextAttrs.push_back(ctxtName);
      // Context attributes cannot be repeated
      assert(contextAttrsSet.find(ctxtName) == contextAttrsSet.end());
      contextAttrsSet.insert(ctxtName);
    } else
      assert(contextAttrsSet.find(ctxtName) != contextAttrsSet.end());
  }
  // The context attributes of this trace are now definitely initialized
  contextAttrsInitialized = true;
  
  // Read all the trace attributes. If traceAttrs is empty, it is filled with the trace attributes of 
  // this observation. Otherwise, we verify that this observation's trace is identical to prior observations.
  //if(ts->traceAttrsInitialized) assert(ts->traceAttrs.size() == numTraceAttrs);
  //for(long i=0; i<numTraceAttrs; i++) {
  for(map<string, string>::const_iterator t=obs.begin(); t!=obs.end(); t++) {
    //string traceName = properties::get(props, txt()<<"tKey_"<<i);
    string traceName = t->first;
    /*if(!ts->traceAttrsInitialized) {
      ts->traceAttrs.push_back(traceName);
      // Trace attributes cannot be repeated
      assert(ts->traceAttrsSet.find(traceName) == ts->traceAttrsSet.end());
      ts->traceAttrsSet.insert(traceName);
    } else
      assert(ts->traceAttrsSet.find(traceName) != ts->traceAttrsSet.end());*/
    
    // If this trace attribute has not yet been observed, record it
    if(traceAttrsSet.find(traceName) == traceAttrsSet.end()) {
      traceAttrs.push_back(traceName);
      traceAttrsSet.insert(traceName);
    }
  }
  // The trace attributes of this trace are now definitely initialized
  //traceAttrsInitialized = true;
  
  ostringstream cmd;
  cmd << "traceRecord(\""<<traceID<<"\", ";
  
  // Emit the observed values of tracer attributes
  cmd << "{";
  //for(long i=0; i<numTraceAttrs; i++) {
  { int i=0;
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++, i++) {
    if(i!=0) cmd << ", ";
    //string tKey = properties::get(props, txt()<<"tKey_"<<i);
    //string tVal = properties::get(props, txt()<<"tVal_"<<i);
    //cmd << "\""<< tKey << "\": \"" << tVal <<"\"";
    attrValue val(o->second, attrValue::unknownT);
    cmd << "\""<< o->first << "\": \"" << val.getAsStr() <<"\"";
    
    // If some observers are listening on this traceStream, record the current observation so they can look at it
    //if(ts->numObservers()>0) obs[tKey] = tVal;
  } }
  cmd << "}, {";
  
  // Emit the observed anchors of tracer attributes
  //for(long i=0; i<numTraceAttrs; i++) {
  { int i=0;
  for(map<string, anchor>::const_iterator o=obsAnchor.begin(); o!=obsAnchor.end(); o++, i++) {
    if(i!=0) cmd << ", ";
    //string tKey = properties::get(props, txt()<<"tKey_"<<i);
    //anchor tAnchor(properties::getInt(props, txt()<<"tAnchorID_"<<i));
    //cmd << "\""<< tKey << "\": \"" << (tAnchor==anchor::noAnchor? "": tAnchor.getLinkJS()) <<"\"";
    cmd << "\""<< o->first << "\": \"" << (o->second==anchor::noAnchor? "": o->second.getLinkJS()) <<"\"";
      
    // If some observers are listening on this traceStream, record the current observation so they can look at it
    //if(ts->numObservers()>0) obsAnchor[tKey] = tAnchor;
  }}
  cmd << "}, {";
  
  // Emit the current values of the context attributes
  //for(long i=0; i<numCtxtAttrs; i++) {
  { int i=0;
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++, i++) {
    if(i!=0) cmd << ", ";
    /*string cKey = properties::get(props, txt()<<"cKey_"<<i);
    string cVal = properties::get(props, txt()<<"cVal_"<<i);
    cmd << "\"" << cKey << "\": \"" << cVal << "\"";*/
    attrValue val(c->second, attrValue::unknownT);
    cmd << "\"" << c->first << "\": \"" << val.getAsStr() << "\"";
    
    // If some observers are listening on this traceStream, record the current observation so they can look at it
    //if(ts->numObservers()>0) ctxt[cKey] = cVal;
  }}
  cmd << "}, \""<<viz2Str(viz)<<"\");";
  
  dbg.widgetScriptCommand(cmd.str());
  
  //emitEmptyObservation(traceID, observers);
}

// Given a traceID returns a pointer to the corresponding trace object
traceStream* traceStream::get(int traceID) {
  std::map<int, traceStream*>::iterator it = active.find(traceID);
  assert(it != active.end());
  return it->second;
}

}; // namespace layout
}; // namespace sight
