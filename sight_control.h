// Special preprocessor macros that make it easy to turn Sight operations on and off
// at compilation to eliminate its cost during runs that do not need Sight.
#pragma once
//#include "sight.h"

// Applications that wish to disable Sight completely #define DISABLE_SIGHT
#ifdef DISABLE_SIGHT

#define dbg common::nullS
#define SIGHT_VERB_DECL(Type, Args, verbThreshold, verbosity)

#define SIGHT_VERB_DECL_REF(Type, Args, ref, verbThreshold, verbosity)

#define SIGHT_VERB(code, verbThreshold, verbosity)

#define SIGHT_VERB_IF(verbThreshold, verbosity)

#define SIGHT_VERB_FI()

#define SIGHT_DECL(Type, Args, cond)

#define SIGHT_DECL_REF(Type, Args, ref, cond)

#define SIGHT(code, cond)

#define SIGHT_IF(cond)

#define SIGHT_FI()

// Definitions for apps that wish to enable Sight selectively based on specific conditions or
// level of verbosity (the _VERB macros)
#else

template<typename Type>
class SightVerbWrapper {
  Type* instance;
  public:
  SightVerbWrapper(Type* instance): instance(instance) { 
//    if(instance) std::cout << "<<"<<std::endl;
  }
  // Sets ref to point to the instance in this object, if any
  SightVerbWrapper(Type* instance, Type** ref): instance(instance) { 
    *ref = instance;
  }
  Type* get() { return instance; }
  ~SightVerbWrapper() {
    if(instance) {
//      std::cout << ">>" << std::endl;
      delete instance;
    }
  }
};

#define CONCAT2(x, y) x ## y
#define CONCAT(x, y) CONCAT2(x, y)

// Declares a local variable of the given type and with the given constructor argumetns 
// if the verbosity argument reaches the given threshold
#define SIGHT_VERB_DECL(Type, Args, verbThreshold, verbosity) \
SightVerbWrapper<Type> CONCAT(SightVerbWrapperVar, __COUNTER__) ((verbThreshold)<=(verbosity)? new Type Args: (Type*)NULL);

// Declares a local variable of the given type and with the given constructor argumetns 
// if the verbosity argument reaches the given threshold.
// Also, sets ref to point to the created variable
#define SIGHT_VERB_DECL_REF(Type, Args, ref, verbThreshold, verbosity) \
SightVerbWrapper<Type> CONCAT(SightVerbWrapperVar, __COUNTER__) ((verbThreshold)<=(verbosity)? new Type Args: (Type*)NULL, &(ref));

#define SIGHT_VERB(code, verbThreshold, verbosity) \
  if((verbThreshold)<=(verbosity)) { code; }

#define SIGHT_VERB_IF(verbThreshold, verbosity) \
  if((verbThreshold)<=(verbosity)) {

#define SIGHT_VERB_FI() }

// Declares a local variable of the given type and with the given constructor argumetns 
// if the condition argument is true.
#define SIGHT_DECL(Type, Args, cond) \
SightVerbWrapper<Type> CONCAT(SightVerbWrapperVar, __COUNTER__) (cond? new Type Args: (Type*)NULL);

// Declares a local variable of the given type and with the given constructor argumetns 
// if the condition argument is true.
// Also, sets ref to point to the created variable.
#define SIGHT_DECL_REF(Type, Args, ref, cond) \
SightVerbWrapper<Type> CONCAT(SightVerbWrapperVar, __COUNTER__) (cond? new Type Args: (Type*)NULL, &(ref));

#define SIGHT(code, cond) \
  if(cond) { code; }

#define SIGHT_IF(cond) \
  if(cond) {

#define SIGHT_FI() }

#endif

