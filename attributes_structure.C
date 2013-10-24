#include "attributes_common.h"
#include "dbglog_structure_internal.h"
#include <assert.h>
#include "utils.h"

using namespace std;
using namespace dbglog;

namespace dbglog {
namespace structure{
  
structure::attributesC attributes;
attrNullOp NullOp;

/******************
 ***** attrOp *****
 ******************/

// Applies the given functor to this given value. Throws an exception if the functor
// is not applicable to this value type.
bool attrOp::apply() const {
  const set<attrValue>& vals = attributes.get(key);
  if(vals.size() == 0) {
    cerr << "attrOp::apply() ERROR: applying operation to empty set of values!"<<endl;
    exit(-1);
  }
  
  for(set<attrValue>::iterator v=vals.begin(); v!=vals.end(); v++) {
    bool ret;
         if(v->type == attrValue::strT   && implementsString()) ret = applyString(*((string*)v->store));
    else if(v->type == attrValue::ptrT   && implementsPtr())    ret = applyPtr   (*((void**)v->store));
    else if(v->type == attrValue::intT   && implementsInt())    ret = applyInt   (*((long*)v->store));
    else if(v->type == attrValue::floatT && implementsFloat())  ret = applyFloat (*((double*)v->store));
    else {
      cerr << "attrOp::apply() ERROR: attribute operation "<<str()<<" not compatible with value "<<v->str()<<"!"<<endl;
      exit(-1);
    }
    
    // If all the applications must return true but one returns false, the result is false
    if(type == all && ret==false) return false;
    // If any the applications must return true and one returns true, the result is true
    if(type == any && ret==true) return true;
  }
  
  // If type==all/any, then allsub-applications must have returned true/false
  if(type == all) return true;
  else if(type == any) return false;
  
  cerr << "attrOp::apply() ERROR: unknown type "<<type<<"!"<<endl;
  exit(-1);
}

bool attrRange::applyInt(long& that)     const { 
  if(lb.getType() == attrValue::intT) {
    //cout << "lb="<<lb.str()<<"="<<lb.getInt()<<" ub="<<ub.str()<<"="<<ub.getInt()<<" that="<<that<<endl;
    return (lb.getInt() <= that) && (that < ub.getInt());
  } else if(lb.getType() == attrValue::floatT) {
    return (lb.getFloat() <= that) && (that < ub.getFloat());
  }
  
  cerr << "attrRange::applyInt() ERROR: invalid type of lb"<<lb.getType()<<"!"<<endl; exit(-1);
}

bool attrRange::applyFloat(double& that) const { 
  if(lb.getType() == attrValue::intT) 
    return (lb.getInt() <= that) && (that < ub.getInt());
  else if(lb.getType() == attrValue::floatT) 
    return (lb.getFloat() <= that) && (that < ub.getFloat());
  
  cerr << "attrRange::applyInt() ERROR: invalid type of lb"<<lb.getType()<<"!"<<endl; exit(-1);
}

/************************
 ***** attrSubQuery *****
 ************************/ 
bool attrSubQuery::query() { return query(attributes); }

bool attrSubQueryAnd::query(const attributesC& attr) {
  //cout << "attrSubQueryAnd::query() apply="<<op->apply()<<" pred="<<pred<<endl;
  // Applies the operator to the values at the given key. The && ensures that if the operator returns true,
  // the query is propagated to the previous attrSubQuery object. If the previous object is NULL, returns true.
  return op->apply() && 
         (pred ? pred->query(attr) : true);
}

bool attrSubQueryOr::query(const attributesC& attr) {
  //cout << "attrSubQueryOr::query() apply="<<op->apply()<<" pred="<<pred<<endl;
  // Applies the operator to the values at the given key. The || ensures that if the operator returns false,
  // the query is propagated to the previous attrSubQuery object. If the previous object is NULL, returns true since by default we emit debug output.
  return op->apply() ||
         (pred ? pred->query(attr) : true);
}

bool attrSubQueryIf::query(const attributesC& attr) {
  //cout << "attrSubQueryIf::query() op="<<op->str()<<", apply="<<op->apply()<<endl;
  // Applies the operator to the values at the given key, returning its result. This object never propagates
  // queries to its predecessors.
  return op->apply();
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
  } else {
    cerr << "attrQuery::pop() ERROR: popping an empty list of sub-queries!"<<endl;
    exit(-1);
  }
}

// Returns the result of this query on the current state of the given attributes object
bool attrQuery::query(const attributesC& attr) {
  // If the list of sub-queries is non-empty, ask the last one in the list. It will propagate the query
  // backwards through the list as needed
  if(lastQ) return lastQ->query(attr);
  // If the list of sub-queries is empty, return true since by default dbglog emits debug output
  else      return true;
}

// ******************************
// ***** Attribute Database *****
// ******************************

attributesC::attributesC() {
  // Queries on an empty attributes object evaluate to true by default (by default we emit debug output)
  lastQRet = true;
  qCurrent = true;
}

attributesC::~attributesC() {
}

// Adds the given value to the mapping of the given key without removing the key's prior mapping.
// Returns true if the attributes map changes as a result and false otherwise.
// This is a thin wrapper that calls the parent class method but updates qCurrent.
bool attributesC::add(string key, const attrValue& val) {
  bool modified = common::attributesC::add(key, val);
  if(modified) qCurrent = false;
  return modified;
}

// Adds the given value to the mapping of the given key, while removing the key's prior mapping, if any.
// Returns true if the attributes map changes as a result and false otherwise.
// This is a thin wrapper that calls the parent class method but updates qCurrent.
bool attributesC::replace(string key, const attrValue& val) {
  bool modified = common::attributesC::replace(key, val);
  if(modified) qCurrent = false;
  return modified;
}

// Removes the mapping of this key to any value.
// Returns true if the attributes map changes as a result and false otherwise.
// This is a thin wrapper that calls the parent class method but updates qCurrent.
bool attributesC::remove(string key, const attrValue& val) {
  bool modified = common::attributesC::remove(key, val);
  if(modified) qCurrent = false;
  return modified;
}
bool attributesC::remove(string key) {
  bool modified = common::attributesC::remove(key);
  if(modified) qCurrent = false;
  return modified;
}

// --- QUERYING ---

// Adds the given sub-query to the list of queries
void attributesC::push(dbglog::structure::attrSubQuery* subQ) {
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
//cout << "attributesC::query()="<<lastQRet<<" qCurrent="<<qCurrent<<endl;
  return lastQRet;
}

// *******************************
// ***** Attribute Interface *****
// *******************************

attr::attr(std::string key, std::string val, properties* props) : key(key), val(val) { init<std::string>(key, val, props); }
attr::attr(std::string key, char*       val, properties* props) : key(key), val(val) { init<char*      >(key, val, props); }
attr::attr(std::string key, const char* val, properties* props) : key(key), val(val) { init<char*      >(key, (char*)val, props); }
attr::attr(std::string key, void*       val, properties* props) : key(key), val(val) { init<void*      >(key, val, props); }
attr::attr(std::string key, int         val, properties* props) : key(key), val(val) { init<long       >(key, val, props); }
attr::attr(std::string key, long        val, properties* props) : key(key), val(val) { init<long       >(key, val, props); }
attr::attr(std::string key, float       val, properties* props) : key(key), val(val) { init<double     >(key, val, props); }
attr::attr(std::string key, double      val, properties* props) : key(key), val(val) { init<double     >(key, val, props); }

template<typename T>
void attr::init(std::string key, T val, properties* props) {
//cout << "attr::init("<<key<<", "<<val<<")\n";
  if(attributes.exists(key)) {
    keyPreviouslySet = true;
    const std::set<attrValue>& curValues = attributes.get(key);
    assert(curValues.size()==1);
    
    oldVal = *(curValues.begin());
    attributes.replace(key, this->val); 
  } else {
    keyPreviouslySet = false;
    attributes.add(key, this->val); 
  }
  
  if(props==NULL) this->props = new properties();
  else            this->props = props;
  
  map<string, string> newProps;
  attrValue v(val);
  newProps["key"] = key;
  newProps["val"] = v.getAsStr();
  newProps["type"] = txt()<<v.getType();
  this->props->add("attr", newProps);
  
  dbg.enter(this);
}

attr::~attr() {
//cout << "attr::~attr("<<key<<", "<<val.str()<<")\n";
  // If this mapping replaced some prior mapping, return key to its original state
  if(keyPreviouslySet)
    attributes.replace(key, oldVal);
  // Otherwise, just remove the entire mapping
  else
    attributes.remove(key);
    
  dbg.exit(this);
}
// Returns the key of this attribute
string attr::getKey() const
{ return key; }

// Returns the value of this attribute
const attrValue& attr::getVal() const
{ return val; }

// Returns the type of this attribute values's contents
attrValue::valueType attr::getVType() const
{ return val.getType(); }

// Return the contents of this attribute values, aborting if there is a type incompatibility
std::string attr::getVStr() const
{ return val.getStr(); }

void*       attr::getVPtr() const
{ return val.getPtr(); }

long        attr::getVInt() const
{ return val.getInt(); }

double      attr::getVFloat() const
{ return val.getFloat(); }

// Implementations of the relational operators
bool attr::operator==(const attr& that) const
{
  return key==that.key &&
         val==that.val;
}
bool attr::operator<(const attr& that) const
{
  return key<that.key ||
        (key==that.key && val==that.val);
}

// C interface
extern "C" {
void* attr_enter_char  (char* key, char* val)       { return new attr(std::string(key), val); }
void* attr_enter_cchar (char* key, const char* val) { return new attr(std::string(key), val); }
void* attr_enter_void  (char* key, void* val)       { return new attr(std::string(key), val); }
void* attr_enter_int   (char* key, int val)         { return new attr(std::string(key), (long)val); }
void* attr_enter_long  (char* key, long val)        { return new attr(std::string(key), val); }
void* attr_enter_float (char* key, float val)       { return new attr(std::string(key), (double)val); }
void* attr_enter_double(char* key, double val)      { return new attr(std::string(key), val); }
}

void* attr_enter(std::string key, char* val)       { return new attr(key, val); }
void* attr_enter(std::string key, const char* val) { return new attr(key, val); }
void* attr_enter(std::string key, std::string val) { return new attr(key, val); }
void* attr_enter(std::string key, void* val)       { return new attr(key, val); }
void* attr_enter(std::string key, int val)         { return new attr(key, (long)val); }
void* attr_enter(std::string key, long val)        { return new attr(key, val); }
void* attr_enter(std::string key, float val)       { return new attr(key, (double)val); }
void* attr_enter(std::string key, double val)      { return new attr(key, val); }

extern "C" {
void attr_exit(void* a) { delete (attr*)a; }
}

// *****************************
// ***** Attribute Queries *****
// *****************************

// C interface
extern "C" {
void* attrAnd_enter(attrOp *op) { return new attrAnd(op); }
void attrAnd_exit(void* subQ) { delete (attrAnd*)subQ; }
}

// C interface
extern "C" {
void* attrOr_enter(attrOp *op) { return new attrOr(op); }
void attrOr_exit(void* subQ) { delete (attrOr*)subQ; }
}

// C interface
extern "C" {
void* attrIf_enter(attrOp *op) { return new attrIf(op); }
void attrIf_exit(void* subQ) { delete (attrIf*)subQ; }
}

// C interface
extern "C" {
void* attrTrue_enter() { return new attrTrue(); }
void attrTrue_exit(void* subQ) { delete (attrTrue*)subQ; }
}

// C interface
extern "C" {
void* attrFalse_enter() { return new attrFalse(); }
void attrFalse_exit(void* subQ) { delete (attrFalse*)subQ; }
}

}; // namespace structure
}; // namespace dbglog

// ***********************
// ***** SELF TESTER *****
// ***********************

#if defined(SELF_TEST)
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
  attrAnd aAnd(vname.str(), new attrEQ((long)1, attrOp::any));
  //cout << indent << "andFunc(cond="<<cond<<"): nextCond="<<nextCond<<", depth="<<depth<<", query="<<attributes.query()<<endl;
 
  assert(verbA(attributes.query() == (cond && nextCond)));
  nextFunc(cond && nextCond, depth+1, indent+"    ");
}

void orFunc(bool cond, int depth, string indent) {
  ostringstream vname; vname << "var_"<<depth;
  bool nextCond = rand()%2;
  attr a(vname.str(), (long)nextCond);
  attrOr aOr(vname.str(), new attrEQ((long)1, attrOp::any));
  //cout << indent << "orFunc(cond="<<cond<<"): nextCond="<<nextCond<<", depth="<<depth<<", query="<<attributes.query()<<endl;
 
  assert(verbA(attributes.query() == (cond || nextCond))); 
  nextFunc(cond || nextCond, depth+1, indent+"    ");
} 

void ifFunc(bool cond, int depth, string indent) {
  ostringstream vname; vname << "var_"<<depth;
  bool nextCond = rand()%2;
  attr a(vname.str(), (long)nextCond);
  attrIf aIf(vname.str(), new attrEQ((long)1, attrOp::any));
  //cout << indent << "ifFunc(cond="<<cond<<"): nextCond="<<nextCond<<", depth="<<depth<<", query="<<attributes.query()<<endl;
 
  assert(verbA(attributes.query() == nextCond)); 
  nextFunc(nextCond, depth+1, indent+"    ");
} 

/*int fib(int x, string indent="") {
  cout << indent << "fib("<<x<<")\n";
  attr a("x", (long)x);
  attrIf aif("x", new attrEQ((long)x, attrOp::any));
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
#endif //defined(SELF_TEST)
