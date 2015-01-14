
#include "sight.h"
#include "sight_pthread.h"
#include <map>
#include <vector>
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// for example 2
#define CHUNKSIZE   10
#define N       100
	
using namespace std;
using namespace sight;

int numThreads = 4;
pthread_mutex_t mutexsum;

void *subfun(void *t)
{
	long tid;
	tid = (long)t;
	dbg << "Thread "<<tid<<" starting..."<<endl;
	int x;
	x = 2;
	int y = 0;
	int rel = 0;
   	#pragma omp parallel num_threads(numThreads) shared(x)
	{
		y = 0;
		if (omp_get_thread_num() == 0) {
			x = 10;
			y = x;
		} else {		
			if(omp_get_thread_num() == tid)
			{
				//printf("Thread# %d: x = %d\n", omp_get_thread_num(),x );
				y = x + 3;
			}
		}
	}

	 /*
   Lock a mutex prior to updating the value in the shared
   structure, and unlock it upon updating.
   */
   pthread_mutex_lock (&mutexsum);
   {
		scope s("Updating result", scope::minimum);
		rel += y;
   }
   pthread_mutex_unlock (&mutexsum);
   
    dbg << "Thread "<<tid<<" done. result = "<< rel <<"\n";
    pthread_exit((void*) t);
}

int main (int argc, char *argv[])
{

	if(argc>=2) numThreads = atoi(argv[1]);

	std::vector<pthread_t> thread(numThreads);
	pthread_attr_t attr;
	int rc;
	long t;
	void *status;
	
	SightInit(argc, argv, "openMPex2", "dbg.openMPex2.individual");
	
	   /* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for(t=0; t<numThreads; t++) {
	  dbg << "Main: creating thread "<<t<<"\n";
	  //cout << pthread_self()<<"Main: creating thread "<<t<<", dbg="<<&dbg<<"\n";
	  pthread_t thr;
	  rc = pthread_create(&thread[t], &attr, subfun, (void *)t); 
	  if (rc) {
	     dbg << "ERROR; return code from pthread_create() is "<<rc<<"\n";
	     exit(-1);
	  }
	}

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

	return 0;
}

