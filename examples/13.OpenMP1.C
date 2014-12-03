#include "sight.h"
#include <map>
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
// for example 2
#define CHUNKSIZE   10
#define N       100

using namespace std;
using namespace sight;

int main(int argc, char** argv)
{

	SightInit(argc, argv, "13.OpenMP1", "dbg.13.OpenMP1");

	// It is possible to write arbitrary text to the debug output
	dbg << "<h1>Example 13: OpenMP1</h1>" << endl;
  
	// example 1 - Hello World  
	/*
	flowgraph g;
	int nthreads , tid;
	// Fork a team of threads giving them their own copies of variables 
	//#pragma omp parallel private(nthreads, tid, g)
	#pragma omp parallel private(nthreads, tid, g)	
	{
		//flowgraph g;
                		
		// Obtain thread number 
		tid = omp_get_thread_num();
		
		dbg << "Hello World from thread = " << tid << endl;
		
		stringstream ss;
		ss << tid;
		string threid= ss.str();
		g.graphNodeStart(threid);
		g.graphNodeEnd(threid);

		// Only master thread does this 
		if (tid == 0) 
		{
			nthreads = omp_get_num_threads();
			dbg << "Number of threads = " << nthreads << endl;
		}

	}  // All threads join master thread and disband 
	*/

	// example 2 - Loop Work-sharing
	 
	int nthreads, tid, i, chunk;
	float a[N], b[N], c[N];

	// Some initializations
	for (i=0; i < N; i++)
	a[i] = b[i] = i * 1.0;
	chunk = CHUNKSIZE;
	flowgraph g;
	
	#pragma omp parallel shared(a,b,c,nthreads,chunk, g) private(i,tid)
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
	

	return 0;
}
