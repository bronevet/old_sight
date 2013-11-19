#include <stdlib.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include "attributes_common.h"

using namespace std;

namespace sight {

/*********************
 ***** attrValue *****
 *********************/ 

attrValue::attrValue() {
  type = unknownT;
}

attrValue::attrValue(const std::string& strV, attrValue::valueType type) : type(type) {
  istringstream iss(strV);
  switch(type) {
    case strT:   store = (void*)(new string(strV));             break;
    case ptrT:   store = new void*;   iss >> *((void**)store);  break;
    case intT:   store = new long*;   iss >> *((long*)store);   break;
    case floatT: store = new double*; iss >> *((double*)store); break;
    default: assert(0);
  }
}

attrValue::attrValue(const string& strV) {
  type  = strT;
  store = (void*)(new string(strV));
}

attrValue::attrValue(char* strV) {
  type  = strT;
  store = (void*)(new string(strV));
}

attrValue::attrValue(void* ptrV) {
  type  = ptrT;
  store = new void*;
  *((void**)store) = ptrV;
}

attrValue::attrValue(int intV) {
  type  = intT;
  store = new long*;
  *((long*)store) = intV;
//cout << this << ": allocated "<<((long*)store)<<"\n";
}

attrValue::attrValue(long intV) {
  type  = intT;
  store = new long*;
  *((long*)store) = intV;
//cout << this << ": allocated "<<((long*)store)<<"\n";
}

attrValue::attrValue(float floatV) {
  type  = floatT;
  store = new double;
  *((double*)store) = floatV;
}

attrValue::attrValue(double floatV) {
  type  = floatT;
  store = new double;
  *((double*)store) = floatV;
}

attrValue::attrValue(const attrValue& that) {
  type = that.type;
       if(type == strT)     { store = (void*)(new string(*((string*)that.store))); }
  else if(type == ptrT)     { store = new void*;  *((void**)store)  = *((void**)that.store);  }
  else if(type == intT)     { store = new long*;  *((long*)store)   = *((long*)that.store);   }
  else if(type == floatT)   { store = new double; *((double*)store) = *((double*)that.store); }
  else if(type == unknownT) { }
  else {
    cerr << "attrValue::attrValue() ERROR: invalid value type "<<type<<"!"<<endl;
    assert(0);
  }
}

attrValue::~attrValue() {
//cout << this << ": deleting "<<((long*)store)<<"="<<*((long*)store)<<"\n";
       if(type == strT)   delete (string*)store;
  else if(type == ptrT)   delete (void**)store;
  else if(type == intT)   delete (long*)store;
  else if(type == floatT) delete (double*)store;
  else if(type != unknownT)
  { cerr << "attrValue::~attrValue() ERROR: invalid value type "<<type<<"!"<<endl; assert(0); }
}


attrValue& attrValue::operator=(const std::string& strV) {
  type  = strT;
  store = (void*)(new string(strV));  
}

attrValue& attrValue::operator=(char* strV) {
  type  = strT;
  store = (void*)(new string(strV));  
}

attrValue& attrValue::operator=(void* ptrV) {
  type  = ptrT;
  store = new void*;
  *((void**)store) = ptrV;
}

attrValue& attrValue::operator=(long intV) {
  type  = intT;
  store = new long*;
  *((long*)store) = intV;
}

attrValue& attrValue::operator=(int intV) {
  type  = intT;
  store = new long*;
  *((long*)store) = intV;
}

attrValue& attrValue::operator=(double floatV) {
  type  = floatT;
  store = new double;
  *((double*)store) = floatV;
}

attrValue& attrValue::operator=(float floatV) {
  type  = floatT;
  store = new double;
  *((double*)store) = floatV;
}

attrValue& attrValue::operator=(const attrValue& that) {
  type = that.type;
       if(type == strT)   { store = (void*)(new string(*((string*)that.store))); }
  else if(type == ptrT)   { store = new void*;  *((void**)store)  = *((void**)that.store);  }
  else if(type == intT)   { store = new long*;  *((long*)store)   = *((long*)that.store);   }
  else if(type == floatT) { store = new double; *((double*)store) = *((double*)that.store); }
  else if(type == unknownT) { }
  else {
    cerr << "attrValue::operator=() ERROR: invalid value type "<<type<<"!"<<endl;
    assert(0);
  }
}

// Returns the type of this attrValue's contents
attrValue::valueType attrValue::getType() const
{ return type; }

// Return the contents of this attrValue, aborting if there is a type incompatibility
std::string attrValue::getStr() const {
  if(type == strT) return *((string*)store);
  cerr << "attrValue::getStr() ERROR: value type is "<<type<<"!"<<endl; assert(0);
}

// Return the contents of this attrValue, aborting if there is a type incompatibility
void*       attrValue::getPtr() const {
  if(type == ptrT) return *((void**)store);
  cerr << "attrValue::getPtr() ERROR: value type is "<<type<<"!"<<endl; assert(0);
}

// Return the contents of this attrValue, aborting if there is a type incompatibility
long        attrValue::getInt() const {
  if(type == intT) return *((long*)store);
  cerr << "attrValue::getInt() ERROR: value type is "<<type<<"!"<<endl; assert(0);
}

// Return the contents of this attrValue, aborting if there is a type incompatibility
double      attrValue::getFloat() const {
  if(type == floatT) return *((double*)store);
  cerr << "attrValue::getFloat() ERROR: value type is "<<type<<"!"<<endl; assert(0);
}

// Encodes the contents of this attrValue into a string and returns the result.
std::string attrValue::getAsStr() const {
  if(type == strT) return *((string*)store);
  else {
    ostringstream oss;
         if(type == ptrT)   oss << *((void**)store);
    else if(type == intT)   oss << *((long*)store);
    else if(type == floatT) oss << *((double*)store);
    else  {
      cerr << "attrValue::str() ERROR: unknown attribute value type: "<<type<<"!"<<endl;
      assert(0);
    }
    return oss.str();
  }
}

bool attrValue::operator==(const attrValue& that) const {
  if(type == that.type) {
         if(type == strT)   return *((string*)store) == *((string*)that.store);
    else if(type == ptrT)   return *((void**)store)  == *((void**)that.store);
    else if(type == intT)   return *((long*)store)   == *((long*)that.store);
    else if(type == floatT) return *((double*)store) == *((double*)that.store);
    else {
      cerr << "attrValue::operator== ERROR: invalid value type "<<type<<"!"<<endl;
      assert(0);
    }
  } else
    return false;
}

bool attrValue::operator<(const attrValue& that) const {
  if(type == that.type) {
         if(type == strT)   return *((string*)store) < *((string*)that.store);
    else if(type == ptrT)   return *((void**)store)  < *((void**)that.store);
    else if(type == intT)   return *((long*)store)   < *((long*)that.store);
    else if(type == floatT) return *((double*)store) < *((double*)that.store);
    else {
      cerr << "attrValue::operator< ERROR: invalid value type "<<type<<"!"<<endl;
      assert(0);
    }
  } else {\
    // All instances of each type are < or > all instances of other types, in some canonical order
    // (the enum values for different types are guaranteed to be different)
    return type < that.type;
  }
}

// Returns a human-readable representation of this object
std::string attrValue::str() const {
  return getAsStr();
}


// ******************************
// ***** Attribute Database *****
// ******************************

namespace common {
// --- STORAGE ---

// Adds the given value to the mapping of the given key without removing the key's prior mapping.
// Returns true if the attributes map changes as a result and false otherwise.
bool attributesC::add(string key, string val)
{ return add(key, attrValue(val)); }
bool attributesC::add(string key, char* val)
{ return add(key, attrValue(val)); }
bool attributesC::add(string key, void* val)
{ return add(key, attrValue(val)); }
bool attributesC::add(string key, long val)
{ return add(key, attrValue(val)); }
bool attributesC::add(string key, double val)
{ return add(key, attrValue(val)); }

bool attributesC::add(string key, const attrValue& val) {
  bool modified = (m.find(key)==m.end()) || m[key].find(val)==m[key].end();
  if(modified) {
    notifyObsPre(key);
    m[key].insert(val);
    notifyObsPost(key);
  }
  return modified;
}

// Adds the given value to the mapping of the given key, while removing the key's prior mapping, if any.
// Returns true if the attributes map changes as a result and false otherwise.
bool attributesC::replace(string key, string val)
{ return replace(key, attrValue(val)); }
bool attributesC::replace(string key, char* val)
{ return replace(key, attrValue(val)); }
bool attributesC::replace(string key, void* val)
{ return replace(key, attrValue(val)); }
bool attributesC::replace(string key, long val)
{ return replace(key, attrValue(val)); }
bool attributesC::replace(string key, double val)
{ return replace(key, attrValue(val)); }

bool attributesC::replace(string key, const attrValue& val) {
  bool modified = (m.find(key)==m.end()) || (m[key].find(val) == m[key].end());
  cout << "attributesC::replace("<<key<<") modified="<<modified<<endl;
  if(modified) {
    notifyObsPre(key);
    m[key].clear();
    m[key].insert(val);
    notifyObsPost(key);
  } else if(m[key].size() == 0) {
    cerr << "attributesC::replace() ERROR: key "<<key<<" is mapped to an empty set of values!"<<endl;
    assert(0);
  }
  
  return modified;
}

// Returns whether this key is mapped to a value
bool attributesC::exists(std::string key) const {
  return m.find(key) != m.end();
}

// Returns the value mapped to the given key
const set<attrValue>& attributesC::get(std::string key) const {
  map<string, set<attrValue> >::const_iterator i = m.find(key);
  if(i != m.end()) {
    if(i->second.size() == 0) {
      cerr << "attributesC::get() ERROR: key "<<key<<" is mapped to an empty set of values!"<<endl;
      assert(0);
    }
    return i->second;
  } else {
    cerr << "attributesC::get() ERROR: key "<<key<<" is not mapped to any value!"<<endl;
    assert(0);
  }
}

// Removes the mapping from the given key to the given value.
// Returns true if the attributes map changes as a result and false otherwise.
bool attributesC::remove(string key, string val)
{ return remove(key, attrValue(val)); }
bool attributesC::remove(string key, char* val)
{ return remove(key, attrValue(val)); }
bool attributesC::remove(string key, void* val)
{ return remove(key, attrValue(val)); }
bool attributesC::remove(string key, long val)
{ return remove(key, attrValue(val)); }
bool attributesC::remove(string key, double val)
{ return remove(key, attrValue(val)); }

bool attributesC::remove(string key, const attrValue& val) {
  bool modified = (m.find(key)!=m.end()) && m[key].find(val)!=m[key].end();
  if(modified) {
    notifyObsPre(key);
    // Remove the key->val mapping and if this is the only mapping for key, remove its entry in m[] as well
    m[key].erase(val);
    if(m[key].size()==0)
      m.erase(key);
    notifyObsPost(key);
  }
  return modified;
}

// Removes the mapping of this key to any value.
// Returns true if the attributes map changes as a result and false otherwise.
bool attributesC::remove(string key) {
  bool modified = (m.find(key)!=m.end());
  if(modified) {
    notifyObsPre(key);
    // Remove the key and all its mappings from m[]
    m.erase(key);
    notifyObsPost(key);
  }
  return modified;
}

// Add a given observer for the given key
void attributesC::addObs(std::string key, attrObserver* obs)
{
  if(o.find(key) == o.end())
    o[key] = map<attrObserver*, int>();
  
  if(o[key].find(obs) == o[key].end())
    o[key][obs] = 1;
  else
    o[key][obs]++;
}

// Remove a given observer from the given key
void attributesC::remObs(std::string key, attrObserver* obs)
{
  if(o.find(key) == o.end()) { cerr << "attributesC::remObs() ERROR: no observers for key "<<key<<"!\n"; assert(0); }
  assert(o[key].size() > 0);
  if(o[key].find(obs) == o[key].end()) { cerr << "attributesC::remObs() ERROR: this observer not registered for key "<<key<<"!\n"; assert(0); }
  assert(o[key][obs] > 0);
  
  cout << "attributesC::remObs() key="<<key<<", obs="<<obs<<", o[key][obs]="<<o[key][obs]<<endl;
  if(o[key][obs]==1) {
    o[key].erase(obs);
    if(o[key].size()==0)
      o.erase(key);
  } else 
    o[key][obs]--;
}

// Remove all observers from a given key
void attributesC::remObs(std::string key)
{
  if(o.find(key) == o.end()) { cerr << "attributesC::remObs() ERROR: no observers for key "<<key<<"!\n"; assert(0); }  
  assert(o[key].size() > 0);
  
  o[key].clear();
  o.erase(key);
}

// Notify all the observers of the given key before its mapping is changed (call attrObserver::observePre())
void attributesC::notifyObsPre(std::string key) {
  cout << "    attributesC::notifyObsPre("<<key<<") #o[key]="<<o[key].size()<<endl;
  for(map<attrObserver*, int>::iterator i=o[key].begin(); i!=o[key].end(); i++) {
    assert(i->second>0);
    i->first->observePre(key);
  }
}

// Notify all the observers of the given key after its mapping is changed (call attrObserver::observePost())
void attributesC::notifyObsPost(std::string key) {
  for(map<attrObserver*, int>::iterator i=o[key].begin(); i!=o[key].end(); i++) {
    assert(i->second>0);
    i->first->observePost(key);
  }
}

}; // namespace common
}; // namespace sight
