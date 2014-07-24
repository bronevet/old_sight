#include <pthread.h>
#include <string.h>
#include "sight_structure.h"
#include "sight_common.h"
using namespace sight;

typedef struct {
  pthread_barrier_t bar;
  int count; // counts the number of times this barrier has been reached
} sight_pthread_barrier_t;

typedef struct {
  long numMutexOwners;
  pthread_t lastMutexOwner;
  pthread_mutex_t mutex;
} sight_pthread_mutex_t;

pthread_mutex_t causalityMutex = PTHREAD_MUTEX_INITIALIZER;
//ThreadLocalStorage0<scalarCausalClock> threadCausality;
std::map<pthread_t, scalarCausalClock*> causality;
//ThreadLocalStorage0<scalarCausalClock> threadCausality;

// Initializes the pthreads suv
/*void init_sight_pthreads() {

}*/

/***************************
 ***** Thread Creation *****
 ***************************/

typedef void *(pthreadRoutine)(void*);

// Encapsulates the arguments to sightThreadInitializer
typedef struct {
  pthreadRoutine* routine; // The routine that sight_pthread_create is asked to invoke in a new thread
  void* arg;              // The void* argument to this routine
} pthreadRoutineData;

// Function that wraps the execution of each thread spawned using pthread_create()
// and ensures that all appropriate initialization is performed
void *sightThreadInitializer(void* data) {
  pthreadRoutine* routine = ((pthreadRoutineData*)data)->routine;
  void* arg = ((pthreadRoutineData*)data)->arg;

  // Deallocate the data wrapper, since it is no longer needed
  free(data);

  // Initialize Sight for this thread before we initialize its clock
  SightInit_NewThread();

  // Force all threads to generate separate logs
  comparison comp(-1);

  // Initialize the new thread's causality clock, while under control of causalityMutex
  int rc = pthread_mutex_lock(&causalityMutex);
  if(rc!=0) { fprintf(stderr, "sightThreadInitializer() ERROR locking causalityMutex! %s\n", strerror(rc)); assert(0); }
  
  causality[pthread_self()] = new scalarCausalClock();
  //cout << "causality["<<pthread_self()<<"]="<<causality[pthread_self()]->send()<<endl;

  rc = pthread_mutex_unlock(&causalityMutex);
  if(rc!=0) { fprintf(stderr, "sightThreadInitializer() ERROR unlocking causalityMutex! %s\n", strerror(rc)); assert(0); }

  // Run the function itself
  return routine(arg);
}

int sight_pthread_create(pthread_t * thread,
                         const pthread_attr_t * attr,
                         void *(*start_routine)(void*), void * arg) {
  // Allocate a wrapper for the arguments to sightThreadInitializer()
  pthreadRoutineData* data = (pthreadRoutineData*)malloc(sizeof(pthreadRoutineData));
  data->routine = start_routine;
  data->arg = arg;
  //cout << "sight_pthread_create"<<endl;

  return pthread_create(thread, attr, sightThreadInitializer, data);
}

/*******************
 ***** Barrier *****
 *******************/

int sight_pthread_barrier_init(sight_pthread_barrier_t* sbar, const pthread_barrierattr_t * attr, unsigned count) {
  sbar->count = 0;
  //cout << pthread_self()<<": sight_pthread_barrier_init("<<sbar<<")"<<endl;
  return pthread_barrier_init(&(sbar->bar), attr, count);
}

int sight_pthread_barrier_destroy(sight_pthread_barrier_t* sbar) {
  //cout << pthread_self()<<": sight_pthread_barrier_destroy("<<sbar<<")"<<endl;
  return pthread_barrier_destroy(&(sbar->bar));
}

int sight_pthread_barrier_wait(sight_pthread_barrier_t* sbar) {
  //cout << pthread_self()<<": sight_pthread_barrier_wait("<<sbar<<")"<<endl;

  int rc = pthread_barrier_wait(&(sbar->bar));
  if(rc!=0 && rc!=PTHREAD_BARRIER_SERIAL_THREAD) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_barrier_wait() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  
  // Increment the number of times this barrier has been reached, but do
  // so only on one thread
  if(rc==PTHREAD_BARRIER_SERIAL_THREAD)
    sbar->count++;

  // Set the local scalar clock to the maximum of each thread's scalar clock
  rc = pthread_mutex_lock(&causalityMutex);
  if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }

  long long maxTime=-1;
  for(map<pthread_t, scalarCausalClock*>::const_iterator i=causality.begin(); i!=causality.end(); i++) {
    if(i->first != pthread_self()) 
      maxTime = (i->second->send()>maxTime? i->second->send(): maxTime);
  }

  //cout << pthread_self() << ": maxTime="<<maxTime<<endl;
  causality[pthread_self()]->recv(maxTime);

  commBar(txt()<<"B_"<<sbar<<"_"<<sbar->count);

  rc = pthread_mutex_unlock(&causalityMutex);
  if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_unlock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }

  rc = pthread_barrier_wait(&(sbar->bar));
  if(rc!=0 && rc!=PTHREAD_BARRIER_SERIAL_THREAD) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_barrier_wait() "<<rc<<"="<<strerror(rc)<<endl; return rc; }

  return 0;
}

/*****************
 ***** Mutex *****
 *****************/

int sight_pthread_mutex_init(sight_pthread_mutex_t *smutex, const pthread_mutexattr_t *attr) {
  smutex->numMutexOwners=0;
  return pthread_mutex_init(&(smutex->mutex), NULL);
}

int sight_pthread_mutex_destroy(sight_pthread_mutex_t *smutex) {
  return pthread_mutex_destroy(&(smutex->mutex));
}

int sight_pthread_mutex_lock(sight_pthread_mutex_t* smutex) {
  // Wait to grab the mutex from another thread
  int rc = pthread_mutex_lock(&(smutex->mutex));
  if(rc!=0) return rc;  

  // If this lock was previously owned by this or another thread, record the happens-before relationship
  if(smutex->numMutexOwners>0) {
    // Update the causality info
    rc = pthread_mutex_lock(&causalityMutex);
    if(rc!=0) return rc;

    //cout << pthread_self()<<": lock time="<<causality[smutex->lastMutexOwner]->send()<<endl;
    long long lastClockTime = causality[smutex->lastMutexOwner]->send();
    causality[pthread_self()]->recv(causality[smutex->lastMutexOwner]->send());
    //cout << pthread_self()<<": local time="<<causality[pthread_self()]->send()<<endl;
    commRecv(txt()<<"S_"<<smutex->lastMutexOwner<<"_"<<lastClockTime,
             txt()<<"R_"<<pthread_self()<<"_"<<lastClockTime);
    
    rc = pthread_mutex_unlock(&causalityMutex);
    if(rc!=0) return rc;
  }

  // Record that this thread was the last owner of this lock
  smutex->numMutexOwners++;
  smutex->lastMutexOwner = pthread_self();

  return 0;
}

int sight_pthread_mutex_unlock(sight_pthread_mutex_t* smutex) {
  // Emit the causality info of this lock acquire, while holding on to causalityMutex
  int rc = pthread_mutex_lock(&causalityMutex); 
  if(rc!=0) return rc;
  
  commSend(txt()<<"S_"<<pthread_self()<<"_"<<causality[pthread_self()]->send(), "");
  rc = pthread_mutex_unlock(&(smutex->mutex));
  if(rc!=0) return rc;

  rc = pthread_mutex_unlock(&causalityMutex);
  if(rc!=0) return rc;

  return 0;
} 

#define pthread_barrier_t       sight_pthread_barrier_t
#define pthread_barrier_init    sight_pthread_barrier_init
#define pthread_barrier_destroy sight_pthread_barrier_destroy
#define pthread_barrier_wait    sight_pthread_barrier_wait

#define pthread_mutex_t         sight_pthread_mutex_t
#define pthread_mutex_init      sight_pthread_mutex_init
#define pthread_mutex_destroy   sight_pthread_mutex_destroy
#define pthread_mutex_lock      sight_pthread_mutex_lock
#define pthread_mutex_unlock    sight_pthread_mutex_unlock

#define pthread_create          sight_pthread_create
