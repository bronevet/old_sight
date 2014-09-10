#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "utils.h"
#include "process.h"
#include "sight_structure.h"
using namespace sight;
using namespace sight::structure;

#define VERBOSE

namespace sight {

namespace merge {
  
// Records all the relevant information about how a group of tags should be merged
class groupStreams {
  public:
  // The indexes of the parsers from which this tag was read
  std::list<int> parserIndexes;
  
  groupStreams() { }
  groupStreams(const std::list<int>& parserIndexes): parserIndexes(parserIndexes) {}
  groupStreams(int parserIdx) : parserIndexes(1, parserIdx) {}
  groupStreams(int minParserIdx, int maxParserIdx) {
    for(int i=minParserIdx; i<maxParserIdx; i++)
      parserIndexes.push_back(i);
  }
  
  // Adds the information for a tag from a given parser
  void add(int parserIdx) {
    parserIndexes.push_back(parserIdx);
  }

  string str() const {
    ostringstream s;
    s << "[groupStreams: ";
    for(std::list<int>::const_iterator i=parserIndexes.begin(); i!=parserIndexes.end(); i++) {
      if(i!=parserIndexes.begin()) s << ",";
      s << *i;
    }
    s << "]";
    return s.str();
  }
}; // class groupStreams

// A unique signature that differentiates tags of incompatible types from each other
class tagGroup {
  public:
  properties::tagType type;
  std::string objName;
  // A list of key strings provided by the object-specific merger
  MergeInfo info;
  
  // Creates a tagGroup by invoking a MergeKeyHandler for the type and properties object of some tag on some incoming 
  // stream, where the current merging state on state stream is captured in inStreamRecord
  tagGroup(properties::tagType type, const properties* props, const std::map<std::string, streamRecord*>& inStreamRecord){
    this->type = type;
    objName = props->name();
    
    if(MergeHandlerInstantiator::MergeKeyHandlers->find(props->name()) == MergeHandlerInstantiator::MergeKeyHandlers->end()) { cerr << "ERROR: no merge handler for tag \""<<props->name()<<"\"!"<<endl; }
    assert(MergeHandlerInstantiator::MergeKeyHandlers->find(props->name()) != MergeHandlerInstantiator::MergeKeyHandlers->end());
    (*MergeHandlerInstantiator::MergeKeyHandlers)[props->name()](type, props->begin(), inStreamRecord, info); 
    
    #ifdef VERBOSE
    dbg << info.str()<<endl;
    #endif
  }
  
  bool operator==(const tagGroup& that) const
  { return type==that.type && objName==that.objName && info==that.info; }
  
  bool operator<(const tagGroup& that) const
  { return type< that.type ||
          (type==that.type && objName< that.objName) ||
          (type==that.type && objName==that.objName  && info<that.info); }

  // Returns the set of objNames within the keys of the given map
  static std::set<std::string> getObjNames(const std::map<tagGroup, groupStreams >& tag2stream) {
    std::set<std::string> objNames;
    for(std::map<tagGroup, groupStreams >::const_iterator i=tag2stream.begin(); i!=tag2stream.end(); i++)
      objNames.insert(i->first.objName);
    return objNames;
  }
  
  // Returns the set of tagTypes within the keys of the given map
  static std::set<properties::tagType> getTagTypes(const std::map<tagGroup, groupStreams >& tag2stream) {
    std::set<properties::tagType> tagTypes;
    for(std::map<tagGroup, groupStreams >::const_iterator i=tag2stream.begin(); i!=tag2stream.end(); i++)
      tagTypes.insert(i->first.type);
    return tagTypes;
  }

  string str() const {
    ostringstream s;
//cout << "objName="<<objName<<endl;
//cout << "info="<<info.str()<<endl;
    s << "[tagGroup: objName="<<objName<<", type="<<(type==properties::enterTag? "enter": "exit")<<", info="<<info.str()<<"]";
    return s.str();
  }
}; // class tagGroup

class MergeState {
  // Records whether this MergeState object was created fresh using the MergeState(const vector<FILEStructureParser*>& parsers) constructor
  // or was derived from another MergeState object using the MergeState(const MergeState& that, ...) constructor.
  bool derived;
  
  // Records whether the outStreamRecords in this object are a private copy or pointers to the outStreamRecords from the parent MergeState
  // (the inStreamRecords are always pointers)
  bool outStreamRecordsAreNew;
  
  public:
  // The parser objects from which we can read new tags on the incoming streams
  vector<baseStructureParser<FILE>*> parsers;
  // For each parser a flag indicating whether the parser is active (true) or has 
  // reached the end of its stream's contents (false)
  std::vector<bool> active;
  // For each parser a flag indicating whether the merging algorithm is ready to read
  // a new tag from the parser  
  std::vector<bool> readyForTag;
  
  // For each parser the tag that was most recently read on the parser but not yet processed
  vector<pair<properties::tagType, const properties*> > nextTag;
    
  // streamRecords that record the current merging state of all the widgets on the outgoing
  // stream and each of the incoming streams
  std::map<std::string, streamRecord*> outStreamRecords;
  std::vector<std::map<std::string, streamRecord*> > inStreamRecords;
  
  // Groups the tags in nextTag according to the values of their tagGroup objects,
  // produced based on the mergeInfo objects returned by their respective widget mergers.
  // Each tagGroup is mapped to a groupStreams object that records the indexes of the
  // parsers the next tags on which share the same tagGroup.
  map<tagGroup, groupStreams> tag2stream;
  
  // Depth of the stack of variants (we may encounter a variant inside a variant, inside a
  // comparison, etc.)
  int variantStackDepth;
  
  // The number of times we called mergeMultipleGroups() to create variant/comparison tags.
  // This makes it possible to give different sets of such tags unique IDs.
  int multGroupID;
  
  // The output dbgStream into which we'll write the merged log data
  structure::dbgStream* out;
  //mergeType mt;
  
  // Tracks the structural information needed to maintained a manageable debug
  // log for the merging process itself
#ifdef VERBOSE
  graph& g;
  anchor incomingA;
  anchor outgoingA;
#endif
  // Records whether a universal tag was read on each given parser
  vector<bool> readUniversalTag;
  
  MergeState(const vector<baseStructureParser<FILE>*>& parsers
             #ifdef VERBOSE
             , graph& g, anchor incomingA, anchor outgoingA
             #endif
            );
  
  // Create a new MergeState by focusing the given MergeState on just the parsers at indexes in gs.parserIndexes, all 
  // of which share the given tagGroup. The resulting state will be considered a variant with the given ID.
  // readyForNewTags - Whether the merger will be ready to read new tags on its incoming streams is controlled by readyForNewTags.
  // createNewOutStreamRecords - Whether we'll create new outStreamRecords objects for this MergeState or whether this MergeState
  //      will maintain pointers to that.outStreamRecords. (the inStreamRecords are always pointers)
  MergeState(const MergeState& that,
             const tagGroup& tg, const groupStreams& gs, int variantID, bool readyForNewTags, bool createNewOutStreamRecords
             #ifdef VERBOSE
             , const anchor& incomingA
             #endif
            );
            
  ~MergeState();
  
  /*************************************************************
   ***** Methods to track the state of the merging process *****
   *************************************************************/
  
  // Called to indicate that we're ready to read a tag on all the parsers in the given groupStreams
  // Updates both readyForTag and tag2stream.
  void readyForNextTag(const tagGroup& tg, const groupStreams& gs);
  
  // Called to indicate that we're ready to read a tag on all the parsers in the given groupStreams
  // Updates both tag2stream but not readyForTag.
  void readyForNextTag(const groupStreams& gs);
  
  // Called to indicate that we're ready to read a tag on all the parsers.
  void readyForNextTag();
  
  // Returns the number of true values in the given boolean vector.
  static int getNumTrues(const vector<bool>& v);
  
  // Returns the total number of parsers.
  int getNumParsers() const { return parsers.size(); }
  
  // Returns the number of active parsers
  int getNumActiveParsers() const;
  
  // Returns the number of parsers on which a universal tag is the current one.
  int getNumUniversalTags() const;
  
  // Return the number of tag groups with the given tag mergeKind
  int getNumTagGroupsByMergeKind(MergeInfo::mergeKindT mergeKind) const;
  
  // Returns whether the same universal tag is the current tag on all the streams.
  bool isSingleUniversal() const;
  
  // Returns whether all the tag groups correspond to entries to comparison tags
  bool isAllComparisonEntry() const;
  
  // Returns the number of tag groups among the current set of tags among all the incoming streams
  int getNumGroups() const;
  
  // Returns the number of tag groups with the given object name among the current set of tags among all the incoming streams
  int getNumGroupsByName(const std::string& objName) const;
  
  // Returns the tagGroup and tagStreams of the single tag group that has the given objectName
  std::pair<tagGroup, groupStreams> getObjNameTS(const std::string& objName) const;
  
  // Returns the number of tag groups that correspond to entries into tags among the current set of tags among all the incoming streams
  int getNumEnterGroups() const;
  
  // Returns the tagGroup and tagStreams of the single enter tag group
  std::pair<tagGroup, groupStreams> getEnterTS() const;
  
  // Returns the number of tag groups that correspond to exits from tags among the current set of tags among all the incoming streams
  int getNumExitGroups() const;
  
  // If isSingleUniversal() is true, returns the tagGroup, groupStreams, objectName or tag type of this tag.
  const tagGroup& getCommonTagGroup() const;
  const groupStreams& getCommonGroupStreams() const;
  const std::string& getCommonObjName() const;
  properties::tagType getCommonTagType() const;
    
  /***************************************
   ***** Management of streamRecords *****
   ***************************************/
  
  // Resume the streamRecord of outgoingStreams from the outgoingStreams all the given MergeStates
  void resumeFrom(const std::vector<MergeState*>& thats);
  
  // Resume the streamRecord of outgoingStreams from the outgoingStreams of the given MergeState
  void resumeFrom(const MergeState& that);
  
  /*************************************************************
   ***** Human-readable representations of data structures *****
   *************************************************************/
  
  template<typename T>
  void printVector(ostream& out, const std::vector<T>& v) const {
    out << "<";
    for(typename std::vector<T>::const_iterator i=v.begin(); i!=v.end(); i++) {
      if(i!=v.begin()) out << ", ";
      out << *i;
    }
    out << ">";
  }
  void printStateVectors(ostream& out) const;
  void printStreamRecords(ostream& out) const;
  void printTags(ostream& out) const;
  void printTag2Stream(ostream& out, const map<tagGroup, groupStreams>& tag2streamArg) const;
  void printTag2Stream(ostream& out) const { printTag2Stream(out, tag2stream); }

  /************************************************
   ***** Aggregation of tag group information *****
   ************************************************/

  // Returns the portion of tag2stream that corresponds to tag groups with non-universal enter tags
  map<tagGroup, groupStreams> filterTag2Stream_EnterNonUniversal() const;
  
  // Given a vector of entities and a list of indexes within the vector,
  // fills groupVec with just the entities at the indexes in selIdxes.
  template<class EltType>
  void collectGroupVectorIdx(const std::vector<EltType>& vec, const std::list<int>& selIdxes, std::vector<EltType>& groupVec);
  
  // Given a vector of entities and a vector of booleans that identify the selected indexes within the vector,
  // fills groupVec with just the entities at the indexes in selIdxes.
  template<class EltType>
  void collectGroupVectorBool(const std::vector<EltType>& vec, const std::vector<bool>& selFlags, std::vector<EltType>& groupVec);
  
  /*******************************************************
   ***** Merging of individual tags and log segments *****
   *******************************************************/
  
  // Reads the next tag on all the incoming stream parsers that are currently active and ready for the next tag.
  void readNextTag();
  
  // Run the tag merger function on the incoming streams with the given tagGroup and groupStreams and return 
  // a pointer to the resulting Merger object.
  Merger* mergeObject(const tagGroup& tg, const groupStreams& gs);
  
  // Given a vector of tag type/properties pairs, returns the same list but with the properties pointer
  // replaced with the iterator to the start of the properties list
  static std::vector<pair<properties::tagType, properties::iterator> > beginTags(
                                                   std::vector<pair<properties::tagType, const properties*> >& tags);
  
  // Invoke the merger on all the tags that share the given tagGroup, along the parsers recorded in the given groupStreams and 
  // - Emit the resulting merged tag to the outgoing stream
  // - Increment stackDepth if the tag is an entry and decrement if it is an exit
  // - If returnMerged is true, return a pointer to the Merger object that results from merging.
  //   Otherwise, deallocate it
  Merger* mergeTag(const tagGroup& tg, const groupStreams& gs, int& stackDepth, bool returnMerged=false);
  
  // Invoke the merger on the single next tag on the given parser and emit it directly to the outgoing stream.
  // Increment stackDepth if the tag is an entry and decrement if it is an exit.
  void mergeSingleTag(int parserIdx, int& stackDepth);
  
  // Run mergeTag() on all the tags that share the given tagGroup and then get ready to read more tags on the streams associated
  // with the tag group.
  Merger* mergeTagAndAdvance(const tagGroup& tg, const groupStreams& gs, int& stackDepth, bool returnMerged=false);
  
  // Invoke the merge() method on the contents of the current current tag in the log, from the tag's
  // enter until its exit, calling merge() multiple times as needed to deal with merge() exiting because
  // the depth if its stack returns to 0 (due to multiple complete enter/exit pairs inside this tag).
  // The current tag itself it not emitted to the outgoing stream
  void mergeInsideTag();
  
  // Creates a sub-directory within the current directory to hold the sub-log that belongs
  // to a log variant or one of the logs in a comparison tag
  // parentStream - the dbgStream pointer to the stream that contains the new one this function creates
  // label - a label that describes the type of sub-directory this is (e.g. "variant" or "comparison")
/*  // subDirCount - the number of sub-directories that have been created within the calling function.
  //    incremented during this function*/
  // Returns the pair:
  //    dbgStream* into which the contents of the sub-log should be written
  //    string that holds the path of the sub-directory
  virtual std::pair<structure::dbgStream*, std::string> createStructSubDir(structure::dbgStream* parentStream, std::string label/*, int& subDirCount*/);
  
  // Break the contents of all the key/value pairs in tags2stream into separate files, emitting to the
  // current outgoing stream a single tag that points to these new files. 
  // pointerTagName - the name of the pointer tag (currently either variant or comparison)
  // focustag2stream - the portion of tag2stream that is limited to the tag groups that should be merged
  // includeCurrentTag - indicates whether the current tag along a given stream should be included
  //    in the emitted output or not
  void mergeMultipleGroups(const string& pointerTagName, map<tagGroup, groupStreams> focustag2stream, bool includeCurrentTag
                           #ifdef VERBOSE
                           , const anchor& incomingA, set<anchor>& lastRecurA
                           #endif
                          );  
  
  // General merge algorithm full application logs and sub-logs
  void merge();

  virtual structure::dbgStream* createStream(properties* props, bool storeProps);

//  virtual MergeState* createGroupState(const MergeState& that,
//          const tagGroup& tg, const groupStreams& gs, int variantID, bool readyForNewTags, bool createNewOutStreamRecords
//#ifdef VERBOSE
//          , const anchor& incomingA
//#endif
//  );

};

} // namespace merge
} // namespace sight
