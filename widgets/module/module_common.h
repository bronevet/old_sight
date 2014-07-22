#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../attributes/attributes_common.h"
#include "sight_common.h"

namespace sight {
namespace common {


class module {
  public:
  
  // Module relations are organized based two data structures: groups and contexts.
  // A group defines the granularity at which different executions of a module are differentiated. A group
  //    may be the module's name, its name+nesting stack within other modules, name+call stack or name+context.
  //    All module groups that map to the same group must have the same number of inputs and outputs.
  // Context defines the properties of a given data item propagated from one module to another. A context is a mapping
  //    from one or more property names to their values. The context of a given group of a module is the combination
  //    of the contexts of its inputs. It is possible to annotate different applicaiton inputs with notes
  //    about their properties. A note may indicate that the given input property should be inherited by modules
  //    nested inside the current module or it may indicate that the values of the given context vary monotonically 
  //    with respect to time or another context attribute.
  
  
  /*
  This is the base class of a wide range of notes one can place on a given context entry.
    The architectures of notes is designed to be simple rather than extensible. While widget,
    attribute and comparator processing allows newly-developed widgets to be inserted 
    transparently by registering callbacks for deserialization, merging and layout, notes
    are expected to be managed centrally inside the widget module. The plan is to have a common
    set of note semantics such as inheritability of attributes across module nesting and
    commutativity of operations. To make sure that queries understand all these notes, their
    semantics will be carefully considered before being included in Sight. Since notes are
    not expected to change very much, the software infrastructure we implement for them is
    designed to be simpler to use but inflexible. Different note classes provide their
    own serialization methods. However, deserialization is managed centrally inside 
    note::deserialize(), which will figure out which sub-class of note is being deserialized
    and directly call its constructor.
    All note instances must provide a constructor that takes a serialized representation 
      of the note!
  The serial representation of each class is in the format "noteClassName:serializedRepresentation", 
    where the ":" and ";" has been escaped in noteClassName and serializedRepresentation. Each note
    class implements a name() method, which is used to specify the noteClassName part and a 
    serialize() method, which is used to provide the serializedRepresentation part.
  The serial representation of a list of notes is a sequential concatenation of its 
    elements, separated by ";".
  
  The notes API is designed to combine generality and efficiency. Generality is achieved
    by implementing each note type as a sub-class of note and specifying a different set of
    internal data structures appropriate for it. Efficiency is achieved by passing notes
    as const references into context and module constructors. This comes from the observation
    that notes only need to be read during the execution of the module constructor and then
    either acted upon or serialized (e.g. module(..., context("key", val, note(args)) )).
    Any notes provided directly as a parameter to a context constructor are guaranteed to
    be live by the time the resulting context object makes its way to the module constructor.
    The only quirk comes when users create a context object and then on a separate line pass
    it to the module constructor. In this case we'll provide a context::add method that 
    explicitly creates a temporary copy of the note so that it can keep a const reference to it
    and then in the context destructor will delete any notes we created in this way. To make
    sure that 
  */
     
  /*class note : public printable {
    protected:
    // Returns the name of this note class, with ":" and ";" characters escaped.
    virtual std::string name() { return "note"; }
    // Returns the serialized representation of this note instance, with ":"  and ";" characters escaped.
    virtual std::string serialize() { return ""; }

    note() {}
    
    public:
    // Given a serialized representation of a note, returns an instance of the note 
    // object that is encoded.
    static note deserialize(std::string serialized);
    
    virtual std::string str(std::string indent="") const { return "[note]"; }
  }; // class note

  // Notes that denote that a given boolean property is either true or false
  class flagNote : public printable {
    public:
    flagNote() {}
    flagNote(std::string serialized);
    std::string name() { return "fn"; }
    std::string str(std::string indent="") const { return "[flagNote]"; }
  }; // class note

  // Blank note that is used as a placeholder in constructor calls
  class noNote : public flagNote {
    public:
    noNote() {}
    // There is no need to serialize instances of noNote
    noNote(std::string serialized) { assert(0); }
    // Returns the serialized representation of this note instance, with ":"  and ";" characters escaped.
    std::string serialize() { assert(0); return ""; }
    std::string str(std::string indent="") const { return "[noNote]"; }
  };
  
  // Indicates that the given input or option context attribute should be provided as
  // an input or option, respectively, to all the module instances that are contained
  // inside the current module.
  class inherited: public flagNote {
    public:
    inherited() {}
    // There is no need to serialize instances of inherited
    inherited(std::string serialized) { assert(0); }
    // Returns the serialized representation of this note instance, with ":"  and ";" characters escaped.
    std::string serialize() { assert(0); return ""; }
    std::string str(std::string indent="") const { return "[inherited]"; }
  };

  // When provided as an input or option context, indicates that the value of this attribute
  // should be retrieved from a module instance that contains the current module instance.
  class container: public flagNote {
    // The name of the container module and index of the input port that are being referenced.
    // If either is not specified, the most closely nested module and the smallest index input is assumed.
    std::string moduleName;
    int input;
    public:
    container(int input=-1) : moduleName("") { }
    container(std::string moduleName, int input=-1) : moduleName(moduleName), input(input) {}
    // There is no need to serialize instances of container
    container(std::string serialized) { assert(0); }

    // Returns the serialized representation of this note instance, with ":"  and ";" characters escaped.
    std::string serialize() { assert(0); return ""; }
    
    std::string str(std::string indent="") const { return "[container]"; }
  };*/
    
  class note : public printable{
    protected:
    std::string name;
    std::map<std::string, std::string> props;
    
    note() {}
    public:
    note(const note& that): name(that.name), props(that.props) {}
    const std::string& getName() const { return name; }
    const std::map<std::string, std::string>& getProps() const { return props; }
    
    /*// Returns the serialized representation of this note instance, with ":"  and ";" characters escaped.
    std::string serialize();
      
    // Given a serialized representation of a note, returns an instance of the note 
    // object that is encoded.
    static note deserialize(std::string serialized);*/
    
    virtual std::string str(std::string indent="") const { return name; }
  };
  
  class publicized: public note {
    public:
    publicized() { name = "publicized"; }
  };
  
  class inherit: public note {
    public:
    inherit() { name = "inherit"; }
  };
  
  class noNote: public note {
    public:
    noNote() { name = "noNote"; }
  };

  // A collection of nodes provided by the user
  /*class notes: public easylist<note> {
    public:
    notes() {}
    // Creates a notes object from a ";"-separated list of serialized notes
    //notes(std::string serialized);
    notes(const notes& that): easylist<note>((easylist<note>)that) {}
  }; // class notes*/
  typedef easylist<note> notes;
  
  
  class context {
    public:
    // Maps each string key to a pair of the value this key is assigned to and the noted attached to 
    std::map<std::string, attrValue>  configuration;
    std::map<std::string, notes>      configNotes;
    
    //typedef common::easymap<const std::string&, const attrValue&> config;
    
    context() {}
    
    context(const context& that) : configuration(that.configuration), configNotes(that.configNotes) {}
    
    // Constructors that omit configuration notes
    context(const std::map<std::string, attrValue>& configuration) : 
                    configuration(configuration) {}
    
    context(const std::string& key0, const attrValue& val0)
    { configuration[key0] = val0; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1)
    { configuration[key0] = val0; configuration[key1] = val1; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7, const std::string& key8, const attrValue& val8)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; configuration[key8] = val8; }

    context(const std::string& key0, const attrValue& val0, const std::string& key1, const attrValue& val1, const std::string& key2, const attrValue& val2, const std::string& key3, const attrValue& val3, const std::string& key4, const attrValue& val4, const std::string& key5, const attrValue& val5, const std::string& key6, const attrValue& val6, const std::string& key7, const attrValue& val7, const std::string& key8, const attrValue& val8, const std::string& key9, const attrValue& val9)
    { configuration[key0] = val0; configuration[key1] = val1; configuration[key2] = val2; configuration[key3] = val3; configuration[key4] = val4; configuration[key5] = val5; configuration[key6] = val6; configuration[key7] = val7; configuration[key8] = val8; configuration[key9] = val9; }
    
    // Constructors that include configuration notes
    context(const std::map<std::string, attrValue>& configuration,
            const std::map<std::string, notes>&     configNotes) : 
                    configuration(configuration), configNotes(configNotes) {}
    
    context(const std::string& key0, const attrValue& val0, const notes& n0)
    { configuration[key0] = val0; configNotes[key0] = n0; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1, const std::string& key2, const attrValue& val2, const notes& n2)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; configuration[key2] = val2; configNotes[key2] = n2; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1, const std::string& key2, const attrValue& val2, const notes& n2, const std::string& key3, const attrValue& val3, const notes& n3)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; configuration[key2] = val2; configNotes[key2] = n2; configuration[key3] = val3; configNotes[key3] = n3; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1, const std::string& key2, const attrValue& val2, const notes& n2, const std::string& key3, const attrValue& val3, const notes& n3, const std::string& key4, const attrValue& val4, const notes& n4)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; configuration[key2] = val2; configNotes[key2] = n2; configuration[key3] = val3; configNotes[key3] = n3; configuration[key4] = val4; configNotes[key4] = n4; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1, const std::string& key2, const attrValue& val2, const notes& n2, const std::string& key3, const attrValue& val3, const notes& n3, const std::string& key4, const attrValue& val4, const notes& n4, const std::string& key5, const attrValue& val5, const notes& n5)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; configuration[key2] = val2; configNotes[key2] = n2; configuration[key3] = val3; configNotes[key3] = n3; configuration[key4] = val4; configNotes[key4] = n4; configuration[key5] = val5; configNotes[key5] = n5; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1, const std::string& key2, const attrValue& val2, const notes& n2, const std::string& key3, const attrValue& val3, const notes& n3, const std::string& key4, const attrValue& val4, const notes& n4, const std::string& key5, const attrValue& val5, const notes& n5, const std::string& key6, const attrValue& val6, const notes& n6)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; configuration[key2] = val2; configNotes[key2] = n2; configuration[key3] = val3; configNotes[key3] = n3; configuration[key4] = val4; configNotes[key4] = n4; configuration[key5] = val5; configNotes[key5] = n5; configuration[key6] = val6; configNotes[key6] = n6; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1, const std::string& key2, const attrValue& val2, const notes& n2, const std::string& key3, const attrValue& val3, const notes& n3, const std::string& key4, const attrValue& val4, const notes& n4, const std::string& key5, const attrValue& val5, const notes& n5, const std::string& key6, const attrValue& val6, const notes& n6, const std::string& key7, const attrValue& val7, const notes& n7)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; configuration[key2] = val2; configNotes[key2] = n2; configuration[key3] = val3; configNotes[key3] = n3; configuration[key4] = val4; configNotes[key4] = n4; configuration[key5] = val5; configNotes[key5] = n5; configuration[key6] = val6; configNotes[key6] = n6; configuration[key7] = val7; configNotes[key7] = n7; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1, const std::string& key2, const attrValue& val2, const notes& n2, const std::string& key3, const attrValue& val3, const notes& n3, const std::string& key4, const attrValue& val4, const notes& n4, const std::string& key5, const attrValue& val5, const notes& n5, const std::string& key6, const attrValue& val6, const notes& n6, const std::string& key7, const attrValue& val7, const notes& n7, const std::string& key8, const attrValue& val8, const notes& n8)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; configuration[key2] = val2; configNotes[key2] = n2; configuration[key3] = val3; configNotes[key3] = n3; configuration[key4] = val4; configNotes[key4] = n4; configuration[key5] = val5; configNotes[key5] = n5; configuration[key6] = val6; configNotes[key6] = n6; configuration[key7] = val7; configNotes[key7] = n7; configuration[key8] = val8; configNotes[key8] = n8; }

    context(const std::string& key0, const attrValue& val0, const notes& n0, const std::string& key1, const attrValue& val1, const notes& n1, const std::string& key2, const attrValue& val2, const notes& n2, const std::string& key3, const attrValue& val3, const notes& n3, const std::string& key4, const attrValue& val4, const notes& n4, const std::string& key5, const attrValue& val5, const notes& n5, const std::string& key6, const attrValue& val6, const notes& n6, const std::string& key7, const attrValue& val7, const notes& n7, const std::string& key8, const attrValue& val8, const notes& n8, const std::string& key9, const attrValue& val9, const notes& n9)
    { configuration[key0] = val0; configNotes[key0] = n0; configuration[key1] = val1; configNotes[key1] = n1; configuration[key2] = val2; configNotes[key2] = n2; configuration[key3] = val3; configNotes[key3] = n3; configuration[key4] = val4; configNotes[key4] = n4; configuration[key5] = val5; configNotes[key5] = n5; configuration[key6] = val6; configNotes[key6] = n6; configuration[key7] = val7; configNotes[key7] = n7; configuration[key8] = val8; configNotes[key8] = n8; configuration[key9] = val9; configNotes[key9] = n9; }

    virtual ~context() {}
    
    // Loads this context from the given properties map. The names of all the fields are assumed to be prefixed
    // with the given string.
    context(properties::iterator props, std::string prefix="");
    
    // Returns a dynamically-allocated copy of this object. This method must be implemented by all classes
    // that inherit from context to make sure that appropriate copies of them can be created.
    virtual context* copy() const { return new context(*this); }
    
    // These comparator routines must be implemented by all classes that inherit from context to make sure that
    // their additional details are reflected in the results of the comparison. Implementors may assume that 
    // the type of that is their special derivation of context rather than a generic context and should dynamically
    // cast from const context& to their sub-type.
    virtual bool operator==(const context& that) const
    { return configuration==that.configuration; }
    
    virtual bool operator<(const context& that) const
    { return configuration<that.configuration; }
    
    // Adds the given key/attrValue pair to this context
    void add(std::string key, const attrValue& val);
    
    // Add all the key/attrValue pairs from the given context to this one, overwriting keys as needed
    void add(const context& that);
    
    const std::map<std::string, attrValue>& getCfg() const { return configuration; }
    
    // Returns whether the given key is mapped within this context
    bool isKey(std::string key) const
    { return configuration.find(key) != configuration.end(); }
    
    // Returns the properties map that describes this context object.
    // The names of all the fields in the map are prefixed with the given string.
    std::map<std::string, std::string> getProperties(std::string prefix="") const;
    
    // Returns a human-readable string that describes this context
    virtual std::string str() const;
  }; // class context

  // The different types of ports: input and output
  typedef enum {input, output} ioT;

  // Returns a string encoding of the key information of a context attribute:
  // moduleClass - the class that inherits from module that produced this attribute
  // ctxtGrouping - the name of the grouping of context attributes within the attributes generated by the 
  //    module class. This may be a given input (e.g. input0) or the configuration options of a compModule.
  // attrName - the name of the context attribute itself.
  static std::string encodeCtxtName(
                         const std::string& moduleClass, const std::string& ctxtGrouping, 
                         const std::string& ctxtSubGrouping, const std::string& attrName);
  // Given a string encoded by encodeCtxtName(), sets moduleClass, ctxtGrouping and attrName to their original values
  static void decodeCtxtName(const std::string& encoded, 
                         std::string& moduleClass, std::string& ctxtGrouping, 
                         std::string& ctxtSubGrouping, std::string& attrName);

}; // class module

} // namespace common
} // namespace sight
