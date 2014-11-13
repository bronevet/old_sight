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
#include "thread_local_storage.h"
#include <signal.h>

#include <deque>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/adjacency_list.hpp>
namespace sight {
namespace structure{

// This mechanism allows different widgets to register their own code to be called when each
//    thread is started and stopped. This includes both the main thread and threads created
//    using pthread_create. Each widget can provide initialization and finalization methods,
//    and order them relative to other widgets' methods to maintain relative order among different
//    initialization/finalization logic.
// Initializers/finalizers are specified by 
//    - deriving a widget-specific class from ThreadInitFinInstantiator that uses method 
//      addFuncs() in its constructor to register the widget-specific functions, and
//    - declaring a single global variable of this type.
typedef void (*ThreadInitializer)();
typedef void (*ThreadFinalizer)();

class ThreadInitFinInstantiator : public sight::common::LoadTimeRegistry {
  // The maximum unique ID assigned to any threadFuncs so far
  static int* maxUID;
  class threadFuncs {
    public:
    int UID;
    ThreadInitializer init;
    ThreadFinalizer   fin;
    set<std::string> mustFollow;
    set<std::string> mustPrecede;

    threadFuncs(ThreadInitializer init, ThreadFinalizer fin,
                const set<std::string>& mustFollow, const set<std::string>& mustPrecede) :
              UID((*maxUID)++), init(init), fin(fin), mustFollow(mustFollow), mustPrecede(mustPrecede) {}
    
    std::string str() const {
      return txt()<<"[threadFuncs: UID="<<UID<<", mustFollow="<<set2str(mustFollow)<<", mustPrecede="<<set2str(mustPrecede)<<"]";
    }
  };
  public:
  // Keep track of the funcs that are currently registered to initialize and finalize threads
  // Maps the string labels of funcs to their threadFuncs data structures
  //   Each threadFuncs has a unique ID that is used as the vertex number in the dependence
  //   graph, which is implemented using the Boost Graph Library
  static std::map<std::string, threadFuncs*>* funcsM;
  // Vector of pointers to the threadFuncs in funcsM, where the index of each record is
  //   equal to its UID.
  static std::vector<threadFuncs*>* funcsV;

  // Records the dependencies among the entries in funcs
  static boost::adjacency_list<boost::listS, boost::vecS, boost::directedS>* depGraph;

  // Records the topological order among the funcs
  static std::deque<int>* topoOrder;
  
  // Records whether the dependence graph is upto-date relative to the current
  // entries in funcs
  static bool* depGraphUptoDate;
  
  // The height of each thread's SOStack after it has been initialized. This is important
  // for ensuring that on shutdown we destroy any sightObjects above this point before
  // calling the finalization routines.
  static ThreadLocalStorage0<int> initializedSOStackHeight;

  ThreadInitFinInstantiator();
  
  // Called exactly once for each class that derives from LoadTimeRegistry to initialize its static data structures.
  static void init();
  
  // Adds the given initialization/finalization funcs under the given widet label.
  // mustFollow - set of widget labels that the given functors must follow during initialization
  // mustPrecede - set of widget labels that the given functors must precede during initialization
  //    finalization is performed in reverse
  static void addFuncs(const std::string& label, ThreadInitializer init, ThreadFinalizer fin,
                       const common::easyset<std::string>& mustFollow, const common::easyset<std::string>& mustPrecede);
  
  // Update the dependence graph based on funcs, if it not already upto-date
  static void computeDepGraph();
  
  // Calls all the initializers in their topological order
  static void initialize();

  // Calls all the finalizers in their topological order (reverse of initializers)
  static void finalize();
  
  static std::string str();
};

} // namespace structure
} // namespace sight
