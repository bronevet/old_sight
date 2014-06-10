#include "sight.h"
#include <map>
#include <assert.h>

using namespace std;
using namespace sight;

int main(int argc, char** argv) {
  SightInit(argc, argv, "5.Tracing", "dbg.5.Tracing");
  
  dbg << "<h1>Example 5: Tracing</h1>" << endl;  
  
  dbg << "This example shows how to trace the values of key variables and visualize this trace in various ways. "<<
         "This is done by creating one or more trace objects and giving them string names. To include a given "<<
         "observation of a value in a given trace the user must call traceAttr(), providing as arguments the name "<<
         "of the trace, the name and value of the traced variable and an optional anchor. The trace will be shown either at the "<<
         "spot in the output where the trace came into scope or where it went out of scope (selected when the trace is created). Further, the traced "<<
         "values will be displayed as a function of user-selected attributes items. There several different ways "<<
         "to visualize traces. This example illustraces Box Plots, Tables, Heatmaps and Line Graphs. Heatmaps support anchors "<<
         "so that if a user clicks on a given heatmap tile (corresponds to a single observation), they are taken to "<<
         "the anchor's target. However, they are restricted to only two context attributes (coorespond to the two dimensions "<<
         "of the heatmap. Box Plots, Tables and Line Graphs ignore any anchors associated with observations and only show values. "<<
         "They place no constraints on the number of context attributes."<<endl;
  
  dbg << "In this example the Box Plot, Table and Heatmap will be shown at the trace start point and the Line Graph will be shown at the trace end point."<<endl;
  
  // First select the names of the context attributes that influence the values of the traced variables
  trace tableTrace("Table", trace::context("i", "j"), trace::showBegin, trace::table);
  trace heatmapTrace("Heatmap", trace::context("i", "j"), trace::showBegin, trace::heatmap);
  
  trace boxplotTrace("Boxplot", "k", trace::showBegin, trace::boxplot);
  
  // For the last trace we'll use k as the context variable
  trace linesTrace("Lines", "k", trace::showEnd, trace::lines);
  trace scatterTrace("Scatter", trace::context("i", "j", "k"), trace::showBegin, trace::scatter3d);
  
  for(int i=0; i<5; i++) {
    attr iAttr("i", i);
    scope s(txt()<<"i="<<i);
    
    for(int j=i; j<5; j++) {
      attr jAttr("j", j);
      scope s(txt()<<"j="<<j);

      traceAttr("Heatmap", "i*j", attrValue(i*j), s.getAnchor());
      traceAttr("Heatmap", "i+j", attrValue(i+j), s.getAnchor());
      
      for(int k=0; k<=j; k++) {
        attr kAttr("k", k);
        scope s(txt()<<"k="<<k);
        
        traceAttr("Table", "k", attrValue(k));
        
        traceAttr("Lines", "i", attrValue(i));
        traceAttr("Lines", "j", attrValue(j));
        
        traceAttr("Boxplot", "j", attrValue(j));
        traceAttr("Boxplot", "i", i);
        
        traceAttr("Scatter", "i*j*k",   i*j*k);
        traceAttr("Scatter", "(i+j)*k", (i+j)*k);
        traceAttr("Scatter", "i+j+k",   i+j+k);
      }
    }
  }
  
  return 0;
}

