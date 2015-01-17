#include "sight.h"
#include "sight_pthread.h"
#include <map>
#include <vector>
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
// for example 2
#define CHUNKSIZE   10
#define N       100

using namespace std;
using namespace sight;

int numThreads = 4;

void *subfun(void *t)
{
	long tid;
	tid = (long)t;
	dbg << "Thread "<<tid<<" starting..."<<endl;
	int x;
	x = 2;
	int y = 0;
   	#pragma omp parallel num_threads(numThreads) shared(x)
	{
		y = 0;
		if (omp_get_thread_num() == 0) {
			x = 1;
			y = x;
		} else {		
			if(omp_get_thread_num() == tid)
			{
				//dbg << "Thread# " << omp_get_thread_num() << ": x =" << x  << endl;
				//printf("Thread# %d: x = %d\n", omp_get_thread_num(),x );
				y = x + tid;
			}
		}
	}
    dbg << "Thread "<<tid<<" done. y = "<< y <<"\n";
    pthread_exit((void*) t);
}

int main(int argc, char** argv)
{

	SightInit(argc, argv, "13.OpenMP1", "dbg.13.OpenMP1");

	// It is possible to write arbitrary text to the debug output
	dbg << "<h1>Example 13: OpenMP1</h1>" << endl;
	
	// example 0_1
	if(argc>=2) numThreads = atoi(argv[1]);

    std::vector<pthread_t> thread(numThreads);
	pthread_attr_t attr;
	int rc;
	long t;
	void *status;

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
	

	// example 0_2
	/*
	omp_set_nested(1);
	omp_set_max_active_levels(8);
	omp_set_dynamic(0);
	omp_set_num_threads(2);
	#pragma omp parallel
	{
		omp_set_num_threads(3);
		#pragma omp parallel
		{
			omp_set_num_threads(4);
			#pragma omp single
			{
				//
				// The following should print:
				// Inner: max_act_lev=8, num_thds=3, max_thds=4
				// Inner: max_act_lev=8, num_thds=3, max_thds=4
				//
				
				printf ("Inner: max_act_lev=%d, num_thds=%d, max_thds=%d\n",
				omp_get_max_active_levels(), omp_get_num_threads(),
				omp_get_max_threads());
				//dbg << "Inner: max_act_lev=" << omp_get_max_active_levels() << ", num_thds=" << omp_get_num_threads() <<", max_thds=" << omp_get_max_threads() << endl;
			}
		}
		#pragma omp barrier
		#pragma omp single
		{
			//
			// The following should print:
			// Outer: max_act_lev=8, num_thds=2, max_thds=3
			//
			printf ("Outer: max_act_lev=%d, num_thds=%d, max_thds=%d\n",
			omp_get_max_active_levels(), omp_get_num_threads(),
			omp_get_max_threads());

			//dbg << "Inner: max_act_lev=" << omp_get_max_active_levels() << ", num_thds=" << omp_get_num_threads() <<", max_thds=" << omp_get_max_threads() << endl;
		}
	}
	*/

    /*
	// example 1 - Hello World  
	//flowgraph g;
	int nthreads , tid;
	// Fork a team of threads giving them their own copies of variables 
	//#pragma omp parallel private(nthreads, tid, g)
	#pragma omp parallel private(nthreads, tid)	
	{
		//flowgraph g;
                		
		// Obtain thread number 
		tid = omp_get_thread_num();
		
		dbg << "Hello World from thread = " << tid << endl;
		
		stringstream ss;
		ss << tid;
		string threid= ss.str();
		//g.graphNodeStart(threid);
		//g.graphNodeEnd(threid);

		// Only master thread does this 
		if (tid == 0) 
		{
			nthreads = omp_get_num_threads();
			dbg << "Number of threads = " << nthreads << endl;
		}

	}  // All threads join master thread and disband 
	*/


	/*
	// example 2 - Loop Work-sharing
	int nthreads, tid, i, chunk;
	float a[N], b[N], c[N];
	
	nthreads = 4;
	// Some initializations
	for (i=0; i < N; i++)
	a[i] = b[i] = i * 1.0;
	chunk = CHUNKSIZE;
	flowgraph g;
	
	#pragma omp parallel shared(a,b,c,nthreads,chunk, g) private(i,tid)
	//#pragma omp parallel shared(a,b,c,nthreads,chunk) private(i,tid)
	{
		//flowgraph g;
		tid = omp_get_thread_num();
		if (tid == 0)
		{
			nthreads = omp_get_num_threads();
			dbg << "Number of threads = " << nthreads << endl;
		}
		dbg << "Thread "<< tid << " starting..." << endl;

		#pragma omp for schedule(dynamic,chunk)
		for (i=0; i<N; i++)
		{
			c[i] = a[i] + b[i];
			dbg << "Thread " << tid <<": c["<< i << "] = " << c[i] << endl;
			stringstream ss;
            ss << c[i];
            string thr= ss.str();

	        g.graphNodeStart(thr);
            g.graphNodeEnd(thr);
		}
	}  // end of parallel section
	*/

 	// example 3 - reduction
 	/* 
	flowgraph g; 
	int   i, n;
	float a[100], b[100], sum; 

	// Some initializations
	n = 100;
	for (i=0; i < n; i++)
		a[i] = b[i] = i * 1.0;
	sum = 0.0;

	#pragma omp parallel for reduction(+:sum)
	for (i=0; i < n; i++)
		sum = sum + (a[i] * b[i]);

	dbg << "   Sum = "<< sum << endl;
        stringstream ss;
        ss << sum;
        string thsum= ss.str();
        g.graphNodeStart(thsum);
        g.graphNodeEnd(thsum);
	*/
	
	/*
	int x;
	x = 2;
	#pragma omp parallel num_threads(2) shared(x)
	{
		if (omp_get_thread_num() == 0) {
			x = 5;
		} else {		
			// Print 1: the following read of x has a race
			printf("1: Thread# %d: x = %d\n", omp_get_thread_num(),x );
		}
		#pragma omp barrier
		if (omp_get_thread_num() == 0) {
			// Print 2 
			printf("2: Thread# %d: x = %d\n", omp_get_thread_num(),x );
		} else {
			// Print 3 
			printf("3: Thread# %d: x = %d\n", omp_get_thread_num(),x );
		}
	}
	*/
	
	return 0;
}
