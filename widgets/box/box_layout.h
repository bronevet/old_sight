#pragma once
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "../../sight_common.h"
#include "../../sight_layout.h"

namespace sight {
namespace layout {

// This layer takes a structure fille and lays it out to produce a visual representation of the log
// as an web page. The first step in this process is to register a handler to be called on each object
// in the log. Objects are presented as enter and exit tags, with the enter tags containing key->value
// pairs that encode each objects and exit tags currently without such information. Since each object
// may be implemented as a hierarchy of classes, its serialization contains a separate set of key->value
// pairs for each class in its inheritance hierarchy, each associated with a different string label
// specific to that class. These mappings are set up in order from most to least derived class when
// the object is first created and encoded into the log and are thus maintained in that order. 
// To lay out an object Sigth calls the handler registered under that object's name. This handler will
//   usually create an instance of that object, passing to it the iterator the start of properties 
//   object that encodes it. At the start this iterator refers the to first key->value mapping, which
//   corresponds to the most derived class. The constructor for the most derived class reads its key->value
//   pairs, performs tha appropriate visualization operations, and calls its parent class constructor
//   on the next key->value mapping. Once the object is fully constructed, the layout work is done
//   and the object is destroyed.
// The same procedure is followed for both entry into and exit from objects, though the latter is simpler
//   since we don't encode key->value pairs in exit tags.

// Registers the handlers for object entry/exit layout. The entry handler is provided
// the beginning iteraror to the properties object that describes the object. It returns
// the C++ object that holds the decoded object or NULL if none was created.
// The exit handler provides a pointer to the object returned by the entry handler so that
// it may be deleted.
// This architecture makes it possible to do all entry/exit processing inside object 
// constructors and destructors, which is consistent with the way the interface and merging
// layers operate.
class boxLayoutHandlerInstantiator  : layoutHandlerInstantiator{
  public:
  boxLayoutHandlerInstantiator();
};
extern boxLayoutHandlerInstantiator boxLayoutHandlerInstance;

// Holds the decoded box object and performs the layout actions for entry into and exit from boxes
class box: public block
{
  // Records the style of this box
  std::string style;
  public:
  box(properties::iterator props);
  
  public:
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
  
  // Called to enable the block to print its entry and exit text
  void printEntry(std::string loadCmd);
  void printExit();
  
  ~box();
}; // box

}; // namespace layout
}; // namespace sight
