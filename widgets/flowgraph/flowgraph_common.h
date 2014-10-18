#pragma once

#include "../../sight_common_internal.h"

namespace sight {
  
namespace structure {
  class flowgraph;
}

namespace layout {
  class flowgraph;
}

namespace common {

template<class anchor>
class flowgraphEdge {
  anchor from;
  anchor to;
  bool directed;
  bool visible;
  
  friend class sight::layout::flowgraph;
  friend class sight::structure::flowgraph;
  
  public:
  flowgraphEdge(anchor from, anchor to, bool directed, bool visible) :
    from(from), to(to), directed(directed), visible(visible)
  {}
  
  const anchor& getFrom() const { return from; }
  const anchor& getTo()   const { return to; }
  
  bool operator==(const flowgraphEdge& that) const {
    return (from == that.from);
  }
  bool operator<(const flowgraphEdge& that) const {
    return (from < that.from) ||
           (from == that.from && to < that.to) ||
           (from == that.from && to == that.to && directed < that.directed);
  }
};

} // namespace common
} // namespace sight
