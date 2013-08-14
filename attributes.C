#include "attributes.h"
#include <assert.h>

using namespace std;

attributesC attributes;

/*********************
 ***** attrValue *****
 *********************/ 

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

attrValue::attrValue(long intV) {
  type  = intT;
  store = new long*;
  *((long*)store) = intV;
//cout << this << ": allocated "<<((long*)store)<<"\n";
}

attrValue::attrValue(double floatV) {
  type  = floatT;
  store = new double;
  *((double*)store) = floatV;
}

attrValue::attrValue(const attrValue& that) {
  type = that.type;
       if(type == strT)   { store = (void*)(new string(*((string*)that.store))); }
  else if(type == ptrT)   { store = new void*;  *((void**)store)  = *((void**)that.store);  }
  else if(type == intT)   { store = new long*;  *((long*)store)   = *((long*)that.store);   }
  else if(type == floatT) { store = new double; *((double*)store) = *((double*)that.store); }
  else {
    cerr << "attrValue::attrValue() ERROR: invalid value type "<<type<<"!"<<endl;
    exit(-1);
  }
}

attrValue::~attrValue() {
//cout << this << ": deleting "<<((long*)store)<<"="<<*((long*)store)<<"\n";
       if(type == strT)   delete (string*)store;
  else if(type == ptrT)   delete (void**)store;
  else if(type == intT)   delete (long*)store;
  else if(type == floatT) delete (double*)store;
  else {
    cerr << "attrValue::~attrValue() ERROR: invalid value type "<<type<<"!"<<endl;
    exit(-1);
  }
}

bool attrValue::operator==(const attrValue& that) const {
  if(type == that.type) {
         if(type == strT)   return *((string*)store) == *((string*)that.store);
    else if(type == ptrT)   return *((void**)store)  == *((void**)that.store);
    else if(type == intT)   return *((long*)store)   == *((long*)that.store);
    else if(type == floatT) return *((double*)store) == *((double*)that.store);
    else {
      ostringstream oss; oss << "attrValue::operator== ERROR: invalid value type "<<type<<"!";
      throw oss.str();
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
      ostringstream oss; oss << "attrValue::operator< ERROR: invalid value type "<<type<<"!";
      throw oss.str();
    }
  } else {
    // All instances of each type are < or > all instances of other types, in some canonical order
    // (the enum values for different types are guaranteed to be different)
    return type < that.type;
  }
}

// Returns a human-readable representation of this object
std::string attrValue::str() const {
  if(type == strT) return *((string*)store);
  else {
    ostringstream oss;
         if(type == ptrT)   *((void**)store);
    else if(type == intT)   *((long*)store);
    else if(type == floatT) *((double*)store);
    else  {
      oss << "attrValue::str() ERROR: unknown attribute value type: "<<type<<"!";
      throw oss.str();
    }
    return oss.str();
  }
}


/******************
 ***** attrOp *****
 ******************/

// Applies the given functor to this given value. Throws an exception if the functor
// is not applicable to this value type.
bool attrOp::apply(const set<attrValue>& vals) {
  if(vals.size() == 0) throw "attrOp::apply() ERROR: applying operation to empty set of values!";
  
  for(set<attrValue>::iterator v=vals.begin(); v!=vals.end(); v++) {
    bool ret;
         if(v->type == attrValue::strT   && implementsString()) ret = applyString(*((string*)v->store));
    else if(v->type == attrValue::ptrT   && implementsPtr())    ret = applyPtr   (*((void**)v->store));
    else if(v->type == attrValue::intT   && implementsInt())    ret = applyInt   (*((long*)v->store));
    else if(v->type == attrValue::floatT && implementsFloat())  ret = applyFloat (*((double*)v->store));
    else throw "attrOp::apply() ERROR: attribute operation "+str()+" not compatible with value "+v->str()+"!";
    
    // If all the applications must return true but one returns false, the result is false
    if(type == all && ret==false) return false;
    // If any the applications must return true and one returns true, the result is true
    if(type == any && ret==true) return true;
  }
  
  // If type==all/any, then allsub-applications must have returned true/false
  if(type == all) return true;
  else if(type == any) return false;
  
  ostringstream mesg; mesg << "attrOp::apply() ERROR: unknown type "<<type<<"!";
  throw mesg.str();
}

/************************
 ***** attrSubQuery *****
 ************************/ 
bool attrSubQuery::query() { return query(attributes); }

bool attrSubQueryAnd::query(const attributesC& attr) {
  //cout << "attrSubQueryAnd::query() apply="<<op->apply(attributes.get(key))<<" pred="<<pred<<endl;
  // Applies the operator to the values at the given key. The && ensures that if the operator returns true,
  // the query is propagated to the previous attrSubQuery object. If the previous object is NULL, returns true.
  return op->apply(attributes.get(key)) && 
         (pred ? pred->query(attr) : true);
}

bool attrSubQueryOr::query(const attributesC& attr) {
  //cout << "attrSubQueryOr::query() apply="<<op->apply(attributes.get(key))<<" pred="<<pred<<endl;
  // Applies the operator to the values at the given key. The || ensures that if the operator returns false,
  // the query is propagated to the previous attrSubQuery object. If the previous object is NULL, returns true since by default we emit debug output.
  return op->apply(attributes.get(key)) ||
         (pred ? pred->query(attr) : true);
}

bool attrSubQueryIf::query(const attributesC& attr) {
  //cout << "attrSubQueryIf::query() apply="<<op->apply(attributes.get(key))<<endl;
  // Applies the operator to the values at the given key, returning its result. This object never propagates
  // queries to its predecessors.
  return op->apply(attributes.get(key));
}

bool attrSubQueryTrue::query(const attributesC& attr) {
  // Always returns true
  return true;
}

bool attrSubQueryFalse::query(const attributesC& attr) {
  // Always returns false
  return true;
}

/*********************
 ***** attrQuery *****
 *********************/ 
 
attrQuery::attrQuery() {
  lastQ = NULL;
}

// Adds the given sub-query to the list of queries
void attrQuery::push(attrSubQuery* subQ) {
  subQ->pred = lastQ;
  lastQ = subQ;
}

// Removes the last sub-query from the list of queries
void attrQuery::pop() {
  if(lastQ) {
    lastQ = lastQ->pred;
  } else
    throw "attrQuery::pop() ERROR: popping an empty list of sub-queries!";
}

// Returns the result of this query on the current state of the given attributes object
bool attrQuery::query(const attributesC& attr) {
  // If the list of sub-queries is non-empty, ask the last one in the list. It will propagate the query
  // backwards through the list as needed
  if(lastQ) return lastQ->query(attr);
  // If the list of sub-queries is empty, return true since by default dbglog emits debug output
  else      return true;
}

/**********************
 ***** attributes *****
 **********************/ 

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
    m[key].insert(val);
    qCurrent = false;
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
  bool modified = (m.find(key)==m.end());
  if(modified) {
    m[key].clear();
    m[key].insert(val);
    qCurrent = false;
  } else if(m[key].size() == 0) {
    ostringstream mesg; mesg << "attributesC::replace() ERROR: key "<<key<<" is mapped to an empty set of values!";
    throw mesg.str();
  }
  
  return modified;
}

// Returns the value mapped to the given key
const set<attrValue>& attributesC::get(std::string key) const {
  map<string, set<attrValue> >::const_iterator i = m.find(key);
  if(i != m.end()) {
    if(i->second.size() == 0) {
      ostringstream mesg; mesg << "attributesC::get() ERROR: key "<<key<<" is mapped to an empty set of values!";
      throw mesg.str();
    }
    return i->second;
  } else {
    ostringstream mesg; mesg << "attributesC::get() ERROR: key "<<key<<" is not mapped to any value!";
    throw mesg.str();
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
    // Remove the key->val mapping and if this is the only mapping for key, remove its entry in m[] as well
    m[key].erase(val);
    if(m[key].size()==0)
      m.erase(key);
    qCurrent = false;
  }
  return modified;
}

// Removes the mapping of this key to any value.
// Returns true if the attributes map changes as a result and false otherwise.
bool attributesC::remove(string key) {
  bool modified = (m.find(key)!=m.end());
  if(modified) {
    // Remove the key and all its mappings from m[]
    m.erase(key);
    qCurrent = false;
  }
  return modified;
}

// --- QUERYING ---

// Adds the given sub-query to the list of queries
void attributesC::push(attrSubQuery* subQ) {
  q.push(subQ);
  qCurrent = false;
}

// Removes the last sub-query from the list of queries
void attributesC::pop() {
  q.pop();
  qCurrent = false;
}

// Returns the result of the current query q on the current state of this attributes object
bool attributesC::query() {
  // Perform the query directly if the value of lastQRet is not consistent with the current state of q and m
  if(!qCurrent) lastQRet = q.query(*this);
  return lastQRet;
}

// ***********************
// ***** SELF TESTER *****
// ***********************

long testSeed;

void andFunc(bool cond, int depth, string indent="");
void orFunc(bool cond, int depth, string indent="");
void ifFunc(bool cond, int depth, string indent="");

void nextFunc(bool cond, int depth, string indent) {
  if(depth==1000) return;
  
  switch(rand()%3) {
    case 0: return orFunc (cond, depth, indent);
    case 1: return andFunc(cond, depth, indent);
    case 2: return ifFunc (cond, depth, indent);
  }
  assert(0);
}

bool verbA(bool cond) {
  if(!cond) { cout << "Error seed "<<testSeed<<endl; }
  return cond;
}

void andFunc(bool cond, int depth, string indent) {
  ostringstream vname; vname << "var_"<<depth;
  bool nextCond = rand()%2;
  attr a(vname.str(), (long)nextCond);
  attrAnd aAnd(vname.str(), new attrEq((long)1, attrOp::any));
  //cout << indent << "andFunc(cond="<<cond<<"): nextCond="<<nextCond<<", depth="<<depth<<", query="<<attributes.query()<<endl;
 
  assert(verbA(attributes.query() == (cond && nextCond)));
  nextFunc(cond && nextCond, depth+1, indent+"    ");
}

void orFunc(bool cond, int depth, string indent) {
  ostringstream vname; vname << "var_"<<depth;
  bool nextCond = rand()%2;
  attr a(vname.str(), (long)nextCond);
  attrOr aOr(vname.str(), new attrEq((long)1, attrOp::any));
  //cout << indent << "orFunc(cond="<<cond<<"): nextCond="<<nextCond<<", depth="<<depth<<", query="<<attributes.query()<<endl;
 
  assert(verbA(attributes.query() == (cond || nextCond))); 
  nextFunc(cond || nextCond, depth+1, indent+"    ");
} 

void ifFunc(bool cond, int depth, string indent) {
  ostringstream vname; vname << "var_"<<depth;
  bool nextCond = rand()%2;
  attr a(vname.str(), (long)nextCond);
  attrIf aIf(vname.str(), new attrEq((long)1, attrOp::any));
  //cout << indent << "ifFunc(cond="<<cond<<"): nextCond="<<nextCond<<", depth="<<depth<<", query="<<attributes.query()<<endl;
 
  assert(verbA(attributes.query() == nextCond)); 
  nextFunc(nextCond, depth+1, indent+"    ");
} 

/*int fib(int x, string indent="") {
  cout << indent << "fib("<<x<<")\n";
  attr a("x", (long)x);
  attrIf aif("x", new attrEq((long)x, attrOp::any));
  cout << indent << "x="<<x<<", query="<<attributes.query()<<endl;
  if(x<=1) {
    cout << indent << "return 1\n";
    return 1;
  }
  return fib(x-1, indent+":   ");// + fib(x-2, indent+":   ");
}*/

int main(int argc, char** argv) {
  if(argc==2) testSeed = atoi(argv[1]);
  else        testSeed = time(NULL);
  srand(testSeed);
  try{
    //fib(4, "");
    while(1) {
      nextFunc(true, 0, "");
      cout << ".";
    }
  } catch (string e) {
    cerr << e<< endl;
  }
}
