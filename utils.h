#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

namespace sight {
// Create directory workDir/dirName. The path of the created directory is returned.
std::string createDir(std::string workDir, std::string dirName);

// Copy dirName from the source code path to the workDir
std::string copyDir(std::string workDir, std::string dirName);
  
// Open a freshly-allocated output stream to write a file with the given name and return a pointer to the object.
std::ofstream& createFile(std::string fName);

// Returns a string that contains n tabs
std::string tabs(int n);

}; // namespace sight