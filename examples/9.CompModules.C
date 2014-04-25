#include "sight.h"
#include <math.h>
#include <map>
#include <assert.h>
#include <unistd.h>
using namespace std;
using namespace sight;

#define idx(i, j, cols) ((i)+(j)*(cols))

int main (int argc, char *argv[])
{
  // Time 1 unit
  // Box 1x1
  // The center region the area has a non-zero temperature while the outer region has 0 temperature
  if(argc<=5) { cerr << "\nUsage: 9.CompModules isReference dx dy dt k\n"; assert(0); }
  
  int isReference=atoi(argv[1]); 
  double dx=strtod(argv[2], NULL); assert(dx>0);
  double dy=strtod(argv[3], NULL); assert(dy>0);
  double dt=strtod(argv[4], NULL); assert(dt>0);
  double k =strtod(argv[5], NULL); assert(k>0); assert(k<1);
  
  SightInit(argc, argv, "9.CompModules", txt()<<"dbg.9.CompModules.dx_"<<dx<<".dy_"<<dy<<".dt_"<<dt<<".k="<<k<<(isReference? ".Reference": ""));

  compModularApp mfemApp("Heat Equation"/*, namedMeasures("time0", new timeMeasure())*/); 
  
  double initTemp = 10; // The initial temperature at the center of the grid
  
  int X = ceil(1/dx)+2; // Number of cells in the X dimension, plus 2 for the boundaries
  int Y = ceil(1/dy)+2; // Number of cells in the Y dimension, plus 2 for the boundaries
  int T = ceil(1/dt); // Number of time steps
  dbg << "X="<<X<<", Y="<<Y<<", T="<<T<<endl;
  
  double* nextTemp = new double[X*Y];
  double* lastTemp = new double[X*Y];
  // Initialize the space to have a temperature spike at the center and 0 temperature elsewhere
  { 
    #if defined(TRACE)
    trace heatmapTrace("Initial Temp", trace::context("x", "y"), trace::showBegin, trace::heatmap);
    #endif
  for(int x=0; x<X; x++) {
    #if defined(TRACE)
    attr xAttr("x", x);
    #endif
    for(int y=0; y<Y; y++) {
      #if defined(TRACE)
      attr yAttr("y", y);
      #endif
      // If this is the center box
      if(x>=X*4/10 && x<=X*6/10 && y>=Y*4/10 && y<=Y*6/10) lastTemp[idx(x,y,X)] = initTemp;
      else                                                 lastTemp[idx(x,y,X)] = 0;
      nextTemp[idx(x,y,X)] = 0;
      #if defined(TRACE)
      traceAttr("Initial Temp", "temperature", attrValue(lastTemp[idx(x,y,Y)])/*, s.getAnchor()*/);
      #endif
      //dbg << "temp["<<x<<"]["<<y<<"]==["<<idx(x,y,Y)<<"] = "<<temp[idx(x,y,Y)]<<endl;
    }
  } }
  
  std::vector<port> externalOutputs;
  compModule mod(instance("Heat Computation", 1, 1), 
                 inputs(port(context("k", k,
                                     "initTemp", initTemp))),
                 externalOutputs,
                 isReference, 
                 context("dx", dx,
                         "dy", dy,
                         "dt", dt),
                 compNamedMeasures("time", new timeMeasure(), noComp(),
                                   "PAPI", new PAPIMeasure(papiEvents(PAPI_L1_TCM, PAPI_L2_TCM, PAPI_L3_TCM)), noComp()));
  
  // Time step 
  for(int t=0; t<T; t++) {
    std::vector<port> externalTSOutputs;
    compModule tsMod(instance("TimeStep", 3, 1), 
                 inputs(port(context("k", k,
                                     "initTemp", initTemp)),
                        port(context("t", t)),
                        port(compContext("temp", sightArray(sightArray::dims(X,Y), nextTemp), LkComp(2, attrValue::floatT, true)))),
                 externalTSOutputs,
                 isReference, 
                 context("dx", dx,
                         "dy", dy,
                         "dt", dt),
                 compNamedMeasures("time", new timeMeasure(), noComp(),
                                   "PAPI", new PAPIMeasure(papiEvents(PAPI_L1_TCM, PAPI_L2_TCM, PAPI_L3_TCM)), noComp()));
    //attr tAttr("t", t);
    //attr vAttr("verbose", t%(T/10) == 0);
    //attrIf aI(new attrNEQ("verbose", 0));
    //attrFalse aF();
    #if defined(TRACE)
    trace heatmapTrace(txt()<<"Temp t="<<t, trace::context("x", "y"), trace::showBegin, trace::heatmap);
    #endif
    
    for(int x=1; x<X-1; x++) {
      #if defined(TRACE)
      attr xAttr("x", x);
      #endif
      for(int y=1; y<Y-1; y++) {
        #if defined(TRACE)
        attr yAttr("y", y);
        #endif
        
        /*scope s(txt()<<"x="<<x<<" y="<<y);
        dbg << "lastTemp["<<x<<"]["<<y<<"] = "<<lastTemp[idx(x,y,Y)]<<endl;*/
        nextTemp[idx(x,y,X)] = lastTemp[idx(x,y,X)]+
                               k*(dt/dx/dx)*
                                 (lastTemp[idx(x+1,y,X)]+
                                  lastTemp[idx(x-1,y,X)]+
                                  lastTemp[idx(x,y+1,X)]+
                                  lastTemp[idx(x,y-1,X)]-4*lastTemp[idx(x,y,X)]);
        
        /*dbg << "temp["<<x-1<<"]["<<y<<"] = "<<lastTemp[idx(x-1,y,Y)]<<endl;
        dbg << "temp["<<x<<"]["<<y-1<<"] = "<<lastTemp[idx(x,y-1,Y)]<<endl;
        dbg << "temp["<<x+1<<"]["<<y<<"] = "<<lastTemp[idx(x+1,y,X)]<<endl;
        dbg << "temp["<<x<<"]["<<y+1<<"] = "<<lastTemp[idx(x,y+1,Y)]<<endl;
        dbg << "nextTemp["<<x<<"]["<<y<<"] = "<<nextTemp[idx(x,y,Y)]<<endl;*/
        
        #if defined(TRACE)
        traceAttr(txt()<<"Temp t="<<t, "temperature", attrValue(nextTemp[idx(x,y,Y)])/*, s.getAnchor()*/);
        #endif
    } }
    
    // swap temp and lastTemp;
    double* tmp = lastTemp;
    lastTemp = nextTemp;
    nextTemp = tmp;
    tsMod.setOutCtxt(0, compContext("temp", sightArray(sightArray::dims(X,Y), nextTemp), LkComp(2, attrValue::floatT, true)));
    //cout << ">"<<endl;
  }
  mod.setOutCtxt(0, compContext("temp", sightArray(sightArray::dims(X,Y), nextTemp), LkComp(2, attrValue::floatT, true)));
  
  delete nextTemp;
  delete lastTemp;
  
  return 0;
}
