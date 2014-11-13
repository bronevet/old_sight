#pragma once

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "mfem.hpp"
#include "sight_structure.h"

namespace sight {
namespace structure {
class mfem: public block
{
 // The path the directory where output files of the graph widget are stored
  // Relative to current path
  static std::string outDir;
  // Relative to root of HTML document
  static std::string htmlOutDir;
    
  // Unique ID of this graph object
  int widgetID;
  
  // Maximum ID assigned to any graph object
  static int maxWidgetID;
  
  // The objects that encode the mesh and the corresponding solution on this mesh
  Mesh* mesh;
  GridFunction* soln;

  public:
  
  mfem(Mesh* mesh, GridFunction* soln, properties* props=NULL);
  mfem(Mesh* mesh, GridFunction* soln, const attrOp& onoffOp, properties* props=NULL);

  // Sets the properties of this object
  static properties* setProperties(Mesh* mesh, GridFunction* soln, int widgetID, const attrOp* onoffOp, properties* props);

  // Initialize the environment within which generated graphs will operate, including
  // the JavaScript files that are included as well as the directories that are available.
  static void initEnvironment();

  ~mfem();
  
  
  // Emits the given MFEM mesh and solution to the sight output
  static void emitMesh(Mesh* mesh, GridFunction* soln);
  
  // Called to notify this block that a sub-block was started/completed inside of it. 
  // Returns true of this notification should be propagated to the blocks 
  // that contain this block and false otherwise.
  bool subBlockEnterNotify(block* subBlock);
  bool subBlockExitNotify (block* subBlock);
};
} // namespace structure
} // namespace sight
