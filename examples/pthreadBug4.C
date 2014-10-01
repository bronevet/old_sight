/******************************************************************************
* FILE: bug4.c
* DESCRIPTION:
*   This program demonstrates a condition variable race/synchronization 
*   problem. It resembles the condvar.c program. One possible solution can
*   be found in bug4fix.c
* SOURCE: 07/06/05 Blaise Barney
* LAST REVISED: 01/29/09  Blaise Barney
******************************************************************************/
//#include <pthread.h>
#include "sight.h"
#include "sight_pthread.h"
using namespace sight;
#include <stdio.h>
#include <stdlib.h>

/* Define and scope what needs to be seen by everyone */
#define NUM_THREADS  3
#define ITERATIONS 10
#define THRESHOLD 12
int sharedCount = 0;
double finalresult=0.0;
pthread_mutex_t count_mutex;
pthread_cond_t count_condvar;


void *sub1(void *t)
{
  scope("sub1");
  int i; 
  long tid = (long)t;
  double myresult=0.0;
 
  /* do some work */
  sleep(1);
  /*
  Lock mutex and wait for signal only if sharedCount is what is expected.  Note
  that the pthread_cond_wait routine will automatically and atomically
  unlock mutex while it waits. Also, note that if THRESHOLD is reached
  before this routine is run by the waiting thread, the loop will be skipped
  to prevent pthread_cond_wait from never returning, and that this thread's
  work is now done within the mutex lock of sharedCount.
  */
  pthread_mutex_lock(&count_mutex);
  dbgprintf("going into wait. sharedCount=%d\n",sharedCount);
  pthread_cond_wait(&count_condvar, &count_mutex);
  dbgprintf("Condition variable signal received.");
  dbgprintf("sharedCount=%d\n",sharedCount);
  sharedCount++;
  finalresult += myresult;
  dbgprintf("sharedCount now equals=%d myresult=%e. Done.\n",
         sharedCount,myresult);
  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
}

void *sub2(void *t) 
{
  int j,i;
  long tid = (long)t;
  double myresult=0.0;
  scope("sub2");

  for (i=0; i<ITERATIONS; i++) {
    scope(txt()<<"Loop i="<<i);
    for (j=0; j<100000; j++)
      myresult += sin(j) * tan(i);
    pthread_mutex_lock(&count_mutex);
    finalresult += myresult;
    sharedCount++;
    /* 
    Check the value of sharedCount and signal waiting thread when condition is
    reached.  Note that this occurs while mutex is locked. 
    */
    if (sharedCount == THRESHOLD) {
      dbgprintf("Threshold reached. sharedCount=%d. ",sharedCount);
      pthread_cond_signal(&count_condvar);
      dbgprintf("Just sent signal.\n");
    }
    else {
      dbgprintf("did work. sharedCount=%d\n",sharedCount);
    }
    pthread_mutex_unlock(&count_mutex);
  }
  dbgprintf("myresult=%e. Done. \n",myresult);
  pthread_exit(NULL);
}



int main(int argc, char *argv[])
{
  long t1=1, t2=2, t3=3;
  int i, rc;
  pthread_t threads[3];
  pthread_attr_t attr;

  SightInit(argc, argv, "Pthreads Bug4", "dbg.pthreadBug4.individual");

  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&count_mutex, NULL);
  pthread_cond_init (&count_condvar, NULL);

  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&threads[0], &attr, sub1, (void *)t1);
  pthread_create(&threads[1], &attr, sub2, (void *)t2);
  pthread_create(&threads[2], &attr, sub2, (void *)t3);

  /* Wait for all threads to complete */
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  dbgprintf ("Waited on %d threads. Final result=%e. Done.\n",
          NUM_THREADS,finalresult);

  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_condvar);
  pthread_exit (NULL);

}
