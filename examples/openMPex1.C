
#include "sight.h"
#include "sight_ompthread.h"
#include <map>
#include <vector>
#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
	
using namespace std;
using namespace sight;

int numThreads = 4;
int x = 2;

void subfun(){
	//long tid = (long)t;
	long tid;
	tid = omp_get_thread_num();

	dbg << "Thread "<<tid<<" starting..."<<endl;
	if (tid == 0) {
		x = 5;
		printf("Master Thread# %d: x = %d\n", tid,x );
		} else {		
		//x = 2;
		// Print 1: the following read of x has a race
		printf("Non-master Thread# %d: x = %d\n", tid,x );
	}
	
	dbg << "Thread# "<< tid <<" done. x = "<< x <<"\n";
}

int main (int argc, char *argv[])
{
	SightInit(argc, argv, "openMPex1", "dbg.openMPex1.individual");

	if(argc>=2) numThreads = atoi(argv[1]);

    sight_ompthread_create();
	#pragma omp parallel num_threads(numThreads) shared(x)
	{
		if(omp_get_thread_num() != 0)
		  	sightOMPThreadInitializer();
	
		subfun();	
	    
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

