#include "sight.h"
#include <math.h>
#include <map>
#include <assert.h>
#include <unistd.h>
using namespace std;
using namespace sight;

#define abs(x) ((x)<0? 0-(x): (x))

int main (int argc, char *argv[])
{
  //if(argc<=4) { cerr << "\nUsage: 11.ExternTraceProcess expNum dx dy dt\n"; assert(0); }
  
  SightInit(argc, argv, "11.ExternTraceProcess", txt()<<"dbg.11.ExternTraceProcess");
 
  {
    trace t("Trace", trace::showBegin, trace::lines);
    processedTrace pt("Processed", processedTrace::commands("./11.ExternTraceProcess.windowing 15"), trace::showBegin, trace::lines);

    for(int i=0; i<50; i++) {
      traceAttr((trace*)&t,  trace::ctxtVals("i", i), trace::observation("x", abs(50-i*2)));
      traceAttr((trace*)&pt, trace::ctxtVals("i", i), trace::observation("x", abs(50-i*2)));
    }
  }
  
  modularApp mfemApp("Processed App"); 
  
  for(int i=0; i<50; i++) {
    std::vector<port> externalOutputs;
    {
      module mod(instance("Unprocessed", 1, 1), 
                 inputs(port(context("i", i))),
                 externalOutputs,
                 namedMeasures("time", new timeMeasure()));
      usleep(abs(50-i*2)*1000);
      mod.setOutCtxt(0, context("x", abs(50-i*2)));
    }
    
    {
      processedModule mod(instance("Processed", 1, 1), 
                          inputs(port(context("i", i))),
                          externalOutputs,
                          processedTrace::commands("./11.ExternTraceProcess.windowing 15"),
                          namedMeasures("time", new timeMeasure()));
      usleep(abs(50-i*2)*1000);
      mod.setOutCtxt(0, context("x", abs(50-i*2)));
    }
  }
  
  return 0;
}
