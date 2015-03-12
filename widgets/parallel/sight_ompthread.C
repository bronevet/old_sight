#include <omp.h>
//#include <pthread.h>
#include <string.h>
#include "sight_structure_internal.h"
#include "sight_common.h"
#include "thread_local_storage.h"
#define OMPTHREAD_C
#include "sight_ompthread.h"
#include "../widgets/clock/clock_structure.h"
#include "../../attributes/attributes_common.h"
#include "../../attributes/attributes_structure.h"
#include "parallel_structure.h"
using namespace sight;

namespace sight {
namespace structure {

static omp_lock_t causalityLock;
static std::map<long, scalarCausalClock*> causalityOMP;

/**************************************************
 ***** Thread Initialization and Finalization *****
 **************************************************/

//ThreadLocalStorage0<comparison*> ompthreadThreadInitFinInstantiator::globalComparisonsOMP;
// Had to use __thread instead of the ompthreads key API because for some reason the key->value mapping
// for the main thread was destroyed by the time the finalizer was invoked in the Sight
// atexit handler.
__thread comparison* globalComparisonsOMP;

ompthreadThreadInitFinInstantiator::ompthreadThreadInitFinInstantiator() { 
  addFuncs("ompthread", 
           ompthreadThreadInitFinInstantiator::initialize, 
           ompthreadThreadInitFinInstantiator::finalize,
           common::easyset<std::string>("mpi"), common::easyset<std::string>());
  omp_init_lock(&causalityLock);
}

void ompthreadThreadInitFinInstantiator::initialize() {
  // Assign each thread to a separate log based on its thread ID
  if(getenv("DISABLE_OMPTHREAD_COMPARISON")==NULL)
    globalComparisonsOMP = new comparison(txt()<< omp_get_thread_num());
  else
    globalComparisonsOMP = NULL;
  //cout << pthread_self()<<": PthreadThreadInitFinInstantiator::initialize() *globalComparisons="<<globalComparisons<<endl;
}

void ompthreadThreadInitFinInstantiator::finalize() {
  
  //cout << pthread_self()<<": PthreadThreadInitFinInstantiator::finalize() A *globalComparisonsOMP="<<globalComparisonsOMP<<endl;
  //dbg << pthread_self()<<": PthreadThreadInitFinInstantiator::finalize() A *globalComparisonsOMP="<<globalComparisonsOMP<<endl;
  // Assign each thread to a separate log based on its thread ID
  if(getenv("DISABLE_OMPTHREAD_COMPARISON")==NULL) {
    //cout << pthread_self()<<": PthreadThreadInitFinInstantiator::finalize() B *globalComparisonsOMP="<<globalComparisonsOMP<<endl;
    //dbg << pthread_self()<<": PthreadThreadInitFinInstantiator::finalize() B *globalComparisonsOMP="<<globalComparisonsOMP<<endl;
    assert(globalComparisonsOMP != NULL);
    delete globalComparisonsOMP;
  } 
  //cout << pthread_self()<<": PthreadThreadInitFinInstantiator::finalize() C *globalComparisonsOMP="<<globalComparisonsOMP<<endl;
  //dbg << pthread_self()<<": PthreadThreadInitFinInstantiator::finalize() C *globalComparisonsOMP="<<globalComparisonsOMP<<endl;
}

ompthreadThreadInitFinInstantiator ompthreadThreadInitFinInstance;


/********************************
 ***** causalityOMP Management *****
 ********************************/

// Updates the causality info from the sender side
// Returns the error code of the pthreads functions called.
// cmHeld - indicates whether the causalityLock is already being held by the calling thread
// If clock!=NULL, it is set to the causal time immediately after the send operation
void sendcausalityOMP(const std::string& sendID, const std::string& recvID, 
                  const std::string& label,
                  bool cmHeld, long long* clock) {
 // Emit the causality info of this lock acquire, while holding on to causalityLock
  if(!cmHeld) {
    omp_set_lock(&causalityLock); 
  }
  //cout << "Thread# " << omp_get_thread_num()<< " commsend sendID="  << sendID << " recvID=" << recvID << endl;
  
  checkcausalityOMP(true);

  //scope s(label, scope::minimum);
  commSend(label, sendID, recvID);
  
  // If a pointer to a clock is provided, set it to the time of the send operation
  if(clock)
    *clock = causalityOMP[omp_get_thread_num()]->send();

  if(!cmHeld) {
    omp_unset_lock(&causalityLock);
  }
}

void sendcausalityOMP(const std::string& sendID, 
                  const std::string& label,
                  bool cmHeld) {
  return sendcausalityOMP(sendID, "", label, cmHeld, NULL);
}

// Updates the causalityOMP info on the receiver side
// Returns the error code of the pthreads functions called.
// caller - If provided, receiveCausality() reads the causal time directly from the caller.
// callerTime - If provieded, takes the causal time from this argument
// cmHeld - indicates whether the causalityLock is already being held by the calling thread
void receivecausalityOMP(const std::string& sendID, int sender,
                     const std::string& recvID, 
                     const std::string& label, bool cmHeld) {
  //dbg << pthread_self()<<": receiveCausality("<<sendID<<", sender="<<sender<<", "<<recvID<<", "<<label<<", "<<cmHeld<<")"<<endl;
  if(!cmHeld) {
    omp_set_lock(&causalityLock);
  }

  checkcausalityOMP(true);
  //scope s(label, scope::minimum);
 
  //dbg << pthread_self()<<": lock time="<<causality[sender]->send()<<endl;
  causalityOMP[omp_get_thread_num()]->recv(causalityOMP[sender]->send());
  //dbg << pthread_self()<<": local time="<<causality[pthread_self()]->send()<<endl;
  commRecv(label, sendID, recvID);

  if(!cmHeld) {
    omp_unset_lock(&causalityLock);
  }
}

void receivecausalityOMP(const std::string& sendID, long long senderTime,
                     const std::string& recvID, 
                     const std::string& label, 
                     bool cmHeld) {
  //dbg << pthread_self()<<": receiveCausality("<<sendID<<", senderTime="<<senderTime<<", "<<recvID<<", "<<label<<", "<<cmHeld<<")"<<endl;
  if(!cmHeld) {
    omp_set_lock(&causalityLock);
  }

  checkcausalityOMP(true);
  //scope s(label, scope::minimum);
 
  ////cout << pthread_self()<<": lock time="<<causality[smutex->lastMutexOwner]->send()<<endl;
  causalityOMP[omp_get_thread_num()]->recv(senderTime);
  //cout << pthread_self()<<": local time="<<causality[pthread_self()]->send()<<endl;
  commRecv(label, sendID, recvID);

  if(!cmHeld) {
    omp_unset_lock(&causalityLock);
  }  
}

// Makes sure that causality[] is initialized.
// cmHeld - indicates whether the causalityLock is already being held by the calling thread
void checkcausalityOMP(bool cmHeld) {
  
  if(!cmHeld) {
    omp_set_lock(&causalityLock);
    //if(rc!=0) { cerr << omp_get_thread_num()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  }

  if(causalityOMP.find(omp_get_thread_num()) == causalityOMP.end())
    causalityOMP[omp_get_thread_num()] = new scalarCausalClock();

  if(!cmHeld) {
    omp_unset_lock(&causalityLock);
    //if(rc!=0) { cerr << omp_get_thread_num()<<": SIGHT ERROR: calling pthread_mutex_lock() "<<rc<<"="<<strerror(rc)<<endl; return rc; }
  }
}


/**********************************
 ***** OpenMP Thread Creation *****
 *********************************/

typedef void *(ompthreadRoutine)(void*);

// Encapsulates the arguments to sightThreadInitializer
typedef struct {
  ompthreadRoutine* routine; // The routine that sight_ompthread_create is asked to invoke in a new thread
  long spawner;       // ID of the thread that spawned this one
  int   spawnCnt;          // The number of threads this thread's spawner created before it created this thread
  void* arg;               // The void* argument to this routine
} ompthreadRoutineData;

// If the application calls pthread_exit deep inside the thread's calling stack 
// we need to unwrap the stack to get back to the call to sightThreadInitializer()
// to ensure all of Sight's state is correctly cleaned up. We do this by calling
// setjmp() to set a return point inside sightThreadInitializer() and than calling
// longjmp() to return to it from inside the call to pthread_exit();
//ThreadLocalStorage1<void*, void*> retVal(NULL);

void ompthreadCleanup(void * arg) {
  //cout << omp_get_thread_num() <<": threadCleanup() <<<<"<<endl;
  sendcausalityOMP(txt()<<"End_"<<omp_get_thread_num(), "Terminating");
  
  // hoa edit
  // use finalizeSight instead of SightThreadFinalize()
  //AbortHandlerInstantiator::finalizeSight();
  SightThreadFinalize();

//  cout << pthread_self()<<": threadCleanup() >>>>"<<endl;
  /*
  cout << omp_get_thread_num()<<": threadCleanup() <<<<"<<endl;
  sendcausalityOMP(txt()<<"End_"<<omp_get_thread_num(), "Terminating");
  //AbortHandlerInstantiator::finalizeSight();
  SightThreadFinalize();
//  cout << pthread_self()<<": threadCleanup() >>>>"<<endl;
*/
}

// Function that wraps the execution of each thread spawned using pthread_create()
// and ensures that all appropriate initialization is performed
void sightOMPThreadInitializer() {

  // Initialize Sight for this thread before we initialize its clock
  SightInit_NewThread();
 
  {
    // Initialize the new thread's causality clock, while under control of causalityLock
    omp_set_lock(&causalityLock);
    
    {
    attr a("Causal", "SpawnJoin"); 
    receivecausalityOMP(txt()<<"Spawner_"<<0,
                          0,
                          txt()<<"Spawnee_"<<omp_get_thread_num(),
                          "Spawned", true);
    }

    omp_unset_lock(&causalityLock);
   
  }
}

// Counts the number of times each thread has spawned another thread
// Making numThreadsSpawned a global variable so that all threads can access it
//static ThreadLocalStorage1<int, int> numThreadsSpawned(0);

void sight_ompthread_create() {
  // Create a scalarCausalClock for the spawning thread if it does not yet have one
  {
    omp_set_lock(&causalityLock);
  
    checkcausalityOMP(true);

    omp_unset_lock(&causalityLock);
  }
  
  {
  attr a("Causal", "SpawnJoin"); 
  sendcausalityOMP(txt()<<"Spawner_"<<0, "Spawn");
  }  
}

void sight_ompthread_exit(void *value_ptr) {
  //cout << omp_get_thread_num()<<": sight_ompthread_exit()"<<endl;
  // Add a causalityOMP send edge from the thread's termination to the join call
  sendcausalityOMP(txt()<<"End_"<<omp_get_thread_num(), "Terminating");
}

void sight_ompthread_join(int tid) {
  // Add a causality send edge from the thread's termination to the join call
  {
    attr a("Causal", "SpawnJoin"); 
    receivecausalityOMP(txt()<<"End_"<<tid, tid, 
                          txt()<<"Joiner_"<<omp_get_thread_num()<<"_"<<tid,
                          "Join");
  }
}

/*****************
 **** OMP Lock ***
 *****************/

void sight_omp_lock_init(sight_omp_lock_t *slock){
  slock->numLockOwners=0;
  checkcausalityOMP(true);
  omp_init_lock(&(slock->ompLock));
}

void sight_omp_lock_destroy(sight_omp_lock_t *slock){
  omp_destroy_lock(&(slock->ompLock));
}

void sight_omp_lock(sight_omp_lock_t* slock){
  //cout << "lock threadID = " << omp_get_thread_num() << " slock->lastLockOwner" << slock->lastLockOwner << "slock->numLockOwners" << slock->numLockOwners<<endl;
  omp_set_lock(&(slock->ompLock));

  // If this lock was previously owned by this or another thread, record the happens-before relationship
  if(slock->numLockOwners>0) {
    omp_set_lock(&causalityLock);
    
    checkcausalityOMP(true);
    // Update the causality info
    long long lastClockTime = causalityOMP[slock->lastLockOwner]->send();
    receivecausalityOMP(txt()<<"S_"<<slock->lastLockOwner<<"_"<<lastClockTime, slock->lastLockOwner,
                          txt()<<"R_"<<omp_get_thread_num()<<"_"<<lastClockTime, 
                          "Lock", true);    
    
    omp_unset_lock(&causalityLock);    
  }

  // Record that this thread was the last owner of this lock
  slock->numLockOwners++;
  slock->lastLockOwner = omp_get_thread_num();
}

void sight_omp_unlock(sight_omp_lock_t* slock){
  omp_set_lock(&causalityLock);
  
  checkcausalityOMP(true);

  // Update the causality info
  sendcausalityOMP(txt()<<"S_"<<omp_get_thread_num()<<"_"<<causalityOMP[omp_get_thread_num()]->send(), "Unlock", true);
  
  omp_unset_lock(&causalityLock);

  omp_unset_lock(&(slock->ompLock));  
}

void sight_omp_receive_single(int singleThread){
  //cout << "Thread# " << omp_get_thread_num() << " sight_omp_receive_single" << endl;
  long long lastClockTime = causalityOMP[singleThread]->send();

  receivecausalityOMP(txt()<<"SingleSend_"<<omp_get_thread_num()<<"_"<<causalityOMP[omp_get_thread_num()]->send(), singleThread,
                      txt()<<"SingleRecv_"<<singleThread<<"_"<<causalityOMP[singleThread]->send(), 
                      "After Single", false);
}

void sight_omp_send_single(int ite){
  //cout << "Thread# " << omp_get_thread_num() << " sight_omp_send_single" << endl;
  sendcausalityOMP(txt()<<"SingleSend_"<<ite<<"_"<<causalityOMP[ite]->send(), "Single Done", false);        
}

/*******************
 *** OMP Barrier ***
 *******************/

void sight_omp_barrier_init(sight_omp_barrier_t* sbar){
  sbar->count = 0;
  checkcausalityOMP(true);   
}

void sight_omp_barrier_wait(sight_omp_barrier_t* sbar){
  //cout << omp_get_thread_num()<<": sight_omp_barrier_wait("<<sbar<<")"<<endl;
  sbar->count++;
  
  // Set the local scalar clock to the maximum of each thread's scalar clock
  omp_set_lock(&causalityLock);
  
  checkcausalityOMP(true); 
  
  sbar->maxTime=0;
  for(map<long, scalarCausalClock*>::const_iterator i=causalityOMP.begin(); i!=causalityOMP.end(); i++) {
    if(i->first != omp_get_thread_num()) 
      sbar->maxTime = (i->second->send()>sbar->maxTime? i->second->send(): sbar->maxTime);
  }
    
  causalityOMP[omp_get_thread_num()]->recv(sbar->maxTime);
  block();
  causalityOMP[omp_get_thread_num()]->recv(sbar->maxTime+1);
  
  commBar("Barrier", txt()<<"B_"<<sbar->count);

  omp_unset_lock(&causalityLock);
}

}; // namespace structure 
}; // namespace sight
