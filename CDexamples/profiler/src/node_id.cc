#include "node_id.h"

using namespace cd;

std::ostream& operator<<(std::ostream& str, const NodeID& node_id)
{
  return str<< '(' << node_id.color_ << ", " << node_id.task_ << "/" << node_id.size_ << ')';
}
