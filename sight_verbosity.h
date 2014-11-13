#pragma once
#include "sight.h"

template<typename Type>
class SightVerbWrapper {
  Type* instance;
  public:
  SightVerbWrapper(Type* instance): instance(instance) {}
  ~SightVerbWrapper() {
    if(instance) delete instance;
  }
};

#define CONCAT2(x, y) x ## y
#define CONCAT(x, y) CONCAT2(x, y)
#define SIGHT_VERB_DECL(Type, Args, verbThreshold, verbosity) \
SightVerbWrapper<Type> CONCAT(SightVerbWrapperVar, __COUNTER__) (verbThreshold<=verbosity? new Type Args: NULL);

/*#define SIGHT_VERB_DECL(Type, Args, verbThreshold, verbosity) \
class SightVerbWrapper##__COUNTER__ {                  \
  Type* instance;                                       \
  public:                                               \
  SightVerbWrapper##__COUNTER__(Type* instance): instance(instance) {                      \
  }                                                     \
  ~SightVerbWrapper##__COUNTER__() {                                 \
    if(instance) delete instance;                       \
  }                                                     \
}; / * class SightVerbWrapper* /                          \
SightVerbWrapper##__COUNTER__ SightVerbWrapperVar##__COUNTER__(verbThreshold<=verbosity? new Type Args: NULL);*/

/*#define SIGHT_VERB_DECL(Type, Args, verbThreshold, verbosity) \
class SightVerbWrapper##__COUNTER__ {                  \
  Type* instance;                                       \
  public:                                               \
  SightVerbWrapper##__COUNTER__(bool enabled) {                      \
    if(enabled) instance = new Type Args;               \
    else        instance = NULL;                        \
  }                                                     \
  ~SightVerbWrapper##__COUNTER__() {                                 \
    if(instance) delete instance;                       \
  }                                                     \
}; / * class SightVerbWrapper* /                          \
SightVerbWrapper##__COUNTER__ SightVerbWrapperVar##__COUNTER__(verbThreshold<=verbosity);
*/
/*#define SIGHT_VERB_0_0(code) code
#define SIGHT_VERB_0_1(code) code
#define SIGHT_VERB_0_2(code) code
#define SIGHT_VERB_0_3(code) code
#define SIGHT_VERB_0_4(code) code
#define SIGHT_VERB_0_5(code) code
#define SIGHT_VERB_0_6(code) code
#define SIGHT_VERB_0_7(code) code
#define SIGHT_VERB_0_8(code) code
#define SIGHT_VERB_0_9(code) code

#define SIGHT_VERB_1_0(code) 
#define SIGHT_VERB_1_1(code) code
#define SIGHT_VERB_1_2(code) code
#define SIGHT_VERB_1_3(code) code
#define SIGHT_VERB_1_4(code) code
#define SIGHT_VERB_1_5(code) code
#define SIGHT_VERB_1_6(code) code
#define SIGHT_VERB_1_7(code) code
#define SIGHT_VERB_1_8(code) code
#define SIGHT_VERB_1_9(code) code

#define SIGHT_VERB_2_0(code) 
#define SIGHT_VERB_2_1(code) 
#define SIGHT_VERB_2_2(code) code
#define SIGHT_VERB_2_3(code) code
#define SIGHT_VERB_2_4(code) code
#define SIGHT_VERB_2_5(code) code
#define SIGHT_VERB_2_6(code) code
#define SIGHT_VERB_2_7(code) code
#define SIGHT_VERB_2_8(code) code
#define SIGHT_VERB_2_9(code) code

#define SIGHT_VERB_3_0(code) 
#define SIGHT_VERB_3_1(code) 
#define SIGHT_VERB_3_2(code) 
#define SIGHT_VERB_3_3(code) code
#define SIGHT_VERB_3_4(code) code
#define SIGHT_VERB_3_5(code) code
#define SIGHT_VERB_3_6(code) code
#define SIGHT_VERB_3_7(code) code
#define SIGHT_VERB_3_8(code) code
#define SIGHT_VERB_3_9(code) code

#define SIGHT_VERB_4_0(code) 
#define SIGHT_VERB_4_1(code) 
#define SIGHT_VERB_4_2(code) 
#define SIGHT_VERB_4_3(code) 
#define SIGHT_VERB_4_4(code) code
#define SIGHT_VERB_4_5(code) code
#define SIGHT_VERB_4_6(code) code
#define SIGHT_VERB_4_7(code) code
#define SIGHT_VERB_4_8(code) code
#define SIGHT_VERB_4_9(code) code

#define SIGHT_VERB_5_0(code) 
#define SIGHT_VERB_5_1(code) 
#define SIGHT_VERB_5_2(code) 
#define SIGHT_VERB_5_3(code) 
#define SIGHT_VERB_5_4(code) 
#define SIGHT_VERB_5_5(code) code
#define SIGHT_VERB_5_6(code) code
#define SIGHT_VERB_5_7(code) code
#define SIGHT_VERB_5_8(code) code
#define SIGHT_VERB_5_9(code) code

#define SIGHT_VERB_6_0(code) 
#define SIGHT_VERB_6_1(code) 
#define SIGHT_VERB_6_2(code) 
#define SIGHT_VERB_6_3(code) 
#define SIGHT_VERB_6_4(code) 
#define SIGHT_VERB_6_5(code) 
#define SIGHT_VERB_6_6(code) code
#define SIGHT_VERB_6_7(code) code
#define SIGHT_VERB_6_8(code) code
#define SIGHT_VERB_6_9(code) code

#define SIGHT_VERB_7_0(code) 
#define SIGHT_VERB_7_1(code) 
#define SIGHT_VERB_7_2(code) 
#define SIGHT_VERB_7_3(code) 
#define SIGHT_VERB_7_4(code) 
#define SIGHT_VERB_7_5(code) 
#define SIGHT_VERB_7_6(code) 
#define SIGHT_VERB_7_7(code) code
#define SIGHT_VERB_7_8(code) code
#define SIGHT_VERB_7_9(code) code

#define SIGHT_VERB_8_0(code) 
#define SIGHT_VERB_8_1(code) 
#define SIGHT_VERB_8_2(code) 
#define SIGHT_VERB_8_3(code) 
#define SIGHT_VERB_8_4(code) 
#define SIGHT_VERB_8_5(code) 
#define SIGHT_VERB_8_6(code) 
#define SIGHT_VERB_8_7(code) 
#define SIGHT_VERB_8_8(code) code
#define SIGHT_VERB_8_9(code) code

#define SIGHT_VERB_9_0(code) 
#define SIGHT_VERB_9_1(code) 
#define SIGHT_VERB_9_2(code) 
#define SIGHT_VERB_9_3(code) 
#define SIGHT_VERB_9_4(code) 
#define SIGHT_VERB_9_5(code) 
#define SIGHT_VERB_9_6(code) 
#define SIGHT_VERB_9_7(code) 
#define SIGHT_VERB_9_8(code) 
#define SIGHT_VERB_9_9(code) code

#define SIGHT_VERB(code, verbThreshold, verbosity) SIGHT_VERB_ ## verbThreshold ## _ ## verbosity(code)*/

#define SIGHT_VERB(code, verbosity, verbThreshold) \
  if((verbosity) > (verbThreshold)) { code; }

#define SIGHT_VERB_IF(verbosity, verbThreshold) \
  if((verbosity) <= (verbThreshold)) {

#define SIGHT_VERB_FI() }


