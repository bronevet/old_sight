#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "../../sight_common.h"
#include "../../sight_structure_internal.h"

namespace sight {
namespace structure {

// A clock based on the current system time
class timeClock: public sightClock {
  private:
  struct timeval curTime;
  
  // Records all the currently active instance of timeClock. Since all instances of timeClock
  // correspond to the same real clock, we register the clock once for all currently active instances of timeClock.
  static ThreadLocalStorageSet<timeClock*> active;
  
  timeClock();
  
  ~timeClock();
  
  properties* setProperties(properties* props);
  
  // Returns true if the clock has been modified since the time of its registration or the last time modified() was called.
  bool modified();
  
  std::string str() const;
}; // class timeClock


// A clock to mark the application's progress through discrete steps of application execution, such as program phases
// or iterations of a loop nest.
class stepClock: public sightClock {
  private:
  // This stepClock's uniqueID
  static ThreadLocalStorage1<int, int> maxStepClockID;
  int stepClockID;
  
  // The current step of this clock
  std::vector<long> curStep;
    
  // Records whether this clock has been modified since its creation or the last time modified() was called;
  bool isModified;
  
  public:
  // Create a step clock with the given number of dimensions in its steps
  stepClock(int numDim);
  
  ~stepClock();
  
  properties* setProperties(properties* props);
  
  // Indicates that we've taken a step in the given dimension. This causes the clock's value at the given
  // dimension to be incremented and for all the subsequent dimensions to be set to 0
  void step(int dim);
  
  // Returns true if the clock has been modified since the time of its registration or the last time modified() was called.
  bool modified();
  
  std::string str() const;
}; // class stepClock

// A scalar clock for tracking causal order in concurrent applications
class scalarCausalClock: public sightClock {
  private:
  long long time;
  // The time value that was most recently reported
  long long lastTime;
  
  // Records all the currently active instance of timeClock. Since all instances of timeClock
  // correspond to the same real clock, we register the clock once for all currently active instances of timeClock.
  static ThreadLocalStorageSet<scalarCausalClock*> active;
 
  public: 
  scalarCausalClock();
  
  ~scalarCausalClock();
  
  properties* setProperties(properties* props);
  
  // Called when information/causality is sent from one thread to another.
  // Returns the current local scalar clock, enabling the calling sender to propagate it to the receiver.
  long long send();
  
  // Called when information/causality is received from one thread to another.
  // Takes as an argument the sender's clock at the time of the send operation.
  void recv(const long long& sendTime);
  
  // Returns true if the clock has been modified since the time of its registration or the last time modified() was called.
  bool modified();
  
  std::string str() const;
}; // class scalarCausalClock

class ClockMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  ClockMergeHandlerInstantiator();
};
extern ClockMergeHandlerInstantiator ClockMergeHandlerInstance;

std::map<std::string, streamRecord*> ClockGetMergeStreamRecord(int streamID);

std::map<std::string, streamRecord*> StepClockGetMergeStreamRecord(int streamID);

// Merger for timeClock tag
class TimeClockMerger : public Merger {
  public:
  TimeClockMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new TimeClockMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class TimeClockMerger

// Merger for stepClock tag
class StepClockMerger : public Merger {
  public:
  StepClockMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new StepClockMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class StepClockMerger

class StepClockStreamRecord: public streamRecord {
  friend class StepClockMerger;
  
  public:
  StepClockStreamRecord(int vID)              : streamRecord(vID, "stepClock") { }
  StepClockStreamRecord(const variantID& vID) : streamRecord(vID, "stepClock") { }
  StepClockStreamRecord(const StepClockStreamRecord& that, int vSuffixID) : streamRecord(that, vSuffixID) {}
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID) { return new StepClockStreamRecord(*this, vSuffixID); }
  
  std::string str(std::string indent="") const {
    std::ostringstream s;
    s << "[StepClockStreamRecord: ";
    s << streamRecord::str(indent+"    ") << "]";
    return s.str();
  }
}; // class StepClockStreamRecord

// Merger for scalarCausalClock tag
class ScalarCausalClockMerger : public Merger {
  public:
  ScalarCausalClockMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ScalarCausalClockMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class ScalarCausalClockMerger

} // namespace structure
} // namespace sight
