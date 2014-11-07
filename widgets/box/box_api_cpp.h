#pragma once

#include <string>
#include <iostream>
#include "../../attributes/attributes_structure.h"
#include "../../sight_structure_internal.h"

namespace sight {

// This is an example of a minimal widget, one that creates a box in the output log. The user
// specifies the HTML style properties of the box directly, allowing them the maximum amount
// of control over the box's look.

// The first step is to define an class that encapsulates object creation and destruction. A boxed
// region of ouput starts when a box is created and ends when it is destroyed. The most common use-case
// is for the user to declare a variable of type box in some lexical scope so that all output emitted
// while in that scope is wrapped in a box.

// The box class inherits from block, which is a generic base for most Sight objects. Blocks 
// are laid out as HTML divs and provide methods to get an anchor to their location in the 
// log, making it possible to create links that point to the block.
class box: public sight::structure::block
{
  public:
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
  box(const std::string& style,                                                 properties* props=NULL);
  box(const std::string& style,                          const sight::structure::attrOp& onoffOp, properties* props=NULL);
  box(const std::string& style, const std::string& label,                       properties* props=NULL);
  box(const std::string& style, const std::string& label,const sight::structure::attrOp& onoffOp, properties* props=NULL);
 
  private:
  // Sets the properties of this object.
  // onOffOp and props are NULL if this object was not provided to the box constructor.
  static properties* setProperties(const std::string& style, const sight::structure::attrOp* onoffOp, properties* props);
    
  public:
  // Ends the boxed region of the log and destroys the object.
  ~box();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(sight::structure::block* subBlock) { return true; }
  bool subBlockExitNotify (sight::structure::block* subBlock) { return true; }
}; // class box

}; // namespace sight
