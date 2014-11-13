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

class parallelLayoutHandlerInstantiator  : layoutHandlerInstantiator{
  public:
  parallelLayoutHandlerInstantiator();
};
extern parallelLayoutHandlerInstantiator parallelLayoutHandlerInstance;

class commSend: public uniqueMark
{
  std::string recvIDs;
  public:
  commSend(properties::iterator props);
  public:
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return false; }
  bool subBlockExitNotify (block* subBlock) { return false; }
  
  // Called to enable the block to print its entry and exit text
  void printEntry(std::string loadCmd);
  void printExit();
  
  ~commSend();
}; // commSend 
void* commSendEnterHandler(properties::iterator props);
void  commSendExitHandler(void* obj);

class commRecv: public uniqueMark
{
  std::string sendIDs;
  public:
  commRecv(properties::iterator props);
  public:
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return false; }
  bool subBlockExitNotify (block* subBlock) { return false; }
  
  // Called to enable the block to print its entry and exit text
  void printEntry(std::string loadCmd);
  void printExit();
  
  ~commRecv();
}; // commRecv
void* commRecvEnterHandler(properties::iterator props);
void  commRecvExitHandler(void* obj);

class commBar: public uniqueMark
{
  std::string barIDs;
  public:
  commBar(properties::iterator props);
  public:
    
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock) { return false; }
  bool subBlockExitNotify (block* subBlock) { return false; }
  
  // Called to enable the block to print its entry and exit text
  void printEntry(std::string loadCmd);
  void printExit();
  
  ~commBar();
}; // commBar
void* commBarEnterHandler(properties::iterator props);
void  commBarExitHandler(void* obj);
}; // namespace layout
}; // namespace sight
