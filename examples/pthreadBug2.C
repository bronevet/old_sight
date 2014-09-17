/******************************************************************************
* FILE: bug2.c
* DESCRIPTION:
*   A "hello world" Pthreads program that dumps core.  Figure out why and
*   then fix it - or else see the solution bug2fix.c.
* AUTHOR: 9/98 Blaise Barney
* LAST REVISED: 01/29/09
******************************************************************************/
//#include <pthread.h>
#include "sight.h"
#include "sight_pthread.h"
using namespace sight;
#include <stdio.h>
#include <stdlib.h>
#define NTHREADS	8
#define ARRAY_SIZE      500000

void *Hello(void *threadid)
{
   scope s("Hello");
   double A[ARRAY_SIZE];
   int i; 
   long tid;

   tid = (long)threadid;
   sleep(3);
   for (i=0; i<ARRAY_SIZE; i++)
   {
      A[i] = i * 1.0;
   }
   dbgprintf("%ld: Hello World!   %f\n", tid, A[ARRAY_SIZE-1]);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  pthread_t threads[NTHREADS];
  size_t stacksize;
  pthread_attr_t attr;
  int rc; 
  long t;

  SightInit(argc, argv, "Pthreads Bug2", "dbg.pthreadBug2.individual");

  pthread_attr_init(&attr);
  pthread_attr_getstacksize (&attr, &stacksize);
  dbgprintf("Thread stack size = %li bytes (hint, hint)\n",stacksize);
  for(t=0;t<NTHREADS;t++){
    rc = pthread_create(&threads[t], NULL, Hello, (void *)t);
    if (rc){
      dbgprintf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
      }
    }
  dbgprintf("Created %ld threads.\n", t);
  pthread_exit(NULL);
}
