#include "clock_structure.h"
#include <mpi.h>
#include <pnmpimod.h>

typedef void (*PNMPIMOD_SClock_getSTime_t)(long long** );

using namespace std;
using namespace sight::common;

namespace sight {
namespace structure {

/*********************
 ***** timeClock *****
 *********************/

// Records all the currently active instance of timeClock. Since all instances of timeClock
// correspond to the same real clock, we register the clock once for all currently active instances of timeClock.
std::set<timeClock*> timeClock::active;

timeClock::timeClock() {
  // Only register this clock with sightObj if a timeClock is not already registered with it
  if(active.size()==0)
    sightObj::addClock("timeClock", this);
  active.insert(this);
}

timeClock::~timeClock() {
  active.erase(this);
  
  // If this was the instance of timeClock that was registered with sightObj
  if(sightObj::isActiveClock("timeClock", this)) {
    // If there are other active instances, update the record to point to one of them
    if(active.size() > 0) sightObj::updClock("timeClock", *active.begin());
    // Otherwise, unregister this clock
    else                  sightObj::remClock("timeClock");
  }
}

properties* timeClock::setProperties(properties* props) {
  assert(props);
  
  // Update the clock
  modified();
  
  // Store the currrent time in props
  map<string, string> pMap;
  pMap["time"] = txt()<<(curTime.tv_sec*1000000 + curTime.tv_usec)/1000000.0;
  props->add("timeClock", pMap);
  return props;
}

// Returns true if the clock has been modified since the time of its registration or the last time modified() was called.
bool timeClock::modified() { 
  // Check the time
  struct timeval newTime;
  gettimeofday(&newTime, NULL);
  
  // If the time has advanced since the last measurement, update it and return true
  if(newTime.tv_sec != curTime.tv_sec || newTime.tv_usec != curTime.tv_usec) {
    curTime.tv_sec  = newTime.tv_sec;
    curTime.tv_usec = newTime.tv_usec;
    return true;
  // If time has not advanced
  } else 
    return false;
}

std::string timeClock::str() const {
  return txt() << "[timeClock: time="<<((curTime.tv_sec*1000000 + curTime.tv_usec)/1000000.0)<<"]";
}

/*********************
 ***** stepClock *****
 *********************/

int stepClock::maxStepClockID=0;

// Create a step clock with the given number of dimensions in its steps
stepClock::stepClock(int numDim) {   
  stepClockID = maxStepClockID++;
  
  // Register this step clock
  sightObj::addClock("stepClock", this);
  
  // Initialize the curStep vector with all 0s
  for(int i=0; i<numDim; i++)
    curStep.push_back(0);
  
  // Initialize isModified() to true since it has been modified (initialized in this case) since its creation
  isModified = true;
}

stepClock::~stepClock() {
  // Unregister this clock
  sightObj::remClock("stepClock");
}

properties* stepClock::setProperties(properties* props) {
  assert(props);
  
  // Store the currrent time in props
  map<string, string> pMap;
  pMap["stepClockID"] = txt()<<stepClockID;
  pMap["numDims"] = txt()<<curStep.size();
  for(int i=0; i<curStep.size(); i++)
    pMap[txt()<<"dim"<<i] = txt()<<curStep[i];
  props->add("stepClock", pMap);
  return props;
}

// Indicates that we've taken a step in the given dimension. This causes the clock's value at the given
// dimension to be incremented and for all the subsequent dimensions to be set to 0
void stepClock::step(int dim) {
  assert(dim < curStep.size());
  curStep[dim]++;
  dim++;
  for(; dim<curStep.size(); dim++)
    curStep[dim] = 0;
}

// Returns true if the clock has been modified since the time of its registration or the last time modified() was called.
bool stepClock::modified() { 
  bool ret = isModified;
  // Reset isModified to false since its state has already been checked.
  isModified = false;
  
  return ret;
}

std::string stepClock::str() const {
  ostringstream s;
  s << "[stepClock: ";
  for(int i=0; i<curStep.size(); i++) {
    if(i>0) s << ", ";
    s << curStep[i];
  }
  s << "]";
  return s.str();
}


  /*********************
   *** Scalar Clock ****
   *********************/

  std::set<mpiClock*> mpiClock::active;

  mpiClock::mpiClock() {
    // Only register this clock with sightObj if a timeClock is not already registered with it
   if(active.size()==0)
     {
       sightObj::addClock("mpiClock", this);
   //   active.insert(this);

       //  static bool initialized=false;
       // if(!initialized) {
       // initialized = true;
       PNMPI_modHandle_t handle;

      int err=PNMPI_Service_GetModuleByName("SClock-module",&handle);
      if (err!=MPI_SUCCESS){
	printf("error loading SClock-module \n");
	return;
      }

      PNMPIMOD_SClock_getSTime_t r_get;
      PNMPI_Service_descriptor_t s_get;

      err=PNMPI_Service_GetServiceByName(handle,"getSTime","p",&s_get);
      if (err!=MPI_SUCCESS){
	printf("error loading getSTime \n");
	return;
      }
      r_get=(PNMPIMOD_SClock_getSTime_t) s_get.fct;

      r_get(&curTime);
    }
   active.insert(this);

  }

  mpiClock::~mpiClock() {
    active.erase(this);
    // If this was the instance of mpiClock that was registered with sightObj                                                                                                                                                      
    if(sightObj::isActiveClock("mpiClock", this)) {
      // If there are other active instances, update the record to point to one of them
      if(active.size() > 0) sightObj::updClock("mpiClock", *active.begin());
      // Otherwise, unregister this clock
      else                  sightObj::remClock("mpiClock");
    }
  }

  properties* mpiClock::setProperties(properties* props) {
    assert(props);
    // Update the clock
    modified();

    // Store the currrent time in props
    map<string, string> pMap;
    pMap["time"] = txt()<<lastTime;
    props->add("mpiClock", pMap);
    return props;
  }

  // Returns true if the clock has been modified since the time of its registration or the last time modified() was called.            
  bool mpiClock::modified() {
  // Check the time
  // If the time has advanced since the last measurement, update it and return true                                 
    if(*curTime != lastTime) {
    lastTime = *curTime;
    return true;
    // If time has not advanced
    } else
    return false;
  }
  std::string mpiClock::str() const {
    return txt() << "[mpiClock: time="<<curTime<<"]";
  }



/*****************
 ***** clock *****
 ***************** /

int clock::maxClockID=0;

clock::clock(int globalID, properties* props) : 
  sightObj(setProperties(globalID, props), true), globalID(globalID) {
  clockID = maxClockID++;
}


// Returns the properties of this object
properties* clock::setProperties(int globalID, properties* props) {
  if(props==NULL) props = new properties();
  
  if(props->active && props->emitTag) {
    map<string, string> pMap;
    pMap["clockID"]   = txt()<<maxClockID;
    pMap["globalID"] = txt()<<globalID;

    props->add("clock", pMap);
  }
  
  return props;
}*/

/*****************************************
 ***** ClockMergeHandlerInstantiator *****
 *****************************************/
ClockMergeHandlerInstantiator::ClockMergeHandlerInstantiator() { 

  (*MergeHandlers   )["timeClock"]  = TimeClockMerger::create;
  (*MergeKeyHandlers)["timeClock"]  = TimeClockMerger::mergeKey;
  (*MergeHandlers   )["stepClock"]  = StepClockMerger::create;
  (*MergeKeyHandlers)["stepClock"]  = StepClockMerger::mergeKey;

  (*MergeHandlers   )["mpiClock"]  = MpiClockMerger::create;
  (*MergeKeyHandlers)["mpiClock"]  = MpiClockMerger::mergeKey;                                                      

  //  MergeGetStreamRecords->insert(&MpiClockGetMergeStreamRecord);
  MergeGetStreamRecords->insert(&StepClockGetMergeStreamRecord);
}
ClockMergeHandlerInstantiator ClockMergeHandlerInstance;

  std::map<std::string, streamRecord*> StepClockGetMergeStreamRecord(int streamID) {
    std::map<std::string, streamRecord*> mergeMap;
    mergeMap["stepClock"] = new StepClockStreamRecord(streamID);
    return mergeMap;
  }
  /*
  std::map<std::string, streamRecord*> MpiClockGetMergeStreamRecord(int streamID) {
    std::map<std::string, streamRecord*> mergeMap; 
    mergeMap["mpiClock"] = new MpiClockStreamRecord(streamID);
    return mergeMap;              
  }
  */

  /*************************** 
   ***** TimeClockMerger *****  
   ***************************/
TimeClockMerger::TimeClockMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        Merger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props))
{ }


// Sets the properties of the merged object
properties* TimeClockMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "timeClock");
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Clock!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Emit the local and global clock ID, which must be the same for all tags
    vector<string> t = getValues(tags, "time");
    assert(allSame<string>(t));
    pMap["time"] = txt()<<*t.begin();
  }
  props->add("timeClock", pMap);
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void TimeClockMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  properties::iterator blockTag = tag;
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Differentiate clocks according to their time
    info.add(properties::getInt(tag, "time"));
  }
}


/***************************
 ***** StepClockMerger *****
 ***************************/

StepClockMerger::StepClockMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         std::map<std::string, streamRecord*>& outStreamRecords,
                         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
        Merger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props))
{ }


// Sets the properties of the merged object
properties* StepClockMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  if(props==NULL) props = new properties();
  
  assert(tags.size()>0);
  vector<string> names = getNames(tags); assert(allSame<string>(names));
  assert(*names.begin() == "stepClock");
  
  map<string, string> pMap;
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Clock!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Merge the step clock IDs along all the streams
    streamRecord::mergeIDs("stepClock", "stepClockID", pMap, tags, outStreamRecords, inStreamRecords);
    
    // Emit the step vector, which must be the same in all tags
    vector<string> numDimsV = getValues(tags, "numDims");
    assert(allSame<string>(numDimsV));
    pMap["numDims"] = txt() << *numDimsV.begin();
    
    long numDims = properties::asInt(*numDimsV.begin());
    for(int i=0; i<numDims; i++) {
      vector<string> vals = getValues(tags, txt()<<"dim"<<i);
      assert(allSame<string>(vals));
      pMap[txt()<<"dim"<<i] = *vals.begin();
    }
  }
  props->add("stepClock", pMap);
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
void StepClockMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                               std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  properties::iterator blockTag = tag;
    
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
  if(type==properties::enterTag) {
    // Differentiate step clocks according to their step vector
    info.add(properties::getInt(tag, "numDims"));
    long numDims = properties::getInt(tag, "numDims");
    for(int i=0; i<numDims; i++)
      info.add(properties::getInt(tag, txt()<<"dim"<<i));
  }
}

  /**************************
   ***** MpiClockMerger *****
   **************************/

  MpiClockMerger::MpiClockMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
				 std::map<std::string, streamRecord*>& outStreamRecords,
				 std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
				 properties* props) :
    Merger(advance(tags), outStreamRecords, inStreamRecords,
	   setProperties(tags, outStreamRecords, inStreamRecords, props))
  { }

  // Sets the properties of the merged object                                                                                                           
  properties* MpiClockMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
					    map<string, streamRecord*>& outStreamRecords,
					    vector<map<string, streamRecord*> >& inStreamRecords,
					    properties* props) {
    if(props==NULL) props = new properties();

    assert(tags.size()>0);
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "mpiClock");

    map<string, string> pMap;
    properties::tagType type = streamRecord::getTagType(tags);
    if(type==properties::unknownTag) {
      cerr << "ERROR: inconsistent tag types when merging Clock!"<<endl; exit(-1); }
    if(type==properties::enterTag) {
      // Emit the local and global clock ID, which must be the same for all tags 
      vector<string> t = getValues(tags, "time");
      assert(allSame<string>(t));
      pMap["time"] = txt()<<*t.begin();
    }
    props->add("mpiClock", pMap);

    return props;
  }

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  void MpiClockMerger::mergeKey(properties::tagType type, properties::iterator tag,
				std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    properties::iterator blockTag = tag;

    if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when computing merge attribute key!"<<endl; exit(-1); }
    if(type==properties::enterTag) {
      // Differentiate clocks according to their time
      info.add(properties::getInt(tag, "time"));
    }
  }

}; // namespace structure 
}; // namespace sight
