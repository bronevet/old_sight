#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <list>
#include <set>
#include <sys/types.h>

namespace sight {
// Create directory workDir/dirName. The path of the created directory is returned.
std::string createDir(std::string workDir, std::string dirName);

// Copy dirName from the source code path to the workDir
std::string copyDir(std::string workDir, std::string dirName);
  
// Open a freshly-allocated output stream to write a file with the given name and return a pointer to the object.
std::ofstream& createFile(std::string fName);

// Returns a string that contains n tabs
std::string tabs(int n);

// Given a string and a line width returns a variant of the string where line breaks are inserted
// at approximately every lineWidth characters.
std::string wrapStr(std::string str, unsigned int lineWidth);

// Returns the human-readable representation of the given list of strings
std::string list2str(const std::list<std::string>& l);

// Returns the human-readable representation of the given set of strings
std::string set2str(const std::set<std::string>& l);

// Adapted from http://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
// Returns the directory and file name portion of the given path
std::pair<std::string, std::string> path2filedir(std::string s);

// Creates the directory with the given path, creating any sub-directories within it
// If isDir is true, s is a directory. Otherwise, it is a file and thus, we need to create its parent directory.
int mkpath(std::string s, mode_t mode, bool isDir=true);

} // namespace sight

