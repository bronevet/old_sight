#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include "sight_common.h"

using namespace sight;
using namespace sight::common;
using namespace std;

class traceWindower : public traceFileReader {
  // The contexts, observations and anchors in the current window and the iterators to their midpoints
  list<map<string, attrValue> > windowCtxt;   list<map<string, attrValue> >::iterator midCtxt;
  list<map<string, attrValue> > windowObs;    list<map<string, attrValue> >::iterator midObs;
  list<map<string, int> >       windowAnchor; list<map<string, int> >::iterator       midAnchor;
  
  // Flag that indicates whether the midpoint iterators have been initialized
  bool midInitialized;
  
  // Maps each observation attribute to its type
  map<string, attrValue::valueType> obsTypes;
  
  int windowSize;
  public:
  traceWindower(int windowSize): midInitialized(false), windowSize(windowSize) {}
  
  void operator()(const std::map<std::string, attrValue>& ctxt,
                  const std::map<std::string, attrValue>& obs,
                  const std::map<std::string, int>& anchor, 
                  int lineNum) {
    // Ensure that all observation fields have the same type
    for(std::map<std::string, attrValue>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
      // If this is the first observation, record the type
      if(lineNum==1) obsTypes[o->first] = o->second.getType();
      // Otherwise, check for type consistency
      else if(obs.find(o->first) == obs.end()) { cerr << "ERROR: Inconsistent observation attributes! Attribute \""<<o->first<<"\" was provided on line "<<lineNum<<" for the first time!"<<endl; exit(-1); }
      else if(obsTypes[o->first] != o->second.getType()) { cerr << "ERROR: Inconsistent types of observation attribute \""<<o->first<<"\"! On line "<<lineNum<<" type is "<<attrValue::type2str(o->second.getType())<<" but on prior lines it is "<<attrValue::type2str(o->second.getType())<<"!"<<endl; exit(-1); }
    }
    if(obs.size() != obsTypes.size()) { cerr << "ERROR: observation attributes on line "<<lineNum<<" inconsistent with those on earlier lines!"<<endl; exit(-1); }
  
    // Add this line to the window
    windowCtxt.push_back(ctxt);
    windowObs.push_back(obs);
    windowAnchor.push_back(anchor);
    
    lineNum++;

    //print "line=\"$line\", ctxt=",obj2Str(\%ctxt),", obs=",obj2Str(\%obs),", anchor=",obj2Str(\%anchor),"\n";
    // If this window is full
    if(windowObs.size() == windowSize) {
      //print "window=",obj2Str(\@window),"\n";
      
      // If the midpoint iterators have not yet been initialized, set them to the window's midpoint
      if(!midInitialized) {
        midCtxt   = windowCtxt.begin();   for(int i=0; i<windowSize/2; i++) { midCtxt++; }
        midObs    = windowObs.begin();    for(int i=0; i<windowSize/2; i++) { midObs++; }
        midAnchor = windowAnchor.begin(); for(int i=0; i<windowSize/2; i++) { midAnchor++; }
        midInitialized = true;
      }

      // Iterate over the window, collecting the averages of all the observation attributes
      map<string, attrValue> avgs;

      for(list<map<string, attrValue> >::iterator i=windowObs.begin(); i!=windowObs.end(); i++) {
        for(map<string, attrValue>::iterator o=i->begin(); o!=i->end(); o++) {
          if(obsTypes[o->first] == attrValue::intT || obsTypes[o->first] == attrValue::floatT) {
            switch(obsTypes[o->first]) {
                case attrValue::intT:   avgs[o->first] = (i==windowObs.begin()? 0: avgs[o->first].getInt())   + 
                                                         o->second.getInt() / windowSize;   
                                        break;
                case attrValue::floatT: avgs[o->first] = (i==windowObs.begin()? 0: avgs[o->first].getFloat()) + 
                                                         o->second.getFloat() / windowSize; 
                                        break;
              default: exit(-1);
            }
          }
        }
        //print "$i: avgs=",obj2Str(\%avgs),"\n";
      }
      
      // Add to avgs mappings for the non-numeric observation attributes to enable us to call serializeTraceObservation()
      for(map<string, attrValue::valueType>::iterator ot=obsTypes.begin(); ot!=obsTypes.end(); ot++) {
        if(ot->second != attrValue::intT && ot->second != attrValue::floatT)
          avgs[ot->first] = (*midObs)[ot->first];
      }
      
      // Serialize the windowed observation and emit it to standard output
      //map<string, attrValue> c = *midCtxt;
      //map<string, int> a = *midAnchor;
      cout << serializeTraceObservation(*midCtxt, avgs, *midAnchor) << endl;
      
      // Remove the oldest entry from the window
      windowObs.pop_front();
      
      // Advance the midpoint iterators
      midCtxt++;
      midObs++;
      midAnchor++;
    }
  }
};

int main(int argc, char** argv) {
  if(argc!=3) { cerr << "Usage: 11.ExternTraceProcess.windowing.pl windowSize inFName"<<endl; exit(-1); }
  int windowSize = atoi(argv[1]);
  char* inFName    = argv[2];
  
  traceWindower reader(windowSize);
  readTraceFile(string(inFName), reader);
}