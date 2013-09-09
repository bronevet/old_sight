#include "dbglog.h"
#include <map>
#include <assert.h>

using namespace std;
using namespace dbglog;

int main(int argc, char** argv) {
  initializeDebug(argc, argv);
  
  dbg << "<h1>Example 5: Tracing</h1>" << endl;  
  
  dbg << "This example shows how to trace the values of key variables and visualize this trace in various ways. "<<
         "This is done by creating one or more trace objects and giving them string names. To include a given "<<
         "observation of a value in a given trace the user must call traceAttr(), providing as arguments the name "<<
         "of the trace and the name and value of the traced variable. The trace will be shown either at the "<<
         "spot in the output where the trace came into scope or where it went out of scope. Further, the traced "<<
         "values will be displayed as a function of user-selected attributes items."<<endl;
  
  dbg << "We create two traces that use two different visualizations: Table and Line Graph. "<<
         "The Table Trace will be shown at the trace start point and the Line Graph will be shown at the trace end point."<<endl;
  
  // First select the names of the context attributes that influence the values of the traced variables
  list<string> contextAttrs;
  contextAttrs.push_back("i");
  contextAttrs.push_back("j");
  trace tableTrace("Table Trace", contextAttrs, trace::showBegin, trace::table);
  
  // For the last trace we'll use k as the context variable
  trace linesTrace("Lines Trace", "k", trace::showEnd, trace::lines);
    
  for(int i=0; i<5; i++) {
    attr iAttr("i", i);
    scope s(txt()<<"i="<<i);
    
    for(int j=i; j<5; j++) {
      attr jAttr("j", j);
      scope s(txt()<<"j="<<j);
      
      for(int k=0; k<=j; k++) {
        attr kAttr("k", k);
        scope s(txt()<<"k="<<k);
        
        traceAttr("Table Trace", "k", attrValue(k));
        
        traceAttr("Lines Trace", "i", attrValue(i));
        traceAttr("Lines Trace", "j", attrValue(j));
      }
    }
  }
  
  return 0;
}

