#pragma once 

#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include "sight_common_internal.h"
//#include "sight_layout.h"
#include "AtomicSyncPrimitives.h"
#include "mrnet_integration.h"

using  namespace atomiccontrols;

namespace sight {

template<typename streamT>
class baseStructureParser : public common::structureParser {
  protected:
  // Holds the data just read from the data source
  char* buf;
  
  // Allocated size of buf
  size_t bufSize;
  
  // Number of bytes currently stored in buf
  size_t dataInBuf;
  
  // The current index of the read pointer within buf[]
  int bufIdx;
  
  // Reference to the data source
  streamT* stream;
  
  // Functions implemented by children of this class that specialize it to take input from various sources.
  
  // readData() reads as much data as is available from the data source into buf[], upto bufSize bytes 
  // and returns the amount of data actually read.
  virtual size_t readData()=0;
  
  // Returns true if we've reached the end of the input stream
  virtual bool streamEnd()=0;
  
  // Returns true if we've encountered an error in input stream
  virtual bool streamError()=0;

  public:  
  baseStructureParser(int bufSize=10000);
  baseStructureParser(streamT* stream, int bufSize=10000);
  void init(streamT* stream);
  
  protected:
  // The location within nextLoc() at which the last call to newLoc() stopped and the
  // next newLoc() call will resume.
  typedef enum {start,
                textRead,
                enterTagRead,
                exitTagRead,
                done} codeLoc;
  codeLoc loc;
  
  // The properties of the object tag that is being currently read
  properties tagProperties;
  
  
  public:
  // Reads more data from the data source, returning the type of the next tag read and the properties of 
  // the object it denotes.
  std::pair<properties::tagType, const properties*> next();
  
  protected:
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
  bool readProperty(std::string& name, std::string& val, char& termChar);
  
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
  bool readUntil(bool inTerm, const char* termChars, int numTermChars, 
                 char& termHit, std::string& result);
  
  // Advances the current reading position by 1 char, reading file contents into buf 
  // if the end of buf is reached. bufSize is the number of bytes in 
  // the buffer. When the function returns bufIdx is updated to hold the current 
  // position inside buf, immediately after the terminator char was encountered and 
  // bufSize is updated to hold the total number of characters currently in buf. 
  // Returns the read char. If there is a next character in the file, returns true.
  // Otherwise, returns false.
  bool nextChar();
  
  // Returns true if character c is in array termChars of size numTermChars and 
  // false otherwise.
  static bool isMember(char c, const char* termChars, int numTermChars);
};


class FILEStructureParser : public baseStructureParser<FILE> {
  // Records whether this object opened the file on its own (in which case it needs to close it)
  // or was given a ready FILE* stream
  bool openedFile;
  
  public:
  FILEStructureParser(std::string fName, int bufSize=10000);
  FILEStructureParser(FILE* f, int bufSize=10000);
  ~FILEStructureParser();
  
  protected:
  // Functions implemented by children of this class that specialize it to take input from various sources.
  
  // readData() reads as much data as is available from the data source into buf[], upto bufSize bytes 
  // and returns the amount of data actually read.
  size_t readData();
  
  // Returns true if we've reached the end of the input stream
  bool streamEnd();
  
  // Returns true if we've encountered an error in input stream
  bool streamError();
};


class MRNetParser : public baseStructureParser<FILE> {
    private:
        std::vector<DataPckt> *inputQueue;
        atomic_cond_t *inQueueSignal;

        AtomicSync *synchronizer;
        //this is needed to synchronize reads from inputQueue
        atomic_mutex_t *inQueueMutex;

        bool stream_end ;
        bool stream_error ;
    public:
        int total_ints;
        int wave;

    public:
        MRNetParser(std::vector<DataPckt>& input, atomic_cond_t* cond, atomic_mutex_t* inQueueMutex, AtomicSync* s)
        :total_ints(0), baseStructureParser<FILE>(TOTAL_PACKET_SIZE){
            inputQueue = &input;
            inQueueSignal = cond ;
            this->inQueueMutex = inQueueMutex;
            this->synchronizer = s;
            this->stream_end = false;
            this->stream_error = false;
            wave = 0;
        }

        ~MRNetParser();

    protected:
        // Functions implemented by children of this class that specialize it to take input from various sources.

        // readData() reads as much data as is available from the data source into buf[], upto bufSize bytes
        // and returns the amount of data actually read.
        size_t readData();

        // Returns true if we've reached the end of the input stream
        bool streamEnd();

        // Returns true if we've encountered an error in input stream
        bool streamError();

    };


} // namespace sight
