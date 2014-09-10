#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "process.h"
#include "sight_common_internal.h"
using namespace std;
using namespace sight::common;

//#define VERBOSE
namespace sight {

/*******************************
 ***** baseStructureParser *****
 *******************************/

template<typename streamT>
baseStructureParser<streamT>::baseStructureParser(int bufSize) : bufSize(bufSize) {
  init(NULL);
}

template<typename streamT>
baseStructureParser<streamT>::baseStructureParser(streamT* stream, int bufSize) : bufSize(bufSize) {
  init(stream);
}

template<typename streamT>
void baseStructureParser<streamT>::init(streamT* stream) {
  this->stream = stream;
  buf = new char[bufSize];
  assert(buf);
  
  loc = start;
  
  tagProperties.clear();
}

template<typename streamT>
pair<typename properties::tagType, const properties*> baseStructureParser<streamT>::next() {
  bool success = true;
  string readTxt; // String where text read by readUntil() will be placed
  char termChar;  // Character where readUntil() places the character that caused parsing to terminate
  string tagName; // The name of the current tag
  string propName, propVal;
  long int numProps;
  bool derived;
  
  std::map<std::string, std::string> pMap;
  
  // Reset tagProperties in preparation of a new tag being read
  tagProperties.clear();

  #ifdef VERBOSE 
  cout << "============"<<(loc==start?"start":(loc==textRead?"textRead":(loc==enterTagRead?"enterTagRead":(loc==exitTagRead?"exitTagRead":(loc==done?"done":"???")))))<<"============"<<endl;
  if(loc!=start) cout << "buf["<<bufIdx<<"]="<<buf[bufIdx]<<endl;
  #endif
 
  // When we enter this function we resume text processing from where we left off
  // in the last call to getNext(). loc records this location and we now jump to it.
  switch(loc) {
    case start:        goto START_LOC;
    case textRead:     goto TEXT_READ_LOC;
    case enterTagRead: goto ENTER_TAG_READ_LOC;
    case exitTagRead:  goto EXIT_TAG_READ_LOC;
    case done:         goto DONE_LOC;
  }

  START_LOC:
  // Start reading the file, filling as much of buf as possible. Store the number of
  // bytes read in dataInBuf and initialize bufIdx to refer to the start of buf.
  //cout << "f="<<f<<", feof(f)="<<feof(f)<<", ferror(f)="<<ferror(f)<<", sizeof(buf)="<<sizeof(buf)<<endl;
  dataInBuf = readData();
  bufIdx=0;
  
  while(success) {
//cout << "main loop, bufIdx="<<bufIdx<<", bufSize="<<bufSize<<endl;
    // We must currently be outside of a tag, although we may be between the multiple individual
    // tags that encode entry into an object with a multi-level inheritance hierarchy (multiple tags)

    // Look for the start of the next tag
    success = readUntil(true, "[", 1, termChar, readTxt);
    // Emit the text before the start of the tag
    #ifdef VERBOSE
    cout << "text=\""<<readTxt<<"\""<<endl;
    #endif
    //dbg << readTxt;
    
    if(readTxt != "") {
      std::map<std::string, std::string> pMap;
      pMap["text"] = readTxt;
      tagProperties.add("text", pMap);
      loc = textRead;
      return make_pair(properties::enterTag, &tagProperties);
    }
    
    if(!success) goto DONE_LOC;
      
    TEXT_READ_LOC:

    nextChar();

    // Now look for the end of name of the tag (a whitespace char) or the / char 
    // that indicates that this is the end rather than beginning of a tag
    if(!(success = readUntil(true, " \t\r\n/|", 6, termChar, tagName))) goto DONE_LOC;

    // If this is the end of a tag, read until its end to determine its name
    if(termChar == '/') {
      if(!nextChar()) goto DONE_LOC;

      if(!(success = readUntil(true, "]", 1, termChar, tagName))) goto DONE_LOC;
      //if(!(success = readUntil(true, " \t\r\n", 4, termChar, tagName))) goto DONE_LOC;

      #ifdef VERBOSE
      cout << "END \""<<tagName<<"\""<<endl;
      #endif
      
      tagProperties.add(tagName, pMap);
      loc = exitTagRead;
      return make_pair(properties::exitTag, &tagProperties);
      
      EXIT_TAG_READ_LOC:
      
      if(!nextChar()) goto DONE_LOC;
    } else {
      // Records whether this entry tag corresponds to a base class that has been derived by another
      derived=false;

      //cout << "START "<<tagName<<" termChar="<<termChar<<endl;
      if(termChar == '|') {
        if(!nextChar()) goto DONE_LOC;
        if(!(success = readUntil(true, " \t\r\n", 4, termChar, tagName))) goto DONE_LOC;

        derived=true;
      }
      	
      // Read the "numProperties" property
      if(!(success = readProperty(propName, propVal, termChar))) { goto DONE_LOC; }
      if(propName != "numProperties") { cerr<< "ERROR: expecting numProperties property in tag "<<tagName<<" but got \""<<propName<<"\"!"<<endl; exit(-1); }
      numProps = strtol(propVal.c_str(), NULL, 10);
//cout << "propName=\""<<propName<<"\", propVal=\""<<propVal<<"\", numProps="<<numProps<<", termChar=\""<<termChar<<"\""<<endl;

      // Skip until the start of the next property or the end of the tag
      if(!(success = readUntil(true, " \t\r\n]", 5, termChar, readTxt))) goto DONE_LOC;

      // Read the properties of this tag
      pMap.clear();
      for(long int p=0; p<numProps; p++) {
        // If we reached the end of the tag before processing all the properties
        if(termChar==']') { cerr << "ERROR: reached the end of tag "<<tagName<<" after processing "<<p<<" properties but expected "<<numProps<<" properties!"<<endl; exit(-1); }
//cout << "  prop "<<p<<": termChar=\""<<termChar<<"\""<<" buf["<<bufIdx<<"]=\""<<buf[bufIdx]<<"\""<<endl;

        // Read the name/val pair that encodes the name of the current property
        string propNameName, propNameVal;
        if(!(success = readProperty(propNameName, propNameVal, termChar))) { goto DONE_LOC; }

        //cout << "  prop "<<p<<": "<<propNameName<<" = "<<propNameVal<<endl;

        // Skip until the start of the next name/val pair
        if(!(success = readUntil(true, " \t\r\n]", 5, termChar, readTxt))) goto DONE_LOC;
        if(termChar==']') { cerr << "ERROR: reached the end of tag "<<tagName<<" reading the name of property "<<p<<" but expected "<<numProps<<" properties!"<<endl; exit(-1); }

        // Read the name/val pair that encodes the name of the current property
        string propValName, propValVal;
        if(!(success = readProperty(propValName, propValVal, termChar))) { goto DONE_LOC; }

        //cout << "  prop "<<p<<": "<<propValName<<" = "<<propValVal<<endl;

        pMap[unescape(propNameVal)] = unescape(propValVal);

//        cout << "  prop "<<p<<": termChar=\""<<termChar<<"\""<<" buf["<<bufIdx<<"]=\""<<buf[bufIdx]<<"\""<<endl;

        // Skip until the start of the next property or the end of the tag
        if(!(success = readUntil(true, " \t\r\n]", 5, termChar, readTxt))) goto DONE_LOC;
//        cout << "  prop "<<p<<": termChar=\""<<termChar<<"\""<<" buf["<<bufIdx<<"]=\""<<buf[bufIdx]<<"\""<<endl;
      }
      //cout << "end tag: termChar=\""<<termChar<<"\""<<" buf["<<bufIdx<<"]=\""<<buf[bufIdx]<<"\""<<endl;

      // If we failed to reach the end of the tag after processing all the properties
      if(termChar!=']') {
        // Keep reading the remaining whitespace
        if(!(success = readUntil(true, " \t\r\n", 4, termChar, readTxt))) goto DONE_LOC;

        // This must be the end of the tag
        if(termChar != ']')
        { cerr << "ERROR: failed to reached the end of tag "<<tagName<<" after processing "<<numProps<<" properties! termChar=\""<<termChar<<"\""<<endl; exit(-1); }
      }
      if(!nextChar()) goto DONE_LOC;

      // If this tag corresponds to an object at the outer-most level of a derivation hierarchy
      // (it was not derived by another)
      if(!derived) {
        // Print out the properties
        #ifdef VERBOSE
        cout << "START "<<tagName<<endl;
        #endif
        tagProperties.add(tagName, pMap);
        #ifdef VERBOSE
        cout << tagProperties.str()<<endl;
        #endif
        
        loc = enterTagRead;
        return make_pair(properties::enterTag, &tagProperties);
        
        ENTER_TAG_READ_LOC:
        ;
      // If this tag's class was derived by another, add its properties to pStack so that it can be
      // picked up when we reach the derived class' tag
      } else
        tagProperties.add(tagName, pMap);
    }
  }

  DONE_LOC:
  loc = done;
  return make_pair(properties::exitTag, &tagProperties);
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
template<typename streamT>
bool baseStructureParser<streamT>::readProperty(std::string& name, std::string& val, char& termChar) {
  string readTxt; // Generic string to hold read data

  // Skip until the next non-whitespace character
  if(!readUntil(false, " \t\r\n", 4, termChar, readTxt)) return false;
  if(!nextChar()) return false;
//cout << "    rp1: readTxt=\""<<readTxt<<"\" termChar=\""<<termChar<<"\""<<" buf[bufIdx]=\""<<buf[bufIdx]<<"\""<<endl;
  
  // Read until the next '='
  if(!readUntil(true, "=", 1, termChar, name)) return false;
  if(!nextChar()) return false;

//cout << "    rp2: name=\""<<name<<"\", termChar=\""<<termChar<<"\""<<endl;
  // Read until the next non-'='
  if(!readUntil(false, "=", 1, termChar, readTxt)) return false;

  //if(readTxt.length()!=1) { cerr << "ERROR: multiple = chars in property "<<name<<"! read \""<<readTxt<<"\" termChar=\""<<termChar<<"\""<<endl; return false; }
//cout << "    rp3: readTxt=\""<<readTxt<<"\", termChar=\""<<termChar<<"\" buf[bufIdx]=\""<<buf[bufIdx]<<"\""<<endl;

  // Read the " that immediately follows
  if(!readUntil(true, "\"", 1, termChar, readTxt)) return false;
  if(!nextChar()) return false;
//cout << "    rp4: readTxt=\""<<readTxt<<"\", termChar=\""<<termChar<<"\" buf[bufIdx]=\""<<buf[bufIdx]<<"\""<<endl;

  if(termChar!='\"' && readTxt.length()!=0) { cerr << "ERROR: erroneous chars between = and \" in tag property "<<name<<"! read \""<<readTxt<<"\" termChar=\""<<termChar<<"\""<<endl; return false; }

  // Read until the following "
  if(!readUntil(true, "\"", 1, termChar, val)) return false;
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
template<typename streamT>
bool baseStructureParser<streamT>::readUntil(bool inTerm, const char* termChars, int numTermChars, 
                                char& termHit, string& result) {
  result = "";
  // Outer loop that keeps reading more chunks of size bufSize from the file
  while(1) {
    //cout << "        ru: bufIdx="<<bufIdx<<", bufSize="<<bufSize<<endl;
    // Iterate through the rest of the file looking for terminator chars
    while(bufIdx<dataInBuf) {
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
        //nextChar();
        return true;
      }
      // If the character is not a terminator, append it to ret
      result += buf[bufIdx];
      bufIdx++;
    }

    //cout << "        ru: feof(f)="<<feof(f)<<", ferror(f)="<<ferror(f)<<endl;

    // If we've reached the end of the file, return unsuccessfuly
    if(streamEnd()) return false;

    // If we've encountered an error, yell
    if(streamError()) {
      fprintf(stderr, "ERROR reading file!");
//      exit(-1);
    }
    
    nextChar();
  }
}

// Advances the current reading position by 1 char, reading file contents into buf 
// if the end of buf is reached. bufSize is the number of bytes in 
// the buffer. When the function returns bufIdx is updated to hold the current 
// position inside buf, immediately after the terminator char was encountered and 
// bufSize is updated to hold the total number of characters currently in buf. 
// Returns the read char. If there is a next character in the file, returns true.
// Otherwise, returns false.
template<typename streamT>
bool baseStructureParser<streamT>::nextChar() {
  // If there is another char in buf to read, advance to it
  if(bufIdx<dataInBuf-1) {
    bufIdx++;
    return true;
  // Otherwise, read more of the file into buf
  } else {
    // Read the next bufSize chunk of chars from the file. The return value is stored
    // in bufSize to make sure that the caller is informed if readData() hits an error 
    // or file end and doesn't fill buf up. When we finally consume all the bytes
    // just read we'll reach the above feof() and ferror() tests and exit. Note that
    // before this happens we may reach the terminating chars and exit/enter 
    // readUntil() multiple times.
    dataInBuf = readData();

    // Reset bufIdx to refer to the start of buf
    bufIdx=0;

    if(dataInBuf>0) return true;
  }

  return false;
}

// Returns true if character c is in array termChars of size numTermChars and 
// false otherwise.
template<typename streamT>
bool baseStructureParser<streamT>::isMember(char c, const char* termChars, int numTermChars) {
  for(int i=0; i<numTermChars; i++) {
    // If c is found in termChars
    if(c==termChars[i]) return true;
  }
  return false;
}


/*******************************
 ***** FILEStructureParser *****
 *******************************/

FILEStructureParser::FILEStructureParser(string path, int bufSize) : 
  baseStructureParser<FILE>(bufSize)
{
  struct stat st;
  int ret = stat(path.c_str(), &st);
  if(ret!=0) { cerr << "ERROR calling stat on file \""<<path<<"! "<<strerror(errno)<<endl; exit(-1); }

  // Get the name of the structure file
  string structFName;
  // If the path is a directory
  if(st.st_mode & S_IFDIR) structFName = txt()<<path<<"/structure";
  else                     structFName = path;
 
  FILE* f = fopen(structFName.c_str(), "r");
  if(f==NULL) { cerr << "ERROR opening file \""<<structFName<<"\" for reading! "<<strerror(errno)<<endl; exit(-1); }
  openedFile=true;
  init(f);
}

FILEStructureParser::FILEStructureParser(FILE* f, int bufSize) : baseStructureParser<FILE>(f, bufSize) {
  openedFile=false;
}

FILEStructureParser::~FILEStructureParser() {
  // If we opened the file, we must close it
  if(openedFile)
    fclose(stream);
}

// Functions implemented by children of this class that specialize it to take input from various sources.

// readData() reads as much data as is available from the data source into buf[], upto bufSize bytes 
// and returns the amount of data actually read.
size_t FILEStructureParser::readData() {
  return fread(buf, 1, bufSize, stream);
}

// Returns true if we've reached the end of the input stream
bool FILEStructureParser::streamEnd() {
  return feof(stream);
}

// Returns true if we've encountered an error in input stream
bool FILEStructureParser::streamError() {
  return ferror(stream);
}

/*******************************
 ***** MRNEtStructureParser *****
 *******************************/
    static inline void printData(std::vector<char>& char_array){
        std::vector<char>::iterator it = char_array.begin();
#ifdef DEBUG_ON
        printf("[MRNEtStructureParser() PRINT METHOD !!...pid : %d arr:size : %d ] \n", getpid(), char_array.size());
        while(it != char_array.end()){
            printf("%c",*it);
            it++;
        }
        printf("\n");
#endif
    }

    size_t MRNetParser::readData() {
//        fprintf(stdout, "[MRNEtStructureParser() [#1] PID : %d ] \n", getpid()) ;
        int num = 0;

        if(streamEnd()){
#ifdef DEBUG_ON
            fprintf(stdout, "[MRNEtStructureParser() [#STREAM END RETURN] PID : %d ] \n", getpid()) ;
#endif
            return (size_t) 0;
        }
        //wait for next 'TOTAL_PACKET_SIZE' number of integers from inputqueue
        //remove from shared queue  and return
        std::vector<char> temp;
        while (true){
            //wait for signal from producer
#ifdef DEBUG_ON
        fprintf(stdout, "[MRNetQueueIterator #next().. PID : %d num : %d condition :  %p mutex : %p ]\n"
                , getpid(),num , inQueueSignal, inQueueMutex)  ;
#endif
            //todo handle this properly
            synchronizer->set_mutex_lock(inQueueMutex);
            synchronizer->set_cond_wait(inQueueSignal, inQueueMutex);

            //get iterator for incoming queue
            std::vector<DataPckt>::iterator it = inputQueue->begin();
            std::vector<DataPckt>::iterator del;
//            fprintf(stdout, "[MRNEtStructureParser() [#2] PID : %d ] \n", getpid()) ;
            bool is_last_pckt = false;
            for (; it != inputQueue->end();) {
                DataPckt curr_pckt = *it ;
                std::vector<char>& char_ar = curr_pckt.getData();
                //if already reached max limit
                if(num >= TOTAL_PACKET_SIZE){
                    break;
                }
                //if total after curr pakt exceeds max packet size limit
                else if(num + (int) char_ar.size() > TOTAL_PACKET_SIZE){
                    //keep packet on queue (don't delete) but remove (TOTAL_PACKET_SIZE - num) characters from packet
                    std::vector<char>::iterator in_packt_it = char_ar.begin();
                    for(int i = 0; i < (TOTAL_PACKET_SIZE - num); i++){
                        std::vector<char>::iterator del_ar = in_packt_it;
                        temp.push_back(*in_packt_it);
                        in_packt_it = char_ar.erase(del_ar);
                    }
                    num = TOTAL_PACKET_SIZE;
                }
                //TOTAL_PACKET_SIZE not reached
                else{
                    //remove packet from queue
                    temp.insert(temp.end(), char_ar.begin(), char_ar.end());
                    del = it;
                    it = inputQueue->erase(del);
                    num = num + char_ar.size();
                }

                is_last_pckt = curr_pckt.isFinal();
                if(is_last_pckt) break;
            }

#ifdef DEBUG_ON
        printf("[MRNetQueueIterator read from incoming queue.. PID : %d [TEMP values ]  --> ", getpid());
        for(std::vector<char>::iterator itr = temp.begin() ; itr != temp.end() ; itr++){
            printf(" : [[ %d ]] " ,*itr);
        }
        printf("[   [END TEMP values ] \n ");
#endif
            synchronizer->set_mutex_unlock(inQueueMutex);
            //indicate end of stream for the last packet
            if(is_last_pckt){
                stream_end = true ;
#ifdef DEBUG_ON
                fprintf(stdout, "[MRNEtStructureParser() [Final packt] PID : %d ] \n", getpid()) ;
#endif
                break;
            }else if(num >= TOTAL_PACKET_SIZE){
                break;
            }

        }
//        printData(temp);
        std::copy(temp.begin(), temp.end(), buf);
#ifdef DEBUG_ON
        fprintf(stdout, "Parser: Read wave %d ..\n",wave++);
        fprintf(stdout, "Parser: Read wave wait send Auc.. bytes read : %d \n", num);
        for(int j = 0 ; j < num ; j++){
            printf("%c",buf[j]);
        }
        printf("\n\n\n\n\n");
#endif

        //update total integers recieved
        total_ints += num;
//        fprintf(stdout, "[MRNEtStructureParser() [#END] PID : %d stream end? %b ] \n", getpid(), streamEnd()) ;
        return (size_t) num ;
    }

// Returns true if we've reached the end of the input stream
    bool MRNetParser::streamEnd() {
        return stream_end;
    }

// Returns true if we've encountered an error in input stream
    bool MRNetParser::streamError() {
        return stream_error;
    }



} // namespace sight
