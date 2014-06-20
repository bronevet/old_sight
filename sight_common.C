#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "sight_common.h"
#include "process.h"

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

// Returns the value mapped to the given key
std::string properties::iterator::get(std::string key)  const {
  assert(!isEnd());
  std::map<std::string, std::string>::const_iterator val = cur->second.find(key);
  if(val == cur->second.end()) { cerr << "properties::get() ERROR: cannot find key \""<<key<<"\"! properties="<<str()<<endl; }
  assert(val != cur->second.end());
  return val->second;
}

// Returns the integer interpretation of the value mapped to the given key
long properties::iterator::getInt(std::string key)  const {
  return strtol(get(key).c_str(), NULL, 10);
}

// Returns the floating-point interpretation of the value mapped to the given key
double properties::iterator::getFloat(std::string key)  const {
  return strtod(get(key).c_str(), NULL);
}

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
  /*assert(!cur.isEnd());
  const map<string, string>& keyvalMap = cur.getMap();
  std::map<std::string, std::string>::const_iterator val = keyvalMap.find(key);
  if(val == keyvalMap.end()) { cerr << "properties::get() ERROR: cannot find key \""<<key<<"\"! properties="<<cur.str()<<endl; }
  assert(val != keyvalMap.end());
  return val->second;*/
  return cur.get(key);
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

/*******************
 ***** nullBuf *****
 *******************/

nullBuf::nullBuf()
{
  init(NULL);
}

nullBuf::nullBuf(std::streambuf* baseBuf)
{
  init(baseBuf);
}

void nullBuf::init(std::streambuf* baseBuf)
{
  this->baseBuf = baseBuf;
}

// This nullBuf has no buffer. So every character "overflows"
// and can be put directly into the teed buffers.
int nullBuf::overflow(int c)
{
  return c;
}

streamsize nullBuf::xsputn(const char * s, streamsize n)
{
  return n;
}

// Sync buffer.
int nullBuf::sync()
{
  return 0;
}

/**********************
 ***** nullStream *****
 **********************/

// An instance of nullStream that apps can write to with low overhead when they do not wish to emit output
nullStream nullS;

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
    // If this is not an encoded character, add it directly to out
    } else {
      out += s[i];
      i++;
    }
  }
  return out;
}

/**********************
 ***** escapedStr *****
 **********************/

// source==unescaped: Creates an escaped string given a regular UNESCAPED string and an explicit list of control characters
// source==escaped:   Creates an ESCAPED string given an escaped string and an explicit list of control characters
escapedStr::escapedStr(std::string s_, std::string control, sourceT source) : control(control) {
  // If s_ is an unescaped string
  if(source == unescaped) {
    for(unsigned int i=0; i<s_.length(); i++) {
      // If s_[i] is the start of an escaped character
      if(s_[i]=='\\') {
        // Add the full text of the escaped character, including all the escape \'s
        while(s_[i]=='\\') {
          s += '\\';
          i++;
        }
      }
    
      // If s_[i] is a control character 
      if(control.find(s_[i]) != string::npos) {
        // Escape it by prepending a \ to it
        s += '\\';
        s += s_[i];
      // Otherwise, add s_[i] as it is
      } else 
        s += s_[i];

      //cout << "s_["<<i<<"]=\""<<s_[i]<<"\", s=\""<<s<<"\""<<endl;
    }

  // Else, if the source is escaped
  } else {
    s = s_;
  }
}

// Searches the string for the first occurrence of the sequence specified by its arguments, starting at pos and returns 
// the location. The search ignores any matches that cross escaped characters in this string.
size_t escapedStr::find(std::string sub, size_t pos) const {
  //cout << "escapedStr::find("<<sub<<", "<<pos<<")"<<endl;
  
  // Iterate through s looking for a substring that is equal to sub, e
  while(pos<s.size()) {
    //cout << "    pos="<<pos<<endl;
    
    // We haven't found a match to sub starting at smaller pos, so start looking at this pos
    int i=pos, j=0;
    while(i<s.size()) {
      //cout << "        s["<<i<<"]="<<s[i]<<", sub["<<j<<"]="<<sub[j]<<endl;
      // If s[i] is the start of a control character, there is no match
      if(s[i]=='\\') break;
      
      // If the current non-control character in s does matches the current character in sub
      if(s[i] == sub[j]) {
        // Advance to the next character in s
        i++;
        j++;
        
        // If we've reached the end of sub, we're done
        if(j==sub.size())
          return i-sub.size();
      // If it does NOT match
      } else
        break;
    }
    
    // If we've reached here, we must have failed to find a match to sub when starting from pos
    // so advance pos to the location that follow the current pos, skipping any escaped characters.
    
    // Move on to the next character
    pos++;
    // If the current character is escaped
    while(s[pos]=='\\') {
      // Advance past all the \'s that escape it
      while(s[pos]=='\\') pos++;
      // We're now at the character that follows the \'s meaning that it is the character that was escaped.
      // Advance immediately past this character
      pos++;
    }
  }
  
  // If we reached here, we could not find a match
  return string::npos;
}

// Searches the string for the first occurrence of any of the characters in string chars, starting at pos and returns 
// the location. The search ignores any matches that cross escaped characters in this string.
size_t escapedStr::findAny(std::string chars, size_t pos) const {
  //cout << "escapedStr::find("<<sub<<", "<<pos<<")"<<endl;
  
  // Iterate through s looking for a substring that is equal to sub, e
  while(pos<s.size()) {
    //cout << "    s["<<pos<<"]="<<s[pos]<<endl;
    
    // If s[pos] is the start of a control character, skip it
    if(s[pos]=='\\') {
      // Skip the /'s that precede the encoded character
      do {
        pos++;
      } while(s[pos]=='\\');
      // Skip the character itself
      pos++;
    // If this is a non-control character
    } else {
      // If the current chracter matches one in chars, we're done
      if(chars.find_first_of(s[pos]) != string::npos) return pos;
      // Otherwise, move on to the next position in s
      else 
        pos++;
    }
  }
  
  // If we reached here, we could not find a match
  return string::npos;
}

// Returns a newly constructed escaped string object with its value initialized to a copy of a substring of this object.
// The substring is the portion of the object that starts at character position pos and spans len characters (or until 
// the end of the string, whichever comes first).
std::string escapedStr::substr(size_t pos, size_t len) const {
  if(len==string::npos) len = s.size();
  
  string out;
  while(pos<len) {
    // If this is the start of an escaped character's encoding
    if(s[pos]=='\\') {
      // At least one additional character must follow this one
      assert(pos+1 < s.length());

      // Iterate through all the \s that may precede the actual character
      while(s[pos+1] == '\\') {
        // Emit the current escape \ since we'll definitely need it
        out += '\\';
        pos++;
      }

      // We're now at the \ that immediately precedes the character that was encoded
      // If it is a control character 
      //cout << "substr s["<<(pos+1)<<"]="<<s[pos+1]<<" control=\""<<control<<"\""<<endl; 
      if(control.find(s[pos+1]) != string::npos) {
        // Unescape it by emitting it without one of its encoding \'s
        out += s[pos+1];
      // If it is not a control character
      } else {
        // Emit it as it is, with the same number of encoding \'s
        out += '\\';
        out += s[pos+1];
      }
      //cout << "       out=\""<<out<<"\""<<endl;
      
      // Advance pos to refer to the next character
      pos+=2;
    // If this is not an encoded character, add it directly to out
    } else {
      out += s[pos];
      pos++;
    }
  }
  return out;
}

// Split the string into sub-strings separated by any character in the separator string and emit a list of the individual
// substrings, which have been unescaped. The separator characters must be a subset of this escapedStr's control characters.
std::vector<std::string> escapedStr::unescapeSplit(std::string separator) {
  //cout << "escapedStr::unescapeSplit("<<separator<<")"<<endl;
  std::vector<std::string> segments;
  size_t start = 0;
  size_t end = 0;
  int v=0;
  do {
    end = findAny(separator, start);
    //cout << "    ["<<start<<"-"<<end<<"]"<<endl;
    
    std::string segment = substr(start, end);
    //cout << "    segment=\""<<segment<<"\""<<endl;
    segments.push_back(segment);

    start = end+1; 
  } while(end != string::npos);
  
  return segments;
}

// Returns the fully unescaped version of this string, with the escaped characters replaced with the originals.
std::string escapedStr::unescape() const {
   return substr();
}

// Assignment 
escapedStr& escapedStr::operator=(const escapedStr& that) {
  s = that.s;
  control = that.control;
  return *this;
}

// Self-testing code for the escapedStr class.
void escapedStr::selfTest() {
  int numIter=100;

  std::string initStr = "ab;c-d:efg"; 
  escapedStr es;
  int i=0;
  for(; i<numIter; i++) {
    std::string control = (i%2==0? ";:": "-");
    es = escapedStr((i==0? initStr: es.escape()), control, escapedStr::unescaped);
    //cout << i << ": Control=\""<<control<<"\", Escaped: \""<<es.escape()<<"\", Unescaped=\""<<es.unescape()<<"\"\n";
  }
  
  //cout << "======="<<endl;

  i--;
  for(; i>=0; i--) {
    std::string control = (i%2==0? ";:": "-");
    es = escapedStr(i==numIter-1? es.escape(): es.unescape(), control, escapedStr::escaped);
    //cout << i << ": Control=\""<<control<<"\", Escaped: \""<<es.escape()<<"\", Unescaped=\""<<es.unescape()<<"\"\n";
  }
  
  assert(es.unescape() == initStr);

  // Encode the maps/keys/vals
  int numMaps = 10;
  int numVals = 3;
  string vals[] = {"ab:c", "d;;;ef", "g:h:i:"};

  ostringstream mapS;
  for(int m=0; m<numMaps; m++) {
    ostringstream keyvalsS;
    for(int v=0; v<numVals; v++) {
      if(v>0) keyvalsS << ";";
      escapedStr val(vals[v], ":;", escapedStr::unescaped);
      keyvalsS << "map_"<<m<<"key_"<<v<<":"<<val.escape();
    }

    escapedStr kv(keyvalsS.str(), ":", escapedStr::unescaped);
    if(m>0) mapS << ":";
    mapS << kv.escape();
  }

  //cout << "keys="<<mapS.str()<<endl;

  // Decode the maps/keys/values by directly calling find/substr
  {
    // Iterate over all the keys
    escapedStr allMaps(mapS.str(), ":", escapedStr::escaped);
    size_t mapStart = 0;
    size_t keysEnd;
    int m=0;
    do {
      keysEnd = allMaps.find(":", mapStart);
      escapedStr curMap(allMaps.substr(mapStart, keysEnd), ";:", escapedStr::escaped);

      //cout << "["<<mapStart<<" - "<<keysEnd<<"], curMap="<<curMap.unescape()<<endl;

      size_t keyvalsStart = 0;
      size_t valsEnd = 0;
      int v=0;
      do {
        valsEnd = curMap.find(";", keyvalsStart);
        std::string curKV = curMap.substr(keyvalsStart, valsEnd);

        //cout << "    ["<<keyvalsStart<<" - "<<valsEnd<<"], curMap="<<curKV<<endl;
        size_t colon = curKV.find(":");
        ostringstream key; key << "map_"<<m<<"key_"<<v;
        assert(key.str() == curKV.substr(0, colon));
        assert(vals[v] == curKV.substr(colon+1));

        keyvalsStart = valsEnd+1;
        v++;
      } while(valsEnd != string::npos);

      mapStart = keysEnd+1;
      m++;
    } while(keysEnd != string::npos);
  }
  
  // Decode the maps/keys/values by calling the split API
  {
    //cout << "mapS = "<<mapS.str()<<endl;
    escapedStr allMaps(mapS.str(), ":", escapedStr::escaped);
    vector<string> mapSegments = allMaps.unescapeSplit(":");
    int m=0;
    for(vector<string>::iterator ms=mapSegments.begin(); ms!=mapSegments.end(); ms++, m++) {
      //cout << "    ms="<<*ms<<endl;
      escapedStr curMap(*ms, ";", escapedStr::escaped);
      vector<string> keyvalSegments = curMap.unescapeSplit(";");
      int v=0;
      for(vector<string>::iterator kvs=keyvalSegments.begin(); kvs!=keyvalSegments.end(); kvs++, v++) {
        //cout << "        "<<*kvs << endl;
        escapedStr curKeyVal(*kvs, ":", escapedStr::escaped);
        vector<string> keyval = curKeyVal.unescapeSplit(":");
        assert(keyval.size()==2);
        //cout << "            "<<*keyval.begin()<<" => "<<*keyval.rbegin()<<endl;
        ostringstream key; key << "map_"<<m<<"key_"<<v;
        assert(key.str() == keyval[0]);
        assert(vals[v] == keyval[1]);
      }
    }
  }
}

/****************************
 ***** LoadTimeRegistry *****
 ****************************/
// The names of all the LoadTimeRegistry's derived classes that have already been initialized.
std::set<std::string>* LoadTimeRegistry::initialized;

// name - Unique string name of the class that derives from LoadTimeRegistry
// init - Function that is called to initialize this class
LoadTimeRegistry::LoadTimeRegistry(std::string name, initFunc init) {
  // Initialize the base LoadTimeRegistry class
  if(!getenv("SIGHT_LOADTIME_INSTANTIATED")) {
    initialized = new std::set<std::string>();
    setenv("SIGHT_LOADTIME_INSTANTIATED", "1", 1);
    initialized->insert("LOADTIME");
  }

  // Initialize the class that derives from LoadTimeRegistry, using environment variables to make sure that
  // only the first instance of this class performs the initialization.
  string envKey = txt()<<"SIGHT_"<<name<<"_INSTANTIATED";
  if(!getenv(envKey.c_str())) {
    init();
    setenv(envKey.c_str(), "1", 1);
    initialized->insert(name);
  }
}

// Removes all the environment variables that record the current mutexes of LoadTimeRegistry
void LoadTimeRegistry::liftMutexes() {
  for(std::set<std::string>::iterator i=initialized->begin(); i!=initialized->end(); i++) {
    string envKey = txt()<<"SIGHT_"<<*i<<"_INSTANTIATED";
    unsetenv(envKey.c_str());
  }
}

// Restores all the environment variables previously removed by liftMutexes
void LoadTimeRegistry::restoreMutexes() {
  for(std::set<std::string>::iterator i=initialized->begin(); i!=initialized->end(); i++) {
    string envKey = txt()<<"SIGHT_"<<*i<<"_INSTANTIATED";
    setenv(envKey.c_str(), "1", 1);
  }
}

// Create an instance of LoadTimeRegistry to ensure that it is initialized even if it is never derived from
LoadTimeRegistry LoadTimeRegInstance("BASE", LoadTimeRegistry::init);

/*********************************
 ***** TagFileReaderRegistry *****
 *********************************/

// Map the names of tags to the functions to be called when these tags are entered/exited
template<typename objType>
std::map<std::string, typename TagFileReaderRegistry<objType>::enterFunc>* TagFileReaderRegistry<objType>::enterHandlers;
template<typename objType>
std::map<std::string, typename TagFileReaderRegistry<objType>::exitFunc>*  TagFileReaderRegistry<objType>::exitHandlers;

// The stack of pointers to objects that encode the tags that are currently entered but not exited
template<typename objType>
std::map<std::string, std::list<objType*> > TagFileReaderRegistry<objType>::stack;

template<typename objType>
TagFileReaderRegistry<objType>::TagFileReaderRegistry(std::string name): 
     LoadTimeRegistry(name, TagFileReaderRegistry<objType>::init)
{} 

template<typename objType>
void TagFileReaderRegistry<objType>::init() {
  enterHandlers = new std::map<std::string, enterFunc>();
  exitHandlers  = new std::map<std::string, exitFunc>();
}

// Call the entry handler of the most recently-entered object with name objName
// and push the object it returns onto the stack.
template<typename objType>
void TagFileReaderRegistry<objType>::enter(string objName, properties::iterator iter) {
  #ifdef VERBOSE
  cout << "<<<"<<stack[objName].size()<<": "<<objName<<endl;
  #endif
  if(enterHandlers->find(objName) == enterHandlers->end()) { cerr << "ERROR: no entry handler for \""<<objName<<"\" tags!" << endl; }
  assert(enterHandlers->find(objName) != enterHandlers->end());
  stack[objName].push_back((*enterHandlers)[objName](iter));
}

// Call the exit handler of the most recently-entered object with name objName
// and pop the object off its stack.
template<typename objType>
void TagFileReaderRegistry<objType>::exit(string objName) {
  #ifdef VERBOSE
  cout << ">>>"<<stack[objName].size()<<": "<<objName<<endl;
  #endif
  assert(stack[objName].size()>0);
  if(exitHandlers->find(objName) == exitHandlers->end()) { cerr << "ERROR: no exit handler for \""<<objName<<"\" tags!" << endl; }
  assert(exitHandlers->find(objName) != exitHandlers->end());
  (*exitHandlers)[objName](stack[objName].back());
  stack[objName].pop_back();
}

template<typename objType>
std::string TagFileReaderRegistry<objType>::str() {
  ostringstream s;
  s << "confHandlers:\n";
  for(typename map<string, enterFunc>::const_iterator i=enterHandlers->begin(); i!=enterHandlers->end(); i++)
    s << i->first << endl;
  return s.str();
}

/*******************************
 ***** Configuration Files *****
 *******************************/

/***********************************
 ***** confHandlerInstantiator *****
 ***********************************/

sightConfHandlerInstantiator::sightConfHandlerInstantiator() {
/*  (*confEnterHandlers)["sight"] = &SightInit;
  (*confExitHandlers )["sight"] = &defaultExitHandler;
  (*confEnterHandlers)["indent"] = &indentEnterHandler;
  (*confExitHandlers )["indent"] = &indentExitHandler;
  (*confEnterHandlers)["link"]   = &anchor::link;
  (*confExitHandlers )["link"]   = &defaultExitHandler;*/
}
sightConfHandlerInstantiator sightConfHandlerInstantance;

// Loads the configuration file(s) stored in the files specified in the given environment variables
void loadSightConfig(const std::list<std::string>& cfgFNameEnv) {
  for(list<string>::const_iterator c=cfgFNameEnv.begin(); c!=cfgFNameEnv.end(); c++) {
    // If the user has specified a file name under the current environment variable
    if(getenv(c->c_str())) {
      escapedStr es(string(getenv(c->c_str())), ":", escapedStr::escaped);
      vector<string> files = es.unescapeSplit(":");
      for(vector<string>::iterator f=files.begin(); f!=files.end(); f++) {
        FILE* cfg = fopen(f->c_str(), "r");
        if(cfg==NULL) { cerr << "ERROR opening configuration file \""<<f->c_str()<<"\" for reading! "<<strerror(errno)<<endl; assert(cfg); }
        FILEStructureParser parser(cfg, 10000);
        loadConfiguration(parser);
        fclose(cfg);
      }
    }
  }
}

// Given a parser that reads a given configuration file, load it
void loadConfiguration(structureParser& parser) {
  #ifdef VERBOSE
  cout << TagFileReaderRegistry<Configuration>::str()<<endl;
  #endif

  pair<properties::tagType, const properties*> props = parser.next();
  while(props.second->size()>0) {
    if(props.first == properties::enterTag) {
      // Ignore all text between tags
      if(props.second->name() != "text")
        // Call the entry handler of the most recently-entered object with this tag name
        // and push the object it returns onto the stack dedicated to objects of this type.
        TagFileReaderRegistry<Configuration>::enter(props.second->name(), props.second->begin());
    } else if(props.first == properties::exitTag) {
      // Call the exit handler of the most recently-entered object with this tag name
      // and pop the object off its stack
      TagFileReaderRegistry<Configuration>::exit(props.second->name());
    }
    props = parser.next();
  }
}

}; // namespace common
}; // namespace sight
