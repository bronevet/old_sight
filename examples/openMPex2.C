
#include "sight.h"
#include "sight_ompthread.h"
#include <map>
#include <vector>
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CHUNKSIZE   10
#define N       100

using namespace std;
using namespace sight;

int numThreads = 4;
static sight_omp_barrier_t ompbarrier;

int main (int argc, char *argv[])
{ 
	SightInit(argc, argv, "openMPex2", "dbg.openMPex2.individual");

	if(argc>=2) numThreads = atoi(argv[1]);

	int tid, i, chunk;
	float a[N], b[N], c[N];
	
	// Some initializations
	for (i=0; i < N; i++)
	a[i] = b[i] = i * 1.0;
	chunk = CHUNKSIZE;

    sight_ompthread_create();
	#pragma omp parallel num_threads(numThreads) shared(a,b,c,numThreads,chunk) private(i,tid)
	{
		if(omp_get_thread_num() != 0)
		  	sightOMPThreadInitializer();
	
		if (omp_get_thread_num() == 0)
		{
			dbg << "Number of threads = " << numThreads << endl;
		}
		{
			scope s("Start:", scope::minimum);
			dbg << "Thread "<< omp_get_thread_num() << " starting..." << endl;
		}
			
		// #pragma omp barrier
	 //        if(omp_get_thread_num() !=0 )
	 //          sight_omp_barrier_wait(&ompbarrier);

		int forID = 1;
		#pragma omp for
		for (i=0; i<N; i++)
		{
			c[i] = a[i] + b[i];
			scopeOMP s("scope:", forID, N, i);
			dbg << "Thread " << omp_get_thread_num() <<": c["<< i << "] = " << c[i] << endl;	
			cout << "Thread " << omp_get_thread_num() <<": c["<< i << "] = " << c[i] << endl;												
		}
		{
			scope s("endFor", scope::minimum);
			dbg<< "End For" << endl;
		}
		// #pragma omp barrier
	 //        if(omp_get_thread_num() !=0 )
	 //          sight_omp_barrier_wait(&ompbarrier);
		{
			scope s("completed:", scope::minimum);
			dbg << "Thread "<< omp_get_thread_num() << " done." << endl;
		}

	    if(omp_get_thread_num() != 0)
	     	ompthreadCleanup(NULL);
  	}
  	for(int t=1; t<numThreads; ++t){
  		sight_ompthread_join(t); 
    	dbg << "Main: completed join with thread "<<t<<endl;
    }

	dbg << "Main: program completed. Exiting.\n";
	
	return 0;
}

