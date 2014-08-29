// Copyright (c) 203 Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory
// Written by Greg Bronevetsky <bronevetsky1@llnl.gov>
//  
// LLNL-CODE-642002.
// All rights reserved.
//  
// This file is part of Sight. For details, see https://github.com/bronevet/sight. 
// Please read the COPYRIGHT file for Our Notice and
// for the BSD License.
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
  int numDims = props.getInt("numDims");
  //boost::shared_ptr<long[]> steps = boost::make_shared<long[]>(numDims);
  boost::shared_ptr<long> steps(new long[numDims]);
  for(int i=0; i<numDims; i++)
    steps.get()[i] = props.getInt(txt()<<"dim"<<i);
    
  // Update the overall sightClock, using a different unique name for each stepClockID
  sightClock::updateTime(txt()<<"stepClock_"<<props.get("stepClockID"), sightArray(sightArray::dims(numDims), steps));
  return NULL;
}

// Parses a given timeClock and records the updated time with sightClock
void* mpiClockEnterHandler(properties::iterator props) {
  sightClock::updateTime("mpiClock", props.getInt("time"));
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
  (*layoutEnterHandlers)["mpiClock"]         = &mpiClockEnterHandler;
  (*layoutEnterHandlers)["scalarCausalClock"] = &scalarCausalClockEnterHandler;
}
clockLayoutHandlerInstantiator clockLayoutHandlerInstance;

}; // namespace layout
}; // namespace sight
