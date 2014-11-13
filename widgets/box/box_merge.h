#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "../../sight_structure_internal.h"

namespace sight {

namespace merge {

// A key step in processing the log is merging multiple logs into one. To accomplish this each widget
// must provide the capabililty to merge multiple instances of the widget into one. This requires
// three component:
// - a mapping from the names of widgets ("specified when adding the widget's key->value pairs in setProperties())
//   to a function that determines whether a given set of instances of a given widget are compatible with each other
//   and therefore may be merged. This is done by BoxMerger::mergeKey().
// - a mapping to the functions that perform the merging. This is done by BoxMergeHandlerInstantiator, which maps "box" to a 
//   function that creates a Merger object, the constructor of which merges multiple instances of a given widget.
//   The procedure for this is very similar to the way objects are serialized inside their normal constructors.
//   He inheritance hierarchy of Merger objects (Merger::BlockMerger::BoxMerger) mirrors that of the regular objects
//   (sightObj::block::box). At the starting point (call to the most derived constructor) we start with multiple 
//   properties objects, each of which contains a sequence of key->value mappings for all the classes in the object's
//   inheritance hierarchy. The mappings are ordered according to their creation order (first box, then block, then sightObj),
//   so the call to the most derived constructor is given an iterator to the first mapping in each object, which 
//   must be the one that the constructor for its object (box::box()) set up. It then processes these mappings and 
//   adds to a new properties object a mapping that describes the merged object. Finally, the constructor calls its 
//   parent constructor with the partially-completed merge properties object and with iterators to the input properties
//   objects that have been advanced by one step. The parent constructor repeats the procedure for its own key->value
//   pairs but instead of creating a new merged properties object, adds to the one that is given to it.

// This object registers a handler for label "box" that determine whether multiple box objects are compatible for merging
// (BoxMerer::mergeKey) and a handler that creates an instance of the BoxMerger class that performs the merge (BoxMerger::create).
class BoxMergeHandlerInstantiator: public sight::structure::MergeHandlerInstantiator {
  public:
  BoxMergeHandlerInstantiator();
};
extern BoxMergeHandlerInstantiator BoxMergeHandlerInstance;

// Created to merge multiple instances of the box widget
class BoxMerger : public sight::structure::BlockMerger {
  public:
  
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
  BoxMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, structure::streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, structure::streamRecord*> >& inStreamRecords,
              properties* props=NULL);

  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, structure::streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, structure::streamRecord*> >& inStreamRecords,
                                   properties* props);
                                   
  // Returns an instance of the Merger that will merge the objects in tags, which are assumed to have
  // the same keys, as returned by mergeKey.  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, structure::streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, structure::streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new BoxMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info,
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       const std::map<std::string, structure::streamRecord*>& inStreamRecords, structure::MergeInfo& info);
}; // class BoxMerger

} // namespace structure
} // namespace sight 
