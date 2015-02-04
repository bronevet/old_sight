
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

int main (int argc, char *argv[])
{
	int numThreads = 4;
	if(argc>=2) numThreads = atoi(argv[1]);

	SightInit(argc, argv, "openMPex3", "dbg.openMPex3.individual");
	
	int j, k, jlast, klast;

	sight_ompthread_create();
	#pragma omp parallel num_threads(numThreads)
	{
		if(omp_get_thread_num() != 0)
		  	sightOMPThreadInitializer();

		#pragma omp for collapse(2) lastprivate(jlast, klast)
		for (k=1; k<=2; k++)
		for (j=1; j<=3; j++)
		{
			jlast=j;
			klast=k;
		}
		
		#pragma omp single
		{
			printf("%d %d\n", klast, jlast);
			dbg << "Thread "<< omp_get_thread_num() << " klast = " << klast << " jlast =" << jlast << endl;
		}

	    dbg << "Thread "<< omp_get_thread_num() << " done." << endl;
	    
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

