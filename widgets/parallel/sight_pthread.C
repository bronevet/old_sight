#include <pthread.h>
#include <string.h>
#include "sight_structure.h"
#include "sight_common.h"
#include "thread_local_storage.h"
#define PTHREAD_C
#include "sight_pthread.h"
using namespace sight;

static pthread_mutex_t causalityMutex = PTHREAD_MUTEX_INITIALIZER;
static std::map<pthread_t, scalarCausalClock*> causality;

/**************************************************
 ***** Thread Initialization and Finalization *****
 **************************************************/

ThreadLocalStorage0<comparison*> PthreadThreadInitFinInstantiator::globalComparisons;

PthreadThreadInitFinInstantiator::PthreadThreadInitFinInstantiator() { 
  addFuncs("pthread", 
           PthreadThreadInitFinInstantiator::initialize, 
           PthreadThreadInitFinInstantiator::finalize,
           common::easyset<std::string>("mpi"), common::easyset<std::string>());
}

void PthreadThreadInitFinInstantiator::initialize() {
  cout << "PthreadThreadInitFinInstantiator::initialize()"<<endl;
  // Assign each thread to a separate log based on its thread ID
  globalComparisons = new comparison(txt()<<pthread_self());
}

void PthreadThreadInitFinInstantiator::finalize() {
  // Assign each thread to a separate log based on its thread ID
  assert(*globalComparisons != NULL);
  delete *globalComparisons;
}

PthreadThreadInitFinInstantiator PthreadThreadInitFinInstance;

/********************************
 ***** Causality Management *****
 ********************************/

// Updates the causality info from the sender side
// Returns the error code of the pthreads functions called.
// cmHeld - indicates whether the causalityMutex is already being held by the calling thread
// If clock!=NULL, it is set to the causal time immediately after the send operation
int sendCausality(const std::string& sendID, const std::string& recvID, 
                  const std::string& label,
                  bool cmHeld, long long* clock) {
 // Emit the causality info of this lock acquire, while holding on to causalityMutex
  if(!cmHeld) {
    int rc = pthread_mutex_lock(&causalityMutex); 
    if(rc!=0) return rc;
  }
  
  checkCausality(true);

  //scope s(label, scope::minimum);
  commSend(label, sendID, recvID);
  
  // If a pointer to a clock is provided, set it to the time of the send operation
  if(clock)
    *clock = causality[pthread_self()]->send();

  if(!cmHeld) {
    int rc = pthread_mutex_unlock(&causalityMutex);
    if(rc!=0) return rc;
  }
  
  return 0;
}

int sendCausality(const std::string& sendID, 
                  const std::string& label,
                  bool cmHeld) {
  return sendCausality(sendID, "", label, cmHeld, NULL);
}

// Updates the causality info on the receiver side
// Returns the error code of the pthreads functions called.
// caller - If provided, receiveCausality() reads the causal time directly from the caller.
// callerTime - If provieded, takes the causal time from this argument
// cmHeld - indicates whether the causalityMutex is already being held by the calling thread
int receiveCausality(const std::string& sendID, pthread_t sender,
                     const std::string& recvID, 
                     const std::string& label, bool cmHeld) {
  //dbg << pthread_self()<<": receiveCausality("<<sendID<<", sender="<<sender<<", "<<recvID<<", "<<label<<", "<<cmHeld<<")"<<endl;
  if(!cmHeld) {
    int rc = pthread_mutex_lock(&causalityMutex);
    if(rc!=0) return rc;
  }

  checkCausality(true);
  //scope s(label, scope::minimum);
 
  //dbg << pthread_self()<<": lock time="<<causality[sender]->send()<<endl;
  causality[pthread_self()]->recv(causality[sender]->send());
  //dbg << pthread_self()<<": local time="<<causality[pthread_self()]->send()<<endl;
  commRecv(label, sendID, recvID);

  if(!cmHeld) {
    int rc = pthread_mutex_unlock(&causalityMutex);
    if(rc!=0) return rc;
  }
  
  return 0;
}

int receiveCausality(const std::string& sendID, long long senderTime,
                     const std::string& recvID, 
                     const std::string& label, 
                     bool cmHeld) {
  //dbg << pthread_self()<<": receiveCausality("<<sendID<<", senderTime="<<senderTime<<", "<<recvID<<", "<<label<<", "<<cmHeld<<")"<<endl;
  if(!cmHeld) {
    int rc = pthread_mutex_lock(&causalityMutex);
    if(rc!=0) return rc;
  }

  checkCausality(true);
  //scope s(label, scope::minimum);
 
  ////cout << pthread_self()<<": lock time="<<causality[smutex->lastMutexOwner]->send()<<endl;
  causality[pthread_self()]->recv(senderTime);
  //cout << pthread_self()<<": local time="<<causality[pthread_self()]->send()<<endl;
  commRecv(label, sendID, recvID);

  if(!cmHeld) {
    int rc = pthread_mutex_unlock(&causalityMutex);
    if(rc!=0) return rc;
  }
  
  return 0;
}

// Makes sure that causality[] is initialized.
// cmHeld - indicates whether the causalityMutex is already being held by the calling thread
int checkCausality(bool cmHeld) {
  int rc;

  if(!cmHeld) {
    rc = pthread_mutex_lock(&causalityMutex);
    if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  }

  if(causality.find(pthread_self()) == causality.end())
    causality[pthread_self()] = new scalarCausalClock();

  if(!cmHeld) {
    rc = pthread_mutex_unlock(&causalityMutex);
    if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  }

  return 0;
}


/***************************
 ***** Thread Creation *****
 ***************************/

typedef void *(pthreadRoutine)(void*);

// Encapsulates the arguments to sightThreadInitializer
typedef struct {
  pthreadRoutine* routine; // The routine that sight_pthread_create is asked to invoke in a new thread
  pthread_t spawner;       // ID of the thread that spawned this one
  int   spawnCnt;          // The number of threads this thread's spawner created before it created this thread
  void* arg;               // The void* argument to this routine
} pthreadRoutineData;

// If the application calls pthread_exit deep inside the thread's calling stack 
// we need to unwrap the stack to get back to the call to sightThreadInitializer()
// to ensure all of Sight's state is correctly cleaned up. We do this by calling
// setjmp() to set a return point inside sightThreadInitializer() and than calling
// longjmp() to return to it from inside the call to pthread_exit();
//ThreadLocalStorage1<void*, void*> retVal(NULL);

void threadCleanup(void * arg) {
//  cout << pthread_self()<<": threadCleanup() <<<<"<<endl;
  int rc = sendCausality(txt()<<"End_"<<pthread_self(), "Terminating");
  //AbortHandlerInstantiator::finalizeSight();
  SightThreadFinalize();
//  cout << pthread_self()<<": threadCleanup() >>>>"<<endl;
}

// Function that wraps the execution of each thread spawned using pthread_create()
// and ensures that all appropriate initialization is performed
void *sightThreadInitializer(void* data) {
  pthreadRoutine* routine = ((pthreadRoutineData*)data)->routine;
  void* arg = ((pthreadRoutineData*)data)->arg;
  
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  // Assign each thread to a separate log based on its thread ID
  //globalComparisonIDs.push_back(make_pair("pthread", std::string(txt()<<pthread_self())));
  
  // Initialize Sight for this thread before we initialize its clock
  SightInit_NewThread();
 
  void* ret;

  pthread_cleanup_push(threadCleanup, NULL);
  
  {
    //modularApp ma("", namedMeasures("time",      new timeMeasure()));
    //comparison c(txt()<<pthread_self());

    //dbg << "Starting sightThreadInitializer("<<arg<<") dbg="<<&dbg<<endl;

    // Initialize the new thread's causality clock, while under control of causalityMutex
    int rc = pthread_mutex_lock(&causalityMutex);
    if(rc!=0) { fprintf(stderr, "sightThreadInitializer() ERROR locking causalityMutex! %s\n", strerror(rc)); assert(0); }

    //checkCausality(true);
    //cout << "causality["<<pthread_self()<<"]="<<causality[pthread_self()]->send()<<endl;

    // Add a causality send edge from the spawner thread to the spawnee thread
    /*commRecv(txt()<<"Spawner_"<<((pthreadRoutineData*)data)->spawner<<"_"<<((pthreadRoutineData*)data)->spawnCnt,
             txt()<<"Spawnee_"<<pthread_self());*/

    rc = receiveCausality(txt()<<"Spawner_"<<((pthreadRoutineData*)data)->spawner<<"_"<<((pthreadRoutineData*)data)->spawnCnt,
                          ((pthreadRoutineData*)data)->spawner,
                          txt()<<"Spawnee_"<<pthread_self(),
                          "Spawned", true);

    rc = pthread_mutex_unlock(&causalityMutex);
    if(rc!=0) { fprintf(stderr, "sightThreadInitializer() ERROR unlocking causalityMutex! %s\n", strerror(rc)); assert(0); }

    // Deallocate the data wrapper, since it is no longer needed
    free(data);  

    if(rc!=0) { fprintf(stderr, "sightThreadInitializer() ERROR unlocking causalityMutex! %s\n", strerror(rc)); assert(0); }

    //dbg << "Calling routine("<<arg<<") dbg="<<&dbg<<endl;
    // Run the function itself
    ret = routine(arg);
    //dbg << "Finished routine("<<arg<<") dbg="<<&dbg<<endl;

    // Add a causality send edge from the thread's termination to the join call
    //commSend(txt()<<"End_"<<pthread_self(), "");
    //rc = sendCausality(txt()<<"End_"<<pthread_self(), "Terminating");

    threadCleanup(NULL);
  }

  /*cout << pthread_self()<<": Calling SightThreadFinalize()\n"; cout.flush();
  SightThreadFinalize();
  cout << pthread_self()<<": End of thread\n"; cout.flush();*/

  pthread_cleanup_pop(0);
 
  return ret;
}

// Counts the number of times each thread has spawned another thread
static ThreadLocalStorage1<int, int> numThreadsSpawned(0);

int sight_pthread_create(pthread_t * thread,
                         const pthread_attr_t * attr,
                         void *(*start_routine)(void*), void * arg) {
  // Allocate a wrapper for the arguments to sightThreadInitializer()
  pthreadRoutineData* data = (pthreadRoutineData*)malloc(sizeof(pthreadRoutineData));
  data->routine = start_routine;
  data->spawner = pthread_self();
  data->spawnCnt = numThreadsSpawned;
  data->arg = arg;
  //cout << "sight_pthread_create("<<data->arg<<", arg="<<arg<<")"<<endl;
  
  // Create a scalarCausalClock for the spawning thread if it does not yet have one
  {
    int rc = pthread_mutex_lock(&causalityMutex);
    if(rc!=0) { fprintf(stderr, "sightThreadInitializer() ERROR locking causalityMutex! %s\n", strerror(rc)); assert(0); }

    checkCausality(true);

    rc = pthread_mutex_unlock(&causalityMutex);
    if(rc!=0) { fprintf(stderr, "sightThreadInitializer() ERROR unlocking causalityMutex! %s\n", strerror(rc)); assert(0); }
  }
  
  // Add a causality send edge from the spawner thread to the spawnee thread
  //commSend(txt()<<"Spawner_"<<pthread_self()<<"_"<<data->spawnCnt, "");
  int rc = sendCausality(txt()<<"Spawner_"<<pthread_self()<<"_"<<data->spawnCnt, "Spawn");
  if(rc!=0) return rc;
  
  numThreadsSpawned++;
  
  return pthread_create(thread, attr, sightThreadInitializer, data);
}

void sight_pthread_exit(void *value_ptr) {
  //cout << pthread_self()<<": sight_pthread_exit()"<<endl;
  // Add a causality send edge from the thread's termination to the join call
  //commSend(txt()<<"End_"<<pthread_self(), "");
  int rc = sendCausality(txt()<<"End_"<<pthread_self(), "Terminating");
  
  //AbortHandlerInstantiator::finalizeSight();

  pthread_exit(value_ptr);
}

int sight_pthread_join(pthread_t thread, void **value_ptr) {
  int rc = pthread_join(thread, value_ptr);

  // Add a causality send edge from the thread's termination to the join call
  //commRecv(txt()<<"End_"<<thread, txt()<<"Joiner_"<<pthread_self()<<"_"<<thread);
  rc = receiveCausality(txt()<<"End_"<<thread, thread,
                        txt()<<"Joiner_"<<pthread_self()<<"_"<<thread,
                        "Join");
  
  return rc;
}

/*******************
 ***** Barrier *****
 *******************/

int sight_pthread_barrier_init(sight_pthread_barrier_t* sbar, const pthread_barrierattr_t * attr, unsigned count) {
  sbar->count = 0;

  checkCausality(true);
   
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
  if(rc==PTHREAD_BARRIER_SERIAL_THREAD) {
    sbar->count++;

    // Set the local scalar clock to the maximum of each thread's scalar clock
    rc = pthread_mutex_lock(&causalityMutex);
    if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  
    sbar->maxTime=-1;
    for(map<pthread_t, scalarCausalClock*>::const_iterator i=causality.begin(); i!=causality.end(); i++) {
      if(i->first != pthread_self()) 
        sbar->maxTime = (i->second->send()>sbar->maxTime? i->second->send(): sbar->maxTime);
    }
  }

  // All threads wait for the serial thread to compute the maximum of the local causal times
  rc = pthread_barrier_wait(&(sbar->bar));
  if(rc!=0 && rc!=PTHREAD_BARRIER_SERIAL_THREAD) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_barrier_wait() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  
  //cout << pthread_self() << ": maxTime="<<maxTime<<endl;
  causality[pthread_self()]->recv(sbar->maxTime);
  // Insert a dummy block before the barrier to enable the JavaScript to vertically align all the barrier instances
  block();
  causality[pthread_self()]->recv(sbar->maxTime+1);
  //{ scope s("Barrier", scope::minimum); 
  commBar("Barrier", txt()<<"B_"<<sbar<<"_"<<sbar->count);

  rc = pthread_mutex_unlock(&causalityMutex);
  if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_unlock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  //}
  rc = pthread_barrier_wait(&(sbar->bar));
  if(rc!=0 && rc!=PTHREAD_BARRIER_SERIAL_THREAD) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_barrier_wait() "<<rc<<"="<<strerror(rc)<<endl; return rc; }

  return 0;
}

/*****************
 ***** Mutex *****
 *****************/

int sight_pthread_mutex_init(sight_pthread_mutex_t *smutex, const pthread_mutexattr_t *attr) {
  smutex->numMutexOwners=0;
  checkCausality(true);
    
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
    rc = pthread_mutex_lock(&causalityMutex);
    if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
    
    // Update the causality info
    long long lastClockTime = causality[smutex->lastMutexOwner]->send();
    rc = receiveCausality(txt()<<"S_"<<smutex->lastMutexOwner<<"_"<<lastClockTime, smutex->lastMutexOwner,
                          txt()<<"R_"<<pthread_self()<<"_"<<lastClockTime, 
                          "Mutex Lock", true);
    if(rc!=0) return rc;
    
    rc = pthread_mutex_unlock(&causalityMutex);
    if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  }

  // Record that this thread was the last owner of this lock
  smutex->numMutexOwners++;
  smutex->lastMutexOwner = pthread_self();

  return 0;
}

int sight_pthread_mutex_unlock(sight_pthread_mutex_t* smutex) {
  int rc;

  rc = pthread_mutex_lock(&causalityMutex);
  if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }

  checkCausality(true);

  // Update the causality info
  rc = sendCausality(txt()<<"S_"<<pthread_self()<<"_"<<causality[pthread_self()]->send(), "Mutex Unlock", true);
  if(rc!=0) return rc;
 
  rc = pthread_mutex_unlock(&causalityMutex);
  if(rc!=0) { cerr << pthread_self()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  
  rc = pthread_mutex_unlock(&(smutex->mutex));
  if(rc!=0) return rc;

  return 0;
} 

/*int pthread_orig_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{ return pthread_mutex_init(mutex, attr); }
int pthread_orig_mutex_destroy(pthread_mutex_t *mutex)
{ return pthread_mutex_destroy(mutex); }
int pthread_orig_mutex_lock(pthread_mutex_t* mutex)
{ return pthread_mutex_lock(mutex); }
int pthread_orig_mutex_unlock(pthread_mutex_t* mutex)
{ return pthread_mutex_unlock(mutex); }*/

/*******************************
 ***** Condition Variables *****
 *******************************/

int sight_pthread_cond_init(sight_pthread_cond_t *scond, const pthread_condattr_t *attr) {
  scond->numAwakenings = 0;
  
  int rc = pthread_mutex_init(&(scond->signalMutex), NULL);
  if(rc!=0) return rc;
  
  scond->signalers.insert(pthread_self());

  checkCausality(true);
  
  return pthread_cond_init(&(scond->cond), attr);
}
int sight_pthread_cond_destroy(sight_pthread_cond_t *scond) {
  int rc = pthread_mutex_destroy(&(scond->signalMutex));
  if(rc!=0) return rc;
  
  return pthread_cond_destroy(&(scond->cond));
}

int sight_pthread_cond_wait(sight_pthread_cond_t *scond, sight_pthread_mutex_t *smutex) {
  //cout << "sight_pthread_cond_wait() waiting"<<endl;
  int rc = pthread_cond_wait(&(scond->cond), &(smutex->mutex));
  if(rc!=0) return rc;
  //cout << "sight_pthread_cond_wait() done, #scond->signalers="<<scond->signalers.size()<<endl;
  
  // Receive causality from the signaler(s)
  pthread_mutex_lock(&scond->signalMutex);
  //cout << "sight_pthread_cond_wait() grabbed signalMutex"<<endl;
  
  for(set<pthread_t>::const_iterator s=scond->signalers.begin();
      s!=scond->signalers.end(); s++) {
    //cout << "sight_pthread_cond_wait() SC_"<<*s<<"_"<<scond->numAwakenings<<" => "<<"RC_"<<pthread_self()<<"_"<<scond->numAwakenings<<endl;
    int rc = receiveCausality(txt()<<"SC_"<<*s<<"_"<<scond->numAwakenings, *s,
                              txt()<<"RC_"<<pthread_self()<<"_"<<scond->numAwakenings,
                              "Cond Awoken");
    if(rc!=0) return rc;
  }
  //cout << "sight_pthread_cond_wait() grabbed signalMutex"<<endl;
  
  ++scond->numAwakenings;
  pthread_mutex_unlock(&scond->signalMutex);
  return 0;
}

int sight_pthread_cond_timedwait(sight_pthread_cond_t * scond, sight_pthread_mutex_t *smutex,
                                 const struct timespec *abstime) {
  int rc = pthread_cond_timedwait(&(scond->cond), &(smutex->mutex), abstime);
  if(rc!=0) return rc;
  
  // Receive causality from the signaler(s)
  rc = pthread_mutex_lock(&scond->signalMutex);
  if(rc!=0) return rc;
  
  for(set<pthread_t>::const_iterator s=scond->signalers.begin();
      s!=scond->signalers.end(); s++) {
    int rc = receiveCausality(txt()<<"SC_"<<*s<<"_"<<scond->numAwakenings, *s,
                              txt()<<"RC_"<<pthread_self()<<"_"<<scond->numAwakenings,
                              "Cond Timed Awoken");
    if(rc!=0) return rc;
  }
  
  ++scond->numAwakenings;
  rc = pthread_mutex_unlock(&scond->signalMutex);
  if(rc!=0) return rc;
  
  return 0;
}

int sight_pthread_cond_signal(sight_pthread_cond_t *scond) {
  // Send causality from the signaler(s) and record this sender in second->signalers.
  int rc = pthread_mutex_lock(&scond->signalMutex);
  if(rc!=0) return rc;
  
  //cout << "sight_pthread_cond_signal() SC_"<<pthread_self()<<"_"<<scond->numAwakenings<<" => ???"<<endl;
  rc = sendCausality(txt()<<"SC_"<<pthread_self()<<"_"<<scond->numAwakenings, "Signal");
  if(rc!=0) return rc;
  scond->signalers.insert(pthread_self());
  
  rc = pthread_mutex_unlock(&scond->signalMutex);
  if(rc!=0) return rc;
  
  // Signal the waiting thread(s)
  rc = pthread_cond_signal(&(scond->cond));
  if(rc!=0) return rc;
  
  return 0;
}

int sight_pthread_cond_broadcast(sight_pthread_cond_t *scond) {
  // Send causality from the signaler(s) and record this sender in second->signalers.
  int rc = pthread_mutex_lock(&scond->signalMutex);
  if(rc!=0) return rc;
  
  rc = sendCausality(txt()<<"SC_"<<pthread_self()<<"_"<<scond->numAwakenings, "Broadcast");
  if(rc!=0) return rc;
  scond->signalers.insert(pthread_self());
  
  rc = pthread_mutex_unlock(&scond->signalMutex);
  if(rc!=0) return rc;
  
  // Signal the waiting thread(s)
  rc = pthread_cond_broadcast(&(scond->cond));
  if(rc!=0) return rc;
  
  return 0;
}

