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
#include "../dbglog.h"

namespace dbglog {
class valSelector
{
  protected:
  // If an attribute key was provided to this selector, records it here
  std::string attrKey;
  // Records whether the key was provided
  bool attrKeyKnown;
  
  public:
  
  valSelector() { 
    attrKeyKnown = false;
  }
    
  valSelector(std::string attrKey) : attrKey(attrKey) { 
    attrKeyKnown = true;
  }
  
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  virtual void getSelFunction(const attrValue& val, std::string arg)=0;
    
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  // It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
  virtual void getSelFunction(std::string arg)=0;
};

class colorSelector : public valSelector {
  static int maxSelID;
  int selID;
  
  public:
  colorSelector();
  colorSelector(std::string attrKey);
  
  // Color selector with an explicit color gradient
  colorSelector(float startR, float startG, float startB,
                float endR,   float endG,   float endB);

  colorSelector(std::string attrKey,
                float startR, float startG, float startB,
                float endR,   float endG,   float endB);

  void init(float startR, float startG, float startB,
            float endR,   float endG,   float endB);

  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  void getSelFunction(const attrValue& val, std::string arg);
    
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  // It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
  void getSelFunction(std::string arg);
};

class textColor
{
  public:
  static std::string start(valSelector& sel, const attrValue& val);
  static std::string start(valSelector& sel);
  
  static std::string end();
};

class bgColor
{
  public:
  static std::string start(valSelector& sel, const attrValue& val);
  static std::string start(valSelector& sel);
  
  static std::string end();
};

class borderColor
{
  public:
  static std::string start(valSelector& sel, const attrValue& val);
  static std::string start(valSelector& sel);
  
  static std::string end();
};

}; // namespace dbglog