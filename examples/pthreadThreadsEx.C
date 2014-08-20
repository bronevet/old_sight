//#include <pthread.h>
#include "sight.h"
#include "sight_pthread.h"
using namespace sight;
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
using namespace std;

void *BusyWork(void *t)
{
   int i;
   long tid;
   double result=0.0;
   tid = (long)t;
   dbg << "Thread "<<tid<<" starting..."<<endl;
   //cout << pthread_self() << ": Thread "<<tid<<" starting..., dbg="<<(&dbg)<<"\n";
   for (i=0; i<1000000; i++)
   {
      result = result + sin(i) * tan(i);
   }
   dbg << "Thread "<<tid<<" done. Result = "<<result<<"\n";
   //cout << pthread_self() << ": Thread "<<tid<<" done. Result = "<<result<<", dbg="<<(&dbg)<<"\n";
   pthread_exit((void*) t);
}

int main (int argc, char *argv[])
{
   int numThreads = 4;
   if(argc>=2) numThreads = atoi(argv[1]);
   
   std::vector<pthread_t> thread(numThreads);
   pthread_attr_t attr;
   int rc;
   long t;
   void *status;

   SightInit(argc, argv, "Pthread Threads Ex", "dbg.pthreadThreadsEx.individual");

   /* Initialize and set thread detached attribute */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   for(t=0; t<numThreads; t++) {
      dbg << "Main: creating thread "<<t<<"\n";
      //cout << pthread_self()<<"Main: creating thread "<<t<<", dbg="<<&dbg<<"\n";
      pthread_t thr;
      rc = pthread_create(&thread[t], &attr, BusyWork, (void *)t); 
      if (rc) {
         dbg << "ERROR; return code from pthread_create() is "<<rc<<"\n";
         exit(-1);
      }
   }
   
   /* Free attribute and wait for the other threads */
   pthread_attr_destroy(&attr);
   for(t=0; t<numThreads; t++) {
      rc = pthread_join(thread[t], &status);
      if (rc) {
         dbg << "ERROR; return code from pthread_join() is "<<rc<<"\n";
         exit(-1);
      }
      dbg << "Main: completed join with thread "<<t<<" having a status of "<<long(status)<<endl;
   }
 
   dbg << "Main: program completed. Exiting.\n";
   pthread_exit(NULL);
}
