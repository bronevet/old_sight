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

using namespace std;

namespace dbglog {
// Create directory workDir/dirName. The path of the created directory is returned.
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

} // namespace dbglog