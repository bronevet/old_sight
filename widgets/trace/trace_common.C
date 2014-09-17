#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <assert.h>
#include "attributes_common.h"
#include "trace_common.h"

using namespace std;

namespace sight {
namespace common {


// Returns a string representation of a common::trace::showLocT object
string trace::showLoc2Str(trace::showLocT showLoc) 
{ return (showLoc==trace::showBegin? "showBegin": (showLoc==trace::showEnd? "showEnd": "???")); }

// Returns a string representation of a vizT object
string trace::viz2Str(trace::vizT viz) {
  switch(viz) {
    case trace::table:     return "table";
    case trace::lines:     return "lines";
    case trace::scatter3d: return "scatter3d";
    case trace::decTree:   return "decTree";
    case trace::heatmap:   return "heatmap";
    case trace::boxplot:   return "boxplot";
    default:               return "???";
  }
}

// Returns a string representation of a mergeT object
string trace::mergeT2Str(mergeT merge) {
  switch(merge) {
    case trace::disjMerge: return "disjMerge";
    case trace::avgMerge:  return "avgMerge";
    case trace::maxMerge:  return "maxMerge";
    case trace::minMerge:  return "minMerge";
    default:               return "???";
  }
}

/*******************************************
 ***** Support for parsing trace files *****
 *******************************************/

// Reads the given file of trace observations, calling the provided functor on each instance.
// Each line is a white-space separated sequence of mappings in the format group:key:value, where group
// may be ctxt, obs or anchor. Each call to functor f is provided with a description of a single
// trace file line, with three different maps for each group. For ctxt and obs we map keys to attrValues,
// while for anchor we map keys to integer anchor IDs.

class readTraceFileReader : public attrFileReader {
  public:
  traceFileReader& f;
  public:
  readTraceFileReader(traceFileReader& f): f(f) {}

  void operator()(const std::map<std::string, std::map<std::string, std::string> >& readData, int lineNum) {
/*    cout << "readTraceFileReader: lineNum="<<lineNum<<", readData(#"<<readData.size()<<")="<<endl;
    for(std::map<std::string, std::map<std::string, std::string> >::const_iterator d=readData.begin(); d!=readData.end(); d++) {
      cout << "    "<<d->first<<":"<<endl;
      for(std::map<std::string, std::string>::const_iterator i=d->second.begin(); i!=d->second.end(); i++)
        cout << "        "<<i->first<<" => "<<i->second<<endl;
    }
    cout.flush();*/

    // All trace files must have one or more instances of ctxt and obs and zero or more of anchor
    assert(readData.size()==2 || readData.size()==3);
    assert(readData.find("ctxt") != readData.end());
    assert(readData.find("obs") != readData.end());
    if(readData.size()==3) assert(readData.find("anchor") != readData.end());
    
    // Load and parse the context, observation and anchor portions of the read data
    map<string, attrValue> ctxt;   // The context name=>value mappings
    map<string, attrValue> obs;    // The observation name=>value mappings
    map<string, int>       anchor; // The anchor name=>anchorID mappings
    
    std::map<std::string, std::map<std::string, std::string> >::const_iterator readCtxt    = readData.find("ctxt");
    std::map<std::string, std::map<std::string, std::string> >::const_iterator readObs     = readData.find("obs");
    std::map<std::string, std::map<std::string, std::string> >::const_iterator readAnchor  = readData.find("anchor");
    
    for(std::map<std::string, std::string>::const_iterator c=readCtxt->second.begin();   c!=readCtxt->second.end();   c++)
      ctxt[c->first] = attrValue(c->second, attrValue::unknownT);
    
    for(std::map<std::string, std::string>::const_iterator o=readObs->second.begin();    o!=readObs->second.end();    o++)
      obs[o->first] = attrValue(o->second, attrValue::unknownT);
   
    if(readData.size()==3) 
      for(std::map<std::string, std::string>::const_iterator a=readAnchor->second.begin(); a!=readAnchor->second.end(); a++)
        anchor[a->first] = attrValue::parseInt(a->second);
    
    // Call functor f on this observation
    f(ctxt, obs, anchor, lineNum);
  }
};

// Reads the given file of trace observations, calling the provided functor on each instance.
// Each line is a white-space separated sequence of mappings in the format group:key:value, where group
// may be ctxt, obs or anchor. Each call to functor f is provided with a description of a single
// trace file line, with three different maps for each group. For ctxt and obs we map keys to attrValues,
// while for anchor we map keys to integer anchor IDs.
void readTraceFile(std::string fName, traceFileReader& f) {
  readTraceFileReader reader(f);
  readAttrFile(fName, reader);
}

// Given the context, observables and anchor information for an observation, returns its serialized representation
std::string serializeTraceObservation(const std::map<std::string, attrValue>& ctxt,
                                      const std::map<std::string, attrValue>& obs,
                                      const std::map<std::string, int>&       anchor) {
  ostringstream s;
  
  bool first = true;
  // Emit the average of the window to the output file
  for(map<string, attrValue>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    if(!first) s << " "; else first = false;
    escapedStr cName(c->first,  ": ", escapedStr::unescaped);
    escapedStr cVal(c->second.serialize(), ": ", escapedStr::unescaped);
    s << "ctxt:"<<cName.escape()<<":"<<cVal.escape();
  }

  for(map<string, attrValue>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
    if(!first) s << " "; else first = false;
    escapedStr oName(o->first,  ": ", escapedStr::unescaped);
    escapedStr oVal(o->second.serialize(), ": ", escapedStr::unescaped);
    s << "obs:"<<oName.escape()<<":"<<oVal.escape();
  }

  for(map<string, int>::const_iterator a=anchor.begin(); a!=anchor.end(); a++) {
    if(!first) s << " "; else first = false;
    escapedStr aName(a->first,  ": ", escapedStr::unescaped);
    s << "anchor:"<<aName.escape()<<":"<<a->second;
  }
  
  return s.str();
}

}; // namespace common
}; // namespace sight
