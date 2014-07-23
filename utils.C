#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "binreloc.h"
#include <errno.h>
#include "getAllHostnames.h" 
#include "utils.h"
#include <sys/stat.h>

using namespace std;

namespace sight {
// Create the directory workDir/dirName and return the string that contains the absolute
// path of the created directory
string createDir(string workDir, string dirName) {
  ostringstream fullDirName; fullDirName<<workDir<<"/"<<dirName;
  ostringstream cmd; cmd << "mkdir -p "<<fullDirName.str();
  int ret = system(cmd.str().c_str());
  if(ret == -1) { cout << "ERROR creating directory \""<<workDir<<"/"<<dirName<<"\"!"; exit(-1); }
  return fullDirName.str();
}

// Copy dirName from the source code path to the workDir
string copyDir(string workDir, string dirName) {
  ostringstream fullDirName; fullDirName << workDir << "/" << dirName;
  
  ostringstream cmd; cmd << "cp -fr "<<ROOT_PATH<<"/"<<dirName<<" "<<workDir;
  //cout << "Command \""<<cmd.str()<<"\"\n";
  int ret = system(cmd.str().c_str());
  if(ret == -1) { cout << "ERROR copying files from directory \""<<ROOT_PATH<<"/"<<dirName<<"\" directory \""<<fullDirName.str()<<"\"!"; exit(-1); }
    
  chmod(fullDirName.str().c_str(), S_IRWXU);
    
  return fullDirName.str();
}

// Open a freshly-allocated output stream to write a file with the given name and return a pointer to the object.
ofstream& createFile(string fName) {
  ofstream *f = new ofstream();
   try {
    f->open(fName.c_str(), std::ios::out);
  } catch (ofstream::failure e)
  { cout << "createFile() ERROR opening file \""<<fName<<"\" for writing!"; exit(-1); }
  
  return *f;
}

// Returns a string that contains n tabs
string tabs(int n)
{
  string s;
  for(int i=0; i<n; i++)
    s+="\t";
  return s;
}

// Given a string and a line width returns a variant of the string where line breaks are inserted
// at approximately every lineWidth characters.
std::string wrapStr(std::string str, unsigned int lineWidth) {
  string multiLineStr = "";
  unsigned int i=0;
  while(i<str.length()-lineWidth) {
    // Look for the next line-break
    unsigned int nextLB = str.find_first_of("\n", i);
    // If the next line is shorter than lineWidth, add it to labelMulLineStr and move on to the next line
    if(nextLB-i < lineWidth) {
      multiLineStr += str.substr(i, nextLB-i+1);
      i = nextLB+1;
    // If the next line is longer than lineWidth, add just lineWidth characters to labelMulLineStr
    } else {
      // If it is not much longer than lineWidth, don't break it up
      if(i>=str.length()-lineWidth*1.25) break;
      multiLineStr += str.substr(i, lineWidth) + "\\n";
      i += lineWidth;
    }
  }
  // Add the last line in str to labelMulLineStr
  if(i<str.length())
    multiLineStr += str.substr(i, str.length()-i);
  
  return multiLineStr;
}

// Returns the human-readable representation of the given list of strings
std::string list2str(const std::list<std::string>& l) {
  ostringstream s;
  s << "[";
  for(list<string>::const_iterator i=l.begin(); i!=l.end(); i++) {
    if(i!=l.begin()) s << ", ";
    s << *i;
  }
    
  s << "]";
  return s.str();
}

// Returns the human-readable representation of the given set of strings
std::string set2str(const std::set<std::string>& l) {
  ostringstream s;
  s << "[";
  for(set<string>::const_iterator i=l.begin(); i!=l.end(); i++) {
    if(i!=l.begin()) s << ", ";
    s << *i;
  }
    
  s << "]";
  return s.str();
}

// Adapted from http://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
// Returns the directory and file name portion of the given path
std::pair<std::string, std::string> path2filedir(std::string s)
{
    size_t pre=0;
    std::string dir;
    int mdret;

    if(s[s.size()-1]!='/'){
        // force trailing / so we can handle everything in loop
        s+='/';
    }

    size_t pos=s.find_first_of('/',pre);
    size_t nextPos=s.find_first_of('/',pos+1);
    // While another path portion is available and 
    // either we're generating the entire path or we're omitting the last portion but we have not yet reached it
    while(pos!=std::string::npos && nextPos !=std::string::npos){
        pre=pos;
        pos=nextPos+1;
        nextPos=s.find_first_of('/',nextPos+1);
    }
//cout << "path2filedir() #s="<<s.size()<<" [0, "<<pre<<"] "<<s.substr(0, pre)<<" / ["<<(pre+1)<<", "<<(pos-1)<<"] "<<s.substr(pre+1, pos-1-(pre+1))<<endl;
    return make_pair(s.substr(0, pre), s.substr(pre, pos-1-pre));
}
// Adapted from http://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
// Creates the directory with the given path, creating any sub-directories within it
// If isDir is true, s is a directory. Otherwise, it is a file and thus, we need to create its parent directory.
int mkpath(std::string s,mode_t mode, bool isDir)
{
    size_t pre=0;
    std::string dir;
    int mdret;

    if(s[s.size()-1]!='/'){
        // force trailing / so we can handle everything in loop
        s+='/';
    }

    size_t pos=s.find_first_of('/',pre);
    size_t nextPos=s.find_first_of('/',pos+1);
    // While another path portion is available and 
    // either we're generating the entire path or we're omitting the last portion but we have not yet reached it
    while(pos!=std::string::npos &&
          (isDir || nextPos !=std::string::npos)){
        dir=s.substr(0,pos++);
        pre=pos;
        if(dir.size()==0) continue; // if leading / first time is 0 length
        if((mdret=mkdir(dir.c_str(),mode)) && errno!=EEXIST){
            return mdret;
        }

        pos=nextPos+1;
        nextPos=s.find_first_of('/',nextPos+1);
    }
    return mdret;
}

} // namespace sight
