#pragma once

#include "../../sight_common_internal.h"

namespace sight {
  
namespace structure {
  class graph;
}

namespace layout {
  class graph;
}

namespace common {

template<class anchor>
class graphEdge {
  anchor from;
  anchor to;
  bool directed;
  
  friend class sight::layout::graph;
  friend class sight::structure::graph;
  
  public:
  graphEdge(anchor from, anchor to, bool directed) :
    from(from), to(to), directed(directed)
  {}
  
  const anchor& getFrom() const { return from; }
  const anchor& getTo()   const { return to; }
  
  bool operator==(const graphEdge& that) const {
    return (from == that.from);
  }
  bool operator<(const graphEdge& that) const {
    return (from < that.from) ||
           (from == that.from && to < that.to) ||
           (from == that.from && to == that.to && directed < that.directed);
  }
};

  
} // namespace common
} // namespace sight
