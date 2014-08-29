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
//#include "process.C"
#include "sight_structure.h"
using namespace std;
using namespace sight;
using namespace sight::structure;


//#define VERBOSE

// Records all the relevant information about how a group of tags should be merged
class StreamTags {
  public:
  // The indexes of the parsers from which this tag was read
  list<int> parserIndexes;
  // Records whether this tag can only be merged if it appears with the same key on all the incoming streams
  bool universal;
  
  StreamTags() : universal(false) { }
  StreamTags(const list<int>& parserIndexes, bool universal=false): parserIndexes(parserIndexes), universal(universal) {}
  StreamTags(int parserIdx, bool universal=false) : parserIndexes(1, parserIdx), universal(universal) {}
  
  // Adds the information for a tag from a given parser
  void add(MergeInfo& info, int parserIdx) {
    universal = universal || info.getUniversal();
    parserIndexes.push_back(parserIdx);
  }

  string str() {
    ostringstream s;
    for(list<int>::iterator i=parserIndexes.begin(); i!=parserIndexes.end(); i++) {
      if(i!=parserIndexes.begin()) s << ",";
      s << *i;
    }
    return s.str();
  }
}; // class StreamTags

// A unique signature that differentiates tags of incompatible types from each other
class tagGroup {
  public:
  properties::tagType type;
  std::string objName;
  // A list of key strings provided by the object-specific merger
  MergeInfo::mergeKey key;
  
  tagGroup(properties::tagType type, const properties* props, MergeInfo::mergeKey key) {
    this->type = type;
    objName = props->name();
/*    if(properties::exists(props->begin(), "callPath")) {
    	//dbg << objName<*<": "<<properties::get(props->begin(), "callPath")<<endl;
      cp = make_path(properties::get(props->begin(), "callPath"));
    }*/
    this->key = key;
  }
  
  bool operator==(const tagGroup& that) const
  { return type==that.type && objName==that.objName && ((key.size()==0 && that.key.size()==0) || key==that.key); }
  
  bool operator<(const tagGroup& that) const
  { return type< that.type ||
          (type==that.type && objName< that.objName) ||
          (type==that.type && objName==that.objName  && key<that.key); }

  // Returns the set of objNames within the keys of the given map
  static std::set<std::string> getObjNames(const std::map<tagGroup, StreamTags >& tag2stream) {
    std::set<std::string> objNames;
    for(std::map<tagGroup, StreamTags >::const_iterator i=tag2stream.begin(); i!=tag2stream.end(); i++)
      objNames.insert(i->first.objName);
    return objNames;
  }
  
  // Returns the set of tagTypes within the keys of the given map
  static std::set<properties::tagType> getTagTypes(const std::map<tagGroup, StreamTags >& tag2stream) {
    std::set<properties::tagType> tagTypes;
    for(std::map<tagGroup, StreamTags >::const_iterator i=tag2stream.begin(); i!=tag2stream.end(); i++)
      tagTypes.insert(i->first.type);
    return tagTypes;
  }

  string str() const {
    ostringstream s;
    s << "[tagGroup: objName="<<objName<<", type="<<(type==properties::enterTag? "enter": "exit")<<", key=";
    for(list<attrValue>::const_iterator k=key.begin(); k!=key.end(); k++) {
      if(k!=key.begin()) s << ", ";
      s << k->getAsStr();
    }
    s << "]";
    return s.str();
  }
}; // class tagGroup

// The different types of merging 
typedef enum {commonMerge, // merge the common parts of the logs
              zipper, // gather data from all the logs without merging any common parts
              diff    // merge the common log parts, while highlighting their differences
             } mergeType;

// Returns the string representation of the given mergeType
string mergeType2Str(mergeType mt) {
  switch(mt) {
    case commonMerge: return "common";
    case zipper: return "zipper";
    case diff:   return "diff";
  }
  assert(0);
  return "";
}

// Given a string repreesntation of the given mergeType, returns the corresponding enum
mergeType str2MergeType(string mtStr) {
  if(mtStr == "common") return commonMerge;
  if(mtStr == "zipper") return zipper;
  if(mtStr == "diff")   return diff;
  cerr << "ERROR: Unknown merge type \""<<mtStr<<"\"!"<<endl;
  assert(0);
}

// parsers - Vector of parsers from which information will be read
// nextTag - If merge() is called recursively after a given tag is entered on some but not all the parsers,
//    contains the information of this entered tag.
// readyForTag - Records whether we're ready to read another tag from each parser
// activeParser - Records whether each parser is still active or whether we've reached its end and the 
//    number of active parsers
// tag2stream - Maps the next observed tag name/type to the input streams on which tags that match 
//    this signature were read
// numTextTags - Records the number of parsers on which the last read tag was text. We alternate between reading text 
//    and reading tags and if text is read from some but not all parsers, the contributions from the other 
//    parsers are considered to be the empty string.
// variantStackDepth - depth of calls to merge(). Each level of recursion corresponds to some structural
//    difference between the different output streams where in one parser we enter a tag and in another
//    we do not. For each such difference we call merge() recursively on all the parsers that enter a tag
//    and exit this call when we exit this tag.
// out - stream to which data will be written
// targetStackDepth - Sets the depth of the object nesting stack at which we'll exit merge. 
//    If this is set to 0 this means that we'll exit when object entries and exits are balanced.
//    If it is set to -1 this means that we'll exit when we've exited from the object that contains the 
//      current region of the log and will not exit when the entries/exits of the objects inside of it
//      have become balanced.
//
// Returns the number of tags emitted during the course of this merge.
int merge(vector<FILEStructureParser*>& parsers, 
                   vector<pair<properties::tagType, const properties*> >& nextTag, 
                   std::map<std::string, streamRecord*>& outStreamRecords,
                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                   std::vector<bool>  readyForTag,
                   std::vector<bool>& activeParser,
                   map<tagGroup, StreamTags > tag2stream,
                   int numTextTags,
                   int variantStackDepth,
                   structure::dbgStream* out, 
                   mergeType mt,
                   int targetStackDepth,
#ifdef VERBOSE
                   graph& g, anchor incomingA, anchor& outgoingA,
#endif
                   string indent);

// After the merging process enters some tag on some incoming stream it may call
// this function to directly copy the entire contents of this tag to the given 
// outgoing stream out. These contents are not merged with any other stream but
// any updates recorded in outStreamRecords will subsequently be incorporated by 
// the calling function.
// Returns whether the parser was active at the end of the log region.
bool emitLogRegion(FILEStructureParser* parsers, 
                  std::map<std::string, streamRecord*>& outStreamRecords,
                  std::map<std::string, streamRecord*>& inStreamRecords,
                  structure::dbgStream& out, 
#ifdef VERBOSE
                  graph& g, anchor incomingA, anchor& outgoingA,
#endif
                  string indent);

// Given a vector of entities and a list of indexes within the vector,
// fills groupVec with just the entities at the indexes in selIdxes.
template<class EltType>
void collectGroupVectorIdx(std::vector<EltType>& vec, const std::list<int>& selIdxes, std::vector<EltType>& groupVec);

// Given a vector of entities and a vector of booleans that identify the selected indexes within the vector,
// fills groupVec with just the entities at the indexes in selIdxes.
template<class EltType>
void collectGroupVectorBool(std::vector<EltType>& vec, const std::vector<bool>& selFlags, std::vector<EltType>& groupVec);

void printStreamRecords(ostream& out, 
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        string indent);
void printStreamRecords(ostream& out, 
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::map<std::string, streamRecord*>& inStreamRecords,
                        string indent);
void printTags(ostream& out,
               std::vector<bool>& activeParser,
               vector<pair<properties::tagType, const properties*> >& tags, 
               string indent);
//#define VERBOSE

int main(int argc, char** argv) {
  if(argc<3) { cerr<<"Usage: hier_merge outDir mergeType [fNames]"<<endl; exit(-1); }
  vector<FILEStructureParser*> fileParsers;
  const char* outDir = argv[1];

  mergeType mt = str2MergeType(string(argv[2]));
  for(int i=3; i<argc; i++) {
    fileParsers.push_back(new FILEStructureParser(argv[i], 10000));
  }
  
  #ifdef VERBOSE
  SightInit(argc, argv, "hier_merge", txt()<<outDir<<".hier_merge");
  dbg << "#fileParserRefs="<<fileParsers.size()<<endl;
  #else
  SightInit_LowLevel();
  #endif
  
  // Set the working directory in the dbgStreamMerger class (it is a static variable). This must be done before an instance of this class is created
  // since the first and only instance of this class will read this working directory and write all output there.
  dbgStreamMerger::workDir = string(outDir);
    
  vector<pair<properties::tagType, const properties*> > emptyNextTag;
    
  // Initialize the streamRecords for the incoming and outgoing stream. The variant
  // ID starts as 0 as will grow deeper as we discover unmergeable differences
  // between or within the input streams.
  std::map<std::string, streamRecord*> outStreamRecords = MergeHandlerInstantiator::GetAllMergeStreamRecords(0);
  dbgStreamStreamRecord::enterBlock(outStreamRecords);
  
  std::vector<std::map<std::string, streamRecord*> > inStreamRecords;
  for(int i=0; i<fileParsers.size(); i++) {
    inStreamRecords.push_back(MergeHandlerInstantiator::GetAllMergeStreamRecords(i));
  }
  dbgStreamStreamRecord::enterBlock(inStreamRecords);
  
  // Records that we're ready to read another tag from each parser
  vector<bool> readyForTagFromAll(fileParsers.size(), true);
  // Records that each parser is still active
  vector<bool> allParsersActive(fileParsers.size(), true);
  
  // Maps the next observed tag name/type to the input streams on which tags that match this signature were read
  map<tagGroup, StreamTags > tag2stream;
  
  // Records the number of parsers on which the last read tag was text. We alternate between reading text 
  // and reading tags and if text is read from some but not all parsers, the contributions from the other 
  // parsers are considered to be the empty string.
  int numTextTags = 0;

#ifdef VERBOSE
  graph g;
  anchor outgoingA; 
#endif
  merge(fileParsers, emptyNextTag, outStreamRecords, inStreamRecords, 
        readyForTagFromAll, allParsersActive,
        tag2stream, numTextTags,
        0, // variantStackDepth
        NULL, // output stream
        mt, 
        0, // targetStackDepth
#ifdef VERBOSE
        g, anchor::noAnchor, outgoingA,
#endif
        "");
  
  // Close all the parsers and their files
  for(vector<FILEStructureParser*>::iterator p=fileParsers.begin(); p!=fileParsers.end(); p++)
    delete *p;
  
  return 0;
}

// Given a vector of tag type/properties pairs, returns the same list but with the properties pointer
// replaced with the iterator to the start of the properties list
vector<pair<properties::tagType, properties::iterator> > beginTags(
                           vector<pair<properties::tagType, const properties*> >& tags)
{
  vector<pair<properties::tagType, properties::iterator> > ret;
  for(vector<pair<properties::tagType, const properties*> >::iterator t=tags.begin(); t!=tags.end(); t++)
    ret.push_back(make_pair(t->first, t->second->begin()));
  return ret;
}

// Merge the tags on the given incoming streams, emitting the merged tags to the outgoing stream, if this
// is requested by the merger. Returns the number of tags emitted.
int mergeTags(properties::tagType type, string objName,
              vector<pair<properties::tagType, const properties*> >& nextTag, 
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              int& stackDepth, structure::dbgStream& out,
              string indent) {
  // Merge the properties of all tags
  //Merger* m = mergers[objName]->merge(beginTags(nextTag), outStreamRecords, inStreamRecords);
  Merger* m = (*MergeHandlerInstantiator::MergeHandlers)[objName](beginTags(nextTag), outStreamRecords, inStreamRecords, NULL);
  #ifdef VERBOSE
  {scope s(txt()<<"merged "<<(m->getProps().size()>0?m->getProps().name():"???"), scope::medium); 
  dbg << indent << m->getProps().str()<<endl;
  if(objName == "sight")
    dbg << indent << "dir="<<structure::dbg->workDir<<endl; }
  dbg << indent << "emit="<<m->emitTag()<<", #moreTagsBefore="<<m->moreTagsBefore.size()<<", #moreTagsAfter="<<m->moreTagsAfter.size()<<endl;
  
  //printStreamRecords(dbg, outStreamRecords, inStreamRecords, indent+"   ;");
  #endif
  
  int numTagsEmitted;
  
  // If the merger requests that this tag be emitted, do so
  if(m->emitTag()) {
    // Emit all the tags that appear before the tag that was actually read
    for(list<pair<properties::tagType, properties> >::iterator t=m->moreTagsBefore.begin(); t!=m->moreTagsBefore.end(); t++) {
    	//dbg << "before: "<<(t->first == properties::enterTag? "enter": "exit")<<": "<<t->second.str()<<endl;
           if(t->first == properties::enterTag) out.enter(t->second);
      else if(t->first == properties::exitTag)  out.exit (t->second);
    }
    
    // Perform the common action of entering/exiting this tag
    if(objName == "text") {
      out << properties::get(m->getProps().find("text"), "text");
    } else {
      if(type == properties::enterTag) {
      	#ifdef VERBOSE
      	{ scope s("Entering", scope::medium); dbg<< "props="<<m->getProps().str()<<"\n"; }
        #endif
        stackDepth++;
        out.enter(m->getProps());

        ((dbgStreamStreamRecord*)outStreamRecords["sight"])->push(true);
      } else {
        #ifdef VERBOSE
      	{ scope s("Exiting", scope::medium); dbg<< "props="<<m->getProps().str()<<"\n"; }
      	#endif
        stackDepth--;
        // Emit the exit portion of this tag only if the entry was
        if(((dbgStreamStreamRecord*)outStreamRecords["sight"])->pop())
          out.exit(m->getProps());
      }
    }
    
    // Emit all the tags that appear after the tag that was actually read
    for(list<pair<properties::tagType, properties> >::iterator t=m->moreTagsAfter.begin(); t!=m->moreTagsAfter.end(); t++) {
           if(t->first == properties::enterTag) out.enter(t->second);
      else if(t->first == properties::exitTag)  out.exit (t->second);
    }
    
    numTagsEmitted=1;
  // If we don't need to emit the tag, still adjust the stackDepth to account for the fact
  // that the tag was read
  } else {
    if(objName != "text") {
      if(type == properties::enterTag) {
      	//dbg << "Entering props="<<m->getProps().str()<<"\n";
        stackDepth++;
        ((dbgStreamStreamRecord*)outStreamRecords["sight"])->push(false);
      } else {
      	//dbg << "Exiting props="<<m->getProps().str()<<"\n";
        stackDepth--;
        ((dbgStreamStreamRecord*)outStreamRecords["sight"])->pop();
      }
    }
    numTagsEmitted=0;
  }
  delete(m);
  return numTagsEmitted;
}

// Emit the tags on the given incoming streams to the outgoing stream in the order of the incoming streams
// in nextTag.
void zipperTags(properties::tagType type, string objName,
                vector<pair<properties::tagType, const properties*> >& nextTag, 
                std::map<std::string, streamRecord*>& outStreamRecords,
                std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                int& stackDepth, structure::dbgStream& out,
                string indent) {
  
  for(vector<pair<properties::tagType, const properties*> >::iterator t=nextTag.begin(); 
      t!=nextTag.end(); t++) {
    if(t->first == properties::enterTag) {
      #ifdef VERBOSE
    	dbg << "zipperTags: Entering props="<<t->second->str()<<"\n";
      #endif
      stackDepth++;
      out.enter(*t->second);
    } else if(t->first == properties::exitTag) {
      #ifdef VERBOSE
    	dbg << "zipperTags: Exiting props="<<t->second->str()<<"\n";
      #endif
      stackDepth--;
      out.exit(*t->second);
    }
  }
}

// Creates a sub-directory withini the current directory to hold the sub-log that belongs
// to a log variant or one of the logs in a comparison tag
// parentStream - the dbgStream pointer to the stream that contains the new one this function creates
// label - a label that describes the type of sub-directory this is (e.g. "variant" or "comparison")
// subDirCount - the number of sub-directories that have been created within the calling function.
//    incremented during this function
// Returns the pair:
//    dbgStream* into which the contents of the sub-log should be written
//    string that holds the path of the sub-directory
std::pair<structure::dbgStream*, std::string> createStructSubDir(structure::dbgStream* parentStream, std::string label, int& subDirCount) {
  string subDir = txt()<<parentStream->workDir<<"/"<<label<<"_"<<subDirCount;
  //dbg << "subDir="<<subDir<<endl;

  // Create the directory structure for the structural information
  // Main output directory
  createDir(subDir, "");

  // Directory where client-generated images will go
  string imgDir = createDir(subDir, "html/dbg_imgs");

  // Directory that widgets can use as temporary scratch space
  string tmpDir = createDir(subDir, "html/tmp");

  //dbg << "Creating groupStream\n";
  structure::dbgStream* groupStream = new structure::dbgStream(NULL, txt()<<"Compare "<<subDirCount, subDir, imgDir, tmpDir);
  
  subDirCount++;
  
  return make_pair(groupStream, subDir);
}

// Creates a copy of the outgoing stream records map OutSR, adds it to all allOutSRs vector and returns it
// variantID: unique ID of the stream variant that we're creating
std::map<std::string, streamRecord*>
    createOutSRCopy(const std::map<std::string, streamRecord*>& OutSR,
                    int variantID,
                     vector<std::map<std::string, streamRecord*> >& allOutSRs) {
  // Create the copy
  std::map<std::string, streamRecord*> OutSRCopy;
  for(std::map<std::string, streamRecord*>::const_iterator o=OutSR.begin(); o!=OutSR.end(); o++)
    OutSRCopy[o->first] = o->second->copy(variantID);
  
  // Add it
  allOutSRs.push_back(OutSRCopy);
  
  return OutSRCopy;
}

// After processing multiple variant or comparable logs this function combines their 
// intermediate state into one and emits a tag of the appropriate type documenting
// the locations of all the sub-files.
// parentStream - the dbgStream to which we'll write the merged tag
// outStreamRecords - the stream records on the outgoing streams that will be updated
// allOutSRs - the stream records from all the individual streams that will be used to update outStreamRecords
// subDirs - paths of all the sub-log directories
// tagName - the name of the tag that will be emitted
// pMap - the initial properties map of the tag to which we'll add the sub-directory paths
void completeSubLogProcessing(structure::dbgStream* parentStream, 
                              std::map<std::string, streamRecord*>& outStreamRecords, 
                              std::vector<std::map<std::string, streamRecord*> >& allOutSRs,
                              std::vector<std::string>& subDirs,
                              const std::string& tagName,
                              const std::map<std::string, std::string>& initialPMap) {
  // Resume the streamRecord of the outgoing stream from the sub-streams of the group's variants
  for(map<string, streamRecord*>::iterator o=outStreamRecords.begin(); o!=outStreamRecords.end(); o++) {
    o->second->resumeFrom(allOutSRs);
    // Delete the sub-streams streamRecords for each variant in this group
    for(vector<std::map<std::string, streamRecord*> >::iterator i=allOutSRs.begin(); 
        i!=allOutSRs.end(); i++)
    delete (*i)[o->first];
  }

  // If we produced any output tags within the above variants
  if(subDirs.size()>0) {        
    // Output the [variant] tag that points to the directories that hold the contents of each variant
    //out.ownerAccessing();
    properties props;
    map<string, string> pMap = initialPMap;
    pMap["numSubDirs"] = txt()<<subDirs.size();
    for(int v=0; v<subDirs.size(); v++)
      pMap[txt()<<"sub_"<<v] = subDirs[v];
    props.add(tagName, pMap);
    parentStream->tag(props);
  }
  dbgStreamStreamRecord::exitBlock(outStreamRecords);
}

// Function called when it is necessary to generate a separate log for a set of incoming streams.
// This may happen when we encounter 
//   - the entry to a "sight" tag and are explicitly asked to not merge logs
//   - a "comparison" tag, which explicitly indicates that the portion of the logs 
//     inside the tag should shown separately rather than merged
// This function calls merge() on each log separately and places pointers in the original
// log to the files generated for each sub-log.
void separateMergeForEachParser(vector<FILEStructureParser*>& parsers, 
                                vector<pair<properties::tagType, const properties*> >& nextTag, 
                                std::map<std::string, streamRecord*>& outStreamRecords,
                                std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                std::vector<bool>  readyForTag,
                                std::vector<bool>& activeParser,
                                // This call must be focused on a single key->value pair in the
                                // caller's instance of tag2stream (organizes the current tags on the 
                                // incoming streams according to their structural similarity)
                                // sepGroup: the key in tag2stream that describes the commonality between these tags
                                const tagGroup& sepGroup, 
                                // sepTags: identifies how they should be merged
                                const StreamTags& sepTags,
                                int numTextTags,
                                int variantStackDepth,
                                structure::dbgStream* out, 
                                mergeType mt,
                                int& numActive,
                                int& subDirCount,
                                const std::string& tagName, const std::map<std::string, std::string>& initialPMap,
#ifdef VERBOSE
                                graph& g, anchor incomingA, anchor& outgoingA,
#endif
                                string indent) {
  // The processing of the comparison tag on each incoming stream group gets a separate copy of 
  // outStreamRecords. This list stores them all so that we can later merge them.
  vector<std::map<std::string, streamRecord*> > allGroupOutStreamRecords;

  // Sub-directories that hold the contents of all the comparison sub-logs
  vector<string> compSubDirs;

  dbgStreamStreamRecord::enterBlock(outStreamRecords);
  
  // Iterate over all the active parsers
  for(list<int>::const_iterator p=sepTags.parserIndexes.begin();
      p!=sepTags.parserIndexes.end(); p++) {
#ifdef VERBOSE
    scope sepS(txt()<<"Separate merge for parser "<<*p);
#endif
    
    // Create a sub-directory and stream for the comparable log segment from the current incoming stream
    pair<structure::dbgStream*, string> ret = createStructSubDir(out, "comparison", subDirCount);
    dbgStream* groupStream = ret.first;
    string subDir = ret.second;

    /* // Contains just the current parser
    vector<FILEStructureParser*> groupParsers(1, parsers[*p]);
    // Contains the next read tag on the current parser
    vector<pair<properties::tagType, const properties*> > groupNextTag(1, nextTag[*p]);
    // Create a copy of outStreamRecords for this parser and add it to allGroupOutStreamRecords
    std::map<std::string, streamRecord*> groupOutStreamRecords = 
              createOutSRCopy(outStreamRecords, *p, allGroupOutStreamRecords);
    // The streamRecords of the incoming streams from the current parser
    std::vector<std::map<std::string, streamRecord*> > groupInStreamRecords(1, inStreamRecords[*p]);
    // The activeParser record for just the current parser (must be true)
    std::vector<bool> groupActiveParser(1, activeParser[*p]);
    // We are ready for another tag on any stream within this parser.
    std::vector<bool> groupNotReadyForTag(1, true);

    // The tag2stream map of this parser, which maps all the group's streams to the
    // common tag
    map<tagGroup, StreamTags > groupTag2stream;
    //groupTag2stream[sepGroup] = StreamTags(*p, true);

    //dbg << "start outLocation="<<((dbgStreamStreamRecord*)groupOutStreamRecords["sight"])->getLocation().str()<<endl;
    int numVariantTagsEmitted = 
      merge(groupParsers, groupNextTag, 
            groupOutStreamRecords, groupInStreamRecords, 
            groupNotReadyForTag, groupActiveParser,
            groupTag2stream,
            numTextTags,
            variantStackDepth+1, groupStream, 
            mt, 
#ifdef VERBOSE
            g, incomingA, outgoingA,
#endif
            txt()<<indent<<"| "<<*p);
    */
    
    assert(activeParser[*p]);
    
    std::map<std::string, streamRecord*> groupOutStreamRecords = 
              createOutSRCopy(outStreamRecords, *p, allGroupOutStreamRecords);
    
    // Returns whether the parser was active at the end of the log region
    activeParser[*p] = 
            emitLogRegion(parsers[*p], 
                          groupOutStreamRecords, inStreamRecords[*p],
                          *groupStream, 
#ifdef VERBOSE
                          g, incomingA, outgoingA,
#endif
                          indent+"    ");

    // If we emitted at least one tag within this variant, we record this variant 
    // to include it in the [variants] tag that points to it.
    //if(numVariantTagsEmitted>0) {
      compSubDirs.push_back(subDir);
    /*  //subDirCount++;
    // Otherwise, if this variant is empty, we delete it
    } else {
      rmdir(subDir.c_str());
    }*/

    delete groupStream;
/*
    // Update activeParsers[] based on the reading activity that occurred inside the merge() call.
    // If entry/exit tags were balanced we'd be guaranteed that all of the group's parsers would
    // be active but since the log generating application may terminate prematurely, we may hit
    // the end of its log inside this merge call.
    // If the current parser was previously active and has now terminated, update state
    if(groupActiveParser[0]==false) {
      activeParser[*p]=false;
      numActive--;
    }*/
    if(activeParser[*p]==false)
      numActive--;
  } // foreach tags2stream.begin()->parserIndexes
  
  // Complete processing of sub-logs to be compared, merging their outgoing stream records into one
  // and emitting the comparison tag that refers to their sub-directories

  // Then use the properties of the merged object to emit the merged comparison tag
  completeSubLogProcessing(out, outStreamRecords, allGroupOutStreamRecords, compSubDirs, 
                           tagName, initialPMap);
}

#ifdef VERBOSE
#define ITER_ACTION(text)  \
    scope actionS(text);   \
    if(lastIterA  != anchor::noAnchor) g.addDirEdge(lastIterA,  actionS.getAnchor()); \
    if(incomingA  != anchor::noAnchor) g.addUndirEdge(incomingA,  actionS.getAnchor()); \
    if(lastRecurA != anchor::noAnchor) g.addUndirEdge(lastRecurA, actionS.getAnchor()); \
    incomingA  = anchor::noAnchor;     \
    lastIterA  = actionS.getAnchor();  \
    curIterA   = actionS.getAnchor();  \
    outgoingA  = actionS.getAnchor();  \
    lastRecurA = anchor::noAnchor;
#else
#define ITER_ACTION(text) 
#endif

// parsers - Vector of parsers from which information will be read
// nextTag - If merge() is called recursively after a given tag is entered on some but not all the parsers,
//    contains the information of this entered tag. The contents of nextTag may change as a result to
//    the call to merge() because parsers are allowed to overwrite the provided properties* objects inside
//    nextTag with information read from incoming streams.
// readyForTag - Records whether we're ready to read another tag from each parser
// activeParser - Records whether each parser is still active or whether we've reached its end and the 
//    number of active parsers
// tag2stream - Maps the next observed tag name/type to the input streams on which tags that match 
//    this signature were read
// numTextTags - Records the number of parsers on which the last read tag was text. We alternate between reading text 
//    and reading tags and if text is read from some but not all parsers, the contributions from the other 
//    parsers are considered to be the empty string.
// variantStackDepth - depth of calls to merge(). Each level of recursion corresponds to some structural
//    difference between the different output streams where in one parser we enter a tag and in another
//    we do not. For each such difference we call merge() recursively on all the parsers that enter a tag
//    and exit this call when we exit this tag.
// out - stream to which data will be written
// targetStackDepth - Sets the depth of the object nesting stack at which we'll exit merge. 
//    If this is set to 0 this means that we'll exit when object entries and exits are balanced.
//    If it is set to -1 this means that we'll exit when we've exited from the object that contains the 
//      current region of the log and will not exit when the entries/exits of the objects inside of it
//      have become balanced.
//
// Returns the number of tags emitted during the course of this merge.
int merge(vector<FILEStructureParser*>& parsers, 
          vector<pair<properties::tagType, const properties*> >& nextTag, 
          std::map<std::string, streamRecord*>& outStreamRecords,
          std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
          std::vector<bool>  readyForTag,
          std::vector<bool>& activeParser,
          map<tagGroup, StreamTags > tag2stream,
          int numTextTags,
          int variantStackDepth,
          structure::dbgStream* out, 
          mergeType mt,
          int targetStackDepth,
#ifdef VERBOSE
          graph& g, anchor incomingA, anchor& outgoingA,
#endif
          string indent) {
  #ifdef VERBOSE
  scope s("Merge", variantStackDepth==0? scope::medium: scope::medium);
  dbg << "#parsers="<<parsers.size()<<", variantStackDepth="<<variantStackDepth<<", dir="<<(out? out->workDir: "???")<<endl;
  #endif
  //cout << "#parsers="<<parsers.size()<<", variantStackDepth="<<variantStackDepth<<", dir="<<(out? out->workDir: "???")<<endl;
  int numTagsEmitted = 0;
  
  // If this is not the root call to merge (variantStackDepth>1), nextTag is set to the tag entry items that were
  // just read on every parser in parsers. Otherwise, nextTag is empty
  assert((variantStackDepth==0 && nextTag.size()==0) ||
         (variantStackDepth>0  && nextTag.size()==parsers.size()));
  
  /* // The current depth of the nesting stack of tags that have been entered but not yet exited. If nextTag
  // is not empty, stackDepth is set to 1 to account for the entry tag read by the caller of merge(). 
  // Otherwise stackDepth is set to 0.
  // ?!?!?! This was previously commented out. Why? What bug does this cause?
  // ?!?!?! We need it to account for cases where merge() is called from 
  // ?!?!?! separateMergeForEachParser() and we have log regions 
  int stackDepth=(variantStackDepth==0? 0: 1);*/
  int stackDepth=0;
  
  int numActive = 0;
  for(vector<bool>::const_iterator p=activeParser.begin(); p!=activeParser.end(); p++)
    numActive += (*p? 1: 0);
  
  // Counts the number of times we recursively consider variant tags. The output of processing
  // such variants needs to be written to a separate uniquely-named location. These unique names
  // are generated using this counter.
  int subDirCount=0;
 
  anchor lastIterA  = anchor::noAnchor;
  anchor curIterA   = anchor::noAnchor;
  anchor lastRecurA = anchor::noAnchor;
  while(numActive>0) {
    #ifdef VERBOSE
    //dbg << "=================================, numActive="<<numActive<<"\n";
    scope loopS(txt()<<"Loop depth="<<variantStackDepth<<", numActive="<<numActive);
    { scope streamS("streamRecords", scope::high); printStreamRecords(dbg, outStreamRecords, inStreamRecords, ""); }
    #endif
    
    // Read the next item from each parser
    int parserIdx=0;
    
    // The number of sight enter/exit tags read on the incoming stream (key for initialization of the output stream)
    int numSightEnterTags=0;
    int numSightExitTags=0;
    
    // Read the next tag on each parser, updating nextTag and tag2stream
    for(vector<FILEStructureParser*>::iterator p=parsers.begin(); p!=parsers.end(); p++, parserIdx++) {
      #ifdef VERBOSE
      dbg << "readyForTag["<<parserIdx<<"]="<<readyForTag[parserIdx]<<", activeParser["<<parserIdx<<"]="<<activeParser[parserIdx]<<endl;
      #endif
      
      // If we're ready to read a tag on this parser
      if(readyForTag[parserIdx] && activeParser[parserIdx]) {
        pair<properties::tagType, const properties*> props = (*p)->next();
        #ifdef VERBOSE
        {scope s(txt()<</*indent << "| "<<*/parserIdx << ": "<<
                       (props.first==properties::enterTag? "enter": "exit")<<" "<<
                       (props.second->size()>0? props.second->name(): "???"), scope::high);
        dbg << const_cast<properties*>(props.second)->str()<<endl;}
        #endif
        
        // If we've reached the end of this parser's data
        if(props.second->size()==0) {
          activeParser[parserIdx] = false;
          numActive--;
        } else {
          // Record the properties of the newly-read tag
          
          // If nextTag has not yet been filled in upto this index, add the info to the end of nextTag
          if(nextTag.size()<=parserIdx) {
            assert(nextTag.size()==parserIdx);
            nextTag.push_back(props);
          // Otherwise, set this index of nextTag directly
          } else 
            nextTag[parserIdx] = props;
            
          // Group this parser with all the other parsers that just read a tag with the same name and type (enter/exit)
//          dbg << "props.second="<<props.second->str()<<endl;
          //dbg << "MergeHandlerInstantiator="<<MergeHandlerInstantiator::str()<<endl;
          //std::list<std::string> key = mergers[props.second->name()]->mergeKey(props.first, props.second->begin(), inStreamRecords[parserIdx]);
          if(MergeHandlerInstantiator::MergeKeyHandlers->find(props.second->name()) == MergeHandlerInstantiator::MergeKeyHandlers->end()) { cerr << "ERROR: no merge handler for tag \""<<props.second->name()<<"\"!"<<endl; }
          assert(MergeHandlerInstantiator::MergeKeyHandlers->find(props.second->name()) != MergeHandlerInstantiator::MergeKeyHandlers->end());
          MergeInfo info;
          (*MergeHandlerInstantiator::MergeKeyHandlers)[props.second->name()](props.first, props.second->begin(), inStreamRecords[parserIdx], info);

          #ifdef VERBOSE
          dbg << info.str()<<endl;
          #endif

/*          dbg << "key=";
          for(list<string>::iterator k=key.begin(); k!=key.end(); k++) dbg << *k << " ";
          dbg << endl;*/
          
          tag2stream[tagGroup(props.first, props.second, 
                              //(mt==commonMerge? 
                                /*mergers[props.second->name()]->
                                       mergeKey(props.first, props.second->begin(), inStreamRecords[parserIdx])*/
                                info.getKey()/*: 
                                list<string>())*/)
                    ].add(info, parserIdx);
          
          // Record whether we read a text tag on any parser
          numTextTags += (props.second->name() == "text"? 1: 0);
          
          // Record whether we read a sight tag on any parser
          if(props.first==properties::enterTag && props.second->name() == "sight") 
            numSightEnterTags++;
          if(props.first==properties::exitTag  && props.second->name() == "sight") 
            numSightExitTags++;
          
          // We've just read a tag and are thus not ready for another on this incoming parser
          // until this one is processed
          readyForTag[parserIdx] = false;
        }
      }
    }
    
    #ifdef VERBOSE
    dbg << "numSightEnterTags="<<numSightEnterTags<<", numSightExitTags="<<numSightExitTags<<", numTextTags="<<numTextTags<<", numActive="<<numActive<<", nextTag("<<nextTag.size()<<")"<<endl;
    
    { scope s("tag2stream", scope::high);
      dbg << "<table>";
      for(map<tagGroup, StreamTags >::iterator i=tag2stream.begin(); i!=tag2stream.end(); i++) {
        dbg << "<tr><td>"<<i->first.str()<< "</td><td>=></td><td>"<<i->second.str()<<"</td></tr>"<<endl;
      }
      dbg<<"</table>";
    }
    #endif
/*cout << "nextTag="<<endl;
for(vector<pair<properties::tagType, const properties*> >::iterator i=nextTag.begin(); i!=nextTag.end(); i++)
  cout << "    <"<<(i->first==properties::enterTag?"enter":"exit")<<"|"<<i->second<<">"<<i->second->str()<<endl;*/
    if(numActive==0) break;
    #ifdef VERBOSE
    { scope s("Tags", scope::high); 
      printTags(dbg, activeParser, nextTag, ""); }
    #endif
    
    // If we read a sight tag on all parsers (it must be all or none), merge these tags
    // and initialize an output stream from the merged result.
    if(numSightEnterTags>0) {
      ITER_ACTION(txt()<<indent<<": "<<"Sight Tag. depth="<<variantStackDepth);
      assert(numSightEnterTags == parsers.size());
      assert(numActive == parsers.size());
      assert(variantStackDepth==0);
      assert(nextTag.size()>0);
      
      // On entry, merge the entry tags and create a new dbgStream object for the merged stream
      // Ensure that there is a tag merger for sight
      if(MergeHandlerInstantiator::MergeKeyHandlers->find("sight") == MergeHandlerInstantiator::MergeKeyHandlers->end()) 
        cerr << "ERROR: cannot find a merger for tag \"sight\"!"<<endl;
      assert(MergeHandlerInstantiator::MergeKeyHandlers->find("sight") != MergeHandlerInstantiator::MergeKeyHandlers->end());

      Merger* m = (*MergeHandlerInstantiator::MergeHandlers)["sight"](beginTags(nextTag), outStreamRecords, inStreamRecords, NULL);
      assert(out==NULL);

      // Create the new dbgStream using a freshly-allocated properties object to enable the 
      // Merger and the dbgStream to have and ultimately deallocate their own copies 
      // (optimization opportunity to use smart pointers and avoid the extra allocation)
      out = createDbgStream(new properties(m->getProps()), true);

      // If the user has indicated that the logs should be separated rather than merged, do so
      if(getenv("SIGHT_MERGE_SEPARATE"))
        separateMergeForEachParser(parsers, nextTag, 
                             outStreamRecords, inStreamRecords,
                             readyForTag, activeParser,
                             tag2stream.begin()->first, tag2stream.begin()->second,
                             numTextTags,
                             variantStackDepth,
                             out, mt, numActive, subDirCount,
                             "comparison", map<string, string>(),
#ifdef VERBOSE
                             g, curIterA, lastRecurA,
#endif
                             txt()<<indent<<"|sep ");

      // The merger is no longer needed
      delete m;
      
      // We're ready to read a new tag on all parsers
      for(std::vector<bool>::iterator r=readyForTag.begin(); r!=readyForTag.end(); r++)
        *r = true;
      
      // Reset tag2stream since we'll be reloading it based on the next tag read from each parser
      tag2stream.clear();
    // If we're finished reading on all parsers and we're at the top-most level
    } else if(numActive==0 && variantStackDepth==0) {
      assert(nextTag.size()>0);
      // On exit delete the previously-created dbgStream object
      assert(out);
      delete out;
      // Set out to NULL so make sure it is not deleted again on exit from this function
      // (need account for the case where the sight close tag is not observed)
      out = NULL;
      
      // We're ready to read a new tag on all parsers
      for(std::vector<bool>::iterator r=readyForTag.begin(); r!=readyForTag.end(); r++)
        *r = true;
      
      // Reset tag2stream since we'll be reloading it based on the next tag read from each parser
      tag2stream.clear();
      
    // If we read text on any parser, emit the text immediately
    } else if(numTextTags>0) {
      ITER_ACTION(txt()<<indent<<": "<<"Text Tag. depth="<<variantStackDepth);
      assert(out);
      
      for(map<tagGroup, StreamTags >::iterator i=tag2stream.begin(); i!=tag2stream.end(); i++) {
      //assert(tag2stream.find(make_pair(properties::enterTag, "text")) != tag2stream.end());
      if(i->first.type==properties::enterTag && i->first.objName=="text") {
        // Parsers on which we read text
        const list<int>& textParsers = i->second.parserIndexes; 
        //dbg << "#textParsers="<<textParsers.size()<<endl;
        assert(textParsers.size()>0);
        for(list<int>::const_iterator j=textParsers.begin(); j!=textParsers.end(); j++) {
          //dbg << "j="<<*j<<", #nextTag="<<nextTag.size()<<endl;
          assert(activeParser[*j]);
        }
        
        // Contains the next read tag of just this group
        vector<pair<properties::tagType, const properties*> > groupNextTag;
        collectGroupVectorIdx<pair<properties::tagType, const properties*> >(nextTag, textParsers, groupNextTag);
        
        // Gather the streamRecords of just the incoming streams within this group
        std::vector<std::map<std::string, streamRecord*> > groupInStreamRecords;
        collectGroupVectorIdx<std::map<std::string, streamRecord*> >(inStreamRecords, textParsers, groupInStreamRecords);
        
        ///if(mt == commonMerge)
          numTagsEmitted +=
            mergeTags(properties::enterTag, "text", 
                      groupNextTag, outStreamRecords, groupInStreamRecords,
                      stackDepth, *out, "");
        /*else if(mt == zipper) {
          zipperTags(properties::enterTag, "text", 
                    groupNextTag, outStreamRecords, groupInStreamRecords,
                    stackDepth, out, "");
          numTagsEmitted += groupNextTag.size();
        }*/
        for(list<int>::const_iterator j=textParsers.begin(); j!=textParsers.end(); j++)
          // We're ready to read a new tag on this parser
          readyForTag[*j] = true;
        
        // Reset the tag2stream key associated with text reading
        tag2stream.erase(i);
          
        // Reset numTextTags
        numTextTags = 0;
        
        break;
      } }
      // We'll now repeat the loop and read more tags on all the parsers from which we just read text
      // If we only entered or exited tags
    
    // If we did not read text on any parser
    } else {
      assert(out);
      
      #ifdef VERBOSE
      dbg << tag2stream.size() << " key groups"<<endl;
      #endif

      // If we observed a sight exit tag on any of the streams, pass over it
      bool sightExitTagFound = false;
      for(map<tagGroup, StreamTags >::iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ) {
        if(ts->first.objName=="sight" && ts->first.type == properties::exitTag) {
          sightExitTagFound = true;
          for(list<int>::iterator parserIdx=ts->second.parserIndexes.begin(); parserIdx!=ts->second.parserIndexes.end(); parserIdx++)
            readyForTag[*parserIdx] = true;
          tag2stream.erase(ts++);
        } else
          ts++;
      }
      
      // Group all the streams with the same next tag name/type. Since all parsers must have the same
      // stack of tags that have been entered, if we read the exit of a tag on any parser(s), they all 
      // must exit the same tag. However, different parsers may enter different tags.
      //
      // If there is just one group, then we merge the properties of the tags (all same type)
      // read from all the parsers, and perform the enter/exit action of this group
      if(!sightExitTagFound) {
        // =============================================================================================
        if(tag2stream.size()==1 && 
           !(tag2stream.begin()->first.type==properties::enterTag && tag2stream.begin()->first.objName=="comparison")) {
          ITER_ACTION(txt()<<indent<<": "<<"1 "<<tag2stream.begin()->first.objName<<" "<<(tag2stream.begin()->first.type==properties::enterTag?"enter":"exit")<<" Tag. depth="<<variantStackDepth);
          if(MergeHandlerInstantiator::MergeKeyHandlers->find(tag2stream.begin()->first.objName) == MergeHandlerInstantiator::MergeKeyHandlers->end()) 
            cerr << "ERROR: cannot find a merger for tag \""<<tag2stream.begin()->first.objName<<"\"!"<<endl;
          assert(MergeHandlerInstantiator::MergeKeyHandlers->find(tag2stream.begin()->first.objName) != MergeHandlerInstantiator::MergeKeyHandlers->end());

          /* // Universal tags can only be processed at the top-most merge() call
          assert(!tag2stream.begin()->second.universal || variantStackDepth==0); */

          // Contains the next read tag of just the active incoming streams
          vector<pair<properties::tagType, const properties*> > groupNextTag;
          collectGroupVectorBool<pair<properties::tagType, const properties*> >(nextTag, activeParser, groupNextTag);

          // Gather the streamRecords of just the incoming streams within this group
          std::vector<std::map<std::string, streamRecord*> > groupInStreamRecords;
          collectGroupVectorBool<std::map<std::string, streamRecord*> >(inStreamRecords, activeParser, groupInStreamRecords);
          
/*          // If this is a comparison tag and we're reading it from multiple streams
          // (i.e. we're comparing multiple streams as opposed to just a single stream)
          //       or a recursive merge call for a comparison tag where we process the
          //       comparison tag of each individual stream separately),
          // emit the contents of this tag in each incoming log into a separate sub-file
          if(tag2stream.begin()->first.objName=="comparison") {
            // Case for multiple comparison tags
            if(tag2stream.begin()->second.parserIndexes.size()>1) {
//              // The processing of the comparison tag on each incoming stream group gets a separate copy of 
//              // outStreamRecords. This list stores them all so that we can later merge them.
//              vector<std::map<std::string, streamRecord*> > allGroupOutStreamRecords;
//
//              // Sub-directories that hold the contents of all the comparison sub-logs
//              vector<string> compSubDirs;
//
//              dbgStreamStreamRecord::enterBlock(outStreamRecords);
/ *  dbg << "comparison merging #"<<beginTags(nextTag).size()<<endl; 
  dbg << "nextTag="<<endl;
  for(vector<pair<properties::tagType, const properties*> >::iterator i=nextTag.begin(); i!=nextTag.end(); i++)
    dbg << "    :"<<(i->first==properties::enterTag?"enter":"exit")<<":"<<i->second<<":"<<i->second->str()<<endl;* /

              // Merge the comparison tags before merging their contents. This is necessary
              // because parsers are allowed to reuse properties objects between calls, meaning
              // that the calls to merge() below may corrupt the nextTag. Thus, we need to
              // use it now.
    #ifdef VERBOSE
    { scope s("Tags", scope::high); 
      printTags(dbg, activeParser, nextTag, ""); }
    #endif
              ComparisonMerger* cm = (ComparisonMerger*)((*MergeHandlerInstantiator::MergeHandlers)["comparison"](
                                                                   beginTags(nextTag), 
                                                                   outStreamRecords, 
                                                                   inStreamRecords, NULL));
              assert(cm);
              
              separateMergeForEachParser(parsers, nextTag, 
                                         outStreamRecords, inStreamRecords,
                                         readyForTag, activeParser,
                                         tag2stream.begin()->first, tag2stream.begin()->second,
                                         numTextTags,
                                         variantStackDepth,
                                         out, mt, numActive, subDirCount,
                                         "comparison", cm->getProps().find("comparison").getMap(),
#ifdef VERBOSE
                                         g, curIterA, lastRecurA,
#endif
                                         txt()<<indent<<"|sep ");
//              // Iterate over all the active parsers
//              for(list<int>::iterator p=tag2stream.begin()->second.parserIndexes.begin();
//                  p!=tag2stream.begin()->second.parserIndexes.end(); p++) {
//                // Create a sub-directory and stream for the comparable log segment from the current incoming stream
//                pair<structure::dbgStream*, string> ret = createStructSubDir(out, "comparison", subDirCount);
//                dbgStream* groupStream = ret.first;
//                string subDir = ret.second;
//
//                // Contains just the current parser
//                vector<FILEStructureParser*> groupParsers(1, parsers[*p]);
//                // Contains the next read tag on the current parser
//                vector<pair<properties::tagType, const properties*> > groupNextTag(1, nextTag[*p]);
//                // Create a copy of outStreamRecords for this parser and add it to allGroupOutStreamRecords
//                std::map<std::string, streamRecord*> groupOutStreamRecords = 
//                          createOutSRCopy(outStreamRecords, *p, allGroupOutStreamRecords);
//                // The streamRecords of the incoming streams from the current parser
//                std::vector<std::map<std::string, streamRecord*> > groupInStreamRecords(1, inStreamRecords[*p]);
//                // The activeParser record for just the current parser (must be true)
//                std::vector<bool> groupActiveParser(1, activeParser[*p]);
//                // We are not ready for another tag on any stream within this parser.
//                std::vector<bool> groupNotReadyForTag(1, false);
//
//                // The tag2stream map of this parser, which maps all the group's streams to the
//                // common tag
//                map<tagGroup, StreamTags > groupTag2stream;
//                groupTag2stream[tag2stream.begin()->first] = StreamTags(*p, true);
//
//                #ifdef VERBOSE
//                dbg << "<<<<<<<<<<<<<<<<<<<<<<"<<endl;
//                #endif
//                //dbg << "start outLocation="<<((dbgStreamStreamRecord*)groupOutStreamRecords["sight"])->getLocation().str()<<endl;
//                int numVariantTagsEmitted = 
//                  merge(groupParsers, groupNextTag, 
//                        groupOutStreamRecords, groupInStreamRecords, 
//                        groupNotReadyForTag, groupActiveParser,
//                        groupTag2stream,
//                        numTextTags,
//                        variantStackDepth+1, groupStream, 
//                        mt, 
//  #ifdef VERBOSE
//                        g, curIterA, lastRecurA,
//  #endif
//                        txt()<<indent<<"| "<<*p);
//                //dbg << "end outLocation="<<((dbgStreamStreamRecord*)groupOutStreamRecords["sight"])->getLocation().str()<<endl;
//                //dbg << "end numVariantTagsEmitted="<<numVariantTagsEmitted<<endl;
//                #ifdef VERBOSE
//                dbg << ">>>>>>>>>>>>>>>>>>>>>>"<<endl;
//                #endif
//  //cout << "nextTag["<<*p<<"]=;"<<(nextTag[*p].first==properties::enterTag?"enter":"exit")<<";i"<<nextTag[*p].second<<"; "<<nextTag[*p].second->str()<<endl;
//
//                // If we emitted at least one tag within this variant, we record this variant 
//                // to include it in the [variants] tag that points to it.
//                if(numVariantTagsEmitted>0) {
//                  compSubDirs.push_back(subDir);
//                  //subDirCount++;
//                // Otherwise, if this variant is empty, we delete it
//                } else {
//                  rmdir(subDir.c_str());
//                }
//
//                delete groupStream;
//
//                // Update activeParsers[] based on the reading activity that occurred inside the merge() call.
//                // If entry/exit tags were balanced we'd be guaranteed that all of the group's parsers would
//                // be active but since the log generating application may terminate prematurely, we may hit
//                // the end of its log inside this merge call.
//                // If the current parser was previously active and has now terminated, update state
//                if(groupActiveParser[0]==false) {
//                  activeParser[*p]=false;
//                  numActive--;
//                }
//              } // foreach tags2stream.begin()->parserIndexes
//
//              // Complete processing of sub-logs to be compared, merging their outgoing stream records into one
//              // and emitting the comparison tag that refers to their sub-directories
//
//              // Then use the properties of the merged object to emit the merged comparison tag
//              completeSubLogProcessing(out, outStreamRecords, allGroupOutStreamRecords, compSubDirs, 
//                                       "comparison", cm->getProps().find("comparison").getMap());

              // Comparison tags for a single log are skipped. The only thing we do is update stackDepth
            } else {
              if(tag2stream.begin()->first.type==properties::enterTag) stackDepth++;
              else                                                     stackDepth--;
            }
          
          // If this is not a comparison tag or a comparison tag on just one stream, 
          // merge the contents of this tag within the incoming logs
          } else {
*/
            //dbg << "calling mergeTags, objName="<<tag2stream.begin()->first.objName<<endl;
            if(tag2stream.begin()->first.objName!="comparison") {
            // Merge the tags if we're merging common log components or if we're zippering 
            // but this is the highest-level sight object that does need to be merged
            //if(mt == commonMerge || (mt == zipper && tag2stream.begin()->first.objName=="sight"))
              numTagsEmitted +=
                mergeTags(tag2stream.begin()->first.type, tag2stream.begin()->first.objName,
                        groupNextTag, outStreamRecords, groupInStreamRecords,
                        stackDepth, *out, ".");
            }
            /*else if(mt == zipper) {
              zipperTags(tag2stream.begin()->first.type, tag2stream.begin()->first.objName,
                      groupNextTag, outStreamRecords, groupInStreamRecords,
                      stackDepth, out, ".");
              numTagsEmitted += groupNextTag.size();
            }*/
            //dbg << "post-mergeTags() stackDepth="<<stackDepth<<endl;
/*          }*/
          
          // Record that we're ready for more tags on all the parsers
          for(vector<bool>::iterator i=readyForTag.begin(); i!=readyForTag.end(); i++)
            *i = true;

          /*// Reset readyForTag and tag2stream to forget this tag on the parsers within the current group
          // and get ready to read more from them.
          for(list<int>::const_iterator i=tag2stream.begin()->second.begin(); i!=tag2stream.begin()->second.end(); i++)
            readyForTag[*i] = true;*/

          // Reset tag2stream since we'll be reloading it based on the next tag read from each parser
          tag2stream.clear();

          dbg << "stackDepth="<<stackDepth<<", variantStackDepth="<<variantStackDepth<<endl;
          // If we've exited out of the highest-level tag at this variant level, exit out to the parent
          // call to merge() unless this is the root call to merge()
          if(stackDepth==targetStackDepth && variantStackDepth>0)
            return numTagsEmitted;
        } // if(tag2stream.size()==1)

        // =============================================================================================
        // If there are multiple groups consider the ones that entered a tag. These clearly diverge from
        // each other and from groups that exited a tag since all the parsers must have entered this
        // tag and now some are trying to exit while others are trying to enter another, more deeply-nested tag.
        // Thus, we recursively call merge() on each group that is trying to enter a tag, allowing it to process
        // this tag until it is exited. The groups that are trying to exit a tag are left alone to wait for the
        // enterers to complete. Eventually all the parsers that entered a tag will exit it and thus exit the 
        // merge() call. At this point we'll read the next tag from them and see if they can be merged.
        else { //if(tag2stream.size()>1) {
          ITER_ACTION(txt()<<indent<<": "<<tag2stream.size()<<" Variants. depth="<<variantStackDepth);

          // Each group gets a separate copy of outStreamRecords. This list stores them all so that we can later merge them.
          vector<std::map<std::string, streamRecord*> > allGroupOutStreamRecords;

          #ifdef VERBOSE
          dbg << "::: "<<tag2stream.size()<<" Variants ::::"<<endl;
          dbg << "        keys="<<endl; 
          for(map<tagGroup, StreamTags >::iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++)
            dbg << "            "<<ts->first.str()<<endl;
          #endif

          dbgStreamStreamRecord::enterBlock(outStreamRecords);

          // Iterate over all the groups that entered a tag
          int variantID=0;
          // Sub-directories that hold the contents of all the variants
          vector<string> ComparVariantSubdirs;

         
          // Perform a filtering pass on tag2stream where only the tag groups that can be
          // processed right now are placed in filteredTag2Stream.
          map<tagGroup, StreamTags> filteredTag2Stream;
          
          // Iterate over all stream groups, merging the streams in each group independently
          for(map<tagGroup, StreamTags >::iterator ts=tag2stream.begin(); ts!=tag2stream.end(); variantID++, ts++) {
            #ifdef VERBOSE
            dbg << "    Variant "<<variantID<<", "<<(ts->first.type == properties::enterTag? "enter": "exit")<<", "<<ts->first.objName<<endl;
            #endif

            // Skip over universal tags since if we're here, we know that we're operating on only a subset
            // of the parsers and thus cannot merge universal tags, which require representatives along
            // all incoming streams
            if(ts->second.universal) {
              #ifdef VERBOSE
              dbg << "        Skipping universal tag."<<endl;
              #endif
              continue;
            }
            
            // Do nothing for exit tags since these parsers wait for the ones that entered tags to complete their
            // processing of these tags
            if(ts->first.type == properties::exitTag) {
              continue;
            }
            
            filteredTag2Stream.insert(*ts);
          }
          
          // Determine whether all the elements in tag2stream are comparison tags or whether some but not all are. 
          // - If all are comparison tags then we'll chop up the streams according to the ID in the comparison tag (different IDs 
          //   produce different tagGroup) and call merge on each one separately.
          // - If some are comparison tags but others are not, we'll skip the comparison tags until we get the next comparison tag
          //   on all the incoming streams. This ensures that we'll find all the streams with the same ID and won't merge a subset 
          //   of them prematurely.
          set<std::string> objNames = tagGroup::getObjNames(filteredTag2Stream);
          set<properties::tagType> tagTypes = tagGroup::getTagTypes(filteredTag2Stream);
          bool allComparisonTags = (objNames.size()==1 && *objNames.begin()=="comparison" && 
                                    tagTypes.size()==1);
          bool subsetComparisonTags = (objNames.find("comparison")!=objNames.end() && objNames.size()>1);
          #ifdef VERBOSE
          dbg << "allComparisonTags="<<allComparisonTags<<", subsetComparisonTags="<<subsetComparisonTags<<endl;
          #endif

  
          // Iterate over all stream groups, merging the streams in each group independently
          for(map<tagGroup, StreamTags >::iterator ts=filteredTag2Stream.begin(); ts!=filteredTag2Stream.end(); variantID++, ts++) {
            assert(ts->first.type == properties::enterTag);
            assert(ts->second.parserIndexes.size()>0);
            
            // If this is a comparison tag and such tags appear on only a sub-set of the incoming streams
            // rather than all of them, skip this tag until we do see comparison tags on all streams
            if(ts->first.objName=="comparison" && subsetComparisonTags) {
              #ifdef VERBOSE
              dbg << "        Skipping comparison tag since comparison appears only on a subset of the streams."<<endl;
              #endif
              continue;
            }            

            // Contains the parsers of just this group
            vector<FILEStructureParser*> groupParsers;
            collectGroupVectorIdx<FILEStructureParser*>(parsers, ts->second.parserIndexes, groupParsers);

            // Contains the next read tag of just this group
            vector<pair<properties::tagType, const properties*> > groupNextTag;
            collectGroupVectorIdx<pair<properties::tagType, const properties*> >(nextTag, ts->second.parserIndexes, groupNextTag);

            // Create a copy of outStreamRecords for this group
            std::map<std::string, streamRecord*> groupOutStreamRecords;
            for(std::map<std::string, streamRecord*>::iterator o=outStreamRecords.begin(); o!=outStreamRecords.end(); o++)
              groupOutStreamRecords[o->first] = o->second->copy(variantID);
            allGroupOutStreamRecords.push_back(groupOutStreamRecords);

            // Gather the streamRecords of just the incoming streams within this group
            std::vector<std::map<std::string, streamRecord*> > groupInStreamRecords;
            collectGroupVectorIdx<std::map<std::string, streamRecord*> >(inStreamRecords, ts->second.parserIndexes, groupInStreamRecords);

            // Gather the activeParser of just the incoming streams within this group
            std::vector<bool> groupActiveParser;
            collectGroupVectorIdx<bool>(activeParser, ts->second.parserIndexes, groupActiveParser);

            // We are not ready for another tag on any stream within this group. We will process the tags
            // we just read
            //std::vector<bool> groupNotReadyForTag(ts->second.parserIndexes.size(), false);

            // If we don't have comparison tags on all the incoming streams, we'll process the next tag now 
            //   so we're not yet ready for the next one.
            // However, if we do have comparison tags on all the incoming streamd, we skip over the comparison tags before
            //   processing their internals. As such, we set readyForTag[] to true on all streams to force the
            //   recursive call to merge to read the next tag on each stream
            std::vector<bool> groupReadyForTag(ts->second.parserIndexes.size(), allComparisonTags);

            // The tag2stream map of this group, which maps all the group's streams to the
            // common tag
            map<tagGroup, StreamTags > groupTag2stream;
            groupTag2stream.insert(*ts);

            // Note: This code assumes that the disagreement point is not a text tag.
            //       If we wish to remove this constraint, we'll need to adjust the code
            //       that reads the next tag to not read the next tag in this case but still
            //       check if the tags that are currently in groupNextTag are text.

            // <<< Recursively Call merge()
            if(mt == zipper && !allComparisonTags) {
              #ifdef VERBOSE
              dbg << "<<<<<<<<<<<<<<<<<<<<<<"<<endl;
              #endif
              //dbg << "start outLocation="<<((dbgStreamStreamRecord*)groupOutStreamRecords["sight"])->getLocation().str()<<endl;
              int numVariantTagsEmitted = 
                merge(groupParsers, groupNextTag, 
                      groupOutStreamRecords, groupInStreamRecords, 
                      groupReadyForTag, groupActiveParser,
                      groupTag2stream,
                      numTextTags,
                      variantStackDepth+1, out, 
                      mt,
                      0, // targetStackDepth
#ifdef VERBOSE
                      g, curIterA, lastRecurA,
#endif
                      indent+"| "+ts->second.str());
              //dbg << "end outLocation="<<((dbgStreamStreamRecord*)groupOutStreamRecords["sight"])->getLocation().str()<<endl;
              //dbg << "end numVariantTagsEmitted="<<numVariantTagsEmitted<<endl;
              #ifdef VERBOSE
              dbg << ">>>>>>>>>>>>>>>>>>>>>>"<<endl;
              #endif
            } else {
             /*          // If we have a comparison tag on all incoming streams, merge these tags before merging their 
              // contents. This is necessary because parsers are allowed to reuse properties objects between 
              // calls, meaning that the calls to merge() below may corrupt the nextTag. Thus, we need to
              // use it now. /
              //ComparisonMerger* cm=NULL;
              if(allComparisonTags && *tagTypes.begin()==properties::enterTag) {
                / *cm = (ComparisonMerger*)((*MergeHandlerInstantiator::MergeHandlers)["comparison"](
                                                                       beginTags(groupNextTag), 
                                                                       groupOutStreamRecords, 
                                                                       groupInStreamRecords, NULL));
                assert(cm);* /
                numTagsEmitted +=
                    mergeTags(ts->first.type, ts->first.objName,
                              groupNextTag, outStreamRecords, groupInStreamRecords,
                              stackDepth, *out, "");
              }*/
              if(allComparisonTags)
                groupTag2stream.clear();
              // Create a sub-directory and stream for the current variant/comparison
              pair<structure::dbgStream*, string> ret = 
                  createStructSubDir(out, 
                                     (allComparisonTags? "comparison": "variants"), 
                                     subDirCount);
              dbgStream* groupStream = ret.first;
              string subDir = ret.second;

              #ifdef VERBOSE
              dbg << "<<<<<<<<<<<<<<<<<<<<<<"<<endl;
              #endif
              //dbg << "start outLocation="<<((dbgStreamStreamRecord*)groupOutStreamRecords["sight"])->getLocation().str()<<endl;
              int numVariantTagsEmitted = 
                merge(groupParsers, groupNextTag, 
                      groupOutStreamRecords, groupInStreamRecords, 
                      groupReadyForTag, groupActiveParser,
                      groupTag2stream,
                      numTextTags,
                      variantStackDepth+1, groupStream, 
                      mt, 
                      // If we have all comparison tags we'll run merge until we exit them.
                      // Otherwise, we'll run merge until we exit the current tag
                      (allComparisonTags? -1: 0), // targetStackDepth
#ifdef VERBOSE
                      g, curIterA, lastRecurA,
#endif
                      indent+"| "+ts->second.str());
              //dbg << "end outLocation="<<((dbgStreamStreamRecord*)groupOutStreamRecords["sight"])->getLocation().str()<<endl;
              //dbg << "end numVariantTagsEmitted="<<numVariantTagsEmitted<<endl;
              #ifdef VERBOSE
              dbg << ">>>>>>>>>>>>>>>>>>>>>>"<<endl;
              #endif

              // If we emitted at least one tag within this variant, we record this variant 
              // to include it in the [variants] tag that points to it.
              if(numVariantTagsEmitted>0) {
                ComparVariantSubdirs.push_back(subDir);

                subDirCount++;
              // Otherwise, if this variant is empty, we delete it
              } else {
                rmdir(subDir.c_str());
              }

              delete groupStream;
            }
            // >>>

            // We're done with processing the tag that was just entered

            // Update activeParsers[] based on the reading activity that occurred inside the merge() call.
            // If entry/exit tags were balanced we'd be guaranteed that all of the group's parsers would
            // be active but since the log generating application may terminate prematurely, we may hit
            // the end of its log inside this merge call.
            // groupIdx - the index of a given parser within this group (groupParsers[])
            // globalIdx - the index of the same parser within the entire parsers[] vector
            int groupIdx=0;
            for(list<int>::iterator globalIdx=ts->second.parserIndexes.begin(); 
                globalIdx!=ts->second.parserIndexes.end(); 
                globalIdx++, groupIdx++) {
              assert(activeParser[*globalIdx]);

              // If the current parser was previously active and has now terminated, update state
              if(groupActiveParser[groupIdx]==false) {
                activeParser[*globalIdx]=false;
                numActive--;
              }
            }

            // Reset readyForTag and tag2stream to forget this tag on the parsers within the current group
            // and get ready to read more from them.
            for(list<int>::const_iterator i=ts->second.parserIndexes.begin(); i!=ts->second.parserIndexes.end(); i++)
              readyForTag[*i] = true;
            tag2stream.erase(ts->first);
          } // Iterate over all the groups that entered a tag

          // Complete processing of variants, merging their outgoing stream records into one
          // and emitting the variants tag that refers to their sub-directories
          completeSubLogProcessing(out, outStreamRecords, allGroupOutStreamRecords, ComparVariantSubdirs, 
                                   (allComparisonTags? "comparison":                               "variants"), 
                                   /*(allComparisonTags? cm->getProps().find("comparison").getMap(): map<string, string>())*/
                                   map<string, string>());
        } // if(tag2stream.size()>1)
      } // if(!sightExitTagFound)
    } // If we only entered or exited tags
  } // while(numActive>0)
  dbg << "END: numActive="<<numActive<<endl;
  // We reach this point if all parsers have terminated
  
// If we're at the highest level of the merging stack and the output stream has been 
  // created but not deleted, delete it now
  if(variantStackDepth==0 && out!=NULL)
     delete out;
  
  #ifdef VERBOSE
  { scope lastS("END", scope::minimum);
    if(lastIterA  != anchor::noAnchor) g.addDirEdge(lastIterA,  lastS.getAnchor());
    if(lastRecurA != anchor::noAnchor) g.addDirEdge(lastRecurA, lastS.getAnchor());
    outgoingA = lastS.getAnchor();
  }
  #endif
  
  return numTagsEmitted;
}

// After the merging process enters some tag on some incoming stream it may call
// this function to directly copy the entire contents of this tag to the given 
// outgoing stream out. These contents are not merged with any other stream but
// any updates recorded in outStreamRecords will subsequently be incorporated by 
// the calling function.
// Returns whether the parser was active at the end of the log region.
bool emitLogRegion(FILEStructureParser* parser, 
                  std::map<std::string, streamRecord*>& outStreamRecords,
                  std::map<std::string, streamRecord*>& inStreamRecords,
                  structure::dbgStream& out, 
#ifdef VERBOSE
                  graph& g, anchor incomingA, anchor& outgoingA,
#endif
                  string indent) {
  
#ifdef VERBOSE
  scope s("emitLogRegion");
  dbg << "dir="<<out.workDir<<endl;
#endif
  
  // The current depth of the nesting stack of tags that have been entered but not yet exited.
  int stackDepth=0;
 
  anchor lastIterA  = anchor::noAnchor;
  anchor curIterA   = anchor::noAnchor;
  anchor lastRecurA = anchor::noAnchor;
  
  // Keep iterating until the stack depth reaches below 0. This means that we'll pass through
  // as many balanced enters/exits as possible until we see an unbalanced exit. This exit
  // will not be visible to the caller but this is ok since emitLogRegion() is specifically 
  // used in cases where we enter a tag, need to emit all of its contents all the way until 
  // this initial tag container is exited.
  while(stackDepth>=0) {
    #ifdef VERBOSE
    //dbg << "=================================, numActive="<<numActive<<"\n";
    scope loopS(txt()<<"Loop");
    { scope streamS("streamRecords", scope::high); printStreamRecords(dbg, outStreamRecords, inStreamRecords, ""); }
    dbg << "stackDepth="<<stackDepth<<endl;
    #endif
    
    // Read the next tag
    pair<properties::tagType, const properties*> props = parser->next();
    #ifdef VERBOSE
    {scope s(txt()<<(props.first==properties::enterTag? "enter": "exit")<<" "<<
                   (props.second->size()>0? props.second->name(): "???"), scope::medium);
     dbg << const_cast<properties*>(props.second)->str()<<endl;}
    #endif
    
    // If we've reached the end of this parser's data
    if(props.second->size()==0) {
      ITER_ACTION("Parser Inactive");
      // Return, indicating that the parser is no longer active
      return false;
    }
    // We read a valid tag
    
    // If we read a text tag, emit the text into the output log
    if(props.second->name() == "text") {
      ITER_ACTION("Read text");
      out << props.second->find("text").get("text");
    // If this is a generic widget tag
    } else {
      // If it is an entry tag
      if(props.first == properties::enterTag) {
        ITER_ACTION(txt()<<"Read entry to "<<props.second->name());
        stackDepth++;
        out.enter(*(props.second));
      // If it is an exit tag
      } else {
        ITER_ACTION(txt()<<"Read exit from "<<props.second->name());
        stackDepth--;
        // If we've reached the exit of the tag that contains the current log region
        // return without emitting the exit to the log, while indicating to the 
        // caller that the parser is still active
        if(stackDepth<0) return true;
        out.exit(*(props.second));
      }
    }
  }
  
  // Return, indicating that the parser is still active
  return true;
}

// Given a vector of entities and a list of indexes within the vector,
// fills groupVec with just the entities at the indexes in selIdxes.
template<class EltType>
void collectGroupVectorIdx(std::vector<EltType>& vec, const std::list<int>& selIdxes, std::vector<EltType>& groupVec) {
  for(list<int>::const_iterator i=selIdxes.begin(); i!=selIdxes.end(); i++) {
    assert(*i < vec.size());
    groupVec.push_back(vec[*i]);
  }
}

// Given a vector of entities and a vector of booleans that identify the selected indexes within the vector,
// fills groupVec with just the entities at the indexes in selIdxes.
template<class EltType>
void collectGroupVectorBool(std::vector<EltType>& vec, const std::vector<bool>& selFlags, std::vector<EltType>& groupVec) {
  int idx=0;
  for(vector<bool>::const_iterator i=selFlags.begin(); i!=selFlags.end(); i++, idx++) {
    if(*i) groupVec.push_back(vec[idx]);
  }
}


void printStreamRecords(ostream& out, 
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        string indent) {
  {scope sout("outStreamRecords");
  for(std::map<std::string, streamRecord*>::iterator o=outStreamRecords.begin(); o!=outStreamRecords.end(); o++)
    out << indent << "    "<<o->first<<": "<<o->second->str(indent+"    ")<<endl;
  }
  
  {scope sin("inStreamRecords");
  int idx=0;
  for(std::vector<std::map<std::string, streamRecord*> >::iterator i=inStreamRecords.begin(); i!=inStreamRecords.end(); i++, idx++)
    for(std::map<std::string, streamRecord*>::iterator j=i->begin(); j!=i->end(); j++)
      out << indent << "    "<<idx<<": "<<j->first<<": "<<j->second->str(indent+"    ")<<endl;
  }
}

void printStreamRecords(ostream& out, 
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::map<std::string, streamRecord*>& inStreamRecords,
                        string indent) {
  {scope sout("outStreamRecords");
  for(std::map<std::string, streamRecord*>::iterator o=outStreamRecords.begin(); o!=outStreamRecords.end(); o++)
    out << indent << "    "<<o->first<<": "<<o->second->str(indent+"    ")<<endl;
  }
  
  {scope sin("inStreamRecords");
    for(std::map<std::string, streamRecord*>::iterator j=inStreamRecords.begin(); j!=inStreamRecords.end(); j++)
      out << indent << "    "<<j->first<<": "<<j->second->str(indent+"    ")<<endl;
  }
}

void printTags(ostream& out,
               std::vector<bool>& activeParser,
               vector<pair<properties::tagType, const properties*> >& tags, 
               string indent) {
  //for(vector<pair<properties::tagType, properties::iterator> >::iterator t=tags.begin(); t!=tags.end(); t++)
  for(int i=0; i<tags.size(); i++) {
    if(activeParser[i])
      out << "    "<<i<<": "<<(tags[i].first==properties::enterTag? "enterTag": (tags[i].first==properties::exitTag? "exitTag": "unknownTag"))<<", "<<tags[i].second->str()<<endl;
    else
      out << "    "<<i<<": Inactive"<<endl;
  }
}
