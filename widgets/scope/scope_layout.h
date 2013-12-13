#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../sight_common.h"
#include "../../sight_layout.h"

namespace sight {
namespace layout {

class scopeLayoutHandlerInstantiator  : layoutHandlerInstantiator{
  public:
  scopeLayoutHandlerInstantiator();
};
extern scopeLayoutHandlerInstantiator scopeLayoutHandlerInstance;

class scope: public block, public common::scope
{
  static std::vector<std::string> colors;
  static int colorIdx; // The current index into the list of colors 
  
  public:
  //scopeLevel level;

  // properties: maps property names to their values
  // level - the type of visualization used, with higher levels associated with more amounts of debug output
  //    There are several features that are enabled by the levels:
  //       own_file: Text inside the scope is written to a separate file. Users must manually click a button to see it.
  //       own_color: The background color of the scope is different from its parent scope.
  //       label_interactive: The label of this scope is shown in a larger font, along with controls to minimize the scope 
  //              by clicking on the label and open GDB to the point in the execution where the scope started
  //       label_shown: The label of this scope is printed out
  //       summary_entry: A record of this scope is added to the summary frame
  //    Different levels
  //    high: own_file, own_color, label_interactive, label_shown, summary_entry
  //    medium: own_color, label_interactive, label_shown, summary_entry
  //    low: label_interactive, label_shown, summary_entry
  //    min: label_shown, summary_entry
  scope(properties::iterator props);
  scope(std::string label, scopeLevel level);
  scope(std::string label, bool ownFile, bool ownColor, bool labelInteractive, bool labelShown, bool summaryEntry);
  bool ownFile;
  bool ownColor;
  bool labelInteractive;
  bool labelShown;
  bool summaryEntry;
  
  // Index of this block within its parent block
  int blockIndex;
    
  private:
  // Sets the scope's configuration flags based on the given level
  void level2Config(scopeLevel level);
  
  // Common initialization code
  void init();
  
  public:
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return true; }
  bool subBlockExitNotify (block* subBlock) { return true; }
  
  // Called to enable the block to print its entry and exit text
  void printEntry(std::string loadCmd);
  void printExit();
  
  ~scope();
}; // scope

}; // namespace layout
}; // namespace sight
