#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

namespace dbglog {
// Create directory workDir/dirName. The path of the created directory is returned.
std::string createDir(std::string workDir, std::string dirName);

// Copy dirName from the source code path to the workDir
std::string copyDir(std::string workDir, std::string dirName);
  
// Open a freshly-allocated output stream to write a file with the given name and return a pointer to the object.
std::ofstream& createFile(std::string fName);

// Returns a string that contains n tabs
std::string tabs(int n);
  
// Class that makes it possible to generate string labels by using the << syntax.
// Examples: Label() << "a=" << (5+2) << "!"
//           Label("a=") << (5+2) << "!"
struct txt : std::string {
  txt() {}
  txt(const std::string& initTxt) {
  	 _stream << initTxt;
  	 assign(_stream.str());
  }
  
  template <typename T>
  txt& operator<<(T const& t) {
    _stream << t;
    assign(_stream.str());
    return *this;
  }

  std::string str() const { return _stream.str(); }
  std::ostringstream _stream;
};

}; // namespace dbglog