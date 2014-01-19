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
  if(argc<=4) { cerr << "\nUsage: 10.SpringModules expNum dx dy dt\n"; assert(0); }
  
  int expNum=atoi(argv[1]); 
  double dx=strtod(argv[2], NULL); assert(dx>0);
  double dy=strtod(argv[3], NULL); assert(dy>0);
  double dt=strtod(argv[4], NULL); assert(dt>0);
  
  SightInit(argc, argv, "10.SpringModules", txt()<<"dbg.10.SpringModules."<<getenv("SPRING_BUF_SIZE")<<"."<<expNum);
  springModularApp mfemApp("Heat Equation"/*, namedMeasures("time0", new timeMeasure())*/); 
  
  double k = .05; // Heat transfer coefficient
  double initTemp = 10; // The initial temperature at the center of the grid
  
  int X = ceil(1/dx)+2; // Number of cells in the X dimension, plus 2 for the boundaries
  int Y = ceil(1/dy)+2; // Number of cells in the Y dimension, plus 2 for the boundaries
  int T = ceil(1/dt); // Number of time steps
  dbg << "X="<<X<<", Y="<<Y<<", T="<<T<<endl;
  
  double* nextTemp = new double[X*Y];
  double* lastTemp = new double[X*Y];
  // Initialize the space to have a temperature spike at the center and 0 temperature elsewhere
  for(int x=0; x<X; x++) {
    for(int y=0; y<Y; y++) {
      // If this is the center box
      if(x>=X*4/10 && x<=X*6/10 && y>=Y*4/10 && y<=Y*6/10) lastTemp[idx(x,y,X)] = initTemp;
      else                                                 lastTemp[idx(x,y,X)] = 0;
      nextTemp[idx(x,y,X)] = 0;
    }
  }
  
  std::vector<port> externalOutputs;
  springModule mod(instance("Heat Computation", 2, 0), 
                 inputs(port(context("k", k)),
                        port(context("initTemp", initTemp))),
                 externalOutputs,
                 context("dx", dx,
                         "dy", dy,
                         "dt", dt),
                 compNamedMeasures("time", new timeMeasure(), RelComp(attrValue::floatT),
                                   "PAPI", new PAPIMeasure(papiEvents(PAPI_L1_TCM, PAPI_L2_TCM, PAPI_L3_TCM)), RelComp(attrValue::intT)));
  
  // Time step 
  for(int t=0; t<T; t++) {
    for(int x=1; x<X-1; x++) {
      for(int y=1; y<Y-1; y++) {
        nextTemp[idx(x,y,X)] = lastTemp[idx(x,y,X)]+
                               k*(dt/dx/dx)*
                                 (lastTemp[idx(x+1,y,X)]+
                                  lastTemp[idx(x-1,y,X)]+
                                  lastTemp[idx(x,y+1,X)]+
                                  lastTemp[idx(x,y-1,X)]-4*lastTemp[idx(x,y,X)]);
    } }
    
    // swap temp and lastTemp;
    double* tmp = lastTemp;
    lastTemp = nextTemp;
    nextTemp = tmp;
  }
  
  delete nextTemp;
  delete lastTemp;
  
  return 0;
}
