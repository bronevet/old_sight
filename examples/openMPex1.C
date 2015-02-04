
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
static omp_lock_t omplock;

void subfun(){
	
	dbg << "Thread "<<omp_get_thread_num()<<" starting..."<<endl;
	if (omp_get_thread_num() == 0) {
		x = 5;
		printf("Master Thread# %d: x = %d\n", omp_get_thread_num(),x );
		} else {		
		//x = 2;
		// Print 1: the following read of x has a race
		printf("Non-master Thread# %d: x = %d\n", omp_get_thread_num(),x );
	}
	dbg << "Thread# "<< omp_get_thread_num() <<" done. x = "<< x <<"\n";	
}

int main (int argc, char *argv[])
{
	omp_init_lock(&omplock);
	SightInit(argc, argv, "openMPex1", "dbg.openMPex1.individual");

	if(argc>=2) numThreads = atoi(argv[1]);
	
		comparison c("-1");
		{
		    sight_ompthread_create();
			#pragma omp parallel num_threads(numThreads) shared(x)
			{
				if(omp_get_thread_num() != 0)
				  	sightOMPThreadInitializer();
			
				subfun();	
			    
			    if(omp_get_thread_num() != 0)
			     	ompthreadCleanup(NULL);

			     dbg << "Thread# "<< omp_get_thread_num() <<" finished. \n";
		  	}
		  	for(int t=1; t<numThreads; ++t){
		  		sight_ompthread_join(t); 
		    dbg << "Main: completed join with thread "<<t<<endl;
			}
		}
    
	dbg << "Main: program completed. Exiting.\n";
	return 0;
}

