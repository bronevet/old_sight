// Licence information included in file LICENCE
#include "../../sight_structure.h"
#include "../../sight_common.h"
#include "box_api_cpp.h"
#include <assert.h>
#include <iostream>

using namespace std;
using namespace sight::structure;
  
namespace sight {

// style - the string that determines the visual properties of the generated box
// onoffOp - an operation (defined in sight/attributes/) that controls whether this box is or is 
//    not emitted to the output. This parameter is not required but it is good form to enable
//    users to turn output elements on and off using such conditionals since its difficult
//    to control the creation of lexically scoped objects using normal control primitives like if()
// props - an object that encodes the structure of this object for use for serializing it.
//    The properties object is created during the construction of the box object. Since each Sight
//    object represents a multi-level inheritance hierarchy (here its sightObj::block::box), each
//    class alon the hierarchy can add new sets of key->value pairs to the properties object under
//    its own unique label. When the constructor is called directly by user code, props is 
//    unspecified and thus left as NULL. box::box() then calls the static setProperties() method
//    to create it and add box-specific properties. box::box() then forwards it to the constructor
//    of its parent class. If another widget inherits from box, the props pointer passed to 
//    box::box() will be non-NULL so setProperties does not create a fresh object.

box::box(const std::string& style,                        properties* props) :
  // The box constructor calls the block constructor with a label, which can be used to identify
  // this box in the log's summary views. This label is left empty since individual boxes have 
  // no semantically identifying features.
  block("", 
  // setProperties() is called to add box-specific key->value pairs for serialization, before the block
  // constructor gets to set its own pairs.
        setProperties(style, NULL, props))
{}

box::box(const std::string& style, const attrOp& onoffOp, properties* props) :
  block("", setProperties(style, &onoffOp, props))
{}

box::box(const std::string& style, const std::string& label,                        properties* props) :
  block(label, setProperties(style,  NULL, props))
{}

box::box(const std::string& style, const std::string& label, const attrOp& onoffOp, properties* props) :
  block(label, setProperties(style, &onoffOp, props))
{}

// Sets the properties of this object.
// onOffOp and props are NULL if this object was not provided to the box constructor.
properties* box::setProperties(const std::string& style, const attrOp* onoffOp, properties* props)
{
  // If props is not provided (box was created directly by the user), create it now.
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes->query() && (onoffOp? onoffOp->apply(): true)) {
    // Specify that this object is active
    props->active = true;

    // Create a key->vale mapping for this instance of box
    map<string, string> pMap;
    pMap["style"] = style;

    // Add this mapping to props under a label unique to box
    props->add("box", pMap);
  }
  else
    // Specify that this object is inactive
    props->active = false;
  
  return props;
}

// Directly calls the destructor of this object. This is necessary because when an application crashes
// Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
// there is no way to directly call the destructor of a given object when it may have several levels
// of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
// it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
// an object will invoke the destroy() method of the most-derived class.
void box::destroy() {
  this->~box();
}

box::~box() {
  assert(!destroyed);
}

}; // namespae sight

