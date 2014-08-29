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
#include "../../sight_layout.h"

namespace sight {
namespace layout {

class valSelectorLayoutHandlerInstantiator : layoutHandlerInstantiator {
  public:
  valSelectorLayoutHandlerInstantiator();
};
extern valSelectorLayoutHandlerInstantiator valSelectorLayoutHandlerInstance;

class valSelector : public sightObj
{
  protected:
  // Maps the selIDs of all the active valSelectors to their objects
  static std::map<int, valSelector*> active;
  
  int selID;
  
  public:
  
  valSelector(properties::iterator props): sightObj(props.next()) {}
  
  static void observeSelection(properties::iterator props, std::string fieldName, const std::map<std::string, std::string>& fieldSettings);
  
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  // It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
  virtual void getSelFunction(std::string val, std::string arg)=0;
};

class colorSelector : public valSelector {
  
  public:
  colorSelector(properties::iterator prop);
  ~colorSelector();
  
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  // It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
  void getSelFunction(std::string val, std::string arg);
};

class textColor
{
  public:
  static void* start(properties::iterator props);
  static void end   (void* obj);
};

class bgColor
{
  public:
  static void* start(properties::iterator props);
  static void end   (void* obj);
};

class borderColor
{
  public:
  static void* start(properties::iterator props);
  static void end   (void* obj);
};

}; // namespace layout
}; // namespace sight
