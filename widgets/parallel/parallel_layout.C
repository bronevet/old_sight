// Licence information included in file LICENCE
#include "../../sight_layout.h"
#include "../../sight_common.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include "parallel_layout.h"

using namespace std;
using namespace sight::common;

namespace sight {
namespace layout {

 
// Record the layout handlers in this file
parallelLayoutHandlerInstantiator::parallelLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["commSend"]  = &commSendEnterHandler;
  (*layoutExitHandlers) ["commSend"]  = &commSendExitHandler;
  (*layoutEnterHandlers)["commRecv"]  = &commRecvEnterHandler;
  (*layoutExitHandlers) ["commRecv"]  = &commRecvExitHandler;
  (*layoutEnterHandlers)["commBar"]   = &commBarEnterHandler;
  (*layoutExitHandlers) ["commBar"]   = &commBarExitHandler;
}
parallelLayoutHandlerInstantiator parallelLayoutHandlerInstance;

/********************
 ***** commSend *****
 ********************/

static bool parJSInitialized=false;
commSend::commSend(properties::iterator props) : uniqueMark(properties::next(props))
{
  // Load uniqueMark-specific JavaScript files into the final document
  if(!parJSInitialized) {
    // Create the directory that holds the parallel-specific scripts
    dbg.createWidgetDir("parallel");
    
    dbg.includeFile("parallel/canvasutilities.js"); dbg.includeWidgetScript("parallel/canvasutilities.js", "text/javascript");
    dbg.includeFile("parallel/parallel.js");        dbg.includeWidgetScript("parallel/parallel.js",        "text/javascript");
    
    // Add command to show arrows once all the unique marks have been registered
    dbg.widgetScriptEpilogCommand(txt()<<"showParallelArrows();");
    
    parJSInitialized=true;
  } 
  
   // Read all the unique IDs of the receives the match this send from props and 
  // create edges from this send to these receives.
  int numIDs = props.getInt("numRecvIDs");
  if(numIDs > 0) {
    ostringstream recvIDsS;
    recvIDsS << "[";
    for(int i=0; i<numIDs; i++) {
      if(i>0) recvIDsS << ",";
      recvIDsS << "'"<<props.get(txt()<<"ID"<<i)<<"'";
    }
    recvIDsS << "]";
    recvIDs = recvIDsS.str();
  }
  
  dbg.enterBlock(this, /*newFileEntered*/ false, /*summaryEntry*/ false);
}

commSend::~commSend()
{
  dbg.exitBlock();
}

// Called to enable the block to print its entry and exit text
void commSend::printEntry(string loadCmd) {
  //cout << "commSend::printEntry("<<loadCmd<<")"<<endl;
  uniqueMark::printEntry(loadCmd);
  
  // If any receive IDs were provided, create an arrow to them to this send
  if(recvIDs!="") dbg.widgetScriptCommand(txt()<<"createUniqueMarkArrowFrom('"<<getBlockID()<<"',"<<recvIDs<<");");
  
  /*dbg.ownerAccessing();
  dbg << "Send blockID="<<getBlockID()<<", uniqueMark="<<allIDs<<", recvIDs="<<recvIDs;
  dbg.userAccessing();  */
}

void commSend::printExit() {
  uniqueMark::printExit();
}

void* commSendEnterHandler(properties::iterator props) { return new commSend(props); }
void  commSendExitHandler(void* obj) { commSend* s = static_cast<commSend*>(obj); delete s; }
 
/********************
 ***** commRecv *****
 ********************/

commRecv::commRecv(properties::iterator props) : uniqueMark(properties::next(props))
{
  // Load uniqueMark-specific JavaScript files into the final document
  if(!parJSInitialized) {
    // Create the directory that holds the parallel-specific scripts
    dbg.createWidgetDir("parallel");
    
    dbg.includeFile("parallel/canvasutilities.js"); dbg.includeWidgetScript("parallel/canvasutilities.js", "text/javascript");
    dbg.includeFile("parallel/parallel.js");        dbg.includeWidgetScript("parallel/parallel.js",        "text/javascript");
    
    // Add command to show arrows once all the unique marks have been registered
    dbg.widgetScriptEpilogCommand(txt()<<"showParallelArrows();");
    
    parJSInitialized=true;
  }

  // Read all the unique IDs of the receives the match this send from props and 
  // create edges from this send to these receives.
  int numIDs = props.getInt("numSendIDs");
  if(numIDs>0) {
    ostringstream sendIDsS;
    sendIDsS << "[";
    for(int i=0; i<numIDs; i++) {
      if(i>0) sendIDsS << ",";
      sendIDsS << "'"<<props.get(txt()<<"ID"<<i)<<"'";
    }
    sendIDsS << "]";
    sendIDs = sendIDsS.str();
  }
  
  dbg.enterBlock(this, /*newFileEntered*/ false, /*summaryEntry*/ false);
}

commRecv::~commRecv()
{
  dbg.exitBlock();
}

// Called to enable the block to print its entry and exit text
void commRecv::printEntry(string loadCmd) {
  //cout << "commRecv::printEntry("<<loadCmd<<")"<<endl;
  uniqueMark::printEntry(loadCmd);
  
  // If any send IDs were provided, create an arrow from them to this receive
  if(sendIDs!="") dbg.widgetScriptCommand(txt()<<"createUniqueMarkArrowTo('"<<getBlockID()<<"',"<<sendIDs<<");");
  
  /*dbg.ownerAccessing();
  dbg << "Recv blockID="<<getBlockID()<<", uniqueMark="<<allIDs<<", sendIDs="<<sendIDs;
  dbg.userAccessing();  */
}

void commRecv::printExit() {
  uniqueMark::printExit();
}

void* commRecvEnterHandler(properties::iterator props) { return new commRecv(props); }
void  commRecvExitHandler(void* obj) { commRecv* s = static_cast<commRecv*>(obj); delete s; }

/********************
 ***** commBar *****
 ********************/

commBar::commBar(properties::iterator props) : uniqueMark(properties::next(props))
{
  // Load uniqueMark-specific JavaScript files into the final document
  if(!parJSInitialized) {
    // Create the directory that holds the parallel-specific scripts
    dbg.createWidgetDir("parallel");
    
    dbg.includeFile("parallel/canvasutilities.js"); dbg.includeWidgetScript("parallel/canvasutilities.js", "text/javascript");
    dbg.includeFile("parallel/parallel.js");        dbg.includeWidgetScript("parallel/parallel.js",        "text/javascript");
    
    // Add command to show arrows once all the unique marks have been registered
    dbg.widgetScriptEpilogCommand(txt()<<"showParallelArrows();");
    
    parJSInitialized=true;
  }

  dbg.enterBlock(this, /*newFileEntered*/ false, /*summaryEntry*/ false);
}

commBar::~commBar()
{
  dbg.exitBlock();
}

// Called to enable the block to print its entry and exit text
void commBar::printEntry(string loadCmd) {
  //cout << "commBar::printEntry("<<loadCmd<<")"<<endl;
  uniqueMark::printEntry(loadCmd);
  
  // If any send IDs were provided, create an arrow from them to this receive
  assert(allIDs!="");
  dbg.widgetScriptCommand(txt()<<"createUniqueMarkArrowTo('"<<getBlockID()<<"',"<<allIDs<<");");
  
  /*dbg.ownerAccessing();
  dbg << "Recv blockID="<<getBlockID()<<", uniqueMark="<<allIDs<<", sendIDs="<<sendIDs;
  dbg.userAccessing();  */
}

void commBar::printExit() {
  uniqueMark::printExit();
}

void* commBarEnterHandler(properties::iterator props) { return new commBar(props); }
void  commBarExitHandler(void* obj) { commBar* s = static_cast<commBar*>(obj); delete s; }
}; // namespace layout
}; // namespace sight
