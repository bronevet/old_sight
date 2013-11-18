#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../sight_common.h"
#include "../sight_layout.h"
#include "module_common.h"

namespace sight {
namespace layout {

class moduleLayoutHandlerInstantiator : layoutHandlerInstantiator {
  public:
  moduleLayoutHandlerInstantiator();
};
extern moduleLayoutHandlerInstantiator moduleLayoutHandlerInstance;

class module: public block, public common::module
{
  protected:
  
  // The path the directory where output files of the graph widget are stored
  // Relative to current path
  static std::string outDir;
  // Relative to root of HTML document
  static std::string htmlOutDir;
    
  // Unique ID of this module object
  int moduleID;
  
  // Maps IDs of nodes to their full contexts
  std::map<int, common::module::context> knownCtxt;
    
  // Stack of the modules that are currently in scope
  static std::list<module*> mStack;
  
  // The dot file that will hold the representation of the module interaction graph
  std::ofstream dotFile;
  
  public:
  
  module(properties::iterator props);
  ~module();
  
  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();
  
  // Add a node to the modules graph
  void addNode(common::module::context node, int ID, int count);
  static void* addNode(properties::iterator props);
  
  // Add a directed edge from the location of the from anchor to the location of the to anchor
  void addEdge(int fromC, common::module::ioT fromT, int fromP, 
               int toC,   common::module::ioT toT,   int toP);
  static void* addEdge(properties::iterator props);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return false; }
  bool subBlockExitNotify (block* subBlock) { return false; }
}; // class module

}; // namespace layout
}; // namespace sight
