#include <stdlib.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include "attributes_common.h"
#include <typeinfo>
#include <string.h>

using namespace std;

namespace sight {

/*********************
 ***** attrValue *****
 *********************/ 

// Returns the size of a single instance of the given type
int attrValue::sizeofType(valueType type) {
  switch(type) {
    case strT:       return sizeof(string);
    case ptrT:       return sizeof(void*);
    case intT:       return sizeof(long);
    case floatT:     return sizeof(double);
    case customT:    assert(0);
    case customSerT: assert(0);
    case unknownT:   assert(0);
  }
  assert(0);
}

// Returns the string representation of the given type
std::string attrValue::type2str(valueType type) {
  switch(type) {
    case strT:       return "strT";
    case ptrT:       return "ptrT";
    case intT:       return "intT";
    case floatT:     return "floatT";
    case customT:    return "customT";
    case customSerT: return "customT";
    case unknownT: return "unknownT";
  }
  assert(0);
}

  
attrValue::attrValue() {
  type = unknownT;
  store = NULL;
}

// Creates an attribute value based on the encoding within the given string. If type is set to unknownT,
// the constructor figures out the type based on the encoding in the string, which must have been set
// by attrValue::serialize(). Otherwise, it specializes to the specified types and ignores alternate 
// encoding possibilities.
attrValue::attrValue(const std::string& strV, attrValue::valueType type) : type(type) {
  switch(type) {
    case strT:   store = (void*)(new string(strV));                         break;
    case ptrT:   store = new void*;   *((void**)store)  = parsePtr(strV);   break;
    case intT:   store = new long*;   *((long*)store)   = parseInt(strV);   break;
    case floatT: store = new double*; *((double*)store) = parseFloat(strV); break;
    case customT:
        store = new customAttrValue*; 
        *((customAttrValue**)store) = customAttrValueInstantiator::deserialize(strV); 
        //cout << "attrValue::attrValue(type): ("<<((customAttrValue**)store)<<") "<<(*((customAttrValue**)store))->serialize()<<endl;
        break;
    case customSerT: store = (void*)(new string(strV));                     break;
    
    case unknownT:
      // Initialize this object by deserializing it from the given serial representation
      deserialize(strV);
      break;
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
  store = new long;
  *((long*)store) = intV;
//cout << this << ": allocated "<<((long*)store)<<"\n";
}

attrValue::attrValue(long intV) {
  type  = intT;
  store = new long;
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

// Produces a customSerT type
attrValue::attrValue(const customAttrValue& customV) {
  type = customSerT;
  store = (void*)(new string(customV.serialize()));
  /*store = new customAttrValue*;
  *((customAttrValue**)store) = customV.copy();*/
}

attrValue::attrValue(const attrValue& that) {
  type = that.type;
       if(type == strT)       { store = (void*)(new string(*((string*)that.store))); }
  else if(type == ptrT)       { store = new void*;            *((void**)store)            = *((void**)that.store);           }
  else if(type == intT)       { store = new long;             *((long*)store)             = *((long*)that.store);            }
  else if(type == floatT)     { store = new double;           *((double*)store)           = *((double*)that.store);          }
  else if(type == customT)    { store = new customAttrValue*; 
  *((customAttrValue**)store) = (*((customAttrValue**)that.store))->copy(); 
  //cout << "attrValue::attrValue(that): ("<<((customAttrValue**)store)<<") "<<(*((customAttrValue**)store))->serialize()<<endl;
  }
  else if(type == customSerT) { store = (void*)(new string(*((string*)that.store))); }
  else if(type == unknownT) { }
  else {
    cerr << "attrValue::attrValue() ERROR: invalid value type "<<type2str(type)<<"!"<<endl;
    assert(0);
  }
}

// Deallocates the current store
void attrValue::deallocate() {
  switch(type) {
    case strT:       delete (string*)store; break;
    case ptrT:       delete (void**)store;  break;
    case intT:       delete (long*)store;   break;
    case floatT:     delete (double*)store; break;
    case customT: 
      // Delete the copy of the custom attribute that store refers to
      delete *((customAttrValue**)store);
      // Delete the store pointer
      delete (customAttrValue**)store;
      break;
    case customSerT: delete (string*)store; break;
    case unknownT: break;
      //cerr << "attrValue::deallocate() ERROR: invalid value type "<<type2str(type)<<"!"<<endl; assert(0);
    default: assert(0);
  }
}

attrValue::~attrValue() {
  deallocate();
}

attrValue& attrValue::operator=(const std::string& strV) {
  // If the attrValue's current type is already strT
  if(type == strT)
    // Copy the new value to the current store
    *((string*)store) = strV;
  // Otherwise, deallocate the old store, allocate a fresh one for the new value
  else {
    deallocate();
    type  = strT;
    store = (void*)(new string(strV));
  }

  return *this;
}

attrValue& attrValue::operator=(char* strV) {
  // If the attrValue's current type is already strT
  if(type == strT)
    // Copy the new value to the current store
    *((string*)store) = strV;
  // Otherwise, deallocate the old store, allocate a fresh one for the new value
  else {
    deallocate();
    type  = strT;
    store = (void*)(new string(strV));  
  }

  return *this;
}

attrValue& attrValue::operator=(void* ptrV) {
  // If the attrValue's current type is already ptrT
  if(type == ptrT)
    // Copy the new value to the current store
    *((void**)store) = ptrV;
  // Otherwise, deallocate the old store, allocate a fresh one for the new value
  else {
    deallocate();
    type  = ptrT;
    store = new void*;
    *((void**)store) = ptrV;
  }

  return *this;
}

attrValue& attrValue::operator=(long intV) {
  // If the attrValue's current type is already intT
  if(type == intT)
    // Copy the new value to the current store
    *((long*)store) = intV;
  // Otherwise, deallocate the old store, allocate a fresh one for the new value
  else {
    deallocate();
    type  = intT;
    store = new long*;
    *((long*)store) = intV;
  }

  return *this;
}

attrValue& attrValue::operator=(int intV) {
  // If the attrValue's current type is already intT
  if(type == intT)
    // Copy the new value to the current store
    *((long*)store) = intV;
  // Otherwise, deallocate the old store, allocate a fresh one for the new value
  else {
    deallocate();
    type  = intT;
    store = new long*;
    *((long*)store) = intV;
  }

  return *this;
}

attrValue& attrValue::operator=(double floatV) {
  // If the attrValue's current type is already floatT
  if(type == floatT)
    // Copy the new value to the current store
    *((double*)store) = floatV;
  // Otherwise, deallocate the old store, allocate a fresh one for the new value
  else {
    deallocate();
    type  = floatT;
    store = new double;
    *((double*)store) = floatV;
  }

  return *this;
}

attrValue& attrValue::operator=(float floatV) {
  // If the attrValue's current type is already floatT
  if(type == floatT)
    // Copy the new value to the current store
    *((double*)store) = floatV;
  // Otherwise, deallocate the old store, allocate a fresh one for the new value
  else {
    deallocate();
    type  = floatT;
    store = new double;
    *((double*)store) = floatV;
  }

  return *this;
}

// Produces a customSerT type
attrValue& attrValue::operator=(customAttrValue& customV) {
  // If the attrValue's current type is already customT
  if(type == customSerT)
    // Copy the new value to the current store
    *((string*)store) = customV.serialize();
  // Otherwise, deallocate the old store, allocate a fresh one for the new value
  else {
    deallocate();
    type  = customSerT;
    store = (void*)(new string(customV.serialize()));
  }

  return *this;
}

attrValue& attrValue::operator=(const attrValue& that) {
  // If the types are the same
  if(type == that.type) {
    // Copy the value from that.store into store
         if(type == strT)       { *((string*)store)           = *((string*)that.store);           }
    else if(type == ptrT)       { *((void**)store)            = *((void**)that.store);            }
    else if(type == intT)       { *((long*)store)             = *((long*)that.store);             }
    else if(type == floatT)     { *((double*)store)           = *((double*)that.store);           }
    else if(type == customT)    { 
//      *((customAttrValue**)store) = *((customAttrValue**)that.store); }
      *((customAttrValue**)store) = (*((customAttrValue**)that.store))->copy(); 
      cout << "attrValue::attrValue(that1): ("<<((customAttrValue**)store)<<") "<<(*((customAttrValue**)store))->serialize()<<endl; }
    else if(type == customSerT) { *((string*)store)           = *((string*)that.store);           }    
    else if(type == unknownT) { }
    else {
      cerr << "attrValue::operator=() ERROR: invalid value type "<<type2str(type)<<"!"<<endl;
      assert(0);
    }
  // Otherwise, if the types are different, 
  } else {
    // Deallocate the old store
    deallocate();
    
    type = that.type;
  
    // Allocate a fresh one and copy the value from that into it
         if(type == strT)       { store = (void*)(new string(*((string*)that.store))); }
    else if(type == ptrT)       { store = new void*;            *((void**)store)            = *((void**)that.store);            }
    else if(type == intT)       { store = new long;             *((long*)store)             = *((long*)that.store);             }
    else if(type == floatT)     { store = new double;           *((double*)store)           = *((double*)that.store);           }
    else if(type == customT)    { store = new customAttrValue*; //*((customAttrValue**)store) = *((customAttrValue**)that.store); }
    *((customAttrValue**)store) = (*((customAttrValue**)that.store))->copy(); 
    //cout << "attrValue::attrValue(that2): ("<<((customAttrValue**)store)<<") "<<(*((customAttrValue**)store))->serialize()<<endl;
    }
    else if(type == customSerT) { store = (void*)(new string(*((string*)that.store))); }
    else if(type == unknownT) { }
    else {
      cerr << "attrValue::operator=() ERROR: invalid value type "<<type2str(type)<<"!"<<endl;
      assert(0);
    }
  }

  return *this;
}

// Returns the type of this attrValue's contents
attrValue::valueType attrValue::getType() const
{ return type; }

// Return the contents of this attrValue, aborting if there is a type incompatibility
std::string attrValue::getStr() const {
  if(type == strT) return *((string*)store);
  cerr << "attrValue::getStr() ERROR: value type is "<<type2str(type)<<"!"<<endl; assert(0);
}

// Return the contents of this attrValue, aborting if there is a type incompatibility
void*       attrValue::getPtr() const {
  if(type == ptrT) return *((void**)store);
  cerr << "attrValue::getPtr() ERROR: value type is "<<type2str(type)<<"!"<<endl; assert(0);
}

// Return the contents of this attrValue, aborting if there is a type incompatibility
long        attrValue::getInt() const {
  if(type == intT) return *((long*)store);
  cerr << "attrValue::getInt() ERROR: value type is "<<type2str(type)<<"!"<<endl; assert(0);
}

// Return the contents of this attrValue, aborting if there is a type incompatibility
double      attrValue::getFloat() const {
  if(type == floatT) return *((double*)store);
  cerr << "attrValue::getFloat() ERROR: value type is "<<type2str(type)<<"!"<<endl; assert(0);
}

// Return the contents of this attrValue, aborting if there is a type incompatibility
customAttrValue* attrValue::getCustom() const {
  if(type == customT) return *((customAttrValue**)store);
  cerr << "attrValue::getCustom() ERROR: value type is "<<type2str(type)<<"!"<<endl; assert(0);
}

// Return the contents of this attrValue, aborting if there is a type incompatibility
std::string attrValue::getCustomSer() const {
  if(type == customT) return *((string*)store);
  cerr << "attrValue::getCustomSer() ERROR: value type is "<<type2str(type)<<"!"<<endl; assert(0);
}

// Parse the given string as a void* pointer
void*  attrValue::parsePtr  (std::string s) {
  void* ret;
  istringstream iss(s);
  iss >> ret;
  return ret;
}

// Parse the given string as an integral value
long   attrValue::parseInt  (std::string s) {
  return strtol(s.c_str(), NULL, 10);
}

// Parse the given string as an floating point value
double attrValue::parseFloat(std::string s) {
  return strtod(s.c_str(), NULL);
}

// Encodes the contents of this attrValue into a string and returns the result.
std::string attrValue::getAsStr() const {
  if(type == strT || type == customSerT) return *((string*)store);
  else {
    ostringstream oss;
         if(type == ptrT)       oss << *((void**)store);
    else if(type == intT)       oss << *((long*)store);
    else if(type == floatT)     oss << *((double*)store);
    else if(type == customT)    { /*cout << "getAsStr() store="<<((customAttrValue**)store)<<endl; */oss << (*((customAttrValue**)store))->serialize(); }
    else  {
      cerr << "attrValue::str() ERROR: unknown attribute value type: "<<type2str(type)<<"!"<<endl;
      assert(0);
    }
    
    return oss.str();
  }
}

// Encodes the contents of this attrValue into a floating point numberand returns the result.
double attrValue::getAsFloat() const {
  switch(type) {
    case strT:       assert(0); break;
    case ptrT:       assert(0); break;
    case intT:       return (double)(*((long*)store));
    case floatT:     return *((double*)store);
    case customT:    assert(0);
    case customSerT: assert(0);
    case unknownT:   assert(0);
    default:         assert(0);
  }
}

// Encodes the contents of this attrValue into a string that can be decoded by providing it to the 
// attrValue(const std::string& strV, attrValue::valueType type) constructor
std::string attrValue::serialize() const {
  ostringstream oss;
       if(type == strT)       oss << type << ":" << *((string*)store);
  else if(type == ptrT)       oss << type << ":" << *((void**)store);
  else if(type == intT)       oss << type << ":" << *((long*)store);
  else if(type == floatT)     oss << type << ":" << *((double*)store);
  else if(type == customT)    { //cout << "serialize() store="<<((customAttrValue**)store)<<endl; 
                              oss << type << ":" << (*((customAttrValue**)store))->serialize(); }
  // The serialized type of customSerT is customT, since attrValues of type customSerT actually encode custom values
  else if(type == customSerT) {
    oss << customT << ":" << *((string*)store);
  } else  {
    cerr << "attrValue::str() ERROR: unknown attribute value type: "<<type2str(type)<<"!"<<endl;
    assert(0);
  }

  return oss.str();
}

// Decodes the contents of this string, as encoded by attrValue, setting the contents of this object based on it
void attrValue::deserialize(std::string serialized) {
  // Deallocate any storage of this object before creating a fresh object
  deallocate();
  
  // Decode the type of the serialized value
  int typeEnd = serialized.find(":");
  assert(typeEnd != string::npos);
  type = (valueType) strtol(serialized.substr(0, typeEnd).c_str(), NULL, 10);
  
  // Decode the value itself
  switch(type) {
    case strT:   store = (void*)(new string(serialized.substr(typeEnd+1)));                        break;
    case ptrT:   store = new void*;   *((void**)store)  = parsePtr(serialized.substr(typeEnd+1));   break;
    case intT:   store = new long*;   *((long*)store)   = parseInt(serialized.substr(typeEnd+1));   break;
    case floatT: store = new double*; *((double*)store) = parseFloat(serialized.substr(typeEnd+1)); break;
    case customT:
        store = new customAttrValue*; 
        *((customAttrValue**)store) = customAttrValueInstantiator::deserialize(serialized.substr(typeEnd+1)); 
        //cout << "attrValue::attrValue(type): ("<<((customAttrValue**)store)<<") "<<(*((customAttrValue**)store))->serialize()<<endl;
        break;
    case customSerT: assert(0);
    case unknownT:   assert(0);
    default:         assert(0);
  }
}

// Decodes the contents of this serialized representation of an attrValue and returns it
attrValue::valueType attrValue::getType(std::string serialized) {
  // Decode the type of the serialized value
  int typeEnd = serialized.find(":");
  assert(typeEnd != string::npos);
  return (valueType) strtol(serialized.substr(0, typeEnd).c_str(), NULL, 10);
}

bool attrValue::operator==(const attrValue& that) const {
  if(type == that.type) {
         if(type == strT)       return *((string*)store)           == *((string*)that.store);
    else if(type == ptrT)       return *((void**)store)            == *((void**)that.store);
    else if(type == intT)       return *((long*)store)             == *((long*)that.store);
    else if(type == floatT)     return *((double*)store)           == *((double*)that.store);
    else if(type == customT)    return *((customAttrValue**)store) == *((customAttrValue**)that.store);
    else if(type == customSerT) return *((string*)store)           == *((string*)that.store);
    else {
      cerr << "attrValue::operator== ERROR: invalid value type "<<type2str(type)<<"!"<<endl;
      assert(0);
    }
  } else
    return false;
}

bool attrValue::operator<(const attrValue& that) const {
  if(type == that.type) {
         if(type == strT)       return *((string*)store)           < *((string*)that.store);
    else if(type == ptrT)       return *((void**)store)            < *((void**)that.store);
    else if(type == intT)       return *((long*)store)             < *((long*)that.store);
    else if(type == floatT)     return *((double*)store)           < *((double*)that.store);
    else if(type == customT)    return *((customAttrValue**)store) < *((customAttrValue**)that.store);
    else if(type == customSerT) return *((string*)store)           < *((string*)that.store);
    else {
      cerr << "attrValue::operator< ERROR: invalid value type "<<type2str(type)<<"!"<<endl;
      assert(0);
    }
  } else {
    // All instances of each type are < or > all instances of other types, in some canonical order
    // (the enum values for different types are guaranteed to be different)
    return type < that.type;
  }
}

// Returns a human-readable representation of this object
std::string attrValue::str() const {
  return getAsStr();
}

// Compares this object to that one using the given comparator and returns their relation to each other.
// See below for a detailed explanation of how attrValue comparison works.
// The default implementation can apply only a scalarComparator to strT, ptrT, intT and floatT values
// and for customT values it forwards the comparison to the customAttrValue's own comparison method.
attrValue attrValue::compare(const attrValue& that, comparator& comp) const {
  //cout << "attrValue::compare() type="<<type2str(type)<<", that.type="<<type2str(that.type)<<endl;
  
  // The two values must be of the same type
  if(type != that.type) { cerr << "ERROR: can not compare attrValues with different types! type="<<type2str(type)<<", that.type="<<type2str(that.type)<<endl; assert(0); }
  
  // Cannot compare serialized values
  if(type==customSerT) { cerr << "ERROR: can not compare serialized custom attrValues!"<<endl; assert(0); }
  
  // To compare custom attrValues, forward the query to their comparison operation
  if(type == customT)
    return (*((customAttrValue**)store))->compare(**((customAttrValue**)that.store), comp);
  // Compare scalar attrValues
  else {
    try{
      // Convert the comparator into a scalarComparator since that is the only type of comparator we can use
      scalarComparator& scomp = dynamic_cast<scalarComparator&>(comp);
      scomp.reset(); // Reset the comparator to make it ready for a new comparison
      switch(type) {
        case strT:   scomp.compare(*((string*)store), *((string*)that.store)); break;
        case ptrT:   scomp.compare(*((void**)store),  *((void**)that.store));  break;
        case intT:   scomp.compare(*((long*)store),   *((long*)that.store));   break;
        case floatT: scomp.compare(*((double*)store), *((double*)that.store)); break;
        default: assert(0);
      }
      
      return scomp.relation();
    } catch(std::bad_cast exp) {
      cerr << "ERROR: comparing sightArray attribute value using a comparator that is not a scalarComparator!"<<endl;
      assert(0);
    }
  }
}

// -----------------------
// Custom attribute values
// -----------------------

/***************************
 ***** customAttrValue *****
 ***************************/

// Returns a serialized representation of this value
std::string customAttrValue::serialize() const {
  ostringstream s;
  
  // Add the header that identifies the name of the object that is serialized
  s << name() << ":";
  
  // Add the type-specific serialization text
  serialize(s);
  return s.str();
}

/***************************************
 ***** customAttrValueInstantiator *****
 ***************************************/

std::map<std::string, customAttrDeserialize>* customAttrValueInstantiator::deserializers;

customAttrValueInstantiator::customAttrValueInstantiator(): 
   sight::common::LoadTimeRegistry("customAttrValueInstantiator", 
                                   customAttrValueInstantiator::init) 
{
}

// Called exactly once for each class that derives from LoadTimeInstantiator to initialize its static data structures.
void customAttrValueInstantiator::init() {
  deserializers = new std::map<std::string, customAttrDeserialize>();
}

/*customAttrValueInstantiator::customAttrValueInstantiator() {
  // Initialize the handlers mappings, using environment variables to make sure that
  // only the first instance of this customAttrValueInstantiator creates these objects.
  if(!getenv("SIGHT_CUSTOM_ATTRVALUE_INSTANTIATED")) {
    deserializers          = new std::map<std::string, customAttrDeserialize>();
    setenv("SIGHT_CUSTOM_ATTRVALUE_INSTANTIATED", "1", 1);
  }
}*/

// Deserializes the given serialized customAttrValue and returns a freshly-allocated reference to it
customAttrValue* customAttrValueInstantiator::deserialize(std::string serialized) {
  // Get the name of the custom attribute type that is serialized
  int nameEnd = serialized.find(":");
  assert(nameEnd != string::npos);
  string customAttrName = serialized.substr(0, nameEnd);
  
  if(deserializers->find(customAttrName) == deserializers->end()) 
  { cerr << "ERROR: no deserializer registered for custom attribute \""<<customAttrName<<"\"!"<<endl; assert(0); }
  
  // Use the type-specific deserialization method to deserialize the rest of the text
  return (*deserializers)[customAttrName](serialized.substr(nameEnd+1));
}

std::string customAttrValueInstantiator::str() {
  std::ostringstream s;
  s << "[customAttrValueInstantiator:"<<endl;
  s << "    deserializers=(#"<<deserializers->size()<<"): ";
  for(std::map<std::string, customAttrDeserialize>::const_iterator i=deserializers->begin(); i!=deserializers->end(); i++) {
    if(i!=deserializers->begin()) s << ", ";
    s << i->first;
  }
  s << endl;
  s << "]"<<endl;
  return s.str();
}

/*******************************************
 ***** attrValueComparatorInstantiator *****
 *******************************************/

std::map<std::string, attrValueComparatorInstantiator::genAttrComparator>* attrValueComparatorInstantiator::compGenerators;

attrValueComparatorInstantiator::attrValueComparatorInstantiator(): 
   sight::common::LoadTimeRegistry("attrValueComparatorInstantiator", 
                                   attrValueComparatorInstantiator::init) 
{
}

/*attrValueComparatorInstantiator::attrValueComparatorInstantiator() {
  // Initialize the handlers mappings, using environment variables to make sure that
  // only the first instance of this attrValueComparatorInstantiator creates these objects.
  if(!getenv("SIGHT_ATTRVALUE_COMPARATOR_INSTANTIATED")) {
    compGenerators = new std::map<std::string, genAttrComparator>();
    setenv("SIGHT_ATTRVALUE_COMPARATOR_INSTANTIATED", "1", 1);
  }
}*/

// Called exactly once for each class that derives from LoadTimeInstantiator to initialize its static data structures.
void attrValueComparatorInstantiator::init() {
  compGenerators = new std::map<std::string, genAttrComparator>();
}

// Returns a comparator of the type identified by name that performs the comparison type identified by description
comparator* attrValueComparatorInstantiator::genComparator(std::string name, std::string description) {
  if(compGenerators->find(name) == compGenerators->end()) 
  { cerr << "ERROR: no comparator generator registered for label \""<<name<<"\"!"<<endl; assert(0); }
  
  // Use the type-specific generator method to create the appropriate comparator
  return (*compGenerators)[name](description);
}

std::string attrValueComparatorInstantiator::str() {
  std::ostringstream s;
  s << "[attrValueComparatorInstantiator:"<<endl;
  s << "    compGenerators=(#"<<compGenerators->size()<<"): ";
  for(std::map<std::string, genAttrComparator>::const_iterator i=compGenerators->begin(); i!=compGenerators->end(); i++) {
    if(i!=compGenerators->begin()) s << ", ";
    s << i->first;
  }
  s << endl;
  s << "]"<<endl;
  return s.str();
}

// Registers generation functions for each type of comparator we've defined here
baseAttrValueComparatorInstantiator::baseAttrValueComparatorInstantiator() {
  (*compGenerators)["LkComparator"]       = &genLkComparator;
  (*compGenerators)["RelativeComparator"] = &genRelComparator;
}

/************************
 ***** LkComparator *****
 ************************/

// Returns a comparator that can be used to compare objects of the given valueType
comparator* genLkComparator(std::string description) {
  int kEnd = description.find(":");
  int typeEnd = description.find(":", kEnd+1);
  int k = strtol(description.substr(0, kEnd).c_str(), NULL, 10);
  attrValue::valueType type = (attrValue::valueType)strtol(description.substr(kEnd+1, typeEnd) .c_str(), NULL, 10);
  bool absoluted = (bool)strtol(description.substr(typeEnd+1) .c_str(), NULL, 10);

  
  switch(type) {
    // Create an LkComparator with the appropriate parameters, absoluted is set to true
    // only if the caller requested this and k is odd since for k is even, this has no effect.
    case attrValue::intT: 
      switch(k) {
        case 0: // LkComparator::infinity
          if(absoluted) return new LkComparatorInfinity<long,  true>();
          else          return new LkComparatorInfinity<long,  false>();
        case 1: 
          if(absoluted) return new LkComparatorK<long,  1,  true>(-1);
          else          return new LkComparatorK<long,  1,  false>(-1);
        case 2: 
                        return new LkComparatorK<long,  2,  false>(-1);
        case 3: 
          if(absoluted) return new LkComparatorK<long,  3,  true>(-1);
          else          return new LkComparatorK<long,  3,  false>(-1);
        case 4: 
                        return new LkComparatorK<long,  4,  false>(-1);

        // Pass k as -1 to indicate that the dynamic argument to the constructor should be used instead of 
        // the static template argument
        default:
          if(absoluted) return new LkComparatorK<long,  -1, true>(k);
          else          return new LkComparatorK<long,  -1, false>(k);
      }

    case attrValue::floatT: 
      switch(k) {
        case 0: // LkComparator::infinity
          if(absoluted) return new LkComparatorInfinity<double, true>();
          else          return new LkComparatorInfinity<double, false>();
        case 1: 
          if(absoluted) return new LkComparatorK<double, 1,  true>(-1);
          else          return new LkComparatorK<double, 1,  false>(-1);
        case 2: 
                        return new LkComparatorK<double, 2,  false>(-1);
        case 3: 
          if(absoluted) return new LkComparatorK<double, 3,  true>(-1);
          else          return new LkComparatorK<double, 3,  false>(-1);
        case 4: 
                        return new LkComparatorK<double, 4,  false>(-1);

        // Pass k as -1 to indicate that the dynamic argument to the constructor should be used instead of 
        // the static template argument
        default:
          if(absoluted) return new LkComparatorK<double, -1, true>(k);
          else          return new LkComparatorK<double, -1, false>(k);
      }

    default: assert(0);
  }
} // genLkComparator()

// Returns a comparator that can be used to compare objects of the given valueType
comparator* genRelComparator(std::string description) {
  attrValue::valueType type = (attrValue::valueType)attrValue::parseInt(description);
  switch(type) {
    // Create an RelativeComparator of the appropriate type
    case attrValue::intT:   return new RelativeComparator<long>();
    case attrValue::floatT: return new RelativeComparator<double>();
    default: assert(0);
  }
}

// static instance of baseAttrValueComparatorInstantiator to ensure that its constructor is called
// before main().
baseAttrValueComparatorInstantiator baseAttrValueComparatorInstance;


/**********************
 ***** sightArray *****
 **********************/

sightArray::sightArray(const dims& d, void* array, attrValue::valueType type, bool arrayOwner) : 
      array(array), d(d), type(type),              arrayOwner(arrayOwner)
{ init(); }
sightArray::sightArray(const dims& d, std::string* array,                     bool arrayOwner) : 
      array(array), d(d), type(attrValue::strT),   arrayOwner(arrayOwner)
{ init(); }
sightArray::sightArray(const dims& d, char** array,                           bool arrayOwner) : 
      array(array), d(d), type(attrValue::strT),   arrayOwner(arrayOwner)
{ init(); }
sightArray::sightArray(const dims& d, void** array,                           bool arrayOwner) : 
      array(array), d(d), type(attrValue::ptrT),   arrayOwner(arrayOwner)
{ init(); }
sightArray::sightArray(const dims& d, long* array,                            bool arrayOwner) : 
      array(array), d(d), type(attrValue::intT),   arrayOwner(arrayOwner)
{ init(); }
sightArray::sightArray(const dims& d, int* array,                             bool arrayOwner) : 
      array(array), d(d), type(attrValue::intT),   arrayOwner(arrayOwner)
{ init(); }
sightArray::sightArray(const dims& d, double* array,                          bool arrayOwner) : 
      array(array), d(d), type(attrValue::floatT), arrayOwner(arrayOwner)
{ init(); }
sightArray::sightArray(const dims& d, float* array,                           bool arrayOwner) : 
      array(array), d(d), type(attrValue::floatT), arrayOwner(arrayOwner)
{ init(); }

sightArray::sightArray(const sightArray& that): d(that.d), numElements(that.numElements), type(that.type), arrayOwner(that.arrayOwner) {
  // Allocate an array to hold a copy of that.array
  int elementSize = attrValue::sizeofType(type);
  array = new char[elementSize * numElements];
  
  // Copy the contents of the array over from that.array
  memcpy(array, that.array, elementSize * numElements);
}
  
void sightArray::init() {
  // Check that the dimensions array is non empty and that each dimension has size >0
  if(d.size()==0) { cerr << "ERROR: sightArray created for an array with 0 dimensions!"<<endl; assert(0); }
  
  for(int i=0; i<d.size(); i++) {
    if(d[i]<=0) { cerr << "ERROR: sightArray created with dimensions vector where dimension "<<i<<"=0!"<<endl; assert(0); }
  }
  
  // Calculate the number of elements in the array
  numElements=1;
  for(int i=0; i<d.size(); i++) {
    numElements *= d[i];
  }
}

sightArray::~sightArray() {
  // If this sightArray object owns this array, deallocate the array along with the sightArray
  if(arrayOwner) {
    switch(type) {
      case attrValue::strT:   delete (string*)array; break;
      case attrValue::ptrT:   delete (void**) array; break;
      case attrValue::intT:   delete (long*)  array; break;
      case attrValue::floatT: delete (float*) array; break;
      default: assert(0);
    }
  }
}

// Returns a deep copy of this object
customAttrValue* sightArray::copy() const {
  return new sightArray(*this);
}

// Relational operators
bool sightArray::operator==(const customAttrValue& that_arg) {
  try{
    const sightArray& that = dynamic_cast<const sightArray&>(that_arg);
    
    // Compare them using all the properties of sightArrays, one after another.
    
    // Size of dims
    if(d.size() != that.d.size()) return false;
    
    // Contents of dims
    for(int i=0; i<d.size(); i++)
      if(d[i] != that.d[i]) return false;
    
    // Type of elements
    if(type != that.type) return false;
    
    // Contents of array
    for(int i=0; i<numElements; i++) {
      switch(type) {
        case attrValue::strT:
          if(((string*)array)[i] != ((string*)that.array)[i]) return false;
          break;
        case attrValue::ptrT:
          if(((void**)array)[i]  != ((void**)that.array)[i])  return false;
          break;
        case attrValue::intT:
          if(((long*)array)[i]   != ((long*)that.array)[i])   return false;
          break;
        case attrValue::floatT:
          if(((double*)array)[i] != ((double*)that.array)[i]) return false;
          break;
        default: assert(0);
      }
    }
  } catch(std::bad_cast exp) {
    cerr << "ERROR: comparing sightArray with a different type of customAttrValue using == operator!"<<endl;
    assert(0);
  }
  return false;
}

bool sightArray::operator< (const customAttrValue& that_arg) {
  try{
    const sightArray& that = dynamic_cast<const sightArray&>(that_arg);
    
    // Compare them using all the properties of sightArrays, one after another.
    
    // Size of dims
    if(d.size() != that.d.size()) return d.size() < that.d.size();
    
    // Contents of dims
    for(int i=0; i<d.size(); i++)
      if(d[i] != that.d[i]) return d[i] < that.d[i];
    
    // Type of elements
    if(type != that.type) return type < that.type;
    
    // Contents of array
    for(int i=0; i<numElements; i++) {
      switch(type) {
        case attrValue::strT:
          if(((string*)array)[i] != ((string*)that.array)[i]) return ((string*)array)[i] < ((string*)that.array)[i];
          break;
        case attrValue::ptrT:
          if(((void**)array)[i]  != ((void**)that.array)[i])  return ((void**)array)[i]  < ((void**)that.array)[i];
          break;
        case attrValue::intT:
          if(((long*)array)[i]   != ((long*)that.array)[i])   return ((long*)array)[i]   < ((long*)that.array)[i];
          break;
        case attrValue::floatT:
          if(((double*)array)[i] != ((double*)that.array)[i]) return ((double*)array)[i] < ((double*)that.array)[i];
          break;
        default: assert(0);
      }
    }
  } catch(std::bad_cast exp) {
    cerr << "ERROR: comparing sightArray with a different type of customAttrValue using < operator!"<<endl;
    assert(0);
  }
  return false;
}

// Adds the string representation of this value to the given output stream
void sightArray::serialize(ostream& s) const {
  // The format is:
  // numDims:dim1,dim2,...dim_numDims:type:val0,val1,val0,...
  
  // numDims
  s << d.size() << ":";
  
  // Individual dimensions
  int totalVals=1; // The total number of values in the array
  for(dims::const_iterator i=d.begin(); i!=d.end(); i++) {
    if(i!=d.begin()) s << ",";
    s << *i;
    totalVals *= *i;
  }
  s << ":";
  
  // The type of the values:
  s << type << ":";
  
  // Individual values
  for(int i=0; i<totalVals; i++) {
    if(i>0) s << ",";
    switch(type) {
      case attrValue::strT:   s << ((string*)array)[i]; break;
      case attrValue::ptrT:   s << ((void**)array)[i];  break;
      case attrValue::intT:   s << ((long*)array)[i];   break;
      case attrValue::floatT: s << ((double*)array)[i]; break;
      default: assert(0);
    }
  }
}
  
// Deserializes instances of sightArray
customAttrValue* sightArray::deserialize(std::string serialized) {
  // The format is:
  // numDims:dim1,dim2,...dim_numDims:type:val0,val1,val0,...
  
  // Decode the number of dimensions
  size_t endNumDims = serialized.find(":");
  long numDims = strtol(serialized.substr(0, endNumDims).c_str(), NULL, 10);
  assert(numDims>0);
  
  //size_t endDims = serialized.find(":", endNumDims+1);
  // Decode the sizes of each dimension
  dims d;
  int curStrLoc = endNumDims+1;
  int totalVals=1; // The total number of values in the array
  for(int i=0; i<numDims; i++) {
    // Find the next delimiter character
    size_t endCurVal = serialized.find((i<numDims-1? ",": ":"), curStrLoc);
    
    // Decode the current dimension
    long curDim = strtol(serialized.substr(curStrLoc, endCurVal-curStrLoc).c_str(), NULL, 10);
    assert(curDim>0);
    
    // Add it to dims
    d.push_back(curDim);
    totalVals *= curDim;
    
    curStrLoc = endCurVal+1;
  }
  
  // Decode the type of the values
  size_t endType = serialized.find(":", curStrLoc);
  attrValue::valueType type = (attrValue::valueType)strtol(serialized.substr(curStrLoc, endType-curStrLoc).c_str(), NULL, 10);
  
  // Allocate an array to hold totalVals instances of the given type
  int elementSize = attrValue::sizeofType(type);
  void* array = new char[elementSize * totalVals];
  
  // Read out each element of the matrix
  for(int i=0; i<totalVals; i++) {
    // Find the next delimiter character
    size_t endCurVal = (i<totalVals-1? serialized.find(",", curStrLoc) : string::npos);
    // Decode the current value
    switch(type) {
      case attrValue::strT:   ((string*)array)[i] =                       serialized.substr(curStrLoc, endCurVal-curStrLoc);  break;
      case attrValue::ptrT:   ((void**)array) [i] = attrValue::parsePtr  (serialized.substr(curStrLoc, endCurVal-curStrLoc)); break;
      case attrValue::intT:   ((long*)array)  [i] = attrValue::parseInt  (serialized.substr(curStrLoc, endCurVal-curStrLoc)); break;
      case attrValue::floatT: ((double*)array)[i] = attrValue::parseFloat(serialized.substr(curStrLoc, endCurVal-curStrLoc)); break;
      default: assert(0);
    }
    
    curStrLoc = endCurVal+1;
  }
  
  // Generate and return a new sightArray that owns the array buffer we just allocated and will
  // deallocate this buffer when it is deallocated.
  return new sightArray(d, array, type, true);
}

const sightArray& castToSightArray(const customAttrValue& that) {
  try {
    return dynamic_cast<const sightArray&>(that);
  } catch(std::bad_cast exp) {
    cerr << "ERROR: attempting to compare a sightArray with a non-sightArray!"<<endl;
    assert(0);
  }
}

scalarComparator& castToScalarComparator(comparator& comp) {
  try {
    return dynamic_cast<scalarComparator&>(comp);
  } catch(std::bad_cast exp) {
    cerr << "ERROR: attempting to compare one sightArray with a another using a comparator that is not an scalarComparator!"<<endl;
    assert(0);
  }
}

// Compares this object to that one using the given comparator and returns their relation to each other
attrValue sightArray::compare(const customAttrValue& that_arg, comparator& comp_arg) const {
  const sightArray& that = castToSightArray(that_arg);
  scalarComparator& comp = castToScalarComparator(comp_arg);

  if(numElements != that.numElements) { cerr << "ERROR: cannot compare two sightArrays with different element counts: "<<numElements<<" and "<<that.numElements<<"!" << endl; assert(0); }
  if(d != that.d) { cerr << "ERROR: comparison of sightArrays with different dimensionality is undefined!" << endl; assert(0); }
  
  // Iterate over all the elements in the two sightArrays, comparing them element-wise
  for(int i=0; i<numElements; i++) {
    switch(type) {
      case attrValue::strT:   comp.compare(((string*)array)[i], ((string*)that.array)[i]); break;
      case attrValue::ptrT:   comp.compare(((void**)array)[i],  ((void**)that.array)[i]);  break;
      case attrValue::intT:   comp.compare(((long*)array)[i],   ((long*)that.array)[i]);   break;
      case attrValue::floatT: comp.compare(((double*)array)[i], ((double*)that.array)[i]); break;
      default: assert(0);
    }
  }
  
  return comp.relation();
}

// Registers deserialization functions for each custom attrValue type we've defined here
baseCustomAttrValueInstantiator::baseCustomAttrValueInstantiator() {
  (*deserializers)["sightArray"] = sightArray::deserialize;
}

baseCustomAttrValueInstantiator baseCustomAttrValueInstance;

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
    notifyObsPre(key, attrObserver::attrAdd);
    m[key].insert(val);
    notifyObsPost(key, attrObserver::attrAdd);
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
  //cout << "attributesC::replace("<<key<<") modified="<<modified<<endl;
  if(modified) {
    notifyObsPre(key, attrObserver::attrReplace);
    m[key].clear();
    m[key].insert(val);
    notifyObsPost(key, attrObserver::attrReplace);
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
    notifyObsPre(key, attrObserver::attrRemove);
    // Remove the key->val mapping and if this is the only mapping for key, remove its entry in m[] as well
    m[key].erase(val);
    if(m[key].size()==0)
      m.erase(key);
    notifyObsPost(key, attrObserver::attrRemove);
  }
  return modified;
}

// Removes the mapping of this key to any value.
// Returns true if the attributes map changes as a result and false otherwise.
bool attributesC::remove(string key) {
  bool modified = (m.find(key)!=m.end());
  if(modified) {
    notifyObsPre(key, attrObserver::attrRemove);
    // Remove the key and all its mappings from m[]
    m.erase(key);
    notifyObsPost(key, attrObserver::attrRemove);
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
  
  //cout << "attributesC::remObs() key="<<key<<", obs="<<obs<<", o[key][obs]="<<o[key][obs]<<endl;
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
void attributesC::notifyObsPre(std::string key, attrObserver::attrObsAction action) {
  //cout << "    attributesC::notifyObsPre("<<key<<") #o[key]="<<o[key].size()<<endl;
  for(map<attrObserver*, int>::iterator i=o[key].begin(); i!=o[key].end(); i++) {
    assert(i->second>0);
    i->first->observePre(key, action);
  }
}

// Notify all the observers of the given key after its mapping is changed (call attrObserver::observePost())
void attributesC::notifyObsPost(std::string key, attrObserver::attrObsAction action) {
  for(map<attrObserver*, int>::iterator i=o[key].begin(); i!=o[key].end(); i++) {
    assert(i->second>0);
    i->first->observePost(key, action);
  }
}

/***********************************************************
 ***** Support for parsing files with attribute values *****
 ***********************************************************/

// Reads the given file of key/value mappings, calling the provided functor on each instance.
// Each line is a white-space separated sequence of mappings in the format group:key:value and each
// call to functor f is provided with a 2-level map that maps each group to a map of key->value mappings.
void readAttrFile(std::string fName, attrFileReader& f) {
  FILE* in = fopen(fName.c_str(), "r");
  if(in==NULL) { cerr << "ERROR opening file \""<<fName<<"\" for reading!"<<endl; exit(-1); }
  
  char line[100000];
  int lineNum=1;
  while(fgets(line, 100000, in)) {
    std::map<std::string, std::map<std::string, std::string> > readData;
    escapedStr eLine(line, " ", escapedStr::escaped);
    vector<string> data = eLine.unescapeSplit(" ");
    //cout << "eLine=\""<<eLine.unescape()<<"\""<<endl;
    
    for(vector<string>::iterator d=data.begin(); d!=data.end(); d++) {
      escapedStr eD(*d, ":", escapedStr::escaped);
      vector<string> data = eD.unescapeSplit(":");
      /*cout << "    eD=\""<<eD.unescape()<<"\": ";
      for(vector<string>::iterator i=data.begin(); i!=data.end(); i++)
        cout << *i << " ";
      cout << endl;*/
      // If this is not the end of the line (empty data or data with one element '\n')
      if(data.size()>1) {
        // Data must have 3 fields: group, key, value
        assert(data.size()==3);
        readData[data[0]][data[1]] = data[2];
      }
    }
    
    // Call the user-provided functor
    f(readData, lineNum);
    
    lineNum++;
  }

  fclose(in);
}

}; // namespace common
}; // namespace sight
