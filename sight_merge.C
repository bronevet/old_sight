#include "sight_merge.h"
using namespace std;

namespace sight {
namespace merge {
  
#ifdef VERBOSE
#define ITER_ACTION(text)  \
    scope actionS(text);   \
    if(lastIterA  != anchor::noAnchor) g.addDirEdge  (lastIterA,  actionS.getAnchor()); \
    if(incomingA  != anchor::noAnchor) g.addUndirEdge(incomingA,  actionS.getAnchor()); \
    for(set<anchor>::iterator a=lastRecurA.begin(); a!=lastRecurA.end(); a++)           \
      g.addUndirEdge(*a, actionS.getAnchor());                                          \
    incomingA  = anchor::noAnchor;     \
    lastIterA  = actionS.getAnchor();  \
    curIterA   = actionS.getAnchor();  \
    outgoingA  = actionS.getAnchor();  \
    lastRecurA.clear();
#else
#define ITER_ACTION(text) 
#endif

MergeState::MergeState(const vector<baseStructureParser<FILE>*>& parsers
                       #ifdef VERBOSE
                       , graph& g, anchor incomingA, anchor outgoingA
                       #endif
                      ) : parsers(parsers), derived(false)
                          #ifdef VERBOSE
                          , g(g), incomingA(incomingA), outgoingA(outgoingA)
                          #endif
{
  // All parsers are initially assumed to be active and ready to read the next tag
  readyForTag.assign(parsers.size(), true);
  active.assign(parsers.size(), true);
  
  // Initialize nextTag to empty properties objects
  nextTag.assign(parsers.size(), make_pair(properties::enterTag, (const properties*) NULL));
  
  out=NULL;
  
  variantStackDepth=0;
  multGroupID=0;
  
  outStreamRecords = MergeHandlerInstantiator::GetAllMergeStreamRecords(0);
  outStreamRecordsAreNew = true; // Record that we've allocated fresh copies of outStreamRecords
  for(int i=0; i<parsers.size(); i++)
    inStreamRecords.push_back(MergeHandlerInstantiator::GetAllMergeStreamRecords(i));
  
  // We have not yet read any tags
  readUniversalTag.assign(parsers.size(), false);
}

// Create a new MergeState by focusing the given MergeState on just the parsers at indexes in gs.parserIndexes, all 
// of which share the given tagGroup. The resulting state will be considered a variant with the given ID.
// readyForNewTags - Whether the merger will be ready to read new tags on its incoming streams is controlled by readyForNewTags.
// createNewOutStreamRecords - Whether we'll create new outStreamRecords objects for this MergeState or whether this MergeState
//      will maintain pointers to that.outStreamRecords. (the inStreamRecords are always pointers)
MergeState::MergeState(const MergeState& that, 
                       const tagGroup& tg, const groupStreams& gs, int variantID, bool readyForNewTags, bool createNewOutStreamRecords
                       #ifdef VERBOSE
                       , const anchor& incomingA
                       #endif
                      ) : derived(true)
                        #ifdef VERBOSE
                          , g(that.g), incomingA(incomingA), outgoingA(anchor::noAnchor)
                        #endif
{
  out = that.out;
  variantStackDepth = that.variantStackDepth;
  multGroupID = that.multGroupID;
  
  collectGroupVectorIdx<baseStructureParser<FILE>*>(that.parsers, gs.parserIndexes, parsers);
  collectGroupVectorIdx<pair<properties::tagType, const properties*> >(that.nextTag, gs.parserIndexes, nextTag);

  // Create outStreamRecords for this group
  for(std::map<std::string, streamRecord*>::const_iterator o=that.outStreamRecords.begin(); o!=that.outStreamRecords.end(); o++) {
    // Create a fresh outStreamRecords object if we're directed to do so
    if(createNewOutStreamRecords) outStreamRecords[o->first] = o->second->copy(variantID);
    // Otherwise, maintain a pointer to the original in that
    else                          outStreamRecords[o->first] = o->second;
  }
  outStreamRecordsAreNew = createNewOutStreamRecords;
  
  collectGroupVectorIdx<std::map<std::string, streamRecord*> >(that.inStreamRecords, gs.parserIndexes, inStreamRecords);
  collectGroupVectorIdx<bool>(that.active, gs.parserIndexes, active);

  // Initialize readyForTag to contain readyForTags in all of its indexes
  for(int i=0; i<gs.parserIndexes.size(); i++)
    readyForTag.push_back(readyForNewTags);

  // If we're not ready for new tags, initialize tag2stream to contain just the info for this group
  if(!readyForNewTags) {
    // Map tg to a fresh groupStream that indexes the parsers to align to their indexes within this MergeState
    tag2stream[tg] = groupStreams(0, gs.parserIndexes.size());
  }
  // Otherwise, initialize it to be empty
  
  collectGroupVectorIdx<bool>(that.readUniversalTag, gs.parserIndexes, readUniversalTag);
}

MergeState::~MergeState() {
  // If this object was created fresh, it allocated its own inStreamRecords. It will now delete them.
  if(!derived) {
    for(vector<map<string, streamRecord*> >::iterator i=inStreamRecords.begin(); i!=inStreamRecords.end(); i++) {
      for(map<string, streamRecord*>::iterator j=i->begin(); j!=i->end(); j++) {
        delete j->second;
      }
    }
  }
  
  // If we allocated fresh outStreamRecords for this object, delete them
  if(outStreamRecordsAreNew)
    for(map<string, streamRecord*>::iterator i=outStreamRecords.begin(); i!=outStreamRecords.end(); i++)
      delete i->second;
 
  // Creators of MergeState must take care of deleting out since they're the ones that create it
  /* 
  // If the outgoing stream has not yet been deleted (ex: its deleted when the sight close tag is observed
  // but not deleted when creating sub-logs for variants or comparisons), delete it now
  if(out)
    delete out;*/
}

/*************************************************************
 ***** Methods to track the state of the merging process *****
 *************************************************************/

// Called to indicate that we're ready to read a tag on all the parsers in the given groupStreams
// Updates both readyForTag and tag2stream.
void MergeState::readyForNextTag(const tagGroup& tg, const groupStreams& gs) {
  readyForNextTag(gs);
  
  // Erase this tagGroup from tag2stream since the corresponding tags are no longer current
  assert(tag2stream.find(tg) != tag2stream.end());
  tag2stream.erase(tg);
}

// Called to indicate that we're ready to read a tag on all the parsers in the given groupStreams
// Updates both tag2stream but not readyForTag.
void MergeState::readyForNextTag(const groupStreams& gs) {
  for(list<int>::const_iterator p=gs.parserIndexes.begin(); p!=gs.parserIndexes.end(); p++) {
    assert(*p < readyForTag.size());
    readyForTag[*p]=true;
  }
}


// Called to indicate that we're ready to read a tag on all the parsers.
void MergeState::readyForNextTag() {
  readyForTag.assign(parsers.size(), true);
  
  // Clear out tag2stream since the tags it contains are no longer current
  tag2stream.clear();  
}

// Returns the number of true values in the given boolean vector.
int MergeState::getNumTrues(const vector<bool>& v) {
  int count=0;
  for(vector<bool>::const_iterator p=v.begin(); p!=v.end(); p++)
    count += (*p? 1: 0);
  return count;
}

// Returns the number of active parsers
int MergeState::getNumActiveParsers() const
{ return getNumTrues(active); }

// Returns the number of parsers on which a universal tag is the current one.
int MergeState::getNumUniversalTags() const
{ return getNumTrues(readUniversalTag); }

// Return the number of tag groups with the given tag mergeKind
int MergeState::getNumTagGroupsByMergeKind(MergeInfo::mergeKindT mergeKind) const {
  int count=0;
  for(map<tagGroup, groupStreams>::const_iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++)
    count += (ts->first.info.getMergeKind()==mergeKind? 1: 0);
  return count;
}

// Returns whether the same universal tag is the current tag on all the streams.
bool MergeState::isSingleUniversal() const
{ return getNumUniversalTags()==getNumActiveParsers(); }

// Returns whether all the tag groups correspond to entries to comparison tags
bool MergeState::isAllComparisonEntry() const {
  for(map<tagGroup, groupStreams>::const_iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++) {
    if(ts->first.objName != "comparison" || ts->first.type != properties::enterTag)
      return false;
  }
  return true;
}

// Returns the number of tag groups among the current set of tags among all the incoming streams
int MergeState::getNumGroups() const
{ return tag2stream.size(); }

// Returns the number of tag groups with the given object name among the current set of tags among all the incoming streams
int MergeState::getNumGroupsByName(const std::string& objName) const {
  int count=0;
  for(map<tagGroup, groupStreams >::const_iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++)
    count += (ts->first.objName == objName? 1: 0);
  return count;
}

// Returns the tagGroup and tagStreams of the single tag group that has the given objectName
std::pair<tagGroup, groupStreams> MergeState::getObjNameTS(const std::string& objName) const {
  assert(getNumGroupsByName(objName)==1);
  for(map<tagGroup, groupStreams>::const_iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++)
   if(ts->first.objName==objName)
     return make_pair(ts->first, ts->second);
  assert(0);
}

// Returns the number of tag groups that correspond to entries into tags among the current set of tags among all the incoming streams
int MergeState::getNumEnterGroups() const {
  int count=0;
  for(map<tagGroup, groupStreams >::const_iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++)
    count += (ts->first.type == properties::enterTag? 1: 0);
  return count;
}

// Returns the tagGroup and tagStreams of the single enter tag group
pair<tagGroup, groupStreams> MergeState::getEnterTS() const {
  assert(getNumEnterGroups()==1);
  for(map<tagGroup, groupStreams>::const_iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++)
   if(ts->first.type==properties::enterTag)
    return make_pair(ts->first, ts->second);
  assert(0);
}

// Returns the number of tag groups that correspond to exits from tags among the current set of tags among all the incoming streams
int MergeState::getNumExitGroups() const {
  int count=0;
  for(map<tagGroup, groupStreams >::const_iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++)
    count += (ts->first.type == properties::exitTag? 1: 0);
  return count;
}

// If isSingleUniversal() is true, returns the tagGroup, groupStreams, objectName or tag type of this tag.
const tagGroup& MergeState::getCommonTagGroup() const {
  assert(getNumGroups()==1);
  return tag2stream.begin()->first;
}

const groupStreams& MergeState::getCommonGroupStreams() const {
  assert(getNumGroups()==1);
  return tag2stream.begin()->second;
}

const std::string& MergeState::getCommonObjName() const {
  assert(getNumGroups()==1);
  return getCommonTagGroup().objName;
}

properties::tagType MergeState::getCommonTagType() const {
  assert(getNumGroups()==1);
  return getCommonTagGroup().type;
}

/***************************************
 ***** Management of streamRecords *****
 ***************************************/

// Resume the streamRecord of outgoingStreams from the outgoingStreams all the given MergeStates
void MergeState::resumeFrom(const std::vector<MergeState*>& thats) {
  // The outStreamRecords within each MergeState in thats
  vector<std::map<std::string, streamRecord*> > allOSRs;
  for(std::vector<MergeState*>::const_iterator i=thats.begin(); i!=thats.end(); i++)
    allOSRs.push_back((*i)->outStreamRecords);
  
  for(map<string, streamRecord*>::iterator o=outStreamRecords.begin(); o!=outStreamRecords.end(); o++)
    o->second->resumeFrom(allOSRs);
}
  
// Resume the streamRecord of outgoingStreams from the outgoingStreams of the given MergeState
void MergeState::resumeFrom(const MergeState& that) {
  for(map<string, streamRecord*>::const_iterator o=outStreamRecords.begin(); o!=outStreamRecords.end(); o++)
    o->second->resumeFrom(that.outStreamRecords);
}

/*************************************************************
 ***** Human-readable representations of data structures *****
 *************************************************************/
  
void MergeState::printStateVectors(ostream& out) const {
  out << "active=";      printVector(out, active);      out << endl;
  out << "readyForTag="; printVector(out, readyForTag); out << endl;
}
  
void MergeState::printStreamRecords(ostream& out) const {
  {scope sout("outStreamRecords");
  for(std::map<std::string, streamRecord*>::const_iterator o=outStreamRecords.begin(); o!=outStreamRecords.end(); o++)
    out << o->first<<": "<<o->second->str("    ")<<endl;
  }
  
  {scope sin("inStreamRecords");
  int idx=0;
  for(std::vector<std::map<std::string, streamRecord*> >::const_iterator i=inStreamRecords.begin(); i!=inStreamRecords.end(); i++, idx++)
    for(std::map<std::string, streamRecord*>::const_iterator j=i->begin(); j!=i->end(); j++)
      out << idx<<": "<<j->first<<": "<<j->second->str("    ")<<endl;
  }
}

void MergeState::printTags(ostream& out) const {
  int i=0;
  assert(nextTag.size() == active.size());
  vector<pair<properties::tagType, const properties*> >::const_iterator t=nextTag.begin();
  vector<bool>::const_iterator a=active.begin();
  for(; t!=nextTag.end() && a!=active.end(); t++, a++, i++) {
    if(*a) out << "    "<<i<<": "<<(t->first==properties::enterTag? "enterTag": (t->first==properties::exitTag? "exitTag": "unknownTag"))<<", "<<t->second->str()<<endl;
    else   out << "    "<<i<<": Inactive"<<endl;
  }
}

void MergeState::printTag2Stream(ostream& out, const map<tagGroup, groupStreams>& tag2streamArg) const {
  out << "<table>";
  for(map<tagGroup, groupStreams >::const_iterator i=tag2streamArg.begin(); i!=tag2streamArg.end(); i++) {
    out << "<tr><td>"<<i->first.str()<< "</td><td>=></td><td>"<<i->second.str()<<"</td></tr>"<<endl;
  }
  out<<"</table>";
}

/************************************************
 ***** Aggregation of tag group information *****
 ************************************************/

// Returns the portion of tag2stream that corresponds to tag groups with non-universal enter tags
map<tagGroup, groupStreams> MergeState::filterTag2Stream_EnterNonUniversal() const {
  map<tagGroup, groupStreams> filtered;
  for(map<tagGroup, groupStreams>::const_iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ts++) {
    if(!ts->first.info.getUniversal() && ts->first.type==properties::enterTag)
      filtered[ts->first] = ts->second;
  }
  return filtered;
}

// Given a vector of entities and a list of indexes within the vector,
// fills groupVec with just the entities at the indexes in selIdxes.
template<class EltType>
void MergeState::collectGroupVectorIdx(const std::vector<EltType>& vec, const std::list<int>& selIdxes, std::vector<EltType>& groupVec) {
  for(list<int>::const_iterator i=selIdxes.begin(); i!=selIdxes.end(); i++) {
    assert(*i < vec.size());
    groupVec.push_back(vec[*i]);
  }
}

// Given a vector of entities and a vector of booleans that identify the selected indexes within the vector,
// fills groupVec with just the entities at the indexes in selIdxes.
template<class EltType>
void MergeState::collectGroupVectorBool(const std::vector<EltType>& vec, const std::vector<bool>& selFlags, std::vector<EltType>& groupVec) {
  int idx=0;
  for(vector<bool>::const_iterator i=selFlags.begin(); i!=selFlags.end(); i++, idx++) {
    if(*i) groupVec.push_back(vec[idx]);
  }
}

/*******************************************************
 ***** Merging of individual tags and log segments *****
 *******************************************************/

// Reads the next tag on all the incoming stream parsers that are currently active and ready for the next tag.
void MergeState::readNextTag() {
  #ifdef VERBOSE
  scope s("readNextTag");
  printStateVectors(dbg);
  #endif
  
  // Read the next tag on each parser, updating nextTag and tag2stream
  int parserIdx=0;
  for(vector<baseStructureParser<FILE>*>::iterator p=parsers.begin(); p!=parsers.end(); p++, parserIdx++) {
    /*#ifdef VERBOSE
    dbg << "readyForTag["<<parserIdx<<"]="<<readyForTag[parserIdx]<<", activeParser["<<parserIdx<<"]="<<activeParser[parserIdx]<<endl;
    #endif*/
    
    // If we're ready to read a tag on this parser and it is active
    if(readyForTag[parserIdx] && active[parserIdx]) {
      // Read the next tag on this parser
      pair<properties::tagType, const properties*> props = (*p)->next();
      
      #ifdef VERBOSE
      {scope s(txt()<<parserIdx << ": "<<
                     (props.first==properties::enterTag? "enter": "exit")<<" "<<
                     (props.second->size()>0? props.second->name(): "???"), scope::high);
      dbg << const_cast<properties*>(props.second)->str()<<endl;}
      #endif
      
      // If we've reached the end of this parser's data
      if(props.second->size()==0) {
        active[parserIdx] = false;
      } else {
        // Record the properties of the newly-read tag in nextTa
        nextTag[parserIdx] = props;
          
        // Group this parser with all the other parsers that just read a tag with the same name and type (enter/exit)
        tagGroup tg(props.first, props.second, inStreamRecords[parserIdx]);
        tag2stream[tg].add(parserIdx);
        
        // Record whether we read a universal tag
        readUniversalTag[parserIdx] = tg.info.getUniversal();
        
        // We've just read a tag and are thus not ready for another on this incoming parser
        // until this one is processed
        readyForTag[parserIdx] = false;
      }
    }
  }
  
  #ifdef VERBOSE
  dbg << "Status: isSingleUniversal="<<isSingleUniversal()<<", isAllComparisonEntry="<<isAllComparisonEntry()<<", getNumGroups="<<getNumGroups()<<", getNumEnterGroups="<<getNumEnterGroups()<<", getNumExitGroups="<<getNumExitGroups()<<endl;
  { scope streamS("streamRecords", scope::high); printStreamRecords(dbg); }
  { scope s("Tags", scope::high);                printTags(dbg); }
  { scope s("tag2stream", scope::high);          printTag2Stream(dbg); }
  #endif
}

// Run the tag merger function on the incoming streams with the given tagGroup and groupStreams and return 
// a pointer to the resulting Merger object.
Merger* MergeState::mergeObject(const tagGroup& tg, const groupStreams& gs)
{
  // Contains the next read tag of just this group
  vector<pair<properties::tagType, const properties*> > groupNextTag;
#ifdef VERBOSE
  dbg << "MergeState::mergeObject tg="<<tg.str()<<", gs="<<gs.str()<<endl;
#endif
  collectGroupVectorIdx<pair<properties::tagType, const properties*> >(nextTag, gs.parserIndexes, groupNextTag);

  if(MergeHandlerInstantiator::MergeHandlers->find(tg.objName) == MergeHandlerInstantiator::MergeHandlers->end()) {
    cerr << "ERROR: cannot find a merger for tag \""<<tg.objName<<"\"!"<<endl;
    assert(MergeHandlerInstantiator::MergeHandlers->find(tg.objName) != MergeHandlerInstantiator::MergeHandlers->end());
  }
  return (*MergeHandlerInstantiator::MergeHandlers)[tg.objName](beginTags(groupNextTag), outStreamRecords, inStreamRecords, NULL);  
}

// Given a vector of tag type/properties pairs, returns the same list but with the properties pointer
// replaced with the iterator to the start of the properties list
vector<pair<properties::tagType, properties::iterator> > MergeState::beginTags(
                           vector<pair<properties::tagType, const properties*> >& tags)
{
  vector<pair<properties::tagType, properties::iterator> > ret;
  for(vector<pair<properties::tagType, const properties*> >::iterator t=tags.begin(); t!=tags.end(); t++)
    ret.push_back(make_pair(t->first, t->second->begin()));
  return ret;
}

// Invoke the merger on all the tags that share the given tagGroup, along the parsers recorded in the given groupStreams and 
// - Emit the resulting merged tag to the outgoing stream
// - Increment stackDepth if the tag is an entry and decrement if it is an exit
// - If returnMerged is true, return a pointer to the Merger object that results from merging.
//   Otherwise, deallocate it
Merger* MergeState::mergeTag(const tagGroup& tg, const groupStreams& gs, int& stackDepth, bool returnMerged)
{
  // Merge the properties of all tags
  //Merger* m = mergers[objName]->merge(beginTags(nextTag), outStreamRecords, inStreamRecords);
  //Merger* m = (*MergeHandlerInstantiator::MergeHandlers)[objName](beginTags(nextTag), outStreamRecords, inStreamRecords, NULL);
  Merger* m = mergeObject(tg, gs);
  
  #ifdef VERBOSE
  {scope s(txt()<<"merged "<<(m->getProps().size()>0?m->getProps().name():"???"), scope::medium); 
  dbg << m->getProps().str()<<endl;
  if(tg.objName == "sight")
    dbg << "dir="<<structure::dbg->workDir<<endl; }
  dbg << "emit="<<m->emitTag()<<", #moreTagsBefore="<<m->moreTagsBefore.size()<<", #moreTagsAfter="<<m->moreTagsAfter.size()<<", stackDepth="<<stackDepth<<endl;
  
  //printStreamRecords(dbg, outStreamRecords, inStreamRecords, indent+"   ;");
  #endif
  
  // If the merger requests that this tag be emitted, do so
  if(m->emitTag()) {
    // Emit all the tags that appear before the tag that was actually read
    for(list<pair<properties::tagType, properties> >::iterator t=m->moreTagsBefore.begin(); t!=m->moreTagsBefore.end(); t++) {
    	//dbg << "before: "<<(t->first == properties::enterTag? "enter": "exit")<<": "<<t->second.str()<<endl;
           if(t->first == properties::enterTag) out->enter(t->second);
      else if(t->first == properties::exitTag)  out->exit (t->second);
    }
    
    // Perform the common action of entering/exiting this tag
    
    // If this is a text tag, print out the text
    if(tg.objName == "text") {
      *out << properties::get(m->getProps().find("text"), "text");
    
    // If it is a generic tag, print out its properties object
    } else {
      if(tg.type == properties::enterTag) {
      	#ifdef VERBOSE
      	{ scope s("Entering", scope::medium); dbg<< "props="<<m->getProps().str()<<"\n"; }
        #endif

        // If we've just entered a sight tag, we need to create the outgoing dbgStream that corresponds to it
        if(tg.objName=="sight") {
          // Create the new dbgStream using a freshly-allocated properties object to enable the 
          // Merger and the dbgStream to have and ultimately deallocate their own copies 
          // (optimization opportunity to use smart pointers and avoid the extra allocation)
          assert(out==NULL);
//          out = createDbgStream(new properties(m->getProps()), true);
          out = createStream(new properties(m->getProps()), true);
        } else {
          // Explicitly output other tags
          assert(out);
          out->enter(m->getProps());

          // Record entry into this tag in the outgoing stream
          ((dbgStreamStreamRecord*)outStreamRecords["sight"])->push(true);
        }
        stackDepth++;
        
      } else if(tg.type == properties::exitTag) {
        #ifdef VERBOSE
      	{ scope s("Exiting", scope::medium); dbg<< "props="<<m->getProps().str()<<"\n"; }
      	#endif

        assert(stackDepth>=0);
        
        // If we've just exited a sight tag, delete the outgoing stream
        if(tg.objName=="sight") {
          delete out;
          out = NULL;
        } else {
          // ?!?!? Why not assert that the stream is non-empty?
          assert(((dbgStreamStreamRecord*)outStreamRecords["sight"])->size()>0);
        
          // Emit the exit tag, while checking that the tag got entered in the outgoing stream
          if(((dbgStreamStreamRecord*)outStreamRecords["sight"])->pop())
            out->exit(m->getProps());
        }
        stackDepth--;
      }
      else assert(0);
    }
    
    // Emit all the tags that appear after the tag that was actually read
    for(list<pair<properties::tagType, properties> >::iterator t=m->moreTagsAfter.begin(); t!=m->moreTagsAfter.end(); t++) {
           if(t->first == properties::enterTag) out->enter(t->second);
      else if(t->first == properties::exitTag)  out->exit (t->second);
    }
  
  // If we don't need to emit the tag, still adjust the stackDepth to account for the fact
  // that the tag was read
  } else {
    if(tg.objName != "text") {
      if(tg.type == properties::enterTag) {
      	//dbg << "Entering props="<<m->getProps().str()<<"\n";
        stackDepth++;
        
        // Record entry into this tag in the outgoing stream
        ((dbgStreamStreamRecord*)outStreamRecords["sight"])->push(false);
      } else if(tg.type == properties::exitTag) {
      	//dbg << "Exiting props="<<m->getProps().str()<<"\n";
        stackDepth--;
        
        // ?!?!? Why not assert that the stream is non-empty?
        assert(stackDepth>=0);
        assert(((dbgStreamStreamRecord*)outStreamRecords["sight"])->size()>0);
        
        // Record the exit from this tag in the outgoing stream
        ((dbgStreamStreamRecord*)outStreamRecords["sight"])->pop();
      }
    }
  }
  
  // If the called asked to get the merged object back, return it
  if(returnMerged) return m;
  // Otherwise, delete it
  else {
    delete(m);
    return NULL;
  }
}

// Invoke the merger on the single next tag on the given parser and emit it directly to the outgoing stream.
// Increment stackDepth if the tag is an entry and decrement if it is an exit.
void MergeState::mergeSingleTag(int parserIdx, int& stackDepth) {
  #ifdef VERBOSE
  scope s(txt()<<"mergeSingleTag("<<parserIdx<<")");
  #endif
  mergeTag(tagGroup(nextTag[parserIdx].first, nextTag[parserIdx].second, inStreamRecords[parserIdx]), groupStreams(parserIdx), stackDepth); 
}

// Run mergeTag() on all the tags that share the given tagGroup and then get ready to read more tags on the streams associated
// with the tag group.
Merger* MergeState::mergeTagAndAdvance(const tagGroup& tg, const groupStreams& gs, int& stackDepth, bool returnMerged) {
  Merger* m = mergeTag(tg, gs, stackDepth, returnMerged);

  // Make us ready for more tags on the streams associated with this tag group
  readyForNextTag(tg, gs);

  return m;
}

// Invoke the merge() method on the contents of the current current tag in the log, from the tag's
// enter until its exit, calling merge() multiple times as needed to deal with merge() exiting because
// the depth if its stack returns to 0 (due to multiple complete enter/exit pairs inside this tag).
// The current tag itself it not emitted to the outgoing stream
void MergeState::mergeInsideTag() {
  #ifdef VERBOSE
  scope s("MergeState::mergeInsideTag()");
  #endif
    
  // We must start at the entry into a single tag
  assert(getNumGroups()==1);
  assert(getCommonTagType()==properties::enterTag);
    
  string objName = getCommonObjName();
  
  // Pass over the current enter tag in all the incoming streams
  readyForNextTag();
  
  // Loop over all the tags inside the one we just entered, until we see its close
  // tag, which we'll recognize since its name will be tg.objName and type properties::exitTag
  do {
    #ifdef VERBOSE
    scope s("Loop body()");
    #endif
    // Merge the current tag
    merge();
    
    // Read the next tag on each active stream that is ready for a tag
    readNextTag();
    
    // No parser may finish until we've reached the end of the container tag
    assert(getNumActiveParsers()==getNumParsers());
  
  // Iterate until we find an exit tag with name objName
  } while(getNumGroups()>1 || 
          getCommonObjName()!=objName || 
          getCommonTagType()!=properties::exitTag);
  
  // Pass over the exit of the container tag
  readyForNextTag();
}

// Creates a sub-directory within the current directory to hold the sub-log that belongs
// to a log variant or one of the logs in a comparison tag
// parentStream - the dbgStream pointer to the stream that contains the new one this function creates
// label - a label that describes the type of sub-directory this is (e.g. "variants" or "comparison")
/* // subDirCount - the number of sub-directories that have been created within the calling function.
//    incremented during this function */
// Returns the pair:
//    dbgStream* into which the contents of the sub-log should be written
//    string that holds the path of the sub-directory
std::pair<structure::dbgStream*, std::string> MergeState::createStructSubDir(structure::dbgStream* parentStream, std::string label/*, int& subDirCount*/) {
  string subDir = txt()<<parentStream->workDir<<"/"<<label/*<<"_"<<subDirCount*/;
  //dbg << "subDir="<<subDir<<endl;

  // Create the directory structure for the structural information
  // Main output directory
  createDir(subDir, "");

  // Directory where client-generated images will go
  string imgDir = createDir(subDir, "html/dbg_imgs");

  // Directory that widgets can use as temporary scratch space
  string tmpDir = createDir(subDir, "html/tmp");

  //dbg << "Creating groupStream\n";
  structure::dbgStream* groupStream = new structure::dbgStream(NULL, txt()<<"Compare "<<label/*subDirCount*/, subDir, imgDir, tmpDir);
  
  //subDirCount++;
  
  return make_pair(groupStream, subDir);
}

// Break the contents of all the key/value pairs in tags2stream into separate files, emitting to the
// current outgoing stream a single tag that points to these new files. 
// pointerTagName - the name of the pointer tag (currently either variant or comparison)
// focustag2stream - the portion of tag2stream that is limited to the tag groups that should be merged
// includeCurrentTag - indicates whether the current tag along a given stream should be included
//    in the emitted output or not
void MergeState::mergeMultipleGroups(const string& pointerTagName, map<tagGroup, groupStreams> focustag2stream, 
                                     bool includeCurrentTag
                                     #ifdef VERBOSE
                                     , const anchor& incomingA, set<anchor>& lastRecurA
                                     #endif
                                    )
{
  #ifdef VERBOSE
  scope s(txt()<<"mergeMultipleGroups("<<pointerTagName<<", includeCurrentTag="<<includeCurrentTag<<")");
  { scope s("focustag2stream", scope::high); printTag2Stream(dbg, focustag2stream); }
  #endif
  // Iterate over all the groups that entered a tag
  
  // The numeric ID of each tag group
  int variantID=0;
  
  // Sub-directories that hold the contents of all the variants
  vector<string> allSubDirs;
  
  // If we're dealing with comparison tags, this holds the IDs of all the comparison 
  // tags, one for each dir in allSubDirs
  vector<string> allComparisonIDs;
  
  // Each group gets its own MergeState, each with a separate outStreamRecords. These vectors store them all so that we can later merge them.
  vector<MergeState*> allGroupsMSs;
  
  for(map<tagGroup, groupStreams>::iterator ts=focustag2stream.begin(); ts!=focustag2stream.end(); ts++, variantID++) {
    #ifdef VERBOSE
    scope s("Processing sub-group");
    dbg << ts->first.str()<<" => "<<ts->second.str()<<endl;
    #endif
    MergeState* groupState = 
           new MergeState(*this, ts->first, ts->second, variantID, /*readyForNewTags*/ false, /*createNewOutStreamRecords*/ true
                          #ifdef VERBOSE
                          , incomingA 
                          #endif
            
            );
    #ifdef VERBOSE
    lastRecurA.insert(groupState->outgoingA);
    #endif
                         
    allGroupsMSs.push_back(groupState);
    
    // Create a new directory where the results of this merge will be emitted and set groupState to point to 
    // its dbgStream.
    pair<structure::dbgStream*, string> ret = createStructSubDir(out, txt()<<pointerTagName<<"_"<<multGroupID<<"_"<<variantID/*, variantID*/);
    string subDir = ret.second;
    groupState->out = ret.first;
    
    // Set the variantStack Depth of the new MergeState to be 1 deeper than the current one
    groupState->variantStackDepth = variantStackDepth+1;
    
    // If we're dealing with comparison tags, add the ID of the current tag to allComparisonIDs
    if(pointerTagName=="comparison") {
      // All the tags in groupState must be entry comparison tags with identical IDs
      assert(groupState->nextTag.begin()->first==properties::enterTag);
      // Currently, we only support merging of inline comparison tags. Multi-level merging will 
      // be implemented in the future.
      assert(groupState->nextTag.begin()->second->find("comparison").get("inline") == "1");
      allComparisonIDs.push_back(groupState->nextTag.begin()->second->find("comparison").get("ID"));
    }
    
    // Merge the tag within the current tag group
    if(includeCurrentTag) groupState->merge();
    else                  groupState->mergeInsideTag();
    
    /* // If we emitted at least one tag within this variant, we record this variant 
    // to include it in the [variants] tag that points to it.
    if(numVariantTagsEmitted>0) {*/
    allSubDirs.push_back(subDir);

    /* // Otherwise, if this variant is empty, we delete it
    } else {
      rmdir(subDir.c_str());
    }*/
    
    // Delete the outgoing dbgStream for this merger
    delete groupState->out;
    
    // Get ready to receive new tags on all the parsers of this tag group
    readyForNextTag(ts->first, ts->second);
    tag2stream.erase(ts->first);
  }
  
  // Resume the streamRecord of the outgoing stream from the sub-streams of the group's variants
  resumeFrom(allGroupsMSs);
  
  // Delete the MergeStates for all the groups, which includes outStreamRecords and output dbgStreams
  for(vector<MergeState*>::iterator ms=allGroupsMSs.begin(); ms!=allGroupsMSs.end(); ms++)
    delete *ms;

  // If we produced any output tags within the above variants
  //if(subDirs.size()>0) {        
    // Output the [variant] or [comparison] tag that points to the directories that hold the contents of each variant/comparison
    properties props;
    map<string, string> pMap;
    // Record that the contents of this variant/comparison tag are listed inside another 
    // this log the path of which is specified in this tag (outline) rather than
    // inside this tag.
    pMap["inline"] = "0";
    pMap["numSubDirs"] = txt()<<allSubDirs.size();
    for(int v=0; v<allSubDirs.size(); v++) {
      pMap[txt()<<"sub_"<<v] = allSubDirs[v];
      if(pointerTagName=="comparison") pMap[txt()<<"ID_"<<v] = allComparisonIDs[v];
    }
    props.add(pointerTagName, pMap);
    out->tag(props);
  //}
}

// General merge algorithm full application logs and sub-logs 
void MergeState::merge() {
  #ifdef VERBOSE
  scope s("MergeState::merge()");
  
  anchor lastIterA  = anchor::noAnchor;
  anchor curIterA   = anchor::noAnchor;
  set<anchor> lastRecurA;
  #endif
 
  // The depth of the tag nesting stack 
  int stackDepth=0;

  assert(getNumActiveParsers()>0);
  // Loop until we reach the end of all the incoming streams OR 
  // we balance all tag entries with corresponding exits
  do {
    #ifdef VERBOSE
    scope s(txt()<<"Loop depth="<<stackDepth);
    #endif
    
    // Read the next tag on each active stream that is ready for a tag
    readNextTag();
    
    // If there's a single universal tag on all streams
    if(isSingleUniversal()) {
      // If the universal tag is a sight entry or exit
      if(getCommonObjName()=="sight") {
        // If it is an entry
        if(getCommonTagType()==properties::enterTag) {
          ITER_ACTION("Enter Sight tag");
          //stackDepth++;
          assert(out==NULL);
          
          mergeTagAndAdvance(getCommonTagGroup(), getCommonGroupStreams(), stackDepth, false);
          /*Merger* m = mergeTagAndAdvance(getCommonTagGroup(), getCommonGroupStreams(), stackDepth, true);
          
          // Create the new dbgStream using a freshly-allocated properties object to enable the 
          // Merger and the dbgStream to have and ultimately deallocate their own copies 
          // (optimization opportunity to use smart pointers and avoid the extra allocation)
          assert(out==NULL);
          out = createDbgStream(new properties(m->getProps()), true);
          
          // The merger is no longer needed
          delete m;*/
          assert(out!=NULL);
        
        // If it is an exit
        } else if(getCommonTagType()==properties::exitTag) {
          ITER_ACTION("Exit Sight tag");
          //stackDepth--;
        
          mergeTagAndAdvance(getCommonTagGroup(), getCommonGroupStreams(), stackDepth, false);
          
         /* // Delete the outgoing stream
          delete out;
          out=NULL;*/
          assert(stackDepth==0);
          assert(out==NULL);
        }
        
        goto LOOP_END;
      }
    
    // If we've observed comparison entry tags on all incoming streams. Comparison tags are not universal
    // but rather, get different tagGroups for different values of their IDs
    } else if(isAllComparisonEntry()) {
      ITER_ACTION("All comparison tags");
      assert(out);
      
      // stackDepth is unchanged since mergeInsideTag() will read from the entry upto and 
      // including the exit tag of the comparison
      mergeMultipleGroups("comparison", tag2stream, /*includeCurrentTag*/ false
                              #ifdef VERBOSE
                              , curIterA, lastRecurA
                              #endif
                             );
      multGroupID++;
      
      // Note that the call to mergeMultipleNonUniversalEnterGroups will process the exit tags of
      // all the comparisons that are currently in nextTags (using mergeInsideTag()), so we never 
      // need to worry about comparison exit tags.
    
      goto LOOP_END;
    
    // We have a single generic universal tag or multiple tag groups, which may include enter and exits
    // as well as comparison and non-comparison tags
    }

      assert(out);
      // All exit tags must belong to the same tagGroup
      assert(getNumExitGroups()<=1);
      
      // If any tags are to be merged by interleaving, process them immediately, one stream at a time.
      //   Interleaving tags may contain other tags inside them. Thus, instead of merging and emitting
      //   each tag by itself, we call merge() on each incoming stream independently to ensure that
      //   the tag's enter, contents and exit are all placed in the outgoing stream before we move
      //   on to another incoming stream.
      // One side-effect of this is that during the execution of merge() with a given MergeState it 
      //   is not possible for one stream to be in the middle of an interleaving tag while another stream
      //   is waiting to align an alignment tag. Thus, interleaving tags cannot prevent alignment
      //   and cannot participate in the decision to create tag variants.
      if(getNumTagGroupsByMergeKind(MergeInfo::interleave)>0) {
        // If we're dealing with multiple streams, process the inteave tag on each stream separately
        if(getNumTagGroupsByMergeKind(MergeInfo::interleave)>1 || getCommonGroupStreams().parserIndexes.size()>1) {
          ITER_ACTION("Processing interleaving tags on multiple streams");
          for(map<tagGroup, groupStreams>::iterator ts=tag2stream.begin(); ts!=tag2stream.end(); ) {
            if(ts->first.info.getMergeKind()==MergeInfo::interleave) {
              for(list<int>::iterator p=ts->second.parserIndexes.begin(); p!=ts->second.parserIndexes.end(); p++) {
                MergeState groupState(*this, ts->first, groupStreams(*p), 0, /*readyForNewTags*/ false, /*createNewOutStreamRecords*/ false
                                      #ifdef VERBOSE
                                      , curIterA 
                                      #endif
                                     );
                groupState.merge();
                #ifdef VERBOSE
                lastRecurA.insert(groupState.outgoingA);
                #endif
              }
              map<tagGroup, groupStreams>::iterator ts2=ts;
              ++ts;
              readyForNextTag(ts2->first, ts2->second);
            } else
              ++ts;
          }
        
        // Else, if we're only dealing with single interleaving tag on a single incoming stream
        } else {
          ITER_ACTION("Processing interleaving tags on single stream");
          mergeTagAndAdvance(getCommonTagGroup(), getCommonGroupStreams(), stackDepth);
        }
        
        // We now advance to read more tags on the streams that we just processed. We'll postpone making 
        // decisions about tags that are to be merged by alignment until we have alignment tags on all 
        // streams and thus have the maximum amount of info we'll have to make the alignment decision about them.
        goto LOOP_END;
        
      // Else, if all the tags are to be merged by alignment 
      } else {
        // If there is only 1 tag group in tag2stream (may enter or exit, universal or not, but may not be comparison),
        // merge all the tags in this group together
        if(getNumGroups()==1) {
          ITER_ACTION(txt()<<"Processing single "<<getCommonObjName()<<" "<<properties::tagType2Str(getCommonTagType()));
          mergeTagAndAdvance(getCommonTagGroup(), getCommonGroupStreams(), stackDepth);
        }
        
        // If there are any text tags (all are type enterTag, there are no exitTag), process them immediately
        else if(getNumGroupsByName("text")>0) {
          ITER_ACTION(txt()<<"Processing "<<getNumGroupsByName("text")<<" text tag groups");
          
          // All text tags must have the same MergeInfo and thus, must belong to the same tag group
          assert(getNumGroupsByName("text")==1);
          pair<tagGroup, groupStreams> ts = getObjNameTS("text");
          mergeTagAndAdvance(ts.first, ts.second, stackDepth);
        }
        
        // Else, if there are multiple tag groups but only one that corresponds to entries (no more than one other 
        // may correspond to exits since we only call merge on tags that are in the same tagGroup),
        // merge all the tags in this group together
        else if(getNumEnterGroups()==1) {
          pair<tagGroup, groupStreams> ts = getEnterTS();
          ITER_ACTION(txt()<<"Processing single enter "<<ts.first.objName<<" "<<properties::tagType2Str(ts.first.type));
          
          #ifdef VERBOSE
          dbg << "enterTS="<<ts.first.str()<<" => "<<ts.second.str()<<endl;
          #endif
//          //mergeTagAndAdvance(ts.first, ts.second, stackDepth);
//          {
//            MergeState groupState(*this, ts.first, ts.second, 0, /*readyForNewTags*/ false, /*createNewOutStreamRecords*/ false);
//            groupState.merge();
//          }
//
//          // Make us ready for more tags on the streams associated with this tag group
//          readyForNextTag(ts.first, ts.second);
          
          map<tagGroup, groupStreams> filtered;
          filtered[ts.first] = ts.second;
          mergeMultipleGroups("variants", filtered, /*includeCurrentTag*/true
                              #ifdef VERBOSE
                              , curIterA, lastRecurA
                              #endif
                             );
          multGroupID++;
        
        // Else, there are multiple enter tag groups in tag2stream. (no more than one other 
        // may correspond to exits since we only call merge on tags that are in the same tagGroup). 
        // We now create several log variants, applying marge to the incoming streams of each enter, non-universal 
        // tag group and pointing to the sub-log of each group using a variant tag.
        } else {
          ITER_ACTION(txt()<<"Processing multiple enter tag groups");
          map<tagGroup, groupStreams> filtered = filterTag2Stream_EnterNonUniversal();
          mergeMultipleGroups("variants", filtered, /*includeCurrentTag*/true
                              #ifdef VERBOSE
                              , curIterA, lastRecurA
                              #endif
                             );
          multGroupID++;
        }
      }
    
    LOOP_END:
      ;
  } while(getNumActiveParsers()>0 && stackDepth>0);
}

structure::dbgStream* MergeState::createStream(properties* props, bool storeProps){
    return createDbgStream(props, storeProps);
}

} // namespace merge
} // namespace sight
