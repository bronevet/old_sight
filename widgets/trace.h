#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <assert.h>
#include "../dbglog.h"

namespace dbglog {

void traceAttr(std::string key, const attrValue& val);

class trace: public block
{
  friend void traceAttr(std::string key, const attrValue& val);
  
  // Stack of currently active traces
  static std::list<trace*> stack;
    
  // Records whether the tracer infrastructure has been initialized
  static bool initialized;
  
  // Names of attributes to be used as context when visualizing the values of trace observations
  std::list<std::string> contextAttrs;
  
  public:
  trace(std::string label, const std::list<std::string>& contextAttrs);
  trace(std::string label, std::string contextAttr);
  
  private:
  void init();
  
  public:
  ~trace();
};

}; // namespace dbglog