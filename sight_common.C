#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <assert.h>
#include "sight_common.h"

using namespace std;

namespace sight {
namespace common {

// Returns whether log generation has been enabled or explicitly disabled
bool isEnabled() {
  static bool checked=false;
  static bool enabledDebug; // Records whether log generation has been enabled or explicitly disabled
  if(!checked) {
    checked = true;
    enabledDebug = (getenv("DISABLE_SIGHT") == NULL);
  }
  return enabledDebug;
}

} // namespace common

/*********************
 ***** printable *****
 *********************/
  
// Call the print method of the given printable object
/*std::ofstream& operator<<(std::ofstream& ofs, const printable& p) {
  p.print(ofs);
  return ofs;
}*/

/********************************
 ***** properties::iterator *****
 ********************************/
 
// Returns the string representation of the given properties iterator  
std::string properties::iterator::str() const {
  std::ostringstream oss;
  if(isEnd())
    oss << "[properties::iterator End]";
  else {
    oss << "["<<name()<<":"<<endl;
    for(std::map<std::string, std::string>::const_iterator i=cur->second.begin(); i!=cur->second.end(); i++)
      oss << "    "<<i->first<<" =&gt "<<i->second<<endl;
    oss << "]";
  }
  return oss.str();
}

/**********************
 ***** properties *****
 **********************/

void properties::add(std::string className, const std::map<std::string, std::string>& props)
{ p.push_back(make_pair(className, props)); }

// Returns the start of the list to iterate from the most derived class of an object to the most base
properties::iterator properties::begin() const
{ return iterator(p.begin(), p.end()); }

// The corresponding end iterator
properties::iterator properties::end() const
{ return iterator(p.end(), p.end()); }

// Returns the iterator to the given objectName
properties::iterator properties::find(string name) const { 
  for(iterator i(p.begin(), p.end()); !i.isEnd(); i++)
    if(i.name() == name) return i;
  return end();
}

// Given a properties iterator returns an iterator that refers to the next position in the list.
properties::iterator properties::next(iterator i) {
  i++;
  return i;
}

// Given an iterator to a particular key->value mapping, returns the value mapped to the given key
std::string properties::get(properties::iterator cur, std::string key) {
  assert(!cur.isEnd());
  const map<string, string>& keyvalMap = cur.getMap();
  std::map<std::string, std::string>::const_iterator val = keyvalMap.find(key);
  if(val == keyvalMap.end()) { cerr << "properties::get() ERROR: cannot find key \""<<key<<"\"! properties="<<cur.str()<<endl; }
  assert(val != keyvalMap.end());
  return val->second;
}

// Given the label of a particular key->value mapping, adds the given mapping to it
void properties::set(std::string name, std::string key, std::string value) {
  // Find the given label in the properties map
  for(list<pair<string, map<string, string> > >::iterator i=p.begin();
      i!=p.end(); i++) {
    if(i->first == name) {
      // Add the new key->value mapping under the given label
      (i->second)[key] = value;
      return;
    }
  }
  // The given label must currently exist in the properties map
  assert(0);
}

// Given an iterator to a particular key->value mapping, returns the integer interpretation of the value mapped to the given key
long properties::getInt(properties::iterator cur, std::string key) {
  return strtol(get(cur, key).c_str(), NULL, 10);
}

// Returns the integer interpretation of the given string
long properties::asInt(std::string val)
{ return strtol(val.c_str(), NULL, 10); }

// Given an iterator to a particular key->value mapping, returns the floating-point interpretation of the value mapped to the given key
double properties::getFloat(properties::iterator cur, std::string key) {
  return strtod(get(cur, key).c_str(), NULL);
}

// Returns the floating-point interpretation of the given string
long properties::asFloat(std::string val)
{ return strtod(val.c_str(), NULL); }

/* // Given an iterator to a particular key->value mapping, returns whether the given key is mapped to some value
bool properties::exists(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  return cur->second.find(key) != cur->second.end();
}

// Given an iterator to a particular key->value mapping, returns the number of keys in the map
int properties::getNumKeys(iterator cur) {
  return cur->second.size();
}

// Given an iterator to a particular key->value mapping, returns a const reference to the key/value mapping
const std::map<std::string, std::string>& properties::getMap(iterator cur) {
  return cur->second;
}

// Returns the name of the object type referred to by the given iterator
string properties::name(iterator cur)
{ return cur->first; }*/

// Returns the name of the most-derived class 
string properties::name() const {
  assert(p.size()>0);
  return p.front().first;
}

// Returns the number of tags recorded in this object
int properties::size() const
{ return p.size(); }

// Erases the contents of this object
void properties::clear()
{ p.clear(); }

std::string properties::str(string indent) const {
  ostringstream oss;
  oss << "[properties: active="<<active<<", emitTag="<<emitTag<<endl;
  for(iterator i=begin(); !i.isEnd(); i++)
    oss << indent <<"    "<<i.str()<<endl;
  oss << indent << "]";
  return oss.str();
}

namespace common {

/*********************
 ***** dbgStream *****
 *********************/

// Creates an output directory for the given widget and returns its path as a pair:
// <path relative to the current working directory that can be used to create paths for writing files,
//  path relative to the output directory that can be used inside generated HTML>
pair<std::string, std::string> dbgStream::createWidgetDir(std::string widgetName) {
  if(widgetDirs.find(widgetName) == widgetDirs.end()) {
    createDir(workDir, "html/widgets/"+widgetName);
    widgetDirs.insert(widgetName);
  }
  
  return make_pair(workDir+"/html/widgets/"+widgetName, "widgets/"+widgetName);
}

// Given a string, returns a version of the string with all the control characters that may appear in the
// string escaped to that the string can be written out to Dbg::dbg with no formatting issues.
// This function can be called on text that has already been escaped with no harm.
std::string escape(std::string s)
{
  string out;
  for(unsigned int i=0; i<s.length(); i++) {
    switch(s[i]) {
      // Manage HTML tags
      case '<': out += "&#60;"; break;
      case '>': out += "&#62;"; break;
      case '/': out += "&#47;"; break;
      case '[': out += "&#91;"; break;
      case '\\': out += "&#92;"; break;
      case ']': out += "&#93;"; break;
      case '"': out += "&#34;"; break;
      case '&': out += "&#38;"; break;
      // Manage hashes, since they confuse the C PreProcessor CPP
      case '#': out += "&#35;"; break;
      case ' ': out += "&#160;"; break;
      case '\n': out += "&#0;"; break;
      case '\r': out += "&#1;"; break;
      default:  out += s[i]; break;
    }
  }
  return out;
}

std::string unescape(std::string s) {
  string out;
  unsigned int i=0;
  while(i<s.length()) {
    // If this is the start of a character's encoding
    if(s[i]=='&') {
      assert(i+1 < s.length());
      assert(s[i+1]=='#');
      i+=2;
      string encoding;
      while(s[i]!=';' && i<s.length()) {
        encoding += s[i];
        i++;
      }
      assert(encoding.length()>0);
      long code = strtol(encoding.c_str(),  NULL, 10);
      switch(code) {
        case 60: out+='<'; break;
        case 62: out+='>'; break;
        case 47: out+='/'; break;
        case 91: out+='['; break;
        case 92: out+='\\'; break;
        case 93: out+=']'; break;
        case 34: out+='"'; break;
        case 38: out+='&'; break;
        case 35: out+='#'; break;
        case 160: out+=' '; break;
        case 0:  out+='\n'; break;
        case 1:  out+='\r'; break;
        default: assert(0);
      }
      assert(s[i]==';');
      i++;
    // If this is not an encoded character, add it directly to tou
    } else {
      out += s[i];
      i++;
    }
  }
  return out;
}


}; // namespace common
}; // namespace sight
