// Licence information included in file LICENCE
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

#include "mfem_structure.h"

using namespace std;

namespace sight {
namespace structure {
// The path the directory where output files of the mfem widget are stored
// Relative to current path
std::string mfem::outDir="";
// Relative to root of HTML document
std::string mfem::htmlOutDir="";

// Maximum ID assigned to any mfem object
int mfem::maxWidgetID=0;

mfem::mfem(Mesh* mesh, GridFunction* soln, properties* props) : 
    block("MFEM mesh", setProperties(mesh, soln, maxWidgetID, NULL, props)), mesh(mesh), soln(soln) {
  widgetID = maxWidgetID++;
}

mfem::mfem(Mesh* mesh, GridFunction* soln, const attrOp& onoffOp, properties* props) : 
    block("MFEM mesh", setProperties(mesh, soln, maxWidgetID, &onoffOp, props)), mesh(mesh), soln(soln) {
  widgetID = maxWidgetID++;
}

// Sets the properties of this object
properties* mfem::setProperties(Mesh* mesh, GridFunction* soln, int widgetID, const attrOp* onoffOp, properties* props)
{
  if(props==NULL) props = new properties();
    
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    
    initEnvironment();
    
    map<string, string> newProps;
    newProps["widgetID"] = widgetID;
    newProps["meshFName"] = txt()<<htmlOutDir<<"/mfem_"<<widgetID<<".mesh";
    newProps["solnFName"] = txt()<<htmlOutDir<<"/mfem_"<<widgetID<<".soln";

    props->add("mfem", newProps);
  }
  else
    props->active = false;
  return props;
}

// Initialize the environment within which generated graphs will operate, including
// the JavaScript files that are included as well as the directories that are available.
void mfem::initEnvironment() {
  static bool initialized=false;
  if(initialized) return;
  initialized = true;
  
  pair<string, string> paths = dbg.createWidgetDir("mfem");
  outDir = paths.first;
  htmlOutDir = paths.second;
  //cout << "outDir="<<outDir<<" htmlOutDir="<<htmlOutDir<<endl;
}

mfem::~mfem() {
  if(!props->active) return;
  
  string meshPath = txt()<<outDir<<"/mfem_"<<widgetID<<".mesh";
  string solnPath = txt()<<outDir<<"/mfem_"<<widgetID<<".soln";
  ofstream mesh_ofs(meshPath.c_str());
  mesh_ofs.precision(8);
  mesh->Print(mesh_ofs);
  ofstream sol_ofs(solnPath.c_str());
  sol_ofs.precision(8);
  soln->Save(sol_ofs);
}

// Emits the given MFEM mesh and solution to the sight output
void mfem::emitMesh(Mesh* mesh, GridFunction* soln) {
  mfem m(mesh, soln);
}

// Called to notify this block that a sub-block was started/completed inside of it. 
// Returns true of this notification should be propagated to the blocks 
// that contain this block and false otherwise.
bool mfem::subBlockEnterNotify(block* subBlock) { return true; }

bool mfem::subBlockExitNotify(block* subBlock) { return true; }
} // namespace structure
} // namespace sight
