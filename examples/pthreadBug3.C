/*****************************************************************************
* FILE: bug3.c
* DESCRIPTION:
*   This "hello world" Pthreads program demonstrates an unsafe (incorrect)
*   way to pass thread arguments at thread creation. Compare with hello_arg1.c.
* AUTHOR: Blaise Barney
* LAST REVISED: 07/16/14
******************************************************************************/
//#include <pthread.h>
#include "sight.h"
#include "sight_pthread.h"
using namespace sight;
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS     8

void *PrintHello(void *threadid)
{
   scope s("PrintHello");
   long taskid;
   sleep(1);
   taskid = *(long *)threadid;
   dbgprintf("Hello from thread %ld\n", taskid);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  pthread_t threads[NUM_THREADS];
  int rc;
  long t;
  
  SightInit(argc, argv, "Pthreads Bug3", "dbg.pthreadBug3.individual");

  for(t=0;t<NUM_THREADS;t++) {
    dbgprintf("Creating thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, PrintHello, (void *) &t);
    if (rc) {
      dbgprintf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
      }
     }
  
  pthread_exit(NULL);
}
