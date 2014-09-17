// Licence information included in file LICENCE
#include "../../sight_layout.h"
#include "../../sight_common.h"
#include <iostream>
#include <assert.h>
#include "box_layout.h"

using namespace std;

namespace sight {
namespace layout {

// Registers the handlers for object entry/exit layout. The entry handler is provided
// the beginning iteraror to the properties object that describes the object. It returns
// the C++ object that holds the decoded object or NULL if none was created.
// The exit handler provides a pointer to the object returned by the entry handler so that
// it may be deleted.
// This architecture makes it possible to do all entry/exit processing inside object 
// constructors and destructors, which is consistent with the way the interface and merging
// layers operate.
void* boxEnterHandler(properties::iterator props) { return new box(props); }
void  boxExitHandler(void* obj) { box* s = static_cast<box*>(obj); delete s; }
  
boxLayoutHandlerInstantiator::boxLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["box"] = &boxEnterHandler;
  (*layoutExitHandlers) ["box"] = &boxExitHandler;
}
boxLayoutHandlerInstantiator boxLayoutHandlerInstance;

box::box(properties::iterator props) : block(properties::next(props))
{
  // Extract the style of this box
  style = props.get("style");

  // Record with the output stream that a new block has been entered. Don't create an entry in 
  // the summary pane for this box.
  dbg.enterBlock(this, /*newFileEntered*/ false, /*addSummaryEntry*/ false);
}

box::~box()
{ 
  // Record with the output stream that this block has been exited  
  dbg.exitBlock();
}

// Called to enable the block to print its entry and exit text
void box::printEntry(string loadCmd) {
  // Declare that we're about to emit arbitrary HTML code and ask Sight to not do any formatting
  // as it might to make user-provided text look ok in HTML
  dbg.ownerAccessing();

  // Start the div with the requested style
  dbg << "<div style=\""<<style<<"\">";

  // Return to normal mode;
  dbg.userAccessing();  
}

void box::printExit() {
  // Declare that we're about to emit arbitrary HTML code and ask Sight to not do any formatting
  // as it might to make user-provided text look ok in HTML
  dbg.ownerAccessing();

  // End the div
  dbg << "</div>";

  // Return to normal mode;
  dbg.userAccessing();  
}

}; // namespace layout
}; // namespace sight
