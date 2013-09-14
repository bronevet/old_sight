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

#include "mfem.h"

using namespace std;

namespace dbglog {

// The path the directory where output files of the mfem widget are stored
// Relative to current path
std::string mfem::outDir="";
// Relative to root of HTML document
std::string mfem::htmlOutDir="";

// Maximum ID assigned to any mfem object
int mfem::maxWidgetID=0;

mfem::mfem(Mesh* mesh, GridFunction* soln) : block("MFEM mesh"), mesh(mesh), soln(soln) {
  // If the current attribute query evaluates to true (we're emitting debug output)
  if(attributes.query())
    init();
  else
    active = false;
}

mfem::mfem(Mesh* mesh, GridFunction* soln, const attrOp& onoffOp) : block("MFEM mesh"), mesh(mesh), soln(soln) {
  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes.query() && onoffOp.apply())
    init();
  else
    active = false;
}

void mfem::init() {
  active = true;
  
  dbg.enterBlock(this, false, true);
  
  initEnvironment();  
    
  widgetID = maxWidgetID;
  maxWidgetID++;
  
  mfemOutput = false;
  
  dbg.ownerAccessing();
  dbg << "<iframe id=\"mfem_mesh_container_"<<widgetID<<"\" width=0 height=0></iframe>\n";
  //dbg << "<div id=\"debug_output\"></div>\n";
  dbg.userAccessing();
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
  if(!active) return;
  
  if(!mfemOutput) {
    string meshPath = txt()<<outDir<<"/mfem_"<<widgetID<<".mesh";
    string solnPath = txt()<<outDir<<"/mfem_"<<widgetID<<".soln";
    ofstream mesh_ofs(meshPath.c_str());
    mesh_ofs.precision(8);
    mesh->Print(mesh_ofs);
    ofstream sol_ofs(solnPath.c_str());
    sol_ofs.precision(8);
    soln->Save(sol_ofs);
  
    dbg.ownerAccessing();
//    dbg << "<a href=\"#\" onclick=\"javascript:setGDBLink(this, ':"<<GDB_PORT<<"/apps/mfem/mfemwrap.cgi?HOME="<<getenv("HOME")<<"&USER="<<getenv("USER")<<"&execFile="<<ROOT_PATH<<"/apps/mfem/glvis/glvis&mesh="<<meshPath<<"&soln="<<solnPath<<"')\"\">Mesh</a>\n";
    string divName = txt()<<"mfem_mesh_container_"<<widgetID;
    dbg << "<a href=\"#\" onclick=\""<<
             "javascript:document.getElementsByTagName('iframe')['"<<divName<<"'].width=1024; "<<
                        "document.getElementsByTagName('iframe')['"<<divName<<"'].height=768; "<<
                        "document.getElementsByTagName('iframe')['"<<divName<<"'].src=getHostLink(':"<<GDB_PORT<<"/apps/mfem/mfemwrap.cgi?HOME="<<getenv("HOME")<<"&USER="<<getenv("USER")<<"&execFile="<<ROOT_PATH<<"/apps/mfem/glvis/glvis&mesh="<<meshPath<<"&soln="<<solnPath<<"'); this.innerHTML=''; return false;\">Mesh</a>"<<endl;
    dbg.userAccessing();
  }
  
  dbg.exitBlock();
}

// Emits the given MFEM mesh and solution to the dbglog output
void mfem::emitMesh(Mesh* mesh, GridFunction* soln) {
  mfem m(mesh, soln);
}

// Called to notify this block that a sub-block was started/completed inside of it. 
// Returns true of this notification should be propagated to the blocks 
// that contain this block and false otherwise.
bool mfem::subBlockEnterNotify(block* subBlock) { return true; }

bool mfem::subBlockExitNotify(block* subBlock) { return true; }

} // namespace dbglog
