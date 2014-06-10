#include "attributes_common.h"
#include "../sight_layout_internal.h"
#include <assert.h>
#include "../utils.h"

using namespace std;
using namespace sight;

namespace sight {
namespace layout{

layout::attributesC attributes;

// Record the layout handlers in this file
void* attrEnterHandler(properties::iterator props) { return new attr(props); }
void  attrExitHandler(void* obj) { attr* a = static_cast<attr*>(obj); delete a; }
  
attributesLayoutHandlerInstantiator::attributesLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["attr"] = &attrEnterHandler;
  (*layoutExitHandlers) ["attr"] = &attrExitHandler;
}
attributesLayoutHandlerInstantiator attributesLayoutHandlerInstance;

// ******************************
// ***** Attribute Database *****
// ******************************

// Returns a representation of the attributes database as a JavaScript map
std::string attributesC::strJS() const {
  ostringstream oss;
  
  oss << "{";
  for(map<string, set<attrValue> >::const_iterator i=m.begin(); i!=m.end(); i++) {
    if(i!=m.begin()) oss << ",";
    if(i->second.size()==0) { cerr << "attributesC::strJS() ERROR: key "<<i->first<<" is mapped to 0 values!"; exit(-1); }
    if(i->second.size()>1) { cerr << "attributesC::strJS() ERROR: currently cannot emit JavaScript for keys with multiple values! key="<<i->first; exit(-1); }
    // Emit the name of the key, while prefixing it with "key_" to allow Javascript code to add additional
    // fields without fear of name collisions.
    oss << "\"key_" << i->first << "\":";
    switch((i->second.begin())->getType()) {
      case attrValue::strT   : oss << "\""<<(i->second.begin())->getStr()<<"\"";   break;
      case attrValue::ptrT   : oss << "\""<<(i->second.begin())->getPtr()<<"\"";   break;
      case attrValue::intT   : oss << "\""<<(i->second.begin())->getInt()<<"\"";   break;
      case attrValue::floatT : oss << "\""<<(i->second.begin())->getFloat()<<"\""; break;
      default: cerr << "attributesC::strJS() ERROR: key "<<i->first<<" has value with an unknown type!"; exit(-1);
    }
  }
  oss << "}";
  return oss.str();
}

// *******************************
// ***** Attribute Interface *****
// *******************************

attr::attr(properties::iterator props) : 
  key (properties::get(props, "key")),      
  val(properties::get(props, "val"), attrValue::unknownT) { 
  
  if(!common::isEnabled()) return;
  
  // Update the current attributes map with the new key => value mapping
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
  
  dbg.enterAttrSubBlock();
}

attr::~attr() {
  if(!common::isEnabled()) return;
//cout << "attr::~attr("<<key<<", "<<val.str()<<")\n";
  // If this mapping replaced some prior mapping, return key to its original state
  /*if(keyPreviouslySet)
    attributes.replace(key, oldVal);
  // Otherwise, just remove the entire mapping
  else
    attributes.remove(key);*/
  
  dbg.exitAttrSubBlock();  
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

}; // namespace layout
}; // namespace sight
