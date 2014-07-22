#include "cd_id.h"
#include "node_id.h"
using namespace cd;

uint64_t CDID::object_id_ = 0;

CDID::CDID() // TODO Initialize member values to zero or something, 
             //for now I will put just zero but this is less efficient.
//  : object_id_(CDID::object_id_)
{
  domain_id_     = 0; 
  level_         = 0;
  sibling_id_    = 0;
//      task_id_       = 0;
  sequential_id_ = 0;
}

CDID::CDID(uint64_t level, const NodeID& new_node_id)
//  : object_id_(cd::object_id)
{
  domain_id_     = 0; 
  level_         = level;
  sibling_id_    = 0;
  node_id_.color_ = new_node_id.color_;
  sequential_id_ = 0;
}

CDID::CDID(const CDID& that)
//  : object_id_(that.object_id_)
{
  domain_id_      = that.domain_id_;
  level_         = that.level_;
  sibling_id_    = 0;
//  node_id_       = that.node_id_;
  sequential_id_ = that.sequential_id_;
}
// should be in CDID.h
// old_cd_id should be passed by value
void CDID::UpdateCDID(uint64_t parent_level, const NodeID& new_node_id)
{
  level_ = parent_level++;
  object_id_++;
//  node_id_ = new_node_id;
  sequential_id_ = 0;
}

void CDID::SetCDID(const NodeID& new_node_id)
{
  node_id_ = new_node_id;
}

CDID& CDID::operator=(const CDID& that)
{
  domain_id_      = that.domain_id_;
  level_         = that.level_;
//  node_id_       = that.node_id_;
  object_id_     = that.object_id_;
  sequential_id_ = that.sequential_id_;
  return *this;
}


std::ostream& operator<<(std::ostream& str, const CDID& cd_id)
{
  return str<< "Level: "<< cd_id.level_ << ", CDNode" << cd_id.node_id_.color_ << ", Obj# " << cd_id.object_id_ << ", Seq# " << cd_id.sequential_id_;
}

//std::ostream& operator<<(std::ostream& str, const NodeID& node_id)
//{
//  return str<< '(' << node_id.color_ << ", " << node_id.task_ << "/" << node_id.size_ << ')';
//}
