#ifndef _NODE_ID_H
#define _NODE_ID_H
#include "cd_global.h"
#include <ostream>
#include <utility>
namespace cd {

class NodeID 
{
  public:
  int color_;
  int task_;
  int size_;
  uint64_t sibling_id_;
  NodeID() 
    : color_(-1), task_(-1), size_(-1), sibling_id_(0) 
  {}
  NodeID(int color, int task, int size, uint64_t sibling_id)
    : color_(color), task_(task), size_(size), sibling_id_(sibling_id)
  {}
  NodeID(const NodeID& that)
    : color_(that.color_), task_(that.task_), size_(that.size_), sibling_id_(that.sibling_id_)
  {}
  NodeID(NodeID&& that)
    : task_(that.task_), size_(that.size_), sibling_id_(sibling_id_)
  {
    color_ = std::move(that.color_);
  }
  NodeID& operator=(const NodeID& that) {
    color_      = (that.color_);
    task_       = (that.task_);
    size_       = (that.size_);
    sibling_id_ = that.sibling_id_;
  }
  
};

std::ostream& operator<<(std::ostream& str, const NodeID& node_id);

}
#endif
