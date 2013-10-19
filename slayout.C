#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
using namespace std;

bool readProperty(FILE* f, char buf[], int& bufIdx, int& bufSize, std::string& name, std::string& val, char& termChar);
bool readUntil(bool inTerm, const char* termChars, int numTermChars, char& termHit, FILE* f, char buf[], int& bufIdx, int& bufSize, string& result);
bool nextChar(FILE* f, char buf[], int& bufIdx, int& bufSize);
bool isMember(char c, const char* termChars, int numTermChars);

int main(int argc, char** argv) {
  if(argc!=2) { cerr<<"Usage: slayout fName"<<endl; exit(-1); }
  char* fName = argv[1];
  
  FILE* f = fopen(fName, "r");
  if(f==NULL) { cerr << "ERROR opening file \""<<fName<<"\" for reading! "<<strerror(errno)<<endl; exit(-1); }
  char buf[10000];
  // Start reading the file, filling as much of buf as possible. Store the number of
  // bytes read in bufSize and initialize bufIdx to refer to the start of buf.
//cout << "f="<<f<<", feof(f)="<<feof(f)<<", ferror(f)="<<ferror(f)<<", sizeof(buf)="<<sizeof(buf)<<endl;
  int bufSize = fread(buf, sizeof(char), sizeof(buf), f);
  int bufIdx=0;

  bool success = true;
  string readTxt; // String where text read by readUntil() will be placed
  char termChar; // Character where readUntil() places the character that caused parsing to terminate
  string tagName; // The name of the current tag

  // Stack of properties maps for classes that were derived by some other class.
  // This entire stack will be given to the functor for the most derived class 
  // so that it can pass the sub-property maps to its parents so that they can initialize.
  list<pair<string, map<string, string> > > pStack;
  while(success) {
//cout << "main loop, bufIdx="<<bufIdx<<", bufSize="<<bufSize<<endl;
    // We must currently be outside of any tag

    // Look for the start of the next tag
    success = readUntil(true, "[", 1, termChar, f, buf, bufIdx, bufSize, readTxt);
    // Emit the text before the start of the tag
    cout << "text=\""<<readTxt<<"\""<<endl;
      
    if(!success) goto PARSE_END;

    nextChar(f, buf, bufIdx, bufSize);

    // Now look for the end of name of the tag (a whitespace char) or the / char 
    // that indicates that this is the end rather than beginning of a tag
    if(!(success = readUntil(true, " \t\r\n/|", 6, termChar, f, buf, bufIdx, bufSize, tagName))) goto PARSE_END;
//cout << "tagName=\""<<tagName<<"\" termChar=\""<<termChar<<"\""<<" buf[bufIdx]=\""<<buf[bufIdx]<<"\""<<endl;

    // If this is the end of a tag, read until its end to determine its name
    if(termChar == '/') {
      if(!nextChar(f, buf, bufIdx, bufSize)) goto PARSE_END;

      if(!(success = readUntil(true, " \t\r\n", 4, termChar, f, buf, bufIdx, bufSize, tagName))) goto PARSE_END;
      if(!nextChar(f, buf, bufIdx, bufSize)) goto PARSE_END;
      
      string ID;
      if(!(success = readUntil(true, "]", 1, termChar, f, buf, bufIdx, bufSize, ID))) goto PARSE_END;
      if(!nextChar(f, buf, bufIdx, bufSize)) goto PARSE_END;
      cout << "END \""<<tagName<<"\" ID="<<ID<<endl;
    } else {
      // Records whether this entry tag corresponds to a base class that has been derived by another
      bool derived=false;

      //cout << "START "<<tagName<<" termChar="<<termChar<<endl;
      if(termChar == '|') {
        if(!nextChar(f, buf, bufIdx, bufSize)) goto PARSE_END;
        if(!(success = readUntil(true, " \t\r\n", 4, termChar, f, buf, bufIdx, bufSize, tagName))) goto PARSE_END;

        derived=true;
      }

      map<string, string> properties;
	
      // Read the "numProperties" property
      string propName, propVal;
      if(!(success = readProperty(f, buf, bufIdx, bufSize, propName, propVal, termChar))) { goto PARSE_END; }
      if(propName != "numProperties") { cerr<< "ERROR: expecting numProperties property in tag "<<tagName<<" but got \""<<propName<<"\"!"<<endl; exit(-1); }
      long int numProps = strtol(propVal.c_str(), NULL, 10);
//cout << "propName=\""<<propName<<"\", propVal=\""<<propVal<<"\", numProps="<<numProps<<", termChar=\""<<termChar<<"\""<<endl;

      // Skip until the start of the next property or the end of the tag
      if(!(success = readUntil(true, " \t\r\n]", 5, termChar, f, buf, bufIdx, bufSize, readTxt))) goto PARSE_END;

      // Read the properties of this tag
      for(long int p=0; p<numProps; p++) {
        // If we reached the end of the tag before processing all the properties
        if(termChar==']') { cerr << "ERROR: reached the end of tag "<<tagName<<" after processing "<<p<<" properties but expected "<<numProps<<" properties!"<<endl; exit(-1); }
//cout << "  prop "<<p<<": termChar=\""<<termChar<<"\""<<" buf["<<bufIdx<<"]=\""<<buf[bufIdx]<<"\""<<endl;

        // Read the name/val pair that encodes the name of the current property
        string propNameName, propNameVal;
        if(!(success = readProperty(f, buf, bufIdx, bufSize, propNameName, propNameVal, termChar))) { goto PARSE_END; }

//cout << "  prop "<<p<<": "<<propNameName<<" = "<<propNameVal<<endl;

        // Skip until the start of the next name/val pair
        if(!(success = readUntil(true, " \t\r\n]", 5, termChar, f, buf, bufIdx, bufSize, readTxt))) goto PARSE_END;
        if(termChar==']') { cerr << "ERROR: reached the end of tag "<<tagName<<" reading the name of property "<<p<<" but expected "<<numProps<<" properties!"<<endl; exit(-1); }

        // Read the name/val pair that encodes the name of the current property
        string propValName, propValVal;
        if(!(success = readProperty(f, buf, bufIdx, bufSize, propValName, propValVal, termChar))) { goto PARSE_END; }

//        cout << "  prop "<<p<<": "<<propValName<<" = "<<propValVal<<endl;

        properties[propNameVal] = propValVal;

//        cout << "  prop "<<p<<": termChar=\""<<termChar<<"\""<<" buf["<<bufIdx<<"]=\""<<buf[bufIdx]<<"\""<<endl;

        // Skip until the start of the next property or the end of the tag
        if(!(success = readUntil(true, " \t\r\n]", 5, termChar, f, buf, bufIdx, bufSize, readTxt))) goto PARSE_END;
//        cout << "  prop "<<p<<": termChar=\""<<termChar<<"\""<<" buf["<<bufIdx<<"]=\""<<buf[bufIdx]<<"\""<<endl;
      }
      //cout << "end tag: termChar=\""<<termChar<<"\""<<" buf["<<bufIdx<<"]=\""<<buf[bufIdx]<<"\""<<endl;

      // If we failed to reach the end of the tag after processing all the properties
      if(termChar!=']') {
        // Keep reading the remaining whitespace
        if(!(success = readUntil(true, " \t\r\n", 4, termChar, f, buf, bufIdx, bufSize, readTxt))) goto PARSE_END;

        // This must be the end of the tag
        if(termChar != ']')
        { cerr << "ERROR: failed to reached the end of tag "<<tagName<<" after processing "<<numProps<<" properties! termChar=\""<<termChar<<"\""<<endl; exit(-1); }
      }
      if(!nextChar(f, buf, bufIdx, bufSize)) goto PARSE_END;

      // If this tag corresponds to an object at the outer-most level of a derivation hierarchy
      // (it was not derived by another)
      if(!derived) {
        // Print out the properties
        cout << "START "<<tagName<<endl;
        pStack.push_back(make_pair(tagName, properties));
        for(list<pair<string, map<string, string> > >::iterator s=pStack.begin(); s!=pStack.end(); s++) {
          cout << "    "<<s->first<<":"<<endl;
          for(map<string, string>::iterator p=s->second.begin(); p!=s->second.end(); p++)
            cout << "        \""<<p->first<<"\" : \""<<p->second<<"\""<<endl;
        }
        pStack.clear();
      // If this tag's class was derived by another, add its properties to pStack so that it can be
      // picked up when we reach the derived class' tag
      } else
        pStack.push_back(make_pair(tagName, properties));
    }
  }

  PARSE_END:

  fclose(f);
}

// Read a property name/value pair from the given file, setting name and val to them.
// Reading starts at buf[bufIdx] and continues as far as needed, reading more file 
// contents into buf if the end of buf is reached. bufSize is the number of bytes in 
// the buffer. When the function returns bufIdx is updated to hold the current 
// position inside buf, immediately after the terminator char was encountered and 
// bufSize is updated to hold the total number of characters currently in buf. 
// termChar is set to be the character that immediately follows the property name/value.
// pair. This information enables the caller to determine whether the tag is finished.
// or whether more properties exist in the tag. If readProperty() reads the full 
// property info before it reaches the end of the file, it returns true and false otherwise.
bool readProperty(FILE* f, char buf[], int& bufIdx, int& bufSize, std::string& name, std::string& val, char& termChar) {

  string readTxt; // Generic string to hold read data

  // Skip until the next non-whitespace character
  if(!readUntil(false, " \t\r\n", 4, termChar, f, buf, bufIdx, bufSize, readTxt)) return false;
  if(!nextChar(f, buf, bufIdx, bufSize)) return false;
//cout << "    rp1: readTxt=\""<<readTxt<<"\" termChar=\""<<termChar<<"\""<<" buf[bufIdx]=\""<<buf[bufIdx]<<"\""<<endl;
  
  // Read until the next '='
  if(!readUntil(true, "=", 1, termChar, f, buf, bufIdx, bufSize, name)) return false;
  if(!nextChar(f, buf, bufIdx, bufSize)) return false;

//cout << "    rp2: name=\""<<name<<"\", termChar=\""<<termChar<<"\""<<endl;
  // Read until the next non-'='
  if(!readUntil(false, "=", 1, termChar, f, buf, bufIdx, bufSize, readTxt)) return false;

  //if(readTxt.length()!=1) { cerr << "ERROR: multiple = chars in property "<<name<<"! read \""<<readTxt<<"\" termChar=\""<<termChar<<"\""<<endl; return false; }
//cout << "    rp3: readTxt=\""<<readTxt<<"\", termChar=\""<<termChar<<"\" buf[bufIdx]=\""<<buf[bufIdx]<<"\""<<endl;

  // Read the " that immediately follows
  if(!readUntil(true, "\"", 1, termChar, f, buf, bufIdx, bufSize, readTxt)) return false;
  if(!nextChar(f, buf, bufIdx, bufSize)) return false;
//cout << "    rp4: readTxt=\""<<readTxt<<"\", termChar=\""<<termChar<<"\" buf[bufIdx]=\""<<buf[bufIdx]<<"\""<<endl;

  if(termChar!='\"' && readTxt.length()!=0) { cerr << "ERROR: erroneous chars between = and \" in tag property "<<name<<"! read \""<<readTxt<<"\" termChar=\""<<termChar<<"\""<<endl; return false; }

  // Read until the following "
  if(!readUntil(true, "\"", 1, termChar, f, buf, bufIdx, bufSize, val)) return false;
//cout << "    rp5: val=\""<<val<<"\", termChar=\""<<termChar<<"\" buf[bufIdx]=\""<<buf[bufIdx]<<"\""<<endl;

  return true;
}

// Reads all the characters from FILE* f until the next terminating character.
// If inTerm is true, a terminating char is anything in termChars. If inTerm is 
// false, a terminating char is anything not in termChars. readUntil() then sets 
// result to the read text and sets termHit to the terminator char that was hit. 
// Reading starts at buf[bufIdx] and continues as far as needed, reading more file 
// contents into buf if the end of buf is reached. bufSize is the number of bytes in 
// the buffer. When the function returns bufIdx is updated to hold the current 
// position inside buf, immediately after the terminator char was encountered and 
// bufSize is updated to hold the total number of characters currently in buf. If 
// readUntil() finds a terminator char before it reaches the end of the file, it 
// returns true and false otherwise.
bool readUntil(bool inTerm, const char* termChars, int numTermChars, char& termHit, FILE* f, char buf[], int& bufIdx, int& bufSize, string& result) {
  result = "";
  // Outer loop that keeps reading more chunks of size bufSize from the file
  while(1) {
    //cout << "        ru: bufIdx="<<bufIdx<<", bufSize="<<bufSize<<endl;
    // Iterate through the rest of the file looking for terminator chars
    while(bufIdx<bufSize) {
      for(int i=0; i<numTermChars; i++) {
        // If the current character in buf is a terminator, return the result
        if(isMember(buf[bufIdx], termChars, numTermChars)) {
          if(inTerm) { termHit=buf[bufIdx]; return true; }
        } else {
          if(!inTerm) { termHit=buf[bufIdx]; return true; }
        }
      }
      // If the current character in buf is a terminator, return the result
      if(!inTerm) {
        termHit=buf[bufIdx];
        //nextChar(f, buf, bufIdx, bufSize);
        return true;
      }
      // If the character is not a terminator, append it to ret
      result += buf[bufIdx];
      bufIdx++;
    }

    //cout << "        ru: feof(f)="<<feof(f)<<", ferror(f)="<<ferror(f)<<endl;

    // If we've reached the end of the file, return unsuccessfuly
    if(feof(f)) return false;

    // If we've encountered an error, yell
    if(ferror(f)) {
      fprintf(stderr, "ERROR reading file!");
      exit(-1);
    }
    
    nextChar(f, buf, bufIdx, bufSize);
  }
}

// Advances the current reading position by 1 char, reading file contents into buf 
// if the end of buf is reached. bufSize is the number of bytes in 
// the buffer. When the function returns bufIdx is updated to hold the current 
// position inside buf, immediately after the terminator char was encountered and 
// bufSize is updated to hold the total number of characters currently in buf. 
// Returns the read char. If there is a next character in the file, returns true.
// Otherwise, returns false.
bool nextChar(FILE* f, char buf[], int& bufIdx, int& bufSize) {
  // If there is another char in buf to read, advance to it
  if(bufIdx<bufSize-1) {
    bufIdx++;
    return true;
  // Otherwise, read more of the file into buf
  } else {
    // Read the next bufSize chunk of chars from the file. The return value is stored
    // in bufSize to make sure that the caller is informed if fread() hits an error 
    // or file end and doesn't fill buf up. When we finally consume all the bytes
    // just read we'll reach the above feof() and ferror() tests and exit. Note that
    // before this happens we may reach the terminating chars and exit/enter 
    // readUntil() multiple times.
    bufSize = fread(buf, sizeof(char), bufSize, f);

    // Reset bufIdx to refer to the start of buf
    bufIdx=0;

    if(bufSize>0) return true;
  }

  return false;
}

// Returns true if character c is in array termChars of size numTermChars and 
// false otherwise.
bool isMember(char c, const char* termChars, int numTermChars) {
  for(int i=0; i<numTermChars; i++) {
    // If c is found in termChars
    if(c==termChars[i]) return true;
  }
  return false;
}
