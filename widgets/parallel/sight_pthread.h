#pragma once 
#include <pthread.h>
#include <string.h>
#include "sight_structure.h"
#include "sight_common.h"
#include "thread_local_storage.h"
using namespace sight;

/********************************
 ***** Causality Management *****
 ********************************/

// Updates the causality info from the sender side
// Returns the error code of the pthreads functions called.
// cmHeld - indicates whether the causalityMutex is already being held by the calling thread
// If clock!=NULL, it is set to the causal time immediately after the send operation
int sendCausality(const std::string& sendID, const std::string& recvID, 
                  const std::string& label,
                  bool cmHeld, long long* clock=NULL);
int sendCausality(const std::string& sendID,
                  const std::string& label,
                  bool cmHeld=false);

// Updates the causality info on the receiver side
// Returns the error code of the pthreads functions called.
// cmHeld - indicates whether the causalityMutex is already being held by the calling thread
int receiveCausality(const std::string& sendID, pthread_t sender,
                     const std::string& recvID, 
                     const std::string& label,
                     bool cmHeld=false);
int receiveCausality(const std::string& sendID, long long senderTime,
                     const std::string& recvID, 
                     const std::string& label,
                     bool cmHeld=false);

// Makes sure that causality[] is initialized.
// cmHeld - indicates whether the causalityMutex is already being held by the calling thread
int checkCausality(bool cmHeld);

/***************************
 ***** Thread Creation *****
 ***************************/


int sight_pthread_create(pthread_t * thread,
                         const pthread_attr_t * attr,
                         void *(*start_routine)(void*), void * arg);

void sight_pthread_exit(void *value_ptr);

int sight_pthread_join(pthread_t thread, void **value_ptr);

/*******************
 ***** Barrier *****
 *******************/


typedef struct {
  pthread_barrier_t bar;
  int count; // counts the number of times this barrier has been reached
  long long maxTime; // The maximum causal time reached during the most recent barrier
} sight_pthread_barrier_t;


int sight_pthread_barrier_init(sight_pthread_barrier_t* sbar, const pthread_barrierattr_t * attr, unsigned count);
int sight_pthread_barrier_wait(sight_pthread_barrier_t* sbar);


/*****************
 ***** Mutex *****
 *****************/


typedef struct {
  long numMutexOwners;
  pthread_t lastMutexOwner;
  pthread_mutex_t mutex;
} sight_pthread_mutex_t;

int sight_pthread_mutex_init(sight_pthread_mutex_t *smutex, const pthread_mutexattr_t *attr);
int sight_pthread_mutex_destroy(sight_pthread_mutex_t *smutex);
int sight_pthread_mutex_lock(sight_pthread_mutex_t* smutex);
int sight_pthread_mutex_unlock(sight_pthread_mutex_t* smutex);

/*******************************
 ***** Condition Variables *****
 *******************************/

typedef struct {
  // Number of times a thread has been awoken from sleep on this condition var
  int numAwakenings;
  // The set of threads that signaled this condition before the listener was awoken
  set<pthread_t> signalers;
  
  // Mutex used to control access to signalers (there is no guarantee that different
  // signalers and waiters lock the same mutex when signaling/waking)
  pthread_mutex_t signalMutex;
  
  // The condition being wrapped
  pthread_cond_t cond;
} sight_pthread_cond_t;

int sight_pthread_cond_init(sight_pthread_cond_t *scond, const pthread_condattr_t *attr);
int sight_pthread_cond_destroy(sight_pthread_cond_t *scond);
int sight_pthread_cond_wait(sight_pthread_cond_t *scond, sight_pthread_mutex_t *smutex);
int sight_pthread_cond_timedwait(sight_pthread_cond_t * scond, sight_pthread_mutex_t *smutex,
                                 const struct timespec *abstime);
int sight_pthread_cond_signal(sight_pthread_cond_t *scond);
int sight_pthread_cond_broadcast(sight_pthread_cond_t *scond);

#ifndef PTHREAD_C
// Redirect the names of all the wrapped structs and function calls
// to use the Sight wrappers
#define pthread_create          sight_pthread_create
#define pthread_exit            sight_pthread_exit
#define pthread_join            sight_pthread_join

#define pthread_barrier_t       sight_pthread_barrier_t
#define pthread_barrier_init    sight_pthread_barrier_init
#define pthread_barrier_destroy sight_pthread_barrier_destroy
#define pthread_barrier_wait    sight_pthread_barrier_wait

#define pthread_mutex_t         sight_pthread_mutex_t
#define pthread_mutex_init      sight_pthread_mutex_init
#define pthread_mutex_destroy   sight_pthread_mutex_destroy
#define pthread_mutex_lock      sight_pthread_mutex_lock
#define pthread_mutex_unlock    sight_pthread_mutex_unlock


#define pthread_cond_t          sight_pthread_cond_t
#define pthread_cond_init       sight_pthread_cond_init
#define pthread_cond_destroy    sight_pthread_cond_destroy
#define pthread_cond_wait       sight_pthread_cond_wait     
#define pthread_cond_timedwait  sight_pthread_cond_timedwait
#define pthread_cond_signal     sight_pthread_cond_signal
#define pthread_cond_broadcast  sight_pthread_cond_broadcast
#endif
