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

class clockLayoutHandlerInstantiator  : layoutHandlerInstantiator{
  public:
  clockLayoutHandlerInstantiator();
};
extern clockLayoutHandlerInstantiator clockLayoutHandlerInstance;

}; // namespace layout
}; // namespace sight
