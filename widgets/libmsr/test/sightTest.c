#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
using namespace std;

#include <sight.h>
using namespace sight;

#include "msr_core.h"

int main( int   argc, char *argv[] )
{
   context runCfg;
   if(getenv("POWER")) runCfg.add("power_cap", string(getenv("POWER")));
   //runCfg.add("MPIRank", myid);
   SightInit(argc, argv, "Sight RAPL Test", txt()<<"dbg.SightRAPLTest"<<
                                                        (getenv("POWER")? txt()<<".power_"<<getenv("POWER"): string("")));
   modularApp ma("AMG", namedMeasures(/*"time", new timeMeasure(),*/
                                      "RAPL", new RAPLMeasure()));
   
   module modSolve(instance("Solve", 1, 0), 
                    inputs(port(runCfg)));
/*   double a[8], b[8];
   for(int i=0; i<8; i++) {
      a[i] = rand()%10;
      b[i] = rand()%10;
   }
   for(long i=0; i<10000000; i++) {
     for(int j=0; j<100; j++) {
       for(int k=0; k<8; k++) {
         a[k] *= b[k];
         b[k] /= a[k];
       }
     }
     for(int j=0; j<100; j++) {
       for(int k=0; k<8; k++) {
         b[k] *= a[k];
         a[k] /= b[k];
       }
     }
   }
   for(int k=0; k<8; k++) printf("%le, %le; ", a[k], b[k]);
   printf("\n");*/

   //int pid = fork();
   //if(pid == 0){       /*child*/
   //  execl("mersenne/mprime", "mersenne/mprime", "-t");
   //} else if(pid > 0){  /*parent*/
     sleep(1);
   //  kill(pid, SIGTERM);
   //}

   return 0;  
}

