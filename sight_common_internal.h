#pragma once

#include <stdlib.h>
#include <list>
#include <string>
#include <map>
#include <set>
#include <vector>
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
  
  // Records whether a tag should be emitted for this object
  bool emitTag;
  
  properties(): active(true), emitTag(true) {}
  // Creates properties where the object name objName is mapped to no properties
  properties(std::string objName): active(true), emitTag(true)  {
    std::map<std::string, std::string> emptyMap;
    add(objName, emptyMap);
  }
  properties(const std::list<std::pair<std::string, std::map<std::string, std::string> > >& p, const bool& active, const bool& emitTag): p(p), active(active), emitTag(emitTag) {}
  properties(const properties& that) : p(that.p), active(that.active), emitTag(that.emitTag) {}
    
  void add(std::string className, const std::map<std::string, std::string>& props);
  
  bool operator==(const properties& that) const
  { 
    /*std::cout << "p==that.p = "<<(p==that.p)<<std::endl;
      std::cout << "p.size()="<<p.size()<<", that.p.size()="<<that.p.size()<<std::endl;
      std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator itThis = p.begin();
      std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator itThat = that.p.begin();
      for(; itThis!=p.end(); itThis++, itThat++) {
        std::cout << "    itThis->first = "<<itThis->first<<" itThat->first="<<itThat->first<<std::endl;
        std::cout << "    itThis->first==itThat->first = "<<(itThis->first==itThat->first)<<", itThis->second==itThat->second = "<<(itThis->second==itThat->second)<<std::endl;
      }
    std::cout << "active==that.active = "<<(active==that.active)<<std::endl;
    std::cout << "emitTag==that.emitTag = "<<(emitTag==that.emitTag)<<std::endl;*/
    return p==that.p && active==that.active && emitTag==that.emitTag; }
  
  bool operator<(const properties& that) const
  { return (p< that.p) ||
           (p==that.p && active< that.active) ||
           (p==that.p && active==that.active && emitTag< that.emitTag); }
  
  //typedef std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator iterator;
  
  // Wrapper for iterators to property lists that includes its own end iterator to make it possible to 
  // tell whether the iterator has reached the end of the list without having a reference to the list itself.
  class iterator {
    friend class properties;
    std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator cur;
    std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator end;
    
    public:  
    iterator(const std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator& cur,
             const std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator& end) :
        cur(cur), end(end)
    {}
    
    iterator(const std::list<std::pair<std::string, std::map<std::string, std::string> > >& l) :
      cur(l.begin()), end(l.end())
    {}
    
    iterator(const properties& props) :
      cur(props.p.begin()), end(props.p.end())
    {}
    
    iterator& operator++() {
      cur++;
      return *this;
    }
    
    iterator& operator++(int) {
      cur++;
      return *this;
    }
    
    // Returns the iterator that follows this one without modifying this one
    iterator next() const {
      std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator next = cur;
      next++;
      return iterator(next, end);
    }
    
    // Returns the iterator that precedes this one without modifying this one
    iterator prev() const {
      std::list<std::pair<std::string, std::map<std::string, std::string> > >::const_iterator prev = cur;
      prev--;
      return iterator(prev, end);
    }
    
    const std::pair<std::string, std::map<std::string, std::string> >& operator*() {
      assert(!isEnd());
      return *cur;
    }
    
    // Returns whether this iterator has reached the end of its list
    bool isEnd() const
    { return cur == end; }
    
    // Given an iterator to a particular key->value mapping, returns the number of keys in the map
    int getNumKeys() const
    { return cur->second.size(); }
      
    // Given an iterator to a particular key->value mapping, returns a const reference to the key/value mapping
    const std::map<std::string, std::string>& getMap() const
    { return cur->second; }
    
    public:
    
    // Returns whether the given key is mapped to a value in the key/value map at this iterator
    bool exists(std::string key) const
    { return cur->second.find(key) != cur->second.end(); }
    
    // Returns the name of the object type referred to by the given iterator
    std::string name() const
    { return cur->first; }
    
    // Returns the string representation of the given properties iterator  
    std::string str() const;
  };
  
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
  
  // Given the label of a particular key->value mapping, adds the given mapping to it
  void set(std::string name, std::string key, std::string value);
  
  // Given an iterator to a particular key->value mapping, returns the integer interpretation of the value mapped to the given key
  static long getInt(iterator cur, std::string key);
  
  // Returns the integer interpretation of the given string
  static long asInt(std::string val);
  
  // Given an iterator to a particular key->value mapping, returns the floating-point interpretation of the value mapped to the given key
  static double getFloat(iterator cur, std::string key);
  
  // Returns the floating-point interpretation of the given string
  static long asFloat(std::string val);
  
  /* // Given an iterator to a particular key->value mapping, returns whether the given key is mapped to some value
  static bool exists(iterator cur, std::string key);
  
  // Given an iterator to a particular key->value mapping, returns the number of keys in the map
  static int getNumKeys(iterator cur);
    
  // Given an iterator to a particular key->value mapping, returns a const reference to the key/value mapping
  static const std::map<std::string, std::string>& getMap(iterator cur);
    
  // Returns the name of the object type referred to by the given iterator
  static std::string name(iterator cur);*/
  
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

// Syntactic sugar for specifying lists
template<class T>
class easylist : public std::list<T> {
	public:
	easylist() {}
	  
	easylist(const T& p0)
	{ push_back(p0); }
	
	easylist(const T& p0, const T& p1)
	{ push_back(p0); push_back(p1); }
	
	easylist(const T& p0, const T& p1, const T& p2)
	{ push_back(p0); push_back(p1); push_back(p2); }
	
	easylist(const T& p0, const T& p1, const T& p2, const T& p3)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); }
	
	easylist(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); }
	
	easylist(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); }
	
	easylist(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); }
	
	easylist(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); }
	
	easylist(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7, const T& p8)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); push_back(p8); }
	
	easylist(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7, const T& p8, const T& p9)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); push_back(p8); push_back(p8); push_back(p9); }
}; // class easylist

// Syntactic sugar for specifying vectors
template<class T>
class easyvector : public std::vector<T> {
	public:
	easyvector() {}
	  
	easyvector(const T& p0)
	{ push_back(p0); }
	
	easyvector(const T& p0, const T& p1)
	{ push_back(p0); push_back(p1); }
	
	easyvector(const T& p0, const T& p1, const T& p2)
	{ push_back(p0); push_back(p1); push_back(p2); }
	
	easyvector(const T& p0, const T& p1, const T& p2, const T& p3)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); }
	
	easyvector(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); }
	
	easyvector(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); }
	
	easyvector(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); }
	
	easyvector(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); }
	
	easyvector(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7, const T& p8)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); push_back(p8); }
	
	easyvector(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7, const T& p8, const T& p9)
	{ push_back(p0); push_back(p1); push_back(p2); push_back(p3); push_back(p4); push_back(p5); push_back(p6); push_back(p7); push_back(p8); push_back(p8); push_back(p9); }
}; // class easyvector


// Syntactic sugar for specifying maps
template<class KeyT, class ValT>
class easymap: public std::map<KeyT, ValT> {
	public:
	easymap() {} 
	
  easymap(const KeyT& key0, const ValT& val0)
  { (*this)[key0] = val0; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1)
  { (*this)[key0] = val0; (*this)[key1] = val1; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1, const KeyT& key2, const ValT& val2)
  { (*this)[key0] = val0; (*this)[key1] = val1; (*this)[key2] = val2; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1, const KeyT& key2, const ValT& val2, const KeyT& key3, const ValT& val3)
  { (*this)[key0] = val0; (*this)[key1] = val1; (*this)[key2] = val2; (*this)[key3] = val3; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1, const KeyT& key2, const ValT& val2, const KeyT& key3, const ValT& val3, const KeyT& key4, const ValT& val4)
  { (*this)[key0] = val0; (*this)[key1] = val1; (*this)[key2] = val2; (*this)[key3] = val3; (*this)[key4] = val4; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1, const KeyT& key2, const ValT& val2, const KeyT& key3, const ValT& val3, const KeyT& key4, const ValT& val4, const KeyT& key5, const ValT& val5)
  { (*this)[key0] = val0; (*this)[key1] = val1; (*this)[key2] = val2; (*this)[key3] = val3; (*this)[key4] = val4; (*this)[key5] = val5; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1, const KeyT& key2, const ValT& val2, const KeyT& key3, const ValT& val3, const KeyT& key5, const KeyT& key4, const ValT& val4, const ValT& val5, const KeyT& key6, const ValT& val6)
  { (*this)[key0] = val0; (*this)[key1] = val1; (*this)[key2] = val2; (*this)[key3] = val3; (*this)[key4] = val4; (*this)[key5] = val5; (*this)[key6] = val6; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1, const KeyT& key2, const ValT& val2, const KeyT& key3, const ValT& val3, const KeyT& key5, const KeyT& key4, const ValT& val4, const ValT& val5, const KeyT& key6, const ValT& val6, const KeyT& key7, const ValT& val7)
  { (*this)[key0] = val0; (*this)[key1] = val1; (*this)[key2] = val2; (*this)[key3] = val3; (*this)[key4] = val4; (*this)[key5] = val5; (*this)[key6] = val6; (*this)[key7] = val7; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1, const KeyT& key2, const ValT& val2, const KeyT& key3, const ValT& val3, const KeyT& key5, const KeyT& key4, const ValT& val4, const ValT& val5, const KeyT& key6, const ValT& val6, const KeyT& key7, const ValT& val7, const KeyT& key8, const ValT& val8)
  { (*this)[key0] = val0; (*this)[key1] = val1; (*this)[key2] = val2; (*this)[key3] = val3; (*this)[key4] = val4; (*this)[key5] = val5; (*this)[key6] = val6; (*this)[key7] = val7; (*this)[key8] = val8; }
  
  easymap(const KeyT& key0, const ValT& val0, const KeyT& key1, const ValT& val1, const KeyT& key2, const ValT& val2, const KeyT& key3, const ValT& val3, const KeyT& key5, const KeyT& key4, const ValT& val4, const ValT& val5, const KeyT& key6, const ValT& val6, const KeyT& key7, const ValT& val7, const KeyT& key8, const ValT& val8, const KeyT& key9, const ValT& val9)
  { (*this)[key0] = val0; (*this)[key1] = val1; (*this)[key2] = val2; (*this)[key3] = val3; (*this)[key4] = val4; (*this)[key5] = val5; (*this)[key6] = val6; (*this)[key7] = val7; (*this)[key8] = val8; (*this)[key9] = val9; }
}; // easymap

// Syntactic sugar for specifying sets
template<class T>
class easyset : public std::set<T> {
	public:
	easyset() {}
	  
	easyset(const T& p0)
	{ insert(p0); }
	
	easyset(const T& p0, const T& p1)
	{ insert(p0); insert(p1); }
	
	easyset(const T& p0, const T& p1, const T& p2)
	{ insert(p0); insert(p1); insert(p2); }
	
	easyset(const T& p0, const T& p1, const T& p2, const T& p3)
	{ insert(p0); insert(p1); insert(p2); insert(p3); }
	
	easyset(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4)
	{ insert(p0); insert(p1); insert(p2); insert(p3); insert(p4); }
	
	easyset(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5)
	{ insert(p0); insert(p1); insert(p2); insert(p3); insert(p4); insert(p5); }
	
	easyset(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6)
	{ insert(p0); insert(p1); insert(p2); insert(p3); insert(p4); insert(p5); insert(p6); }
	
	easyset(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7)
	{ insert(p0); insert(p1); insert(p2); insert(p3); insert(p4); insert(p5); insert(p6); insert(p7); }
	
	easyset(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7, const T& p8)
	{ insert(p0); insert(p1); insert(p2); insert(p3); insert(p4); insert(p5); insert(p6); insert(p7); insert(p8); }
	
	easyset(const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5, const T& p6, const T& p7, const T& p8, const T& p9)
	{ insert(p0); insert(p1); insert(p2); insert(p3); insert(p4); insert(p5); insert(p6); insert(p7); insert(p8); insert(p8); insert(p9); }
}; // class easyset

}; // namespace common
}; // namespace sight
