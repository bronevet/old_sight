#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <stdarg.h>
#include <assert.h>
#include "sight_common.h"
#include "utils.h"
#include "tools/callpath/include/Callpath.h"
#include "tools/callpath/include/CallpathRuntime.h"

namespace sight {
namespace structure{

// Records the information needed to call the application
extern bool saved_appExecInfo; // Indicates whether the application execution info has been saved
extern int saved_argc;
extern char** saved_argv;
extern char* saved_execFile;

// The name of the host that this application is currently executing on
extern char hostname[];

// The current user's user name
extern char username[10000];

// The unique ID of this process' output stream
extern int outputStreamID;

extern bool initializedDebug;

class dbgStream;

// Provides the output directory and run title as well as the arguments that are required to rerun the application
void SightInit(int argc, char** argv, std::string title="Debug Output", std::string workDir="dbg");
// Provides the output directory and run title as well but does not provide enough info to rerun the application
void SightInit(std::string title, std::string workDir);
// Low-level initialization of Sight that sets up some basics but does not allow users to use the dbg
// output stream to emit their debug output, meaning that other widgets cannot be used directly.
// Users that using this type of initialization must create their own dbgStreams and emit enter/exit
// tags to these streams directly without using any of the high-level APIs.
// The dummy argument is there to encourage users to read the above text instead of just calling this
// initializer directly.
void SightInit_LowLevel();

void SightInit_internal(properties* props);
// Creates a new dbgStream based on the given properties of a "sight" tag and returns a pointer to it.
// storeProps: if true, the given properties object is emitted into this dbgStream's output file
structure::dbgStream* createDbgStream(properties* props, bool storeProps);

// Empty implementation of Sight initialization, to be used when Sight is disabled via #define DISABLE_SIGHT
void NullSightInit(std::string title, std::string workDir);
void NullSightInit(int argc, char** argv, std::string title="Debug Output", std::string workDir="dbg");

class variantID {
  public:
  std::list<int> ID;
  
  variantID() {}
  variantID(int ID) { this->ID.push_back(ID); }
  variantID(const std::list<int>& ID) : ID(ID) {}
    
  variantID(std::string serialized) {
    // Deserialized the comma-separated list of integers into the ID std::list
    int i=0;
    size_t next = serialized.find(",", i);
    while(next<serialized.length()) {
      ID.push_back(strtol(serialized.substr(i, next-i).c_str(), NULL, 10));
      next = serialized.find(",", i);
    }  
  }
  
  std::string serialize() const {
    // Serialize the ID std::list into a comma-separated list of integers
    std::ostringstream s;
    for(std::list<int>::const_iterator i=ID.begin(); i!=ID.end(); i++) {
      if(i!=ID.begin()) s << ",";
      s << *i;
    }
    return s.str();
  }
  
  // Enter and exit a variant level
  // vSuffixID: ID that identifies this variant within the next level of variants in the heirarchy
  void enterVariant(int vSuffixID) { ID.push_back(vSuffixID); }
  void exitVariant() { assert(ID.size()>0); ID.pop_back(); }
  
  // Relational operators
  bool operator==(const variantID& that) const { return ID==that.ID; }
  bool operator!=(const variantID& that) const { return ID!=that.ID; }
  bool operator< (const variantID& that) const { return ID< that.ID; }
  bool operator<=(const variantID& that) const { return ID<=that.ID; }
  bool operator> (const variantID& that) const { return ID> that.ID; }
  bool operator>=(const variantID& that) const { return ID>=that.ID; }
}; // variantID

// Container for unique IDs that is sensitive to the variant within which the ID is defined
class streamID {
  public:
  int ID;
  variantID vID;
  streamID() {}
  streamID(int ID, const variantID& vID) : ID(ID), vID(vID) {}
  streamID(const streamID& that) : ID(that.ID), vID(that.vID) {}
    
  streamID& operator=(const streamID& that) { ID=that.ID; vID=that.vID; return *this; }
  
  // Relational operators
  bool operator==(const streamID& that) const { return ID==that.ID && vID==that.vID; }
  bool operator!=(const streamID& that) const { return ID!=that.ID || vID!=that.vID; }
  bool operator< (const streamID& that) const { return ID<that.ID || (ID==that.ID && vID<that.vID); }
  
  std::string str() const 
  { return txt()<<"[streamID: vID="<<vID.serialize()<<", ID="<<ID<<"]"; }
}; // class streamID

// Support for call paths
extern CallpathRuntime CPRuntime;
std::string cp2str(const Callpath& cp);

//Callpath str2cp(std::string str);

// Represents a unique location in the sight output
class location : printable {
  std::list<std::pair<int, std::list<int> > > l;
  
  public:
  location();
  ~location();
  
  void enterFileBlock();
  void exitFileBlock();

  void enterBlock();
  void exitBlock();

  void operator=(const location& that);
  bool operator==(const location& that) const;
  bool operator!=(const location& that) const;
  bool operator<(const location& that) const;

  //void print(std::ofstream& ofs) const;
  std::string str(std::string indent="") const;
};

// Container for a variant of locations that is sensitive to the variant the location is referring to
class streamLocation {
  variantID vID;
  location loc;
  public:
  streamLocation() {}
  streamLocation(variantID vID, location loc) : vID(vID), loc(loc) {}
  streamLocation(const streamLocation& that) : vID(that.vID), loc(that.loc) {}
    
  streamLocation& operator=(const streamLocation& that) { vID=that.vID; loc=that.loc; return *this; }
  
  // Relational operators
  bool operator==(const streamLocation& that) const { return loc==that.loc && vID==that.vID; }
  bool operator!=(const streamLocation& that) const { return !(*this == that); }
  bool operator< (const streamLocation& that) const { return loc<that.loc || (loc==that.loc && vID<that.vID); }
  
  void enterFileBlock() { loc.enterFileBlock(); }
  void exitFileBlock()  { loc.exitFileBlock(); }
  void enterBlock()     { loc.enterBlock(); }
  void exitBlock()      { loc.exitBlock(); }
  
  std::string str() const 
  { return txt()<<"[streamLocation: vID="<<vID.serialize()<<", loc="<<loc.str()<<"]"; }
}; // class streamLocation
  
/* #################### DESIGN NOTES ####################
Clocks are a mechanism to associate a timestamps with each emitted tag. These timestamps must 
increase monotonically within a single process' log (they may be equal but never decrease) and
must be comparable across different logs, establishing a partial order between events between 
them. A simple example of a clock is real system time, allowing us to display tags in their
chronological order when we merge multiple logs. Another example is MPI where a clock is a vector
mapping each FIFO channel to the number of messages sent/received on it.

As a log is created users may create one or more clocks. Each clock calls sightObj::addClock()
to register itself with Sight. Whenever a new tag's entry is written out within sightObj::sightObj()
it calls the setProperties methods of all the currently registered clocks, which add descriptions
of themselves to the provided properties object. Clocks unregister themselves using 
sightObj::remClock().

When reading logs Sight need to decode the tags that encode the different clocks. Each clock creates
a mapping between the names of clocks and functions that map their encodings to clock objects. This
is done by creating class that inherits from clockHandlerInstantiator. Each clock widget must create
a static instance of this class and in the constructor of this class must map the name of its clock
to a function called when a sightObj is entered and one called when it is exited. The entry function
takes as input a properties::iterator and returns a reference to a newly-allocated clock object denoted
by the encoding. The exit function deallocated the created object.
Examples:
  (*clockExitHandlers )["clockName"] = &clockExitHandler;
  (*clockEnterHandlers)["clockName"] = &clockTraceStream;

############################################################ */

// Maps the names of various clocks that may be read to the functions
// that instantiate the objects they encode.
// An clock entry handler is called when we read a sightObject's clocks within its tag. It 
//   takes as input the properties of the object to be laid out and returns a pointer 
//   to a sightClock object the tag encodes. NULL may not be returned.
// An clock exit handler is called when the object's exit tag is encountered and takes as 
//   input a pointer to the object so that it can be deallocated.
typedef void* (*clockEnterHandler)(properties::iterator props);
typedef void (*clockExitHandler)(void*);
class clockHandlerInstantiator {
  public:
  static std::map<std::string, clockEnterHandler>* clockEnterHandlers;
  static std::map<std::string, clockExitHandler>*  clockExitHandlers;

  clockHandlerInstantiator() {
    // Initialize the handlers mappings, using environment variables to make sure that
    // only the first instance of this clockHandlerInstantiator creates these objects.
    if(!getenv("SIGHT_CLOCK_HANDLERS_INSTANTIATED")) {
      clockEnterHandlers = new std::map<std::string, clockEnterHandler>();
      clockExitHandlers  = new std::map<std::string, clockExitHandler>();
      setenv("SIGHT_CLOCK_HANDLERS_INSTANTIATED", "1", 1);
    }
  }
};

class sightClock
{
  public:
  virtual properties* setProperties(properties* props)=0;
  // Returns true if the clock has been modified since the time of its registration or the last time modified() was called.
  virtual bool modified()=0;
};


/*
sightObj is the base class for all entities in Sight that correspond to entities in the
structured log. sightObj takes care of all the base details of serializing objects into 
the log and has two major functionalities, one at construction-time and another at 
destruction-time.

Construction:
-------------
When an object is constructed in most cases widgets need to serialize the object into an
entry tag that is specific to this type of object. The data of the object is maintained
inside a properties object that is created by the specific object and then passed to the
sightObj constructor, where it is emitted into the structured log. An object's data is
encoded as a key-value map, which is serialized into the log. However, since Sight
is highly modular, a given object may represent several levels of inheritance, each of which
may have been written by a different group of people. This means that the key-value maps
provided by each object may have keys that accidentally overlap, which may cause complex
bugs due to information loss and/or corruption. As such, properties objects allow different
classes to record separate key-value maps and use the properties::add() method to register 
them under different unique strings. Since this simply requires each class to use a different
string label, this makes name collisions significantly less likely and in general, fairly 
easy to debug.

By default the entry tag of a class is emitted during its constructor and the exit tag is 
emitted during the destructor. If a derived constructor passes NULL as the properties object
to the sightObj constructor, no tags are emitted. If the properties object is non-NULL, but
the isTag flag is set to true then both the entry and exit tag are emitted in the sightObj
constructor and nothing is emitted during the destructor.

The properties object that encodes a given object, including all of the classes in its 
inheritance hierarchy, is created when the object is constructed and passed on to the 
sightObj constructor for serialization into the structured log. The mechanism for this is as
follows. Each object's constructor must take as an argument a pointer to a properties object, 
which is set to NULL by default. Each class must when implement a static method 
(called setProperties() by convention) that 
  - takes this pointer as an argument
  - if the pointer is set to NULL, allocates a fresh properties object
  - creates a key-value map of type map<string, string> into which it records the object's 
    properties from the current class
  - uses properties::add() to include this map into  properties object under its own unique name
  - returns this properties object
This method is called before invoking the constructor of the parent class and its return
value should be passed to the properties argument of the parent class' constructor. 

This setup ensures that:
  - If an object of a given class is used directly by the user, the user is free to ignore this
    scheme and does not specify a properties object.
  - The object's constructor knows whether it corresponds to the most derived class in the object
    or not because the properties pointer is set to NULL if it the most derived and not NULL
	if it is more deeply derived.
  - Each constructor gets to add its own, independent key-value map.
  - The order in which a properties object is constructed directly mirrors the construction of
    the objects themselves and benefits from the compiler's type checking of constructor calls.
  - The serialization task is done once inside the sightObj constructor
  
The major weakness of this setup is that the setProperties() function must be static, which
may require extra copying that would not be necessary if the creation of the properties object
were done after the object were fully constructed. However, if we did it this way, it would be
difficult to make sure that each object always called the properties initialization method 
of the correct parent class.

Destruction:
------------
A key challenge in the design of Sight is that it must produce valid output even if the application
crashes mid-way through its execution. This includes crashes due to application bugs, as well as
crashes induced by Sight tools, such as fault injection. Since the destructor of most sightObjs emit
the object's exit tag and in many cases (e.g. modularApps and modules) object destructors perform a 
lot of processing work, it is necessary to call the destructors of all currently active objects
in the reverse order of their creation (order ensures that enter/exit tags are nested hierarchically).
It is not possible to call the destructors of objects directly because
  - Many of them are allocated on the stack, making it impossible to use delete on them
  - To call the destructor className::~className() of a given object directly we need to know the name
    of the most derived class of this object, which we do not in general.
As such, objects that derive from sightObj are required to implement a virtual destroy() method that 
calls the object's destructor directly. Since the method is virtual, if the destroy() method of any given
sightObj is called, C++ ensures that the most derived instance of this method is called, which then 
invokes the destructor of the most-derived class. Destruction then proceeds as normal.

In some cases the destructors of some sightObj derived classes need to know when the object has 
finished destructing (e.g. in fault injection we disable injection when entering a constructor/
destructor and re-enable it when exiting it). To achieve this it is necessary to implement a function
of type destructCallback and inside the destructor of a derived class register a pointer to this function
using method sightObj::registerDestructNotifier(). This method will be called after the destructor
of the sightObj() completes. The same callback function may be registered multiple times and is called
separatedly for each registration. Callbacks are called in the same order as they are registered.
*/

// Base class of all sight objects that provides some common functionality
class sightObj {
  public:
  properties* props;
  
  protected:
  // Records whether we should emit the exit tag in the destructor (true) or whether this was already taken
  // care of in the constructor (false)
  bool emitExitTag;
  
  // Records whether we should remove this object from the sight object stack
  bool removeFromStack;
  
  // Records whether we've begun the process of destroying all objects. This is to differentiate`
  // the case of Sight doing the destruction and of the runtime system destroying all objects
  // at process termination
  static bool SightDestruction;
  
  // Records whether we've started processing static destructors. In this case
  // the sight object stack may not be valid anymore and thus should not be accessed.
  static bool stackMayBeInvalidFlag;
  // Records that the stack may not be valid
  void stackMayBeInvalid() { stackMayBeInvalidFlag=true; }
  
  // Flag that records whether this object has already been destroyed. This must be tracked explicitly
  // because sightObjs can be be destroyed by calling their destroy() method in addition to calling their
  // destructor.
  bool destroyed;

  // The stack of sightObjs that are currently in scope. We declare it as a static inside this function
  // to make it possible to ensure that the global soStack is constructed before it is used and therefore
  // destructed after all of its user objects are destructed.
  // There is a separate stack for each outgoing stream.
  //static std::map<dbgStream*, std::list<sightObj*> > staticSoStack;
  // The stack of sightObjs that are currently in scope
  //static std::list<sightObj*> soStack;
    
  // The dbgStream on which this sightObj was written. It is possible to concurrently 
  // maintain multiple such streams and we need to be sure that we emit exit tags
  // on the same streams as their corresponding entry tags.
  dbgStream* outStream;
  public:
    
  /*static std::list<sightObj*>& soStack(dbgStream* outStream);
  static std::map<dbgStream*, std::list<sightObj*> >& soStackAllStreams();*/
  
  // If outStream is not specified it defaults to structure::dbg. This default is
  // is implemented in sight_structure.C because structure::dbg must be declared 
  // below the declaration of the sighObj class.
  sightObj(dbgStream* outStream=NULL);
  // isTag - if true, we emit a single enter/exit tag combo in the constructor and nothing in the destructor
  sightObj(properties* props, bool isTag=false, dbgStream* outStream=NULL);
  void init(properties* props, bool isTag=false);
  ~sightObj();

  // Emits this sightObj's exit tag to the outgoing stream.
  // If popStack==true, remove this sightObj from the stack of objects on its dbgStream.
  // It will be set to false inside the dbgStream constructor since 1. its redundant and 
  // 2. if the dbgStream is the static one, the stack may have been destructed before the dbgStream's destructor
  void exitTag(bool popStack=true);
  
  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  private:
  // The of clocks currently being used, mapping the name of each clock class to the set of active 
  // clock objects of this class.
  static std::map<std::string, std::set<sightClock*> > clocks;
  
  public:
  const properties& getProps() const { return *props; }

  // Destroys all the currently live sightObjs on the stack
  static void destroyAll();
    
  // Returns whether this object is active or not
  bool isActive() const;
    
  // Registers a new clock with sightObj
  static void addClock(std::string clockName, sightClock* c);
  
  // Updates the registration of the given clock to refer to the given sightClock object
  static void updClock(std::string clockName, sightClock* c);
  
  // Unregisters the clock with the given name
  static void remClock(std::string clockName);
  
  // Returns whether the given clock object is currently registered
  static bool isActiveClock(std::string clockName, sightClock* c);
  
  // Destruction notification support
  
  // Function to be called when a given sightObj has finished destructing. A pointer to the destructed
  // sighObj (still valid at the time of the call) is provided as an argument.
  typedef void (*destructNotifier)(sightObj* obj);
  private:
  std::list<destructNotifier> sightObjDestructNotifiers;
  public:
  // Registers a given callback function to be called when the destruction of this object completes.
  void registerDestructNotifier(destructNotifier notifier);
}; // class sightObj

/*
When merging logs Sight uses Merger objects that take tags of a given type and either merge them into a 
single tag that combines the properties of the individuals or decide that they cannot be merged and provide
a key that orders them in the outgoing stream. When the merging infrastructure reaches a tag it needs to
find the Merger object responsible for this tag. Sine the set of tags that may appear in the log will grow
over time, Sight does not explicitly encode the mappping of tag names to Mergers in any one location.
Instead, we create a central repository where widgets can register the mapping between their own tags
and the Mergers associated with them. This is done by creating class that inherits from MergerHandlerInstantiator. 
Each widget must create a static instance of this class and in the constructor of this class must map the names
of any tags it may emit to 
  - A function called when a tag is entered or exited, and 
  - A function called to get the unique key of a given tag to make it possible to check if tags on different
    streams are mergeable (they are if their keys are equal) or if not, the order in which they should be
    emitted to the outgoing stream (the order among the keys)
Examples:
  (*MergeEnterHandlers)["widgetName"] = &MergerEnterFunction;
  (*MergeKeyHandler   )["widgetName"] = &MergerKeyFunction;
*/

class streamRecord;
class Merger;

/* 
 * During merging Sight must decide whether two tags of the same type should be 
 * merged or not and if not, which tag should be placed in the outgoing stream 
 * first. This is communicated by the MergeInfo object. For each tag type, widgets
 * are required to implement a method that initializes an object of type MergeInfo.
 * This method, which is called static Merger::mergeKey(type, tag, inStreamRecords, 
 * info) by convention must be registered with the MergeHandlerInstantiator. When 
 * the mergeKey() method of the most derived Merger class is called, it first calls 
 * its parent class' mergeKey() method as ParentMerger::mergeKey(type, tag.next(), inStreamRecords, 
 * info) so that the parent class can add its own keys adds then one or more 
 * identifying strings to the info object. Once the key is generated 
 * the merger algorithm ensures that only tags with identical keys can get merged 
 * and that they are merged in the order defined by the keys. The comparison is 
 * performed in string insertion order, meaning that strings inserted by more deeply 
 * nested classes within an object have a higher priority in the comparison used 
 * first for the comparison.
 * 
 * In some cases we know that a given tag exists on one incoming stream iff it 
 * exists on all other incoming streams. Sight allows widgets to specify this by 
 * calling info.isUniversal(). Tags for which this has been specified will only 
 * be merged only once we reach a tag with an identical key on every other 
 * incoming stream.
 */
class MergeInfo {
  public:
  typedef std::list<std::string> mergeKey;
  
  private:
  mergeKey key;
  bool universal;

  public:  
  MergeInfo() : universal(false) {}
  
  void add(std::string subKey) { key.push_back(subKey); }
  void setUniversal(bool newVal=true) { universal = universal || newVal; }
  bool getUniversal() const { return universal; }
  const mergeKey& getKey() const { return key; }

  std::string str() const {
    std::ostringstream s;
    s << "[MergeInfo: universal="<<universal<<", key=[";
    for(std::list<std::string>::const_iterator k=key.begin(); k!=key.end(); k++) {
      if(k!=key.begin()) s << ", ";
      s << *k;
    }
    s << "]]";
    return s.str();
  }
};

typedef sight::structure::Merger* (*MergeHandler)(
                                       const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                                       std::map<std::string, streamRecord*>& outStreamRecords,
                                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                       properties* props);
typedef void (*MergeKeyHandler)(properties::tagType type,
                                properties::iterator tag, 
                                std::map<std::string, streamRecord*>& inStreamRecords,
                                MergeInfo& info);
// Returns a mapping from the names of objects for which records are kept by a given code module to the freshly-allocated 
// streamRecord objects that keep their records. The records are specialized with the given stream ID.
typedef std::map<std::string, streamRecord*> (*GetMergeStreamRecord)(int streamID);

class MergeHandlerInstantiator : public sight::common::LoadTimeRegistry {
  public:
  static std::map<std::string, MergeHandler>*    MergeHandlers;
  static std::map<std::string, MergeKeyHandler>* MergeKeyHandlers;
  static std::set<GetMergeStreamRecord>*         MergeGetStreamRecords;

  MergeHandlerInstantiator();
  
  // Called exactly once for each class that derives from LoadTimeRegistry to initialize its static data structures.
  static void init();
  
  // Returns a mapping from the names of objects for which records are kept within this MergeHandlerInstantiator
  // object to the freshly-allocated streamRecord objects that keep their records. The records are specialized 
  // with the given stream ID.
  static std::map<std::string, streamRecord*> GetAllMergeStreamRecords(int streamID);
  
  static std::string str();
};

class SightMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  SightMergeHandlerInstantiator();
};
extern SightMergeHandlerInstantiator SightMergeHandlerInstance;
std::map<std::string, streamRecord*> SightGetMergeStreamRecord(int streamID);

// Base class for objects that maintain information for each incoming or outgoing stream during merging
class streamRecord : public printable {
  protected:
  // Uniquely identifies the stream variant this record corresponds to.
  variantID vID;
  
  // Records the maximum ID ever generated on a given outgoing stream
  int maxID;
  
  // Maps the anchorIDs within an incoming stream to the anchorIDs on its corresponding outgoing stream
  std::map<streamID, streamID> in2outIDs;
    
  // The name of the object type this stream corresponds to.
  std::string objName;
  
  public:
  
  streamRecord(int vID,              std::string objName) : vID(vID), maxID(0), objName(objName) {}  
  streamRecord(const variantID& vID, std::string objName) : vID(vID), maxID(0), objName(objName) {}
  // vSuffixID: ID that identifies this variant within the next level of variants in the heirarchy
  streamRecord(const streamRecord& that, int vSuffixID) : vID(that.vID), maxID(that.maxID), in2outIDs(that.in2outIDs), objName(that.objName) {
    vID.enterVariant(vSuffixID);
  }
  
  public:
  const variantID& getVariantID() const { return vID; }
  
  // Return the tagType (enter or exit) that is common to all incoming streams in tags, or 
  // unknownTag if they're not consistent
  static properties::tagType getTagType(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  virtual streamRecord* copy(int vSuffixID)
  { return new streamRecord(*this, vSuffixID); }
  
  public:
  // Merge the IDs of the next ID field (named IDName, e.g. "anchorID" or "blockID") of the current tag maintained 
  // within object named objName (e.g. "anchor" or "block") of each the incoming stream into a single ID in the 
  // outgoing stream, updating each incoming stream's mappings from its IDs to the outgoing stream's 
  // IDs. If a given incoming stream anchorID has already been assigned to a different outgoing stream anchorID, yell.
  // For example, to merge the anchorIDs of anchors, blocks or any other tags that maintain anchorIDs, we need to set 
  //    objName="anchor" and IDName="anchorID", while to merge the blockIDs of Blocks we set objName="block", 
  //    IDName="blockID". Thus, anchor objects maintain the anchorID mappings for blocks as well as others, 
  //    while blocks maintain blockID mappings for themselves. This means that each object type (e.g. anchor, block, 
  //    trace) can only maintain one type of ID in its own streamRecord.
  // The merged ID on the outgoing stream can be explicitly specified via the mergedID argument. If this argument
  //    is not provided, a fresh ID is generated automatically.
  // Returns the merged ID in the outgoing stream.
  static int mergeIDs(std::string objName, std::string IDName, 
                      std::map<std::string, std::string>& pMap, 
                      const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                      std::map<std::string, streamRecord*>& outStreamRecords,
                      std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                      int mergedID=-1);
  
  // Variant of mergeIDs where it is assumed that all the IDs in the incoming stream map to the same ID in the outgoing
  // stream and an error is thrown if this is not the case
  static int sameID(std::string objName, std::string IDName, 
                    std::map<std::string, std::string>& pMap, 
                    const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                    std::map<std::string, streamRecord*>& outStreamRecords,
                    std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  protected:
  // Variant of mergeIDs where it is assumed that all the IDs in the incoming stream map to the same ID in the outgoing
  // stream and an error is thrown if this is not the case. Returns a pair containing a flag that indicates whether
  // an ID on the outgoing stream is known and if so, the ID itself
  static std::pair<bool, streamID> sameID_ex(
                              std::string objName, std::string IDName, 
                              std::map<std::string, std::string>& pMap, 
                              const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                              std::map<std::string, streamRecord*>& outStreamRecords,
                              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);
  
  public:
  // Given a streamID in the current incoming stream return its streamID in the outgoing stream, yelling if it is missing.
  streamID in2outID(streamID inSID) const;
  
  // Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  virtual void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
  
  std::string str(std::string indent="") const;
}; // streamRecord

// Base class for objects used to merge multiple tags into one
class Merger {
  protected:
  properties* props;
  
  // Indicates whether a tag should be emitted for this object or not (we may want to delay tag emission until
  // more information is observed)
  bool emitTagFlag;
  
  public:
  // Sometimes to merge the entry/exit of one tag we need to generate many more additional tags. For example,
  // on exit of a graph we may wish to emit all of its edges. These lists maintain all of the additional tags
  // that should be emitted before/after the primary tag.
  std::list<std::pair<properties::tagType, properties> > moreTagsBefore;
  std::list<std::pair<properties::tagType, properties> > moreTagsAfter;
  
  public:
  Merger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
         std::map<std::string, streamRecord*>& outStreamRecords,
         std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
         properties* props);
  
  const properties& getProps() const { return *props; }
  
  protected:
  // Called by derived objects to indicate that this tag should not be emitted
  void dontEmit() { emitTagFlag=false; }
  
  // Called by merging algorithms to check whether a tag should be emitted
  public:
  bool emitTag() const { return emitTagFlag; }
  
  // Called to reset the properties pointer to NULL. The typical way in which Mergers are created is by inheritance
  // where each Merger's constructor sets its properties and then calls its base class constructor, all the way
  // up to Merger. However, in some cases such as clocks, a given object may have multiple additional sets of 
  // clock properties that are not reflected in its inheritance hierarchy and may vary from run to run. To merge
  // such object we need to use Mergers for each clock but the properties object given to these clock mergers comes
  // from their host Merger object, meaning that they can add properties to it in their constructor but may not 
  // deallocate it in their destructor. This method allows the host Merger to create Mergers for any clocks 
  // associated with the object, pass them a properties object and then prevent the clocks' Mergers from deallocating 
  // it by reseting their reference to this object.
  // Returns true if the value of the props pointer was changed as a result and false otherwise.
  bool resetProps() {
    bool modified = props!=NULL;
    props=NULL;
    return modified;
  }
  
  ~Merger();
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
    
  // Given a vector of tag properties, returns whether the given key exists in all tags
  bool keyExists(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags, std::string key);
  
  // Given a vector of tag properties, returns the vector of values assigned to the given key within the given tag
  static std::vector<std::string> getValues(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags, 
                                            std::string key);
  
  // Given a vector of tag properties, merges their values and returns the merged string
  static std::string getMergedValue(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags, 
                                    std::string key);
  
  // Given a vector of tag properties that must be the same, returns their common value
  static std::string getSameValue(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags, 
                                    std::string key);
  
  // Returns whether all the elements in the given set are equal to each others
  template<class T>
  static bool allSame(const std::vector<T>& s) {
    for(typename std::vector<T>::const_iterator i=s.begin(); i!=s.end(); i++)
      if(i!=s.begin() && *i!=*s.begin()) return false;
    return true;
  }
  
  // Returns the string representation of the given vector 
  template<class T>
  std::string str(const std::vector<T>& v) {
    std::ostringstream s;
    s << "[";
    for(typename std::vector<T>::const_iterator i=v.begin(); i!=v.end(); i++) {
      if(i!=v.begin()) s << ", ";
      s << *i;
    }
    s << "]";
    return s.str();
  }
  
  // Converts the given set of strings to the corresponding set of integral numbers
  static std::vector<long> str2int(const std::vector<std::string>& strSet);
  
  static long vMax(const std::vector<long>& intSet);
  static long vMin(const std::vector<long>& intSet);
  static long vSum(const std::vector<long>& intSet);
  static long vAvg(const std::vector<long>& intSet);
  
  // Converts the given set of strings to the corresponding set of floating point numbers
  static std::vector<double> str2float(const std::vector<std::string>& strSet);
  
  static double vMax(const std::vector<double>& floatSet);
  static double vMin(const std::vector<double>& floatSet);
  static double vSum(const std::vector<double>& floatSet);
  static double vAvg(const std::vector<double>& floatSet);
    
  // Given a vector of tag property iterators, returns the list of names of all the object types they refer to
  static std::vector<std::string> getNames(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags);
  
  // Given a vector of tag property iterators that must be the same, returns their common name
  static std::string getSameName(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags);
    
  // Advance the iterators in the given tags vector, returning a reference to the vector
  static std::vector<std::pair<properties::tagType, properties::iterator> >
              advance(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags);
                
  // Returns true if any of the iterators in tags have reached their end. This must be the same for all iterators.
  // Either all have reached their end or none have.
  static bool isIterEnd(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags);
}; // class Merger

} // namespace structure
} // namespace sight

#include "attributes/attributes_structure.h"

namespace sight {
namespace structure {

class TextMerger : public Merger {
  public:
  TextMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
             std::map<std::string, streamRecord*>& outStreamRecords,
             std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
             properties* props=NULL);

  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new TextMerger(tags, outStreamRecords, inStreamRecords, props); }

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class TextMerger

// Uniquely identifies a location with the debug information, including the file and region hierarchy
// Anchors can be created in two ways:
// - When their host output locations are reached. In this case the anchor's full file and region location is available
// - Before their host locations are reached (when forward links are created). In this case the anchor's location info
//   is not available at anchor creation time and must be initialized subsequently. We'll need to generate output for
//   links to anchors either before (forward links) or after (backward links) the anchor's output location has been 
//   reached. For backward links we can generate code easily since the anchor's target is known at output generation time.
//   To support forward links we do the following:
//   - When the output location is reached, we emit a piece of code to the script file that register's the location
//     in a data sctructure. There is one anchor script file for the entire output (not one per sub-file) to make it
//     easier to deal with anchors that cross files.
//   - When forward links are created we generate code that looks the link's destination in the data structure. If
//     the destination has already been loaded in the data structure, we go directly there. If the destination has not
//     been loaded, this means that the application generating the debug log has crashed and thus, we shouldn't follow
//     the link.
class anchor
{
  protected:
  static int maxAnchorID;
  int anchorID;
  
  // Maps all anchor IDs to their locations, if known. When we establish forward links we create
  // anchors that are initially not connected to a location (the target location has not yet been reached). Thus,
  // when we reach a given location we may have multiple anchors with multiple IDs all for a single location.
  // This map maintains the canonical anchor ID for each location. Other anchors are resynched to used this ID 
  // whenever they are copied. This means that data structures that index based on anchors may need to be 
  // reconstructed after we're sure that their targets have been reached to force all anchors to use their canonical IDs.
  static std::map<int, location> anchorLocs;

  // Associates each anchor with a unique anchor ID. Useful for connecting multiple anchors that were created
  // independently but then ended up referring to the same location. We'll record the ID of the first one to reach
  // this location on locAnchorIDs and the others will be able to adjust themselves by adopting this ID.
  static std::map<location, int> locAnchorIDs;

  // Itentifies this anchor's location in the file and region hierarchies
  location loc;

  // Flag that indicates whether we've already reached this anchor's location in the output
  bool located;
  
  public:
  anchor();
  anchor(const anchor& that);
  anchor(int anchorID);

  ~anchor();

  // Records that this anchor's location is the current spot in the output
  void reachedLocation();

  // Updates this anchor to use the canonical ID of its location, if one has been established
  void update();
  
  int getID() const { return anchorID; }

  bool isLocated() const { return located; }
  
  public:
  static anchor noAnchor;
  
  void operator=(const anchor& that);
  
  bool operator==(const anchor& that) const;
  bool operator!=(const anchor& that) const;
  
  bool operator<(const anchor& that) const;
  
  // Emits to the output an html tag that denotes a link to this anchor. Embeds the given text in the link.
  void link(std::string text) const;
    
  // Emits to the output an html tag that denotes a link to this anchor, using the default link image, which is followed by the given text.
  void linkImg(std::string text="") const;
  
  std::string str(std::string indent="") const;
};

class AnchorStreamRecord: public streamRecord {
  friend class BlockMerger;
  friend class streamAnchor;
    
  // Maps all anchor IDs to their locations, if known. When we establish forward links we create
  // anchors that are initially not connected to a location (the target location has not yet been reached). Thus,
  // when we reach a given location we may have multiple anchors with multiple IDs all for a single location.
  // This map maintains the canonical anchor ID for each location. Other anchors are resynched to used this ID 
  // whenever they are copied. This means that data structures that index based on anchors may need to be 
  // reconstructed after we're sure that their targets have been reached to force all anchors to use their canonical IDs.
  std::map<streamID, streamLocation> anchorLocs;

  // Associates each anchor with a unique anchor ID. Useful for connecting multiple anchors that were created
  // independently but then ended up referring to the same location. We'll record the ID of the first one to reach
  // this location on locAnchorIDs and the others will be able to adjust themselves by adopting this ID.
  std::map<streamLocation, streamID> locAnchorIDs;
  
  public:
  AnchorStreamRecord(variantID vID) : streamRecord(vID, "anchor") { /*maxAnchorID=0;*/ }
  // vSuffixID: ID that identifies this variant within the next level of variants in the heirarchy
  AnchorStreamRecord(const AnchorStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
  
  // Marge the IDs of the next anchorID field of the current tag (named objName) of each the incoming stream into a 
  // single ID in the outgoing stream, updating each incoming stream's mappings from its IDs to the outgoing stream's 
  // IDs. If a given incoming stream anchorID has already been assigned to a different outgoing stream anchorID, yell.
  /*static void mergeIDs(std::string objName, std::map<std::string, std::string>& pMap, 
                       const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);

  // Given an anchor ID on the current incoming stream return its ID in the outgoing stream, yelling if it is missing.
  streamID in2outAnchorID(streamID inSID) const;*/
  
  std::string str(std::string indent="") const;
}; // class AnchorStreamRecord

class LinkMerger : public Merger {
  public:
  LinkMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
               std::map<std::string, streamRecord*>& outStreamRecords,
               std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
               properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new LinkMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) { 
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class LinkMerger

class dbgStreamStreamRecord;

// Analogue of anchors that are explicitly linked to some AnchorStreamRecord of an incoming stream
class streamAnchor: public printable
{
  protected:
  streamID ID;
  
  // Identifies this anchor's location in the file and region hierarchies
  streamLocation loc;

  // Flag that indicates whether we've already reached this anchor's location in the output
  bool located;
  
  // Refers to the record of the incoming stream this anchor is associated with
  //std::map<std::string, streamRecord*>& myStream;
  
  // Refers to the anchor and dbgStream records of the incoming stream this anchor is associated with
  dbgStreamStreamRecord* dbgStreamR;
  // The specific record within myStream associated with anchors
  AnchorStreamRecord* anchorR;
  
  public:
  streamAnchor(std::map<std::string, streamRecord*>& myStream);
  streamAnchor(const streamAnchor& that);
  streamAnchor(int anchorID, std::map<std::string, streamRecord*>& myStream);
  streamAnchor(streamID ID, std::map<std::string, streamRecord*>& myStream);

  ~streamAnchor();

  // Records that this anchor's location is the current spot in the output
  void reachedLocation();

  // Updates this anchor to use the canonical ID of its location, if one has been established
  void update();
  
  streamID getID() const { return ID; }

  bool isLocated() const { return located; }
  
  public:
  void operator=(const streamAnchor& that);
  
  bool operator==(const streamAnchor& that) const;
  bool operator!=(const streamAnchor& that) const;
  
  bool operator<(const streamAnchor& that) const;
  
  std::string str(std::string indent="") const;
}; // class streamAnchor

// A block out debug output, which may be filled by various visual elements
class block : public sightObj
{
  std::string label;
  // The unique ID of this block as well as the static global counter of the maximum ID assigned to any block.
  // Unlike the layout layer, these blockIDs are integers since all we need from them is uniqueness and not
  // any structural information.
  int blockID;
  // maxBlockID also counts the number of times that the block constructor was called, which makes it possible
  // to set conditional breakpoints to run a debugger to the entry into a specific block in the debug output.
  static int maxBlockID;
  
  // The anchor that denotes the starting point of this scope
  anchor startA;

  protected:
  
  public:
  // Initializes this block with the given label
  block(std::string label="", properties* props=NULL);
    
  // Initializes this block with the given label.
  // Includes one or more incoming anchors thas should now be connected to this block.
  block(std::string label, anchor& pointsTo, properties* props=NULL);
  block(std::string label, std::set<anchor>& pointsTo, properties* props=NULL);
    
  // Sets the properties of this object
  static properties* setProperties(std::string label,                             properties* props);
  static properties* setProperties(std::string label, anchor& pointsTo,           properties* props);
  static properties* setProperties(std::string label, std::set<anchor>& pointsTo, properties* props);
 
  ~block();
  
  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  // Increments blockID. This function serves as the one location that we can use to target conditional
  // breakpoints that aim to stop when the block count is a specific number
  int advanceBlockID();
  
  std::string getLabel() const { return label; }
  int getBlockID() const { return blockID; }
  
  protected:
  anchor& getAnchorRef();
  
  public:
  anchor getAnchor() const;
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  virtual bool subBlockEnterNotify(block* subBlock) { return true; }
  virtual bool subBlockExitNotify (block* subBlock) { return true; }
}; // class block

class BlockMerger : public Merger {
  public:
  BlockMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class BlockMerger

class BlockStreamRecord: public streamRecord {
  friend class BlockMerger;
  /* // Records the maximum blockID ever generated on a given outgoing stream
  int maxBlockID;
  
  // Maps the blockIDs within an incoming stream to the blockIDs on its corresponding outgoing stream
  std::map<streamID, streamID> in2outBlockIDs;*/
  
  public:  
  BlockStreamRecord(int vID)              : streamRecord(vID, "block") { /*maxBlockID=0;*/ }
  BlockStreamRecord(const variantID& vID) : streamRecord(vID, "block") { /*maxBlockID=0;*/ }
  // vSuffixID: ID that identifies this variant within the next level of variants in the heirarchy
  BlockStreamRecord(const BlockStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  //void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
  
  // Marge the IDs of the next block (stored in tags) along all the incoming streams into a single ID in the outgoing stream,
  // updating each incoming stream's mappings from its IDs to the outgoing stream's IDs
  /*static void mergeIDs(std::map<std::string, std::string>& pMap, 
                       std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                       std::map<std::string, streamRecord*>& outStreamRecords,
                       std::vector<std::map<std::string, streamRecord*> >& inStreamRecords);*/

  std::string str(std::string indent="") const;
}; // class BlockStreamRecord

// Adapted from http://wordaligned.org/articles/cpp-streambufs
// A extension of stream that corresponds to a single file produced by sight
class dbgBuf: public std::streambuf
{
  friend class dbgStream;
  // True immediately after a new line
  bool synched;
  // True if the owner dbgStream is writing text and false if the user is
  bool ownerAccess;
  std::streambuf* baseBuf;

  // The number of observed '<' characters that have not yet been balanced out by '>' characters.
  //      numOpenAngles = 1 means that we're inside an HTML tag
  //      numOpenAngles > 1 implies an error or text inside a comment
  int numOpenAngles;
  
  public:
  int getNumOpenAngles() const { return numOpenAngles; }

  public:
        
  virtual ~dbgBuf() {};
  // Construct a streambuf which tees output to both input
  // streambufs.
  dbgBuf();
  dbgBuf(std::streambuf* baseBuf);
  void init(std::streambuf* baseBuf);
  
private:
  // This dbgBuf has no buffer. So every character "overflows"
  // and can be put directly into the teed buffers.
  virtual int overflow(int c);
  
  // Prints the given string to the stream buffer
  int printString(std::string s);

  virtual std::streamsize xsputn(const char * s, std::streamsize n);

  // Sync buffer.
  virtual int sync();
        
  // Switch between the owner class and user code writing text
protected:
  void userAccessing();
  void ownerAccessing();

  // b - The block that was just entered
  // isAttrSubBlock - Records whether b is a sub-block that spans between creation points of attribute values and 
  //   terminal points of blocks. For such blocks we record that they exist to make sure they get unique names but
  //   don't add any additional indentation.
  void enterBlock(block* b, bool isAttrSubBlock);
  
  // Called when a block is exited. Returns the block that was exited.
  // isAttrSubBlock - Records whether b is a sub-block that spans between creation points of attribute values and 
  //   terminal points of blocks. For such blocks we record that they exist to make sure they get unique names but
  //   don't add any additional indentation.
  block* exitBlock(bool isAttrSubBlock);
  
  // Returns the depth of enterBlock calls that have not yet been matched by exitBlock calls
  int blockDepth();
}; // class dbgBuf


// Stream that uses dbgBuf
class dbgStream : public common::dbgStream, public sightObj
{
public:
  dbgBuf defaultFileBuf;
  // Stream to the file where the structure will be written
  std::ofstream *dbgFile;
  // Buffer for the above stream
  dbgBuf* buf;
  // Holds any text printed out before the dbgStream is fully initialized
  std::ostringstream preInitStream;
  
  // The total number of images in the output file
  int numImages;
 
  // The current location within the debug output
  location loc;
 
  // Stack of blocks currently in scope
  std::list<block*> blocks;
  
  bool initialized;
  
public:
  // Construct an ostream which tees output to the supplied
  // ostreams.
  dbgStream();
  dbgStream(properties* props, std::string title, std::string workDir, std::string imgDir, std::string tmpDir);
  void init(properties* props, std::string title, std::string workDir, std::string imgDir, std::string tmpDir);
  ~dbgStream();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
 
  // Switch between the owner class and user code writing text into this stream
  void userAccessing();
  void ownerAccessing();
  
  // Return the root working directory
  const std::string& getWorkDir() const { return workDir; }
  // Return the directory where all images will be stored
  const std::string& getImgDir() const { return imgDir; }
  // Return the directory that widgets can use as temporary scratch space
  const std::string& getTmpDir() const { return tmpDir; }

  // Returns the stream's current location
  location getLocation() { return loc; }
  
  // Called when a block is entered.
  // b: The block that is being entered
  void enterBlock(block* b);
  
  // Called when a block is exited. Returns the block that was exited.
  block* exitBlock();
    
  // Called to inform the blocks that contain the given block that it has been entered
  void subBlockEnterNotify(block* subBlock);
  
  // Called to inform the blocks that contain the given block that it has been exited
  void subBlockExitNotify(block* subBlock);
    
  // Adds an image to the output with the given extension and returns the path of this image
  // so that the caller can write to it.
  std::string addImage(std::string ext=".gif");
    
  // ----- Output of tags ----
  // Emit the entry into a tag to the structured output file. The tag is set to the given property key/value pairs
  //void enter(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom);
  void enter(sightObj* obj);
    
  void enter(const properties& props);
    
  // Returns the text that should be emitted to the structured output file that denotes the the entry into a tag. 
  // The tag is set to the given property key/value pairs
  //std::string enterStr(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom);
  std::string enterStr(const properties& props);
    
  // Emit the exit from a given tag to the structured output file
  //void exit(std::string name);
  void exit(sightObj* obj);
  void exit(const properties& props);
    
  // Returns the text that should be emitted to the the structured output file to that denotes exit from a given tag
  //std::string exitStr(std::string name);
  std::string exitStr(const properties& props);
  
  // Emit a full tag an an the structured output file
  //void tag(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom);
  void tag(sightObj* obj);
  void tag(const properties& props);
    
  // Returns the text that should be emitted to the the structured output file to that denotes a full tag an an the structured output file
  //std::string tagStr(std::string name, const std::map<std::string, std::string>& properties, bool inheritedFrom);
  std::string tagStr(const properties& props);
}; // dbgStream

extern dbgStream dbg;

class dbgStreamMerger : public Merger {
  public:
  // The directory into which the merged will be written. This directory must be explicitly set before
  // an instance of the dbgStreamMerger class is created
  static std::string workDir;
  
  dbgStreamMerger(//std::string workDir, 
                  std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                  std::map<std::string, streamRecord*>& outStreamRecords,
                  std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                  properties* props=NULL);
        
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new dbgStreamMerger(tags, outStreamRecords, inStreamRecords, props); }
                  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) { 
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
};

class dbgStreamStreamRecord: public streamRecord {
  friend class dbgStreamMerger;
  // The current location within the debug output
  streamLocation loc;

  // Maintains a record for all the tags that have been entered but not yet existed whether they've been emitted.
  // This is used to ensure that if we choose to not emit the entry of a given tag, we do the same for the exit 
  std::list<bool> emitFlags;
  
  public:  
  dbgStreamStreamRecord(int vID)              : streamRecord(vID, "sight") { }
  dbgStreamStreamRecord(const variantID& vID) : streamRecord(vID, "sight") { }
  // vSuffixID: ID that identifies this variant within the next level of variants in the heirarchy
  dbgStreamStreamRecord(const dbgStreamStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same outgoing stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
  
  // Returns the stream's current location
  streamLocation getLocation() { return loc; }

  // Pushes the given boolean onto the emitFlags stack
  void push(bool v) { emitFlags.push_back(v); }

  // Pops the top element from the emitFlags stack and returns it
  bool pop() { assert(emitFlags.size()>0); bool ret=emitFlags.back(); emitFlags.pop_back(); return ret; }
  
  // Called when a block is entered or exited.
  static void enterBlock(std::vector<std::map<std::string, streamRecord*> >& streamRecords);
  static void enterBlock(std::map<std::string, streamRecord*>&               streamRecord);
  static void exitBlock (std::vector<std::map<std::string, streamRecord*> >& streamRecords);
  static void exitBlock (std::map<std::string, streamRecord*>&               streamRecord);
    
  std::string str(std::string indent="") const;
}; // class dbgStreamStreamRecord

class indent : public sightObj
{
  public:
  //bool active;
  // prefix - the string that will be prepended to all subsequent lines. (default is "    ")
  // repeatCnt - the number of repetitions of this string. If repeatCnt=0 we will not do any indentation. (default is 1).
  // onoffOp - We emit this scope if the current attribute query evaluates to true (i.e. we're emitting debug output) AND
  //           either onoffOp is not provided or its evaluates to true.
  indent(std::string prefix,                                       properties* props=NULL);
  indent(std::string prefix, int repeatCnt, const structure::attrOp& onoffOp, properties* props=NULL);
  indent(std::string prefix, int repeatCnt,                        properties* props=NULL);
  indent(                    int repeatCnt,                        properties* props=NULL);
  indent(std::string prefix,                const structure::attrOp& onoffOp, properties* props=NULL);
  indent(                    int repeatCnt, const structure::attrOp& onoffOp, properties* props=NULL);
  indent(                                   const structure::attrOp& onoffOp, properties* props=NULL);
  indent(                                                          properties* props=NULL);
  static properties* setProperties(std::string prefix, int repeatCnt, const structure::attrOp* onoffOp, properties* props);
    
  ~indent();

  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
}; // indent

class IndentMerger : public Merger {
  public:
  IndentMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
               std::map<std::string, streamRecord*>& outStreamRecords,
               std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
               properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new IndentMerger(tags, outStreamRecords, inStreamRecords, props); }
  
  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info) {
    Merger::mergeKey(type, tag.next(), inStreamRecords, info);
  }
}; // class IndentMerger


int dbgprintf(const char * format, ... );

} // namespace structure
} // namespace sight
