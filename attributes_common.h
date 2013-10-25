#pragma once
#include <string>
#include <map>
#include <set>

namespace dbglog {
namespace layout {
class attributesC;
};
namespace structure {
class attrOp;
class attrQuery;
class attrSubQuery;
class attributesC;
}}

namespace dbglog {
// Definitions below are placed in the generic dbglog namespace because there is no chance of name conflicts
//namespace common {

// ****************************
// ***** Attribute Values *****
// ****************************

// Wrapper class for strings, integers and floating point numbers that keeps track of the 
// type of its contents and allows functors that work on only one of these types to be applied.
class attrValue {
  public:
  typedef enum {strT, ptrT, intT, floatT, unknownT} valueType;
  
  friend class dbglog::structure::attrOp;
  
  // Storage for any one of the different types of values that attrValue may wrap
  /*typedef union {
    std::string strV;
    void* ptrV;
    long intV;
    double floatV;
  } valueStore;*/
  
  // The type of the value's contents
  valueType type;
  
  // The storage for the four possible value types
  void* store;
  
  public:
  attrValue();
  attrValue(const std::string& strV);
  attrValue(char* strV);
  attrValue(void* ptrV);
  attrValue(long intV);
  attrValue(int intV);
  attrValue(double floatV);
  attrValue(float floatV);
  attrValue(const attrValue& that);
  ~attrValue();
  
  attrValue& operator=(const attrValue& that);
  attrValue& operator=(const std::string& strV);
  attrValue& operator=(char* strV);
  attrValue& operator=(void* ptrV);
  attrValue& operator=(long intV);
  attrValue& operator=(int intV);
  attrValue& operator=(double floatV);
  attrValue& operator=(float floatV);
  
  // Returns the type of this attrValue's contents
  attrValue::valueType getType() const;
  
  // Return the contents of this attrValue, aborting if there is a type incompatibility.
  std::string getStr() const;
  void*       getPtr() const;
  long        getInt() const;
  double      getFloat() const;
  
  // Encodes the contents of this attrValue into a string and returns the result.
  std::string getAsStr() const;
  
  // Implementations of the relational operators
  bool operator==(const attrValue& that) const;
  bool operator<(const attrValue& that) const;
  bool operator!=(const attrValue& that) const
  { return !(*this == that); }
  bool operator<=(const attrValue& that) const
  { return *this < that || *this == that; }
  bool operator>(const attrValue& that) const
  { return !(*this == that) && !(*this < that); }
  bool operator>=(const attrValue& that) const
  { return (*this == that) || !(*this < that); }
  
  // Returns a human-readable representation of this object
  std::string str() const;
};

// ******************************
// ***** Attribute Database *****
// ******************************

// Interface implemented by objects that wish to listen for changes to mappings of a given key
class attrObserver {
  public:
  // Called before key's mapping is changed
  virtual void observePre(std::string key) { }
    
  // Called after key's mapping is changed
  virtual void observePost(std::string key) { }
};

namespace common {

// Maintains the mapping from atribute keys to values
class attributesC
{
  //friend class structure::attributesC;
  //friend class layout::attributesC;
  
  // --- STORAGE ---
  protected:
  std::map<std::string, std::set<attrValue> > m;
  
  // Maps each key to a all the attrObserver objects that observe changes in its mappings.
  // We map each observer to the number of times it has been added to make it possible to 
  // add an observer multiple times as long as it is removed the same number of times.
  std::map<std::string, std::map<attrObserver*, int> > o;
   
  // Adds the given value to the mapping of the given key without removing the key's prior mapping.
  // Returns true if the attributes map changes as a result and false otherwise.
  public:
  bool add(std::string key, std::string val);
  bool add(std::string key, char* val);
  bool add(std::string key, void* val);
  bool add(std::string key, long val);
  bool add(std::string key, double val);
  virtual bool add(std::string key, const attrValue& val);
  
  // Adds the given value to the mapping of the given key, while removing the key's prior mapping, if any.
  // Returns true if the attributes map changes as a result and false otherwise.
  public:
  bool replace(std::string key, std::string val);
  bool replace(std::string key, char* val);
  bool replace(std::string key, void* val);
  bool replace(std::string key, long val);
  bool replace(std::string key, double val);
  virtual bool replace(std::string key, const attrValue& val);
  
  // Returns whether this key is mapped to a value
  bool exists(std::string key) const;
    
  // Returns the value mapped to the given key
  const std::set<attrValue>& get(std::string key) const;
  
  // Removes the mapping from the given key to the given value.
  // Returns true if the attributes map changes as a result and false otherwise.
  public:
  bool remove(std::string key, std::string val);
  bool remove(std::string key, char* val);
  bool remove(std::string key, void* val);
  bool remove(std::string key, long val);
  bool remove(std::string key, double val);
  virtual bool remove(std::string key, const attrValue& val);
  
  // Removes the mapping of this key to any value.
  // Returns true if the attributes map changes as a result and false otherwise.
  public:
  virtual bool remove(std::string key);
  
  // These routines manage the mapping from keys to the objects that observe changes in them
  
  // Add a given observer for the given key
  void addObs(std::string key, attrObserver* obs);
  
  // Remove a given observer from the given key
  void remObs(std::string key, attrObserver* obs);
    
  // Remove all observers from a given key
  void remObs(std::string key);
  
  protected:
  // Notify all the observers of the given key before its mapping is changed (call attrObserver::observePre())
  void notifyObsPre(std::string key);
  // Notify all the observers of the given key after its mapping is changed (call attrObserver::observePost())
  void notifyObsPost(std::string key);
}; // class attributes

}; // namespace common
}; // namespace dbglog
