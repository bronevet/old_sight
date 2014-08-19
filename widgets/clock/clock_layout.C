// Licence information included in file LICENCE
#include "clock_layout.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <boost/make_shared.hpp>
        
using namespace std;
using namespace sight::common;

namespace sight {
namespace layout {


// Parses a given timeClock and records the updated time with sightClock
void* timeClockEnterHandler(properties::iterator props) { 
  sightClock::updateTime("timeClock", props.getInt("time"));
  return NULL;
}

// Parses a given stepClock and records the updated time with sightClock
void* stepClockEnterHandler(properties::iterator props) { 
  //int numDims = props.getInt("numDims");
  //boost::shared_ptr<long[]> steps = boost::make_shared<long[]>(numDims);
  /*boost::shared_ptr<long> steps(new long[numDims]);
  for(int i=0; i<numDims; i++)
    steps.get()[i] = props.getInt(txt()<<"dim"<<i);*/
  attrValue curStep(props.get("curStep"), attrValue::unknownT);
  
  // Update the overall sightClock, using a different unique name for each stepClockID
  sightClock::updateTime(txt()<<"stepClock_"<<props.get("stepClockID"), curStep/*sightArray(sightArray::dims(numDims), steps)*/);
  return NULL;
}

// Parses a given scalarCausalClock and records the updated time with sightClock
void* scalarCausalClockEnterHandler(properties::iterator props) { 
  sightClock::updateTime("scalarCausalClock", props.getInt("time"));
  return NULL;
}

// Specify the callbacks that will be invoked when various clocks are encountered
clockLayoutHandlerInstantiator::clockLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["timeClock"]         = &timeClockEnterHandler;
  (*layoutEnterHandlers)["stepClock"]         = &stepClockEnterHandler;
  (*layoutEnterHandlers)["scalarCausalClock"] = &scalarCausalClockEnterHandler;
}
clockLayoutHandlerInstantiator clockLayoutHandlerInstance;

}; // namespace layout
}; // namespace sight
