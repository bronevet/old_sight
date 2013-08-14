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
#include "dbglog.h"

class attrOp;

// Wrapper class for strings, integers and floating point numbers that keeps track of the 
// type of its contents and allows functors that work on only one of these types to be applied.
class attrValue {
  typedef enum {strT, ptrT, intT, floatT} valueType;
  
  friend class attrOp;
  
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
  attrValue(const std::string& strV);
  attrValue(char* strV);
  attrValue(void* ptrV);
  attrValue(long intV);
  attrValue(double floatV);
  attrValue(const attrValue& that);
  ~attrValue();
  
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

// An operation that may be applied to an attrValue to probe its contents
class attrOp
{
  // All/Any mode: the result of applying the operation is true only if it is true for All/Any the values associated with some key
  // Any mode: 
  public:
  typedef enum {any, all} applyType;
  private:
  applyType type;
  
  public:
  attrOp(applyType type) : type(type) {}
  
  // For each type of value the functor must provide an implements*() method that 
  // returns whether the functor is applicable to this value type and an apply*()
  // method that can actually be applied to values of this type.
  
  virtual bool implementsString() { return false; }
  virtual bool applyString(std::string& val) { throw "Cannot call attrOp::applyString()!"; }
  
  virtual bool implementsPtr() { return false; }
  virtual bool applyPtr(void*& val) { throw "Cannot call attrOp::applyPtr()!";  }
  
  virtual bool implementsInt() { return false; }
  virtual bool applyInt(long& val) { throw "Cannot call attrOp::applyInt()!";  }
  
  virtual bool implementsFloat() { return false; }
  virtual bool applyFloat(double& val) { throw "Cannot call attrOp::applyFloat()!";  }
  
  // Applies the given functor to this given value. Throws an exception if the functor
  // is not applicable to this value type.
  bool apply(const std::set<attrValue>& vals);
  
  // Returns a human-readable representation of this object
  virtual std::string str() const=0;
};

// Base class for attrOps that support all value types
class universalAttrOp : public attrOp
{
  public:
  universalAttrOp(applyType type) : attrOp(type) {}
  
  bool implementsString() { return true; }
  bool implementsPtr()    { return true; }
  bool implementsInt()    { return true; }
  bool implementsFloat()  { return true; } 
};

// Special operation that always returns true and is used in cases where we need a no-op
class attrNullOp : public universalAttrOp
{
  public:
  attrNullOp() : universalAttrOp(any) {}
  bool applyString(std::string& that) { return true; }
  bool applyPtr(void*& that)          { return true; }
  bool applyInt(long& that)           { return true; }
  bool applyFloat(double& that)       { return true; }
  
  // Returns a human-readable representation of this object
  std::string str() const { return "attrNullOp"; }
};
attrNullOp NullOp;

class attrEq : public universalAttrOp
{
  attrValue val;
  public:
  attrEq(const attrValue&   val, applyType type) : universalAttrOp(type), val(val) {}
  attrEq(const std::string& val, applyType type) : universalAttrOp(type), val(val) {}
  attrEq(char*              val, applyType type) : universalAttrOp(type), val(val) {}
  attrEq(void*              val, applyType type) : universalAttrOp(type), val(val) {}
  attrEq(long               val, applyType type) : universalAttrOp(type), val(val) {}
  attrEq(double             val, applyType type) : universalAttrOp(type), val(val) {}
  
  bool applyString(std::string& that) { return val == attrValue(that); }
  bool applyPtr(void*& that)          { return val == attrValue(that); }
  bool applyInt(long& that)           { return val == attrValue(that); }
  bool applyFloat(double& that)       { return val == attrValue(that); }
  
  // Returns a human-readable representation of this object
  std::string str() const { return "attrEq"; }
};

// ...

class attributesC;

// An attrQuery implements queries that may be performed on attributes. A query is a list of subQuery objects,
// each of which can evaluate to true or false and optionally propagate the query to its 
// predecessor in the list.
class attrQuery;

class attrSubQuery
{
  protected:
  // The key that is being evaluated
  std::string key;
    
  // The operation that will be performed on the value associated with the key
  attrOp* op;
  
  // The predecessor of this query in the query chain
  attrSubQuery* pred;
  
  friend class attrQuery;
  
  public:
  attrSubQuery(std::string key, attrOp* op) : key(key), op(op) {}
  ~attrSubQuery() { delete op; }
  
  // Performs the query on either the given attributes object or the one defined globally
  virtual bool query(const attributesC& attr)=0;
  bool query();
};  // class attrSubQuery

class attrQuery
{
  // Points to the last query in the linked list of queries
  attrSubQuery* lastQ;
  
  public:
  attrQuery();
  
  // Adds the given sub-query to the list of queries
  void push(attrSubQuery* subQ);
  
  // Removes the last sub-query from the list of queries
  void pop();
  
  // Returns the result of this query on the current state of the given attributes object
  bool query(const attributesC& attr);
}; // class attrQuery

class attrSubQueryAnd : public attrSubQuery
{
  public:
  attrSubQueryAnd(std::string key, attrOp* op) : attrSubQuery(key, op) {}
    
  // Applies the operator to the values at the given key. The && ensures that if the operator returns true,
  // the query is propagated to the previous attrSubQuery object. If the previous object is NULL, returns true.
  bool query(const attributesC& attr);
};

class attrSubQueryOr : public attrSubQuery
{
  public:
  attrSubQueryOr(std::string key, attrOp* op) : attrSubQuery(key, op) {}
    
  // Applies the operator to the values at the given key. The || ensures that if the operator returns false,
  // the query is propagated to the previous attrSubQuery object. If the previous object is NULL, returns false.
  bool query(const attributesC& attr);
};

class attrSubQueryIf : public attrSubQuery
{
  public:
  attrSubQueryIf(std::string key, attrOp* op) : attrSubQuery(key, op) {}
  
  // Applies the operator to the values at the given key, returning its result. This object never propagates
  // queries to its predecessors.
  bool query(const attributesC& attr);
};

class attrSubQueryTrue : public attrSubQuery
{
  public:
  attrSubQueryTrue() : attrSubQuery(std::string(""), &NullOp) {}
  
  // Always returns true
  bool query(const attributesC& attr);
};

class attrSubQueryFalse : public attrSubQuery
{
  public:
  attrSubQueryFalse() : attrSubQuery(std::string(""), &NullOp) {}
  
  // Always returns false
  bool query(const attributesC& attr);
};

class attributesC
{
  // --- STORAGE ---
  private:
  std::map<std::string, std::set<attrValue> > m;
   
  // Adds the given value to the mapping of the given key without removing the key's prior mapping.
  // Returns true if the attributes map changes as a result and false otherwise.
  public:
  bool add(std::string key, std::string val);
  bool add(std::string key, char* val);
  bool add(std::string key, void* val);
  bool add(std::string key, long val);
  bool add(std::string key, double val);
  bool add(std::string key, const attrValue& val);
  
  // Adds the given value to the mapping of the given key, while removing the key's prior mapping, if any.
  // Returns true if the attributes map changes as a result and false otherwise.
  public:
  bool replace(std::string key, std::string val);
  bool replace(std::string key, char* val);
  bool replace(std::string key, void* val);
  bool replace(std::string key, long val);
  bool replace(std::string key, double val);
  bool replace(std::string key, const attrValue& val);
  
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
  bool remove(std::string key, const attrValue& val);
  
  // Removes the mapping of this key to any value.
  // Returns true if the attributes map changes as a result and false otherwise.
  public:
  bool remove(std::string key);
  
  // --- QUERYING ---
  private:
  // The current query that is being evaluates against this attributes map
  attrQuery q;
  
  // The most recent return value of the query object q
  bool lastQRet;
  
  // Records whether 
  // - this query object's most recent return value is current (neither q nor m have changed), in which case we 
  //   can respond to the next query with lastQRet, 
  // - or not, in which case we have to fully execute the next query
  bool qCurrent;
  
  public:
  // Adds the given sub-query to the list of queries
  void push(attrSubQuery* subQ);
  
  // Removes the last sub-query from the list of queries
  void pop();
  
  // Returns the result of the current query q on the current state of this attributes object
  bool query();
  
  public:
  attributesC() {
    // Queries on an empty attributes object evaluate to true by default (by default we emit debug output)
    lastQRet = true;
    qCurrent = true;
  }
  
}; // class attributes

extern attributesC attributes;

// C++ interface for attribute creation, destruction
class attr
{
  // The key/value of this attribute
  std::string key;
  attrValue val;
    
  // Records whether the addition of this key/value mapping changed the attributes map
  bool attrModified;
  
  public:
  attr(std::string key, std::string val) : key(key), val(val) { attrModified = attributes.add(key, this->val); }
  attr(std::string key, char*       val) : key(key), val(val) { attrModified = attributes.add(key, this->val); }
  attr(std::string key, void*       val) : key(key), val(val) { attrModified = attributes.add(key, this->val); }
  attr(std::string key, long        val) : key(key), val(val) { attrModified = attributes.add(key, this->val); }
  attr(std::string key, double      val) : key(key), val(val) { attrModified = attributes.add(key, this->val); }
  
  ~attr() {
    // If the addition of this key/value pair changed the attributes map, remove the mapping.
    // We do it this way since attr objects add/remove key/value pairs in a hierarchical fashion that
    // directly mirrors application scopes. As such, if a given key/value pair already exists
    // in the attributes map, it was added by an attr object that is deeper up the stack. As such,
    // there is no need to remove it when this object is deallocated since we're sure that the 
    // pair's original creator object deeper up the stack will do this when it is deallocated.
    if(attrModified)
      attributes.remove(key, val);
  }
};

// C++ interface for query creation, destruction

class attrAnd: public attrSubQueryAnd { 
  public:
  attrAnd(std::string key, attrOp* op) : attrSubQueryAnd(key, op) 
  { attributes.push(this); }
  ~attrAnd() { attributes.pop(); }
};

class attrOr: public attrSubQueryOr { 
  public:
  attrOr(std::string key, attrOp* op) : attrSubQueryOr(key, op) 
  { attributes.push(this); }
  ~attrOr() { attributes.pop(); }
};

class attrIf: public attrSubQueryIf { 
  public:
  attrIf(std::string key, attrOp* op) : attrSubQueryIf(key, op) 
  { attributes.push(this); }
  ~attrIf() { attributes.pop(); }
};

class attrTrue: public attrSubQueryTrue { 
  public:
  attrTrue(std::string key, attrOp* op) : attrSubQueryTrue() 
  { attributes.push(this); }
  ~attrTrue() { attributes.pop(); }
};
  
class attrFalse: public attrSubQueryFalse { 
  public:
  attrFalse(std::string key, attrOp* op) : attrSubQueryFalse() 
  { attributes.push(this); }
  ~attrFalse() { attributes.pop(); }
};
  
