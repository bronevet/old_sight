//#include <pthread.h>
#include "sight.h"
#include "sight_pthread.h"
using namespace sight;
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TCOUNT 8
#define COUNT_LIMIT 12
#define NUM_THREADS 3

int     counter = 0;
int     thread_ids[3] = {0,1,2};
pthread_mutex_t counter_mutex;
pthread_cond_t counter_threshold_cv;

void *inc_counter(void *t) 
{
   int i;
   long my_id = (long)t;
   dbg << "Starting inc_counter(): thread "<<my_id<<"\n";
 
   for (i=0; i<TCOUNT; i++) {
      pthread_mutex_lock(&counter_mutex);
      {scope s("Incrementing dotstr.sum", scope::minimum);
      counter++;
  
      /* 
      Check the value of counter and signal waiting thread when condition is
      reached.  Note that this occurs while mutex is locked. 
      */
      if (counter == COUNT_LIMIT) {
         pthread_cond_signal(&counter_threshold_cv);
         dbg << "inc_counter(): thread "<<my_id<<", counter = "<<counter<<". Threshold reached.\n";
      }
      dbg << "inc_counter(): thread "<<my_id<<", counter = "<<counter<<", unlocking mutex\n";
      }
      pthread_mutex_unlock(&counter_mutex);
  
      /* Do some "work" so threads can alternate on mutex lock */
      //sleep(1);
      pthread_yield();
      nanosleep((struct timespec[]){{0, 100000000}}, NULL);
   }
   pthread_exit(NULL);
}

void *watch_counter(void *t) 
{
  long my_id = (long)t;

  dbg << "Starting watch_counter(): thread "<<my_id<<"\n";
  //cout << pthread_self()<<": Starting watch_counter(): thread "<<my_id<<" dbg="<<&dbg<<"\n";

  /*
  Lock mutex and wait for signal.  Note that the pthread_cond_wait 
  routine will automatically and atomically unlock mutex while it waits. 
  Also, note that if COUNT_LIMIT is reached before this routine is run by
  the waiting thread, the loop will be skipped to prevent pthread_cond_wait
  from never returning. 
  */
  pthread_mutex_lock(&counter_mutex);
  int local_counter = counter;
  { scope s("Counter check loop"); 
  dbg << "initial counter="<<local_counter<<endl;
  while (local_counter<COUNT_LIMIT) {
    pthread_cond_wait(&counter_threshold_cv, &counter_mutex);
    {scope s("Updating counter");
    dbg << "watch_counter(): thread "<<my_id<<" Condition signal received.\n";
    counter += 125;
    dbg << "watch_counter(): thread "<<my_id<<" counter now = "<<counter<<"\n";
    }
    local_counter = counter;
  }
  dbg << "final counter="<<local_counter<<endl;
  }
  pthread_mutex_unlock(&counter_mutex);
  pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
  int i, rc;
  long t1=1, t2=2, t3=3;
  std::vector<pthread_t> thread(NUM_THREADS);
  pthread_attr_t attr;
  
  SightInit(argc, argv, "Pthread Cond Ex", "dbg.pthreadCondEx.individual");

  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&counter_mutex, NULL);
  pthread_cond_init (&counter_threshold_cv, NULL);

  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  dbg << "Spawning thread 0"<<endl;
  pthread_create(&thread[0], &attr, watch_counter, (void *)t1);
  dbg << "Spawning thread 1"<<endl;
  pthread_create(&thread[1], &attr, inc_counter, (void *)t2);
  dbg << "Spawning thread 3"<<endl;
  pthread_create(&thread[2], &attr, inc_counter, (void *)t3);

  /* Wait for all threads to complete */
  for (i=0; i<NUM_THREADS; i++) {
     dbg << "Joining thread "<<i<<endl;
     pthread_join(thread[i], NULL);
  }
  printf ("Main(): Waited on %d  threads. Done.\n", NUM_THREADS);

  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&counter_mutex);
  pthread_cond_destroy(&counter_threshold_cv);
  pthread_exit(NULL);

}
