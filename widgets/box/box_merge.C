// Licence information included in file LICENCE
#include "../../sight_structure.h"
#include "../../sight_common.h"
#include "box_merge.h"
#include <assert.h>
#include <iostream>

using namespace std;
using namespace sight::structure;
  
namespace sight {
namespace merge {

/*********************************************
 ***** BoxMergeHandlerInstantiator *****
 *********************************************/

// This object registers a handler for label "box" that determine whether multiple box objects are compatible for merging
// (BoxMerger::mergeKey) and a handler that creates an instance of the BoxMerger class that performs the merge (BoxMerger::create).
BoxMergeHandlerInstantiator::BoxMergeHandlerInstantiator() { 
  (*MergeHandlers   )["box"]  = BoxMerger::create;
  (*MergeKeyHandlers)["box"]  = BoxMerger::mergeKey;
}
BoxMergeHandlerInstantiator BoxMergeHandlerInstance;
                                                    

/*********************
 ***** BoxMerger *****
 *********************/

// Performs the process of merging multiple properties objects that encode instances of box
// tags: for each object records the type of tag (properties::enterTag or properties::exitTag) and
//       the current iterator to the properties object that encodes it.
// outStreamRecords: Enables widget-specific mergers to maintain state across mergings, such as for
//       relating unique IDs of objects on incoming streams to the unique IDs their merged counterparts
//       on the outgoing stream. outStreamRecords maps widget-specific IDs to the streamRecord object
//       that has been registered for each on the outgoing stream.
// inStreamRecords: maps of streamRecords for each incoming stream.
// props: pointer to the properties object that describes the merged object. Initially props=NULL and 
//       must be created by the most-derived constructor.
BoxMerger::BoxMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                         map<string, streamRecord*>& outStreamRecords,
                         vector<map<string, streamRecord*> >& inStreamRecords,
                         properties* props) : 
// Call the parent constructor, advancing the tags iterators to refer to the parent's key->value pairs
// and calling setProperties to add the key->value pairs for this class's contributions to the merged object
        BlockMerger(advance(tags), outStreamRecords, inStreamRecords, 
                    setProperties(tags, outStreamRecords, inStreamRecords, props)) { }

// Sets the properties of the merged object
properties* BoxMerger::setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                       map<string, streamRecord*>& outStreamRecords,
                                       vector<map<string, streamRecord*> >& inStreamRecords,
                                       properties* props) {
  // Create a properties object if it has not been provided
  if(props==NULL) props = new properties();
  
  properties::tagType type = streamRecord::getTagType(tags); 
  if(type==properties::unknownTag) { cerr << "ERROR: inconsistent tag types when merging Box!"<<endl; exit(-1); }

  // key->value map of the merged object
  map<string, string> pMap;

  // Enter tags have key->value pairs, while exit tags currently do not.
  if(type==properties::enterTag) {
    // Make sure that we do have some objects on the incoming streams
    assert(tags.size()>0);

    // Make sure that all the tag iterators have the same label and that this label is compatible
    // with this merger class.
    vector<string> names = getNames(tags); assert(allSame<string>(names));
    assert(*names.begin() == "box");

    // BoxMerger::mergeKey() only allows merging of boxes with the same styles. As such, take the style
    // property from the first object and put it into pMap
    pMap["style"] = tags.begin()->second.get("style");
    
    props->add("box", pMap);
  } else {
    props->add("box", pMap);
  }
  
  return props;
}

// Sets a list of strings that denotes a unique ID according to which instances of this merger's 
// tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
// Each level of the inheritance hierarchy may add zero or more elements to the given list and 
// call their parents so they can add any info,
void BoxMerger::mergeKey(properties::tagType type, properties::iterator tag, 
                           const std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
  // First add the keys from the parent object
  BlockMerger::mergeKey(type, tag.next(), inStreamRecords, info);

  // Also as the style field as the key to make sure that only boxes with the same style can be merged
  info.add(tag.get("style"));
}

}; // namespace structure
}; // namespace sight
