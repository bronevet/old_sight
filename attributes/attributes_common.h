#pragma once
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <math.h>
#include "../sight_common_internal.h"
#include <typeinfo>
#include <boost/shared_ptr.hpp>
#include <iomanip>

namespace sight {
namespace layout {
class attributesC;
}
namespace structure {
class attrOp;
class attrQuery;
class attrSubQuery;
class attributesC;
}}

namespace sight {
// Definitions below are placed in the generic sight namespace because there is no chance of name conflicts
//namespace common {

// ****************************
// ***** Attribute Values *****
// ****************************

class customAttrValue;
class comparator;

// Wrapper class for strings, integers and floating point numbers that keeps track of the
// type of its contents and allows functors that work on only one of these types to be applied.
class attrValue {
  public:
  typedef enum {
     // Scalar types:
     strT,       // std::string
     ptrT,       // void* pointer
     intT,       // Integral value (long)
     floatT,     // Floating-point value (double)

     // Complex types:
     customT,    // Custom type (pointer to the object that contains its data)
     customSerT, // Custom type (serialized representation, stored as a string)

     unknownT    // The type is not known
   } valueType;

  // Returns the size of a single instance of the given type
  static int sizeofType(valueType type);

  // Returns the string representation of the given type
  static std::string type2str(valueType type);

  friend class sight::structure::attrOp;

  // The type of the value's contents
  valueType type;

  // The storage for the four possible value types
  void* store;

  public:
  attrValue();
  // Creates an attribute value based on the encoding within the given string. If type is set to unknownT,
  // the constructor figures out the type based on the encoding in the string, which must have been set
  // by attrValue::serialize(). Otherwise, it specializes to the specified types and ignores alternate
  // encoding possibilities.
  attrValue(const std::string& strV, attrValue::valueType type);
  attrValue(const std::string& strV);
  attrValue(char* strV);
  attrValue(void* ptrV);
  attrValue(long intV);
  attrValue(int intV);
  attrValue(double floatV);
  attrValue(float floatV);
  // Produces a customSerT type
  attrValue(const customAttrValue& customV);
  attrValue(const attrValue& that);
  // Deallocates the current store
  void deallocate();
  ~attrValue();

  attrValue& operator=(const attrValue& that);
  attrValue& operator=(const std::string& strV);
  attrValue& operator=(char* strV);
  attrValue& operator=(void* ptrV);
  attrValue& operator=(long intV);
  attrValue& operator=(int intV);
  attrValue& operator=(double floatV);
  attrValue& operator=(float floatV);
  // Produces a customSerT type
  attrValue& operator=(customAttrValue& customV);

  // Returns the type of this attrValue's contents
  valueType getType() const;

  // Return the contents of this attrValue, aborting if there is a type incompatibility.
  std::string      getStr() const;
  void*            getPtr() const;
  long             getInt() const;
  double           getFloat() const;
  customAttrValue* getCustom() const;
  std::string      getCustomSer() const;

  // Parse the given string as one of the scalar types that can be stored in an attrValue
  static void*  parsePtr  (std::string s);
  static long   parseInt  (std::string s);
  static double parseFloat(std::string s);

  // Encodes the contents of this attrValue into a string and returns the result.
  std::string getAsStr() const;

  // Encodes the contents of this attrValue into a floating point number and returns the result.
  double getAsFloat() const;

  // Encodes the contents of this attrValue into a string that can be decoded by providing it to the
  // attrValue(const std::string& strV, attrValue::valueType type) constructor
  std::string serialize() const;

  // Decodes the contents of this string, as encoded by attrValue, setting the contents of this object based on it
  void deserialize(std::string serialized);

  // Decodes the contents of this serialized representation of an attrValue and returns it
  static valueType getType(std::string serialized);

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

  // Compares this object to that one using the given comparator and returns their relation to each other.
  // See below for a detailed explanation of how attrValue comparison works.
  // The default implementation can apply only a scalarComparator to strT, ptrT, intT and floatT values
  // and for customT values it forwards the comparison to the customAttrValue's own comparison method.
  attrValue compare(const attrValue& that, comparator& comp) const;
}; // class attrValue

/********************************************
 ***** Comparison among attributeValues *****
 ********************************************/
/*
The comparableCustomAttrValue class corresponds to customAttrValues that can be compared to each other. The compare
method takes a reference to another instance of the same variant of comparableCustomAttrValue (e.g. if we have a comparable
graph, its compare method takes a reference to comparableCustomAttrValue but it can assume that the given object
is actually another comparable graph and throw an assertion violation if this assumption is not satisfied) and
a reference to a comparator object that defines the comparison that is to be performed and may be instrumental
in performing the comparison (e.g. look at the LkComparator class below). It is expected that individual derivations
of comparableCustomAttrValue will be compatible with individual types of comparators but we don't currently
place any constraints on how the two relate to each other. Also, the only method we require comparators to implement
is a reset() method that reinitializes its state after a comparison to make it ready for another comparison without
the need to create a fresh comparator for each comparison.

The comparableCustomAttrValue::compare() method returns another attrValue that describes the relationship between the
two values. For instance, the relationship between two vectors may be the floating point number that records their root
mean square error, while the relationship between two text documents may be a complex data structure that records
their common and different sub-sections.

It is envisioned that comparison is structured as follows. A given widget in the structure layer takes as input a
comparableCustomAttrValue and an instance of a comparatorDesc class that can be serialized into two strings,
a unique name that identifies the comparator type and a description of the type of comparison this comparator needs
to do. This widget outputs the serialized encoding of the comparableCustomAttrValue and the comparator to the log.

When this information is read back and the widget must perform the comparison, it loads the relevant
comparableCustomAt
 * trValues into memory, uses the comparator's name to get an instance of the comparator object from
the attrValueComparatorInstantiator class (described below). It then calls the compare method of the relevant
comparableCustomAttrValues, passing to them a reference of the comparator object.
*/

// Describes the type of comparison that comparableCustomAttrValue::compare() will need to perform
class comparator {
  public:
  // Resets the state of this comparator, making it ready for another comparison.
  virtual void reset()=0;

  virtual std::string str()=0;

  // Returns whether this comparator performs no comparison. This is important for cases where users do not wish
  // to perform a comparison on a given attribute.
  virtual bool isNoComparator() { return false; }
};

// Describes the comparator object that will be employed during subsequent analysis phases
class comparatorDesc {
  public:
  // Returns the unique name of the given comparator type (e.g. LkComparator)
  virtual std::string name() const=0;

  // Returns the description of the type of comparison needs to be performed (e.g. the value of k)
  virtual std::string description() const=0;
};

// This class corresponds to a common sub-case of comparableCustomAttrValue, where the compare method returns a
// single floating point value. A common use-case for this is clustering algorithms that require a numeric distance
// between every object. This class is employed by inheriting from it instead of comparableCustomAttrValue and
// implementing comparableCustomAttrValue::compare(). This class takes care of pulling the numeric value out of the
// returned attrValue and returning this double instead.
/*class numComparableCustomAttrValue: public comparableCustomAttrValue {
  public:
  double compare(const numComparableCustomAttrValue& that, comparator& comp) {
    attrValue val = comparableCustomAttrValue::compare((const comparableCustomAttrValue&)that, comp);
    assert(val.getType() == attrValue::intT || val.getType() == attrValue::floatT);
    return val.getAsFloat();
  }
};*/ // class numComparableCustomAttrValue


/* A common use-case for comparison is to compare individual scalar data items. These may be the individual data
   items in an attrValue or may be elements within different container objects such as matrixes or lists, where we wish
   to compare the overall objects by comparing their individual elements. This pattern is captured by the
   scalarComparator class, which provides the compare method, which should be applied by callers to pairs
   of elements, one from each container object being compared. As the comparator is applied to more and more of the
   elements in the two containers, it builds up a measure of the overall relationship between the two. This
   relationship can be obtained by calling the relation() method of the comparator.

   A big challenge in making comparators is to keep them generic while ensuring that it is possible to implement them
   efficiently. The approach taken here is to have the comparator provide implementations for compare that work
   for all the four scalar datatypes that can be stored in attrValues. We use the attrValueComparatorInstantiator class
   to register generator functions for comparators. This function returns an instance of the scalar comparator
   specialized to a given type of scalar value. This comparator is guaranteed to only be called using values of the
   appropriate type and as such, can leave the other methods with their default implementations, which emit an assertion violation.
*/

class scalarComparator : public comparator {
  public:
  // Called on each pair of elements from a container object, one from each object. None of these
  // functions is mandatory because individual scalar comparators may only support a subset of the
  // given datatypes (e.g. a numerical comparator may not support strings and vice versa).
  virtual void compare(const std::string& str1,   const std::string& str2)   { std::cerr << "ERROR: This comparator does not support strings!"<<std::endl;                assert(0); }
  virtual void compare(const void*        ptr1,   const void*        ptr2)   { std::cerr << "ERROR: This comparator does not support pointers!"<<std::endl;               assert(0); }
  virtual void compare(long               int1,   long               int2)   { std::cerr << "ERROR: This comparator does not support integral numbers!"<<std::endl;       assert(0); }
  virtual void compare(double             float1, double             float2) { std::cerr << "ERROR: This comparator does not support floating point numbers!"<<std::endl; assert(0); }

  // Called to get the overall relationship between the two objects given all the individual elements
  // observed so far
  virtual attrValue relation()=0;

  // Returns whether the given pointer to or reference to a comparator corresponds to a scalarComparator
  static bool instanceOf(const comparator* comp);
  static bool instanceOf(const comparator& comp);

  // Given a pointer or reference to a comparator returns the corresponding pointer/reference to
  // a scalar comparator or aborts if the object is not a scalar comparator
  static scalarComparator* castTo(comparator* comp);
  static const scalarComparator* castTo(const comparator* comp);
  static scalarComparator& castTo(comparator& comp);
  static const scalarComparator& castTo(const comparator& comp);
}; // class scalarComparator

// This is the most generic comparator class. Its compare method takes in arbitrary attrValues
// and returns the relation between them.
class generalComparator : public comparator {
  public:
  // Called on each pair of elements from a container object, one from each object. Returns
  // the relation between them.
  virtual attrValue compare(const attrValue& str1,   const attrValue& str2)=0;

  // Returns whether the given pointer to or reference to a comparator corresponds to a generalComparator
  static bool instanceOf(const comparator* comp);
  static bool instanceOf(const comparator& comp);

  // Given a pointer or reference to a comparator returns the corresponding pointer/reference to
  // a scalar comparator or aborts if the object is not a scalar comparator
  static generalComparator* castTo(comparator* comp);
  static const generalComparator* castTo(const comparator* comp);
  static generalComparator& castTo(comparator& comp);
  static const generalComparator& castTo(const comparator& comp);
}; // class generalComparator


// Class that manages the registration of comparator factory objects. Each type of comparator creates an instance of a
// factory object and records the mapping from a unique string label to a reference to the factor object in a map.
class attrValueComparatorInstantiator : public sight::common::LoadTimeRegistry {
  // Type of the functions that create comparators.
  // They returns the comparator that can be used to perform the comparison encoded in the description string.
  typedef comparator* (*genAttrComparator)(std::string description);

  public:
  static std::map<std::string, genAttrComparator>* compGenerators;

  attrValueComparatorInstantiator();

  // Unique string name of the class that derives from LoadTimeRegistry
  //virtual std::string name() const { return "attrValueComparatorInstantiator"; }

  // Called exactly once for each class that derives from LoadTimeRegistry to initialize its static data structures.
  static void init();

  // Returns a comparator of the type identified by name that performs the comparison type identified by description
  static comparator* genComparator(std::string name, std::string description);

  static std::string str();
}; // class attrValueComparatorInstantiator

// -----------------------
// Custom attribute values
// -----------------------
// It is possible for individual widgets to extend the set of values that can be encapsulated by attrValues.
// Essentially, there are three challenges with supporting such custom values:
// - The values must be serialized to be encoded into the structured output. This requires the custom attrValue to have
//   a serialization method provided together with it.
// - The serialized representation must be deserialized at some point in order to allow widgets that understand this
//   attrValue type to actually operate on it. This requires a deserialization operation to be available at the point
//   whether either the structure layer reads the serialized representation for merging or at the point where the
//   deserialization layer reads it for doing the final analysis and layouts. Since at this point we only have the
//   serialized text and not an object, we need a system for registering deserialization methods (take in a string
//   and return a customAttrValue object) for each type of custom attribute. This means that each custom attribute
//   must be associated with a unique string label.
// - Various widgets may desire additional functionality from custom attributes such as the ability to compare
//   different instances of the same attribute type. This is handled by having modules create additional custom
//   attributes with new names, either extending the base customAttrValue class or by extending specific classes
//   that derive from it (e.g. tensors, trees or whatever else people come up with).

// The base class from which all custom attributes will be derived
class customAttrValue {
  public:
  // Returns the unique name of this custom attrValue type. This must be the same name used to register
  // a deserializer for this class.
  virtual std::string name() const=0;

  // Returns a deep copy of this object
  virtual customAttrValue* copy() const=0;

  // Relational operators
  virtual bool operator==(const customAttrValue& that)=0;
  virtual bool operator< (const customAttrValue& that)=0;

  // Returns a serialized representation of this value
  std::string serialize() const;

  // Adds the string representation of this value to the given output stream
  virtual void serialize(std::ostream& s) const=0;

  // Compares this object to that one using the given comparator and returns their relation to each other.
  virtual attrValue compare(const customAttrValue& that, comparator& comp) const=0;

  customAttrValue() {}
  virtual ~customAttrValue() {}
}; // class customAttrValue

// Type of deserialization functions for custom attributes
typedef customAttrValue* (*customAttrDeserialize)(std::string serialized);

// Class that manages the registration of deserialization functions. Each widget that creates its own custom attrValues
// needs to create a class that derives from this one and in that class's constructor create an instance of this class
// to map the unique labels of their custom attrValues to their corresponding deserialization functions. Then widgets
// must create a static instance of this class to make sure that the registration occurs before main() is executed.
class customAttrValueInstantiator : public sight::common::LoadTimeRegistry {
  public:
  static std::map<std::string, customAttrDeserialize>* deserializers;

  customAttrValueInstantiator();

  // Called exactly once for each class that derives from LoadTimeInstantiator to initialize its static data structures.
  static void init();

  // Deserializes the given serialized customAttrValue and returns a freshly-allocated reference to it
  static customAttrValue* deserialize(std::string serialized);

  static std::string str();
}; // class customAttrValueInstantiator

// The NULL comparator that does not perform any comparison at all and instead always
// returns the first argument. By convention when we compare a given observation to a
// reference observation we call observation->compare(reference, comparator), which
// causes the first argument to be comparator's compare() method to be the non-reference
// observation.
class noComparator: virtual public scalarComparator, virtual public generalComparator {
  attrValue rel;
  public:
  noComparator() {};

  // Called on each pair of elements from a container object, one from each object
  void compare(const std::string& str1,   const std::string& str2)   { rel = attrValue(str1); }
  void compare(const void*        ptr1,   const void*        ptr2)   { rel = attrValue((void*)ptr1); }
  void compare(long               int1,   long               int2)   { rel = attrValue(int1); }
  void compare(double             float1, double             float2) { rel = attrValue(float1); }

  // Called to get the overall relationship between the two objects given all the individual elements
  // observed so far. In the noComparator we simply return the relationship between the objects
  // passed into the last scalar compare() call.
  attrValue relation() { return rel; }

  attrValue compare(const attrValue& val1, const attrValue& val2) { return val1; }

  // Resets the state of this comparator, making it ready for another comparison.
  // Do nothing since the rel object gets reset on every comparison
  void reset() {}

  std::string str() { return txt()<<"[noComparator]"; }

  // Returns whether this comparator performs no comparison. This is important for cases where users do not wish
  // to perform a comparison on a given attribute.
  bool isNoComparator() { return true; }
}; // class noComparator

// Describes the NoComparator object that will be employed during subsequent analysis phases
class noComp : public comparatorDesc {
  public:
  noComp() {}

  // Returns the unique name of the given comparator type
  std::string name() const { return "NoComparator"; }

  // Returns the description of the type of comparison needs to be performed (e.g. the value of k)
  std::string description() const { return ""; }

  // Returns a comparator that can be used to compare objects of the given valueType
  static comparator* generate(std::string description) { return (scalarComparator*)(new noComparator()); }
}; // class noComp

// The equality comparator checks whether two scalars are equal (same value)
// or not. The relation is an integer that is 0 for equal (no difference) and 1 for unequal (maximal difference).
class eqComparator: virtual public scalarComparator {
  bool eq;
  public:
  eqComparator() : eq(true) {};

  // Called on each pair of elements from a container object, one from each object
  void compare(const std::string& str1,   const std::string& str2)   { eq = eq && (str1  !=str2  ); }
  void compare(const void*        ptr1,   const void*        ptr2)   { eq = eq && (ptr1  !=ptr2  ); }
  void compare(long               int1,   long               int2)   { eq = eq && (int1  !=int2  ); }
  void compare(double             float1, double             float2) { eq = eq && (float1!=float2); }

  // Called to get the overall relationship between the two objects given all the individual elements
  // observed so far. In the noComparator we simply return the relationship between the objects
  // passed into the last scalar compare() call.
  attrValue relation() { return attrValue((int)eq); }

  // Resets the state of this comparator, making it ready for another comparison.
  // Do nothing since the rel object gets reset on every comparison
  void reset() { eq = true; }

  std::string str() { return txt()<<"[eqComparator]"; }

  // Returns whether this comparator performs no comparison. This is important for cases where users do not wish
  // to perform a comparison on a given attribute.
  bool isNoComparator() { return false; }
}; // class noComparator

// Describes the NoComparator object that will be employed during subsequent analysis phases
class eqComp : public comparatorDesc {
  public:
  eqComp() {}

  // Returns the unique name of the given comparator type
  std::string name() const { return "EqComparator"; }

  // Returns the description of the type of comparison needs to be performed (e.g. the value of k)
  std::string description() const { return ""; }

  // Returns a comparator that can be used to compare objects of the given valueType
  static comparator* generate(std::string description) { return (scalarComparator*)(new eqComparator()); }
}; // class eqComp

// A specific instance of scalar comparison: the Lk norm. This is a numeric comparator and therefore can only
// compare integral and floating point values.
template<typename EltType, // The type of elements this comparator operates on
         int k, // The k of the norm. The infinity norm can be requested by specifying k=0. If k is set to -1, we
                // use the value of dynamicK provided to the constructor instead of this fixed template parameter.
         bool absoluted> // Records whether we should compute the absolute value of each element-wise difference
                         // before computing the norm (relevant for k=0 and odd k).
class LkComparator: public scalarComparator {
  protected:

  // Presents infinity to refer to k=0
  static int infinity;

  // The total number of comparison observations
  int count;

  // The running sum of the Lk differences between them
  EltType sum;

  // Dynamic copy of k. This is used of the static k is set to a negative value, meaning that we're not relying
  // static optimizations and will instead dynamically compute powers of k using a loop
  int dynamicK;

  public:
  LkComparator(int dynamicK): count(0), sum(0), dynamicK(dynamicK) {}

  // Absolute value of v
  EltType abs(EltType v) {
    return (v<0? -v: v);
  }

  // v to the power of k (k > 0)
  EltType powk(EltType v, int givenK) {
    // Uses the repeated squaring algorithm, which breaks k down into its binary representation
    // and for each 1 in location i of k multipiles in a factor of 2^i
    int subK = givenK; // Initially, this is k and in each iteration we remove the least significant bit
    EltType res = 1; // The final result, into which we accumulate the product terms
    EltType term = 1; // Holds powers of 2
    while(subK != 0) {
      term = term * v;

      // If the bit i of k is 1
      if(subK%2 == 1)
        // Multiply res by 2^i
        res = res * term;

      // Remove the least significant bit from k and move on to the next iteration
      subK /= 2;
    }

    return res;
  }

  // Called on each pair of elements from a container object, one from each object
  void compare(EltType elt1, EltType elt2) {
    // Update the sum with the k-th power of the difference between elt1 and elt2. We create separate conditions
    //   for different k since small powers of k can be computer very efficiently without the need for the general
    //   exponentiation algorithm.
    // Although we have conditionals here, the fact that k and absoluted are template parameters should allow the
    //   compiler to optimize them away
    EltType diff;
    if(absoluted) diff=abs(elt1-elt2);
    else          diff=elt1-elt2;

    // Infinity norm
    if(k==0) {
      if(count==0) sum = diff;
      else         sum = (diff > sum? diff: sum);
    }
    // k norm
    else if(k==1) sum += diff;
    else if(k==2) sum += diff*diff;
    else if(k==3) sum += diff*diff*diff;
    else if(k==3) sum += diff*diff*diff*diff;
    else if(k>0)  sum += pow(diff, k);
    else          sum += pow(diff, dynamicK);

    count++;
  }

  // Called to get the overall relationship between the two objects given all the individual elements
  // observed so far
  //attrValue relation();

  // Resets the state of this comparator, making it ready for another comparison.
  void reset() {
    count = 0;
    sum = 0;
  }

  std::string str() {
    return txt()<< std::setprecision(16) << "[LkComparator: type=\""<<typeid(EltType).name()<<"\", k="<<(k>=0? k: dynamicK)<<", count="<<count<<", sum="<<sum<<"]";
  }
}; // LkCompatator

// Two specializations of LkComparator::relation for the k=0 and k!=0
template<typename EltType, bool absoluted>
class LkComparatorInfinity: public LkComparator<EltType, 0, absoluted> {
  public:
  LkComparatorInfinity() : LkComparator<EltType, 0, absoluted>(-1) {}

  attrValue relation() {
    // k=0
    return LkComparator<EltType, 0, absoluted>::sum;
  }
};

template<typename EltType, int k, bool absoluted>
class LkComparatorK: public LkComparator<EltType, k, absoluted> {
  public:
  LkComparatorK(int dynamicK): LkComparator<EltType, k, absoluted>(k) {}

  attrValue relation() {
    // k != 0
    return pow(LkComparator<EltType, k, absoluted>::sum, (double)1/k) /
           LkComparator<EltType, k, absoluted>::count;
  }
};

// Describes the Lk Comparator object that will be employed during subsequent analysis phases
class LkComp : public comparatorDesc {
  int k;
  attrValue::valueType type;
  bool absoluted;

  public:
  LkComp(int k, attrValue::valueType type, bool absoluted=false) : k(k), type(type), absoluted(absoluted) {}

  // Returns the unique name of the given comparator type
  std::string name() const { return "LkComparator"; }

  // Returns the description of the type of comparison needs to be performed (e.g. the value of k)
  std::string description() const {
    return txt()<<k<<":"<<type<<":"<<absoluted;
  }

  // Returns a comparator that can be used to compare objects of the given valueType
  static comparator* generate(std::string description);
}; // class LkComp


// A specific instance of elementwise comparison: the average relative difference sigma_i (x_i-x_i')/(max(x_i, x_i').
// This is a numeric comparator and therefore can only compare integral and floating point values.
template<typename EltType> // The type of elements this comparator operates on
class RelativeComparator: public scalarComparator {
  protected:

  // The total number of comparison observations
  int count;

  // The running sum of the Lk differences between them
  double sum;

  public:
  RelativeComparator(): count(0), sum(0) {}

  // Absolute value of v
  EltType abs(EltType v) {
    return (v<0? -v: v);
  }

   // Called on each pair of elements from a container object, one from each object
  void compare(EltType elt1, EltType elt2) {
    sum += ((double)elt1 - elt2) / (abs(elt1) > abs(elt2)? abs(elt1): abs(elt2));
    count++;
    //std::cout << "Rel:compare("<<elt1<<", "<<elt2<<") sum="<<sum<<", count="<<count<<std::endl;
  }

  // Called to get the overall relationship between the two objects given all the individual elements
  // observed so far
  attrValue relation() {
    //std::cout << "Rel:relation: sum="<<sum<<", count="<<count<<", rel="<<attrValue(sum/count).serialize()<<std::endl;
    if(count>0) return attrValue((double)sum/count);
    else        return attrValue(0);
  }

  // Resets the state of this comparator, making it ready for another comparison.
  void reset() {
    count = 0;
    sum = 0;
  }

  std::string str() {
    return txt()<<"[RelativeComparator: type=\""<<typeid(EltType).name()<<"\", count="<<count<<", sum="<<sum<<"]";
  }
}; // RelativeCompatator

// Describes the Relative Comparator object that will be employed during subsequent analysis phases
class RelComp : public comparatorDesc {
  attrValue::valueType type;

  public:
  RelComp(attrValue::valueType type) : type(type) {}

  // Returns the unique name of the given comparator type
  std::string name() const { return "RelativeComparator"; }

  // Returns the description of the type of comparison needs to be performed (e.g. the value of k)
  std::string description() const {
    return txt()<<type;
  }

  // Returns a comparator that can be used to compare objects of the given valueType
  static comparator* generate(std::string description);
}; // class RelComp

// Registers generation functions for each type of comparator we've defined here
class baseAttrValueComparatorInstantiator : public attrValueComparatorInstantiator {
  public:
  baseAttrValueComparatorInstantiator();
};

// static instance of baseAttrValueComparatorInstantiator to ensure that its constructor is called
// before main().
extern baseAttrValueComparatorInstantiator baseAttrValueComparatorInstance;

// We now use the above infrastructure to define a few standard customAttrValues. We define them here rather as part
// of the base attrValue class both to illustrate the mechanism and also to leave attrValue to encode scalars, while
// here we provide implementations for standard collections.


/**********************
 ***** sightArray *****
 **********************/
/*
// Simple implementation of a reference counted shared pointer that allows us to choose whether
// sightArray owns and reference counts an array or is a thin wrapper around an array that is owned
// by the user.
template <class T>
class refCntPtr
{
public:
  explicit refCntPtr(T* pointee, bool owner) : pointee_(pointee), owner_(owner) {
    if(owner) count = 1;
    else      count = 0;
  }
  refCntPtr(const refCntPtr* that) : pointee_(that.pointee), owner_(that.owner_) {
    if(owner_) count_ = that.count_+1;

   }
   refCntPtr& operator=(const refCntPtr& other) {
     if(
   }
   ~refCntPtr();
   T& operator*() const
   {
      return *pointee_;
   }
   T* operator->() const
   {
      return pointee_;
   }
private:
   T* pointee_;
   bool owner_;
   int count;
};*/

// Denotes arrays of any dimensionality.
// sightArrays maintain a pointer to a given array instead of making a private copy. As such, callers must ensure to
// not deallocate the array until they've deallocated any sightArrays that refer to it. Callers also have the option
// of making the sightArray the owner of the array, meaning that it will deallocate the array using delete when
// it is deallocated.
class sightArray : public customAttrValue {
  public:
  // Definition for specifying the dimensionality of arrays.
  typedef common::easyvector<int> dims;

  protected:
  // The dimensionality of the array
  dims d;

  // The number of elements in the array, which is the product of the values in dims
  long numElements;

  // Points to the array contents, which is a buffer of the given scalar type
  void* array;
  boost::shared_ptr<void> sharray;

  // The type of scalar value that is stored in the array
  attrValue::valueType type;

  // Indicates whether this sightArray object owns the array and thus, will deallocate it when the
  // sightObject itself is deallocated.
  bool arrayOwner;

  public:
  sightArray(const dims& d, void* array, attrValue::valueType type/*, bool arrayOwner=false*/);
  sightArray(const dims& d, std::string* array/*, bool arrayOwner=false*/);
  sightArray(const dims& d, char** array/*,       bool arrayOwner=false*/);
  sightArray(const dims& d, void** array/*,       bool arrayOwner=false*/);
  sightArray(const dims& d, long* array/*,        bool arrayOwner=false*/);
  sightArray(const dims& d, int* array/*,         bool arrayOwner=false*/);
  sightArray(const dims& d, double* array/*,      bool arrayOwner=false*/);
  sightArray(const dims& d, float* array/*,       bool arrayOwner=false*/);

  sightArray(const dims& d, boost::shared_ptr<void> array, attrValue::valueType type);
  sightArray(const dims& d, boost::shared_ptr<std::string> array);
  sightArray(const dims& d, boost::shared_ptr<char*> array);
  sightArray(const dims& d, boost::shared_ptr<void*> array);
  sightArray(const dims& d, boost::shared_ptr<long> array);
  sightArray(const dims& d, boost::shared_ptr<int> array);
  sightArray(const dims& d, boost::shared_ptr<double> array);
  sightArray(const dims& d, boost::shared_ptr<float> array);

  sightArray(const sightArray& that);

  void init();

  ~sightArray();

  // Returns the unique name of this custom attrValue type. This must be the same name used to register
  // a deserializer for this class.
  std::string name() const { return "sightArray"; }

  // Get the dimensionality of the array
  const dims& getDims() const { return d; }
  
  // Get the number of elements in the array
  const long& getNumElements() const { return numElements; }
  
  // Get a vector where each entry i = steps[i] * \PI_{j>i} dims[j].
  // This is useful when the caller needs to iterate through the sightArray in steps larger than 1,
  // with different step sizes for different dimensions. The vector returned by this function makes 
  // it possible to walk the array in these step sizes by skipping the number of memory locations
  // specified in each index while advancing from one value in a given dimension to the next.
  std::vector<int> getIndexSteps(const std::vector<int> steps) const;

  // Return the size of the iteration space for iterating the multi-dimensional structure of this
  // sightArray using the given step sizes.
  std::vector<int> getIterSpaceDims(const std::vector<int> steps) const;
  
  // Advances the given index idx into the multi-dimensional structure this sightArray. 
  // Each advancement in dimension i is done by adding step[j]. 
  // This function is intended for advancing a multi-dimensional index through the array.
  // Thus, given a 2D iteration space [0-11]x[0-11], where we wish to step 4 and 3 locations in each dimension.
  // steps = [4, 3]
  // idx advances: [0, 0] [0, 3] [0, 6] [0, 9] [4, 0] [4, 3] [4, 6] [4, 9] [8, 0] [8, 3] [8, 6] [8, 9]
  void advanceMultiDimIdx(std::vector<int>& idx, std::vector<int>& step) const;
  
  // Advances the given index idx into the linear layout of this multi-dimensional sightArray array
  // mIdx: Records the current location in the multi-dimensional array.
  // idx: Records the current location in the array's linear representation.
  // step: The distance between adjacent entries in each dimension that are iterated over
  // aggStep: The total number of locations in the linear representation of the array that are skipped 
  //   when going from one value in a given dimension to the next. 
  //   aggStep[dim] = product_i in [dim, last dim] step[i].
  //   For example, if step=[2,2] (walk over all the even row and column pairs), aggStep = [4,2] since
  //   a single step in the first dimension skips over a total of 4 array entries and a step in the second
  //   dimension skips 2. 
  // box: lower and upper bounds the region of the multi-dimensional array index space that we'll be 
  //    iterating over. The lower bound is inclusive and the upper is exclusive.
  //    mIdx and idx must be is inside box at the start of the call.
  // Thus, given a 2D iteration space [0-11]x[0-11], where we wish to step 4 and 3 locations in each dimension
  //   (step=[4, 3], aggrStep=[12,3]) and a box that eliminates the borders of the space (box=[[4, 8], [3, 9]])
  // idx and mIdx advance:
  // idx:  51    54     57     99     102    105
  // mIdx: [4,3] [4, 6] [4, 9] [8, 3] [8, 6] [8, 9]
  void advanceLinearIdx(int& idx, std::vector<int>& mIdx, 
                        std::vector<int>& step, std::vector<int>& aggStep, 
                        std::vector<std::pair<int, int> >& box) const;
  
  // Given a multi-dimensional index into the multi-dimensional array return the index
  // into its linear representation.
  int multiDimIdx2LinearIdx(const std::vector<int> mIdx) const;

  // Returns a deep copy of this object
  customAttrValue* copy() const;

  // Relational operators
  bool operator==(const customAttrValue& that);
  bool operator< (const customAttrValue& that);
  //void* operator[](unsigned int i) const;

  // Adds the string representation of this value to the given output stream
  void serialize(std::ostream& s) const;

  // Deserializes instances of sightArray
  static customAttrValue* deserialize(std::string serialized);

  // Compares this object to that one using the given comparator and returns their relation to each other
  attrValue compare(const customAttrValue& that, comparator& comp) const;

  // Apply the given scalar comparator to the values at index iThis of this sightArray and iThat of that sightArray
  void scalarCompIdx(unsigned int iThis, const sightArray& that, unsigned int iThat, scalarComparator& comp) const;
}; // class sightArray

// Denotes fields that map n-dimensional vectors that to values.
// The keys in this field are vectors in the integral span of a set of 1-dimensional basis vectors
// (b_0, ..., b_n) starting from origin (o_0, ..., o_n): \Sigma_i o_i + c_i * b_i). 
// Each c_i is an integer specified implicitly by specifying a multi-dimensional sightArray array. 
// The value array[c_0][c_1]...[c_k] (c_i's are the indexes) is associated with key (\Sigma_i o_i + c_i * b_i).
// sightVectorFields maintain a pointer to a given array instead of making a private copy. As such, callers must ensure to
// not deallocate the array until they've deallocated any sightArrays that refer to it. Callers also have the option
// of making the sightVectorField the owner of the array, meaning that it will deallocate the array using delete when
// it is deallocated.
class sightVectorField : public customAttrValue {
  protected:
  std::vector<int> origin;
  std::vector<int> basis;
  sightArray vals;

  public:
  // origin defaults to the vector of all 0s
  sightVectorField(const sightArray& vals, const std::vector<int>& basis);
  sightVectorField(const sightArray& vals, const std::vector<int>& origin, const std::vector<int>& basis);
  sightVectorField(const sightVectorField& that);

  // Returns the unique name of this custom attrValue type. This must be the same name used to register
  // a deserializer for this class.
  std::string name() const { return "sightVectorField"; }

  // Returns a deep copy of this object
  customAttrValue* copy() const;

  // Relational operators
  bool operator==(const customAttrValue& that);
  bool operator< (const customAttrValue& that);

  // Adds the string representation of this value to the given output stream
  void serialize(std::ostream& s) const;

  // Deserializes instances of sightArray
  static customAttrValue* deserialize(std::string serialized);

  // Compares this object to that one using the given comparator and returns their relation to each other
  attrValue compare(const customAttrValue& that, comparator& comp) const;
}; // class sightVectorField

// Registers deserialization functions for each custom attrValue type we've defined here
class baseCustomAttrValueInstantiator: public customAttrValueInstantiator {
  public:
  baseCustomAttrValueInstantiator();
}; // class baseCustomAttrValueInstantiator

// static instance of baseCustomAttrValueInstantiator to ensure that its constructor is called
// before main().
extern baseCustomAttrValueInstantiator baseCustomAttrValueInstance;

// ******************************
// ***** Attribute Database *****
// ******************************

// Interface implemented by objects that wish to listen for changes to mappings of a given key
class attrObserver {
  public:
  typedef enum {attrAdd, attrReplace, attrRemove} attrObsAction;

  // Called before key's mapping is changed
  virtual void observePre(std::string key, attrObsAction action) { }

  // Called after key's mapping is changed
  virtual void observePost(std::string key, attrObsAction action) { }

  std::string attrObsAction2Str(attrObsAction action)
  { return (action==attrAdd? "attrAdd": (action==attrReplace? "attrReplace": (action==attrRemove? "attrRemove": "???"))); }
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
  void notifyObsPre(std::string key, attrObserver::attrObsAction action);
  // Notify all the observers of the given key after its mapping is changed (call attrObserver::observePost())
  void notifyObsPost(std::string key, attrObserver::attrObsAction action);
}; // class attributes

/***********************************************************
 ***** Support for parsing files with attribute values *****
 ***********************************************************/

// Type of the user-provided functor that takes as input a description of the read data.
// readData: Maps field group (e.g. "ctxt", "obs", "anchor") to the mapping of attribute names to their string representations
// lineNum: The current line in the file, which is useful for generating error messages.
class attrFileReader {
  public:
  virtual void operator()(const std::map<std::string, std::map<std::string, std::string> >& readData, int lineNum)=0;
};

// Reads the given file of key/value mappings, calling the provided functor on each instance.
// Each line is a white-space separated sequence of mappings in the format group:key:value and each
// call to functor f is provided with a 2-level map that maps each group to a map of key->value mappings.
void readAttrFile(std::string fName, attrFileReader& f);

} // namespace common
} // namespace sight
