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
 
  trace t("Trace", trace::showBegin, trace::lines);
  processedTrace pt("Processed", processedTrace::commands("./11.ExternTraceProcess.windowing 5"), trace::showBegin, trace::lines);

  for(int i=0; i<50; i++) {
    traceAttr((trace*)&t,  trace::ctxtVals("i", i), trace::observation("x", abs(50-i*2)));
    traceAttr((trace*)&pt, trace::ctxtVals("i", i), trace::observation("x", abs(50-i*2)));
  }
  
  return 0;
}
