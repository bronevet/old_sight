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
std::ofstream& operator<<(std::ofstream& ofs, const printable& p) {
  p.print(ofs);
  return ofs;
}

/**********************
 ***** properties *****
 **********************/
void properties::add(std::string className, const std::map<std::string, std::string>& props)
{ p.push_back(make_pair(className, props)); }

// Returns the start of the list to iterate from the most derived class of an object to the most base
properties::iterator properties::begin() const
{ return p.begin(); }

// The corresponding end iterator
properties::iterator properties::end() const
{ return p.end(); }

// Returns the iterator to the given objectName
properties::iterator properties::find(string name) const { 
  for(list<pair<string, map<string, string> > >::const_iterator i=p.begin(); i!=p.end(); i++)
    if(i->first == name) return i;
  return end();
}

// Given a properties iterator returns an iterator that refers to the next position in the list.
properties::iterator properties::next(iterator i) {
  i++;
  return i;
}

// Given an iterator to a particular key->value mapping, returns the value mapped to the given key
std::string properties::get(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  std::map<std::string, std::string>::const_iterator val = cur->second.find(key);
  if(val == cur->second.end()) { cerr << "properties::get() ERROR: cannot find key \""<<key<<"\"! properties="<<str(cur)<<endl; }
  assert(val != cur->second.end());
  return val->second;
}

// Given an iterator to a particular key->value mapping, returns the integer interpretation of the value mapped to the given key
long properties::getInt(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  std::map<std::string, std::string>::const_iterator val = cur->second.find(key);
  if(val == cur->second.end()) { cerr << "properties::getInt() ERROR: cannot find key \""<<key<<"\"! properties="<<str(cur)<<endl; }
  assert(val != cur->second.end());
  return strtol(val->second.c_str(), NULL, 10);
}

// Given an iterator to a particular key->value mapping, returns the floating-point interpretation of the value mapped to the given key
double properties::getFloat(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  std::map<std::string, std::string>::const_iterator val = cur->second.find(key);
  if(val == cur->second.end()) { cerr << "properties::getFloat() ERROR: cannot find key \""<<key<<"\"! properties="<<str(cur)<<endl; }
  assert(val != cur->second.end());
  return strtod(val->second.c_str(), NULL);
}

// Given an iterator to a particular key->value mapping, returns whether the given key is mapped to some value
bool properties::exists(properties::iterator cur, std::string key) {
  //assert(cur!=end());
  return cur->second.find(key) != cur->second.end();
}

// Returns the name of the object type referred to by the given iterator
string properties::name(iterator cur)
{ return cur->first; }

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

// Returns the string representation of the given properties iterator  
std::string properties::str(iterator props){
  ostringstream oss;
  oss << "["<<props->first<<":"<<endl;
  for(std::map<std::string, std::string>::const_iterator i=props->second.begin(); i!=props->second.end(); i++)
    oss << "    "<<i->first<<" =&gt "<<i->second<<endl;
  oss << "]";
  return oss.str();
}

std::string properties::str() const {
  ostringstream oss;
  oss << "[properties:"<<endl;
  for(iterator i=begin(); i!=end(); i++)
    oss << "    "<<properties::str(i)<<endl;
  oss << "]";
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
