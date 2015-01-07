
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

	int nthreads = 5;
	//if(argc>=2) nthreads = atoi(argv[1]);

	SightInit(argc, argv, "OpenMPex1", "dbg.OpenMPex1.individual");

	// It is possible to write arbitrary text to the debug output
	//dbg << "<h1>Example: OpenMP</h1>" << endl;
  
	// example 2 - Loop Work-sharing
	int tid, i, chunk;
	float a[N], b[N], c[N];


	// Some initializations
	for (i=0; i < N; i++)
	a[i] = b[i] = i * 1.0;
	chunk = CHUNKSIZE;
	flowgraph g;
	
	std::vector<pthread_t> thread(omp_get_num_threads());
	pthread_attr_t attr;
	int rc;
	long t;
	void *status;
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	#pragma omp parallel shared(a,b,c,nthreads,chunk, g) private(i,tid)
	{
		//flowgraph g;
		tid = omp_get_thread_num();
		if (tid == 0)
		{
			nthreads = omp_get_num_threads();
			dbg << "Number of threads = " << nthreads << endl;
		}
		//dbg << "Thread "<< tid << " starting..." << endl;
		pthread_t thr;
		rc = pthread_create(&thread[tid], &attr, BusyWork, (void *)tid); 
		if (rc) {
		 dbg << "ERROR; return code from pthread_create() is "<<rc<<"\n";
		 exit(-1);
		}

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
	
	pthread_attr_destroy(&attr);
	for(t=0; t<nthreads; t++) {
	  rc = pthread_join(thread[t], &status);
	  if (rc) {
	     dbg << "ERROR; return code from pthread_join() is "<<rc<<"\n";
	     exit(-1);
	  }
	  dbg << "Main: completed join with thread "<<t<<" having a status of "<<long(status)<<endl;
	}
	
	//dbg << "Main: program completed. Exiting.\n";
	pthread_exit(NULL);

	//return 0;
}

