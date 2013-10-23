#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <assert.h>
#include "dbglog_common.h"

using namespace std;

namespace dbglog {
namespace common {

// Call the print method of the given printable object
std::ofstream& operator<<(std::ofstream& ofs, const printable& p) {
  p.print(ofs);
  return ofs;
}

/**********************
 ***** properties *****
 **********************/
void properties::add(std::string className, const std::map<std::string, std::string>& props)
{ p.push_back(make_pair(className, props)); }

// Returns the start of the list to iterate from the most derived class of an object to the most base
properties::iterator properties::begin() const
{ return p.begin(); }

// The corresponding end iterator
properties::iterator properties::end() const
{ return p.end(); }

// Given a properties iterator returns an iterator that refers to the next position in the list.
properties::iterator properties::next(iterator i) {
  i++;
  return i;
}

// Given an iterator to a particular key->value mapping, returns the value mapped to the given key
std::string properties::get(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  std::map<std::string, std::string>::const_iterator val = cur->second.find(key);
  if(val == cur->second.end()) { cerr << "properties::get() ERROR: cannot find key \""<<key<<"\"! properties="<<str(cur)<<endl; }
  assert(val != cur->second.end());
  return val->second;
}

// Given an iterator to a particular key->value mapping, returns the integer interpretation of the value mapped to the given key
long properties::getInt(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  std::map<std::string, std::string>::const_iterator val = cur->second.find(key);
  if(val == cur->second.end()) { cerr << "properties::getInt() ERROR: cannot find key \""<<key<<"\"! properties="<<str(cur)<<endl; }
  assert(val != cur->second.end());
  return strtol(val->second.c_str(), NULL, 10);
}

// Given an iterator to a particular key->value mapping, returns the floating-point interpretation of the value mapped to the given key
double properties::getFloat(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  std::map<std::string, std::string>::const_iterator val = cur->second.find(key);
  if(val == cur->second.end()) { cerr << "properties::getFloat() ERROR: cannot find key \""<<key<<"\"! properties="<<str(cur)<<endl; }
  assert(val != cur->second.end());
  return strtod(val->second.c_str(), NULL);
}

// Given an iterator to a particular key->value mapping, returns whether the given key is mapped to some value
bool properties::exists(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  return cur->second.find(key) != cur->second.end();
}

// Returns the name of the most-derived class 
std::string properties::name() const {
  assert(p.size()>0);
  return p.front().first;
}

// Returns the string representation of the given properties iterator  
std::string properties::str(iterator props) {
  ostringstream oss;
  oss << "["<<props->first<<":"<<endl;
  for(std::map<std::string, std::string>::const_iterator i=props->second.begin(); i!=props->second.end(); i++)
    oss << "    "<<i->first<<" =&gt "<<i->second<<endl;
  oss << "]";
  return oss.str();
}

}; // namespace common
}; // namespace dbglog