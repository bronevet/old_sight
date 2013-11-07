#pragma once

#include <stdlib.h>
#include <list>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>

namespace sight {

namespace common {

// Returns whether log generation has been enabled or explicitly disabled
bool isEnabled();

} // namespace common

// Class that makes it possible to generate string labels by using the << syntax.
// Examples: Label() << "a=" << (5+2) << "!"
//           Label("a=") << (5+2) << "!"
// Since this class is meant to be used by client code, it is placed inside the easier-to-use sight namespace
struct txt : std::string {
  txt() {}
  txt(const std::string& initTxt) {
    if(common::isEnabled()) {
    	 _stream << initTxt;
    	 assign(_stream.str());
    }
  }
  
  template <typename T>
  txt& operator<<(T const& t) {
    if(common::isEnabled()) {
      _stream << t;
      assign(_stream.str());
    }
    return *this;
  }

  std::string str() const { 
    if(common::isEnabled()) return _stream.str();
    else            return "";
  }
 
  std::ostringstream _stream;
};

// Definitions for printable and properties below are placed in the generic sight namespace 
// because there is no chance of name conflicts

class printable
{
  public:
  virtual ~printable() {}
  //virtual void print(std::ofstream& ofs) const=0;
  virtual std::string str(std::string indent="") const=0;
};
// Call the print method of the given printable object
//std::ofstream& operator<<(std::ofstream& ofs, const printable& p);

// Records the properties of a given object
class properties
{
  public:
  // Differentiates between the entry tag of an object and its exit tag
  typedef enum {enterTag, exitTag, unknownTag} tagType;
  
  // Lists the mapping the name of a class in an inheritance hierarchy to its map of key-value pairs.
  // Objects are ordered according to inheritance depth with the base class at the end of the 
  // list and most derived class at the start.
  std::list<std::pair<std::string, std::map<std::string, std::string> > > p;
  
  // Records whether this object is active (true) or disabled (false)
  bool active;

  properties(): active(true) {}
  properties(const std::list<std::pair<std::string, std::map<std::string, std::string> > >& p, const bool& active): p(p), active(active) {}
  properties(const properties& that) : p(that.p), active(that.active) {}
    
  void add(std::string className, const std::map<std::string, std::string>& props);
  
  typedef std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator iterator;
  
  // Returns the start of the list to iterate from the most derived class of an object to the most base
  iterator begin() const;
  
  // The corresponding end iterator
  iterator end() const;
  
  // Returns the iterator to the given objectName
  iterator find(std::string name) const;
  
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
    
  // Returns the name of the object type referred to by the given iterator
  static std::string name(iterator cur);
  
  // Returns the name of the most-derived class 
  std::string name() const;
  
  // Returns the number of tags recorded in this object
  int size() const;
  
  // Erases the contents of this object
  void clear();
  
  // Returns the string representation of the given properties iterator  
  static std::string str(iterator props);
  
  std::string str(std::string indent="") const;
}; // class properties

namespace common {

// Base class of all sight objects that provides some common functionality
class sightObj {
  public:
  properties* props;
  sightObj() : props(NULL) {}
  sightObj(properties* props) : props(props) {}

  ~sightObj() {
    assert(props);
    delete(props);
  }
};

// Stream that uses dbgBuf
class dbgStream : public std::ostream
{
  public:
  // The title of the file
  std::string title;
  // The root working directory
  std::string workDir;
  // The directory where all images will be stored
  std::string imgDir;
  // The directory that widgets can use as temporary scratch space
  std::string tmpDir;
    
  // The directories in which different widgets store their output. Created upon request by different widgets.
  std::set<std::string>     widgetDirs;
    
  // Creates an output directory for the given widget and returns its path as a pair:
  // <path relative to the current working directory that can be used to create paths for writing files,
  //  path relative to the output directory that can be used inside generated HTML>
  std::pair<std::string, std::string> createWidgetDir(std::string widgetName);
    
  dbgStream(std::streambuf* buf): std::ostream(buf) {}
}; // dbgStream

// Given a string, returns a version of the string with all the control characters that may appear in the
// string escaped to that the string can be written out to Dbg::dbg with no formatting issues.
// This function can be called on text that has already been escaped with no harm.
std::string escape(std::string s);
std::string unescape(std::string s);

class structureParser {
  public:
 
  // Reads more data from the data source, returning the type of the next tag read and the properties of 
  // the object it denotes.
  virtual std::pair<properties::tagType, const properties*> next()=0;
};

}; // namespace common
}; // namespace sight
