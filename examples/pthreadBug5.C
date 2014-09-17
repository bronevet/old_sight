/******************************************************************************
* FILE: bug5.c
* DESCRIPTION:
*   A simple pthreads program that dies before the threads can do their
*   work. Figure out why.
* AUTHOR: 07/06/05 Blaise Barney
* LAST REVISED: 07/11/12
******************************************************************************/
//#include <pthread.h>
#include "sight.h"
#include "sight_pthread.h"
using namespace sight;
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define NUM_THREADS	5

void *PrintHello(void *threadid)
{
   scope s("PrintHello");
   int i;
   double myresult=0.0;
   dbgprintf("starting...\n");
   for (i=0; i<1000000; i++)
      myresult += sin(i) * tan(i);
   dbgprintf("result=%e. Done.\n", myresult);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  pthread_t threads[NUM_THREADS];
  int rc;
  long t;
  SightInit(argc, argv, "Pthreads Bug5", "dbg.pthreadBug5.individual");
  for(t=0;t<NUM_THREADS;t++){
    dbgprintf("Creating thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
    if (rc){
      dbgprintf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
      }
    }
  dbgprintf("Main: Done.\n");
}
