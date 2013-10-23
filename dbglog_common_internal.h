#pragma once

#include <stdlib.h>
#include <list>
#include <string>
#include <map>

namespace dbglog {
namespace common {

class printable
{
  public:
  virtual ~printable() {}
  virtual void print(std::ofstream& ofs) const=0;
};
// Call the print method of the given printable object
std::ofstream& operator<<(std::ofstream& ofs, const printable& p);

// Records the properties of a given object
class properties
{
  public:
  // Lists the mapping the name of a class in an inheritance hierarchy to its map of key-value pairs.
  // Objects are ordered according to inheritance depth with the base class at the end of the 
  // list and most derived class at the start.
  std::list<std::pair<std::string, std::map<std::string, std::string> > > p;
  
  // Records whether this object is active (true) or disabled (false)
  bool active;

  properties(): active(true) {}
  properties(const std::list<std::pair<std::string, std::map<std::string, std::string> > >& p, const bool& active): p(p), active(active) {}
    
  void add(std::string className, const std::map<std::string, std::string>& props);
  
  typedef std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator iterator;
  
  // Returns the start of the list to iterate from the most derived class of an object to the most base
  iterator begin() const;
  
  // The corresponding end iterator
  iterator end() const;
  
  // Given a properties iterator returns an iterator that refers to the next position in the list
  static iterator next(iterator i);
  
  // Given an iterator to a particular key->value mapping, returns the value mapped to the given key
  static std::string get(iterator cur, std::string key);
  
  // Given an iterator to a particular key->value mapping, returns the integer interpretation of the value mapped to the given key
  static long getInt(iterator cur, std::string key);
  
  // Given an iterator to a particular key->value mapping, returns the floating-point interpretation of the value mapped to the given key
  static double getFloat(iterator cur, std::string key);
  
  // Given an iterator to a particular key->value mapping, returns whether the given key is mapped to some value
  static bool exists(iterator cur, std::string key);
  
  // Returns the name of the most-derived class 
  std::string name() const;
  
  // Returns the string representation of the given properties iterator  
  static std::string str(iterator props);
};

}; // namespace common
}; // namespace dbglog