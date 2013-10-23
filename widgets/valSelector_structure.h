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
#include "../dbglog_structure.h"

namespace dbglog {
namespace structure {

class valSelector: public dbglogObj
{
  protected:
  
  static int maxSelID;
  int selID;
    
  // If an attribute key was provided to this selector, records it here
  std::string attrKey;
  // Records whether the key was provided
  bool attrKeyKnown;
  
  public:
  
  valSelector();
  valSelector(std::string attrKey);
  
  int getID() const;
  
  // Informs the value selector that we have observed a new value that the selector needs to account for
  // Returns the string reprentation of the current value.
  virtual std::string observeSelection(const attrValue& val)=0;
    
  // Informs the value selector that we have observed a new value that the selector needs to account for
  // It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
  // Returns the string reprentation of the current value.
  virtual std::string observeSelection()=0;
};

class colorSelector : public valSelector {
  
  public:
  colorSelector(properties* props=NULL);
  colorSelector(std::string attrKey, properties* props=NULL);
  
  // Color selector with an explicit color gradient
  colorSelector(float startR, float startG, float startB,
                float endR,   float endG,   float endB, 
                properties* props=NULL);

  colorSelector(std::string attrKey,
                float startR, float startG, float startB,
                float endR,   float endG,   float endB, 
                properties* props=NULL);
  
  void init(float startR, float startG, float startB,
            float endR,   float endG,   float endB, 
            properties* props=NULL);

  ~colorSelector();
  
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  // Returns the string reprentation of the current value.
  std::string observeSelection(const attrValue& val);
    
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  // It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
  // Returns the string reprentation of the current value.
  std::string observeSelection();
};

// Classes that modify the appearance of text and more generally, modifiers used as dbg<<modified::start()<<"some text"<<modifier::end()<<...;
namespace textColor
{
  //public:
  //static std::string start(valSelector& sel, const attrValue& val);
  //static std::string start(valSelector& sel);
  //static std::string end();
  
  class start {
    public:
    valSelector& sel;
    attrValue* val;
    start(valSelector& sel, const attrValue& val) : sel(sel), val(const_cast<attrValue*>(&val)) {}
    start(valSelector& sel)                       : sel(sel), val(NULL) {}
  };
  std::ostream& operator<< (std::ostream& stream, const start& s);
    
  class end { };
  std::ostream& operator<< (std::ostream& stream, const end& e);
};


namespace bgColor
{
  /*public:
  static std::string start(valSelector& sel, const attrValue& val);
  static std::string start(valSelector& sel);
  
  static std::string end();*/
  class start {
    public:
    valSelector& sel;
    attrValue* val;
    start(valSelector& sel, const attrValue& val) : sel(sel), val(const_cast<attrValue*>(&val)) {}
    start(valSelector& sel)                       : sel(sel), val(NULL) {}
  };
  std::ostream& operator<< (std::ostream& stream, const start& s);
    
  class end { };
  std::ostream& operator<< (std::ostream& stream, const end& e);
};

namespace borderColor
{
  /*public:
  static std::string start(valSelector& sel, const attrValue& val);
  static std::string start(valSelector& sel);
  
  static std::string end();*/
  class start {
    public:
    valSelector& sel;
    attrValue* val;
    start(valSelector& sel, const attrValue& val) : sel(sel), val(const_cast<attrValue*>(&val)) {}
    start(valSelector& sel)                       : sel(sel), val(NULL) {}
  };
  std::ostream& operator<< (std::ostream& stream, const start& s);
    
  class end { };
  std::ostream& operator<< (std::ostream& stream, const end& e);
};

}; // namespace structure
}; // namespace dbglog