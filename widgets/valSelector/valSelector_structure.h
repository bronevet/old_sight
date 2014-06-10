#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include "../../sight_structure_internal.h"

namespace sight {
namespace structure {

class valSelector: public sightObj
{
  protected:
  
  static int maxSelID;
  int selID;
    
  // If an attribute key was provided to this selector, records it here
  std::string attrKey;
  // Records whether the key was provided
  bool attrKeyKnown;
  
  public:
  
  valSelector(properties* props=NULL);
  valSelector(std::string attrKey, properties* props=NULL);

  ~valSelector();
  
  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
  
  int getID() const;
  
  // Informs the value selector that we have observed a new value that the selector needs to account for
  // Returns the string reprentation of the current value.
  virtual std::string observeSelection(const attrValue& val)=0;
    
  // Informs the value selector that we have observed a new value that the selector needs to account for
  // It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
  // Returns the string reprentation of the current value.
  virtual std::string observeSelection()=0;
};

class colorSelector : public valSelector {
  
  public:
  colorSelector(properties* props=NULL);
  colorSelector(std::string attrKey, properties* props=NULL);
  
  // Color selector with an explicit color gradient
  colorSelector(float startR, float startG, float startB,
                float endR,   float endG,   float endB, 
                properties* props=NULL);

  colorSelector(std::string attrKey,
                float startR, float startG, float startB,
                float endR,   float endG,   float endB, 
                properties* props=NULL);
  
  static properties* setProperties(float startR, float startG, float startB,
                                   float endR,   float endG,   float endB,
                                   properties* props=NULL);

  ~colorSelector();
 
  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  virtual void destroy();
 
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  // Returns the string reprentation of the current value.
  std::string observeSelection(const attrValue& val);
    
  // Returns a string that contains a call to a JavaScipt function that at log view time will return a value
  // It is assumed that the selector is already associated with some attribute and can get the attrValue on its own
  // Returns the string reprentation of the current value.
  std::string observeSelection();
};

// Classes that modify the appearance of text and more generally, modifiers used as dbg<<modified::start()<<"some text"<<modifier::end()<<...;
namespace textColor
{
  //public:
  //static std::string start(valSelector& sel, const attrValue& val);
  //static std::string start(valSelector& sel);
  //static std::string end();
  
  class start {
    public:
    valSelector& sel;
    attrValue* val;
    start(valSelector& sel, const attrValue& val) : sel(sel), val(const_cast<attrValue*>(&val)) {}
    start(valSelector& sel)                       : sel(sel), val(NULL) {}
  };
  std::ostream& operator<< (std::ostream& stream, const start& s);
    
  class end { };
  std::ostream& operator<< (std::ostream& stream, const end& e);
}


namespace bgColor
{
  /*public:
  static std::string start(valSelector& sel, const attrValue& val);
  static std::string start(valSelector& sel);
  
  static std::string end();*/
  class start {
    public:
    valSelector& sel;
    attrValue* val;
    start(valSelector& sel, const attrValue& val) : sel(sel), val(const_cast<attrValue*>(&val)) {}
    start(valSelector& sel)                       : sel(sel), val(NULL) {}
  };
  std::ostream& operator<< (std::ostream& stream, const start& s);
    
  class end { };
  std::ostream& operator<< (std::ostream& stream, const end& e);
}

namespace borderColor
{
  /*public:
  static std::string start(valSelector& sel, const attrValue& val);
  static std::string start(valSelector& sel);
  
  static std::string end();*/
  class start {
    public:
    valSelector& sel;
    attrValue* val;
    start(valSelector& sel, const attrValue& val) : sel(sel), val(const_cast<attrValue*>(&val)) {}
    start(valSelector& sel)                       : sel(sel), val(NULL) {}
  };
  std::ostream& operator<< (std::ostream& stream, const start& s);
    
  class end { };
  std::ostream& operator<< (std::ostream& stream, const end& e);
}

class ColorSelectorMergeHandlerInstantiator: public MergeHandlerInstantiator {
  public:
  ColorSelectorMergeHandlerInstantiator();
};
extern ColorSelectorMergeHandlerInstantiator ColorSelectorMergeHandlerInstance;

std::map<std::string, streamRecord*> ColorSelectorGetMergeStreamRecord(int streamID);

// Merger for colorSelector tag
class ColorSelectorMerger : public Merger {
  public:
  ColorSelectorMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
  
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ColorSelectorMerger(tags, outStreamRecords, inStreamRecords, props); }
              
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class ColorSelectorMerger

// Merger for tags textColor, bgColor, borderColor
class ColorMerger : public Merger {
  public:
  ColorMerger(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
              std::map<std::string, streamRecord*>& outStreamRecords,
              std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
              properties* props=NULL);
    
  static Merger* create(const std::vector<std::pair<properties::tagType, properties::iterator> >& tags,
                        std::map<std::string, streamRecord*>& outStreamRecords,
                        std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                        properties* props)
  { return new ColorMerger(tags, outStreamRecords, inStreamRecords, props); }
              
  // Sets the properties of the merged object
  static properties* setProperties(std::vector<std::pair<properties::tagType, properties::iterator> > tags,
                                   std::map<std::string, streamRecord*>& outStreamRecords,
                                   std::vector<std::map<std::string, streamRecord*> >& inStreamRecords,
                                   properties* props);

  // Sets a list of strings that denotes a unique ID according to which instances of this merger's 
  // tags should be differentiated for purposes of merging. Tags with different IDs will not be merged.
  // Each level of the inheritance hierarchy may add zero or more elements to the given list and 
  // call their parents so they can add any info. Keys from base classes must precede keys from derived classes.
  static void mergeKey(properties::tagType type, properties::iterator tag, 
                       std::map<std::string, streamRecord*>& inStreamRecords, MergeInfo& info);
}; // class ColorMerger


// streamRecord for colorSelector tag
class ColorSelectorStreamRecord: public streamRecord {
  friend class ColorSelectorMerger;
  
  public:
  ColorSelectorStreamRecord(int vID)              : streamRecord(vID, "colorSelector") { }
  ColorSelectorStreamRecord(const variantID& vID) : streamRecord(vID, "colorSelector") { }
  ColorSelectorStreamRecord(const ColorSelectorStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
      
  std::string str(std::string indent="") const;
}; // class ColorSelectorStreamRecord

// streamRecord for tags textColor, bgColor, borderColor
class ColorStreamRecord: public streamRecord {
  friend class ColorMerger;
  
  public:
  ColorStreamRecord(int vID)              : streamRecord(vID, "color") { }
  ColorStreamRecord(const variantID& vID) : streamRecord(vID, "color") { }
  ColorStreamRecord(const ColorStreamRecord& that, int vSuffixID);
  
  // Returns a dynamically-allocated copy of this streamRecord, specialized to the given variant ID,
  // which is appended to the new stream's variant list.
  streamRecord* copy(int vSuffixID);
  
  // Given multiple streamRecords from several variants of the same stream, update this streamRecord object
  // to contain the state that succeeds them all, making it possible to resume processing
  void resumeFrom(std::vector<std::map<std::string, streamRecord*> >& streams);
      
  std::string str(std::string indent="") const;
}; // class ColorStreamRecord


} // namespace structure
} // namespace sight
