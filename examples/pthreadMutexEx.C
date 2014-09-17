//#include <pthread.h>
#include "sight.h"
#include "sight_pthread.h"
using namespace sight;
#include <stdio.h>
#include <stdlib.h>

/*   
The following structure contains the necessary information  
to allow the function "dotprod" to access its input data and 
place its output into the structure.  
*/

typedef struct 
{
   double      *a;
   double      *b;
   double     sum; 
   int     veclen; 
} DOTDATA;

/* Define globally accessible variables and a mutex */

#define VECLEN 100
DOTDATA dotstr; 
pthread_mutex_t mutexsum;

/*
The function dotprod is activated when the thread is created.
All input to this routine is obtained from a structure 
of type DOTDATA and all output from this function is written into
this structure. The benefit of this approach is apparent for the 
multi-threaded program: when a thread is created we pass a single
argument to the activated function - typically this argument
is a thread number. All  the other information required by the 
function is accessed from the globally accessible structure. 
*/

void *dotprod(void *arg)
{

   /* Define and use local variables for convenience */

   int i, start, end, len ;
   long offset;
   double mysum, *x, *y;
   offset = (long)arg;
     
   len = dotstr.veclen;
   start = offset*len;
   end   = start + len;
   x = dotstr.a;
   y = dotstr.b;

   /*
   Perform the dot product and assign result
   to the appropriate variable in the structure. 
   */

   mysum = 0;
   for (i=start; i<end ; i++) 
   {
      mysum += (x[i] * y[i]);
   }
   dbg << "Sum of region ["<<start<<", "<<end<<"] is "<<mysum<<endl;

   /*
   Lock a mutex prior to updating the value in the shared
   structure, and unlock it upon updating.
   */
   pthread_mutex_lock (&mutexsum);
   {scope s("Updating dotstr.sum", scope::minimum);
   dotstr.sum += mysum;
   }
   pthread_mutex_unlock (&mutexsum);
   
   dbg << "dotprod complete"<<endl;

   pthread_exit((void*) 0);
}

/* 
The main program creates threads which do all the work and then 
print out result upon completion. Before creating the threads,
the input data is created. Since all threads update a shared structure, 
we need a mutex for mutual exclusion. The main thread needs to wait for
all threads to complete, it waits for each one of the threads. We specify
a thread attribute value that allow the main thread to join with the
threads it creates. Note also that we free up handles when they are
no longer needed.
*/

int main (int argc, char *argv[])
{
   int numThreads = 4;
   if(argc>=2) numThreads = atoi(argv[1]);
  
   long i;
   double *a, *b;
   void *status;
   pthread_attr_t attr;
   std::vector<pthread_t> thread(numThreads);
   
   SightInit(argc, argv, "Pthread Mutex Ex", "dbg.pthreadMutexEx.individual");

   /* Assign storage and initialize values */
   a = (double*) malloc (numThreads*VECLEN*sizeof(double));
   b = (double*) malloc (numThreads*VECLEN*sizeof(double));
  
   for (i=0; i<VECLEN*numThreads; i++)
   {
      a[i]=1.0;
      b[i]=a[i];
   }

   dotstr.veclen = VECLEN; 
   dotstr.a = a; 
   dotstr.b = b; 
   dotstr.sum=0;

   pthread_mutex_init(&mutexsum, NULL);
         
   /* Create threads to perform the dotproduct  */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   for(i=0; i<numThreads; i++)
   {
      dbg << "Spawning thread "<<i<<endl;
      /* 
      Each thread works on a different set of data.
      The offset is specified by 'i'. The size of
      the data for each thread is indicated by VECLEN.
      */
      pthread_create(&thread[i], &attr, dotprod, (void *)i);
   }

   pthread_attr_destroy(&attr);
   
   /* Wait on the other threads */
   for(i=0; i<numThreads; i++)
   {
      dbg << "Joining thread "<<i<<endl;
      pthread_join(thread[i], &status);
   }

   /* After joining, print out the results and cleanup */
   dbg << "Sum =  "<<dotstr.sum<<" \n";
   free (a);
   free (b);
   pthread_mutex_destroy(&mutexsum);
   pthread_exit(NULL);
}   
