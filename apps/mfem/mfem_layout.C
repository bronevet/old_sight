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

#include "mfem_layout.h"

using namespace std;

namespace sight {
namespace layout {

// Record the layout handlers in this file
void* mfemEnterHandler(properties::iterator props) { return new mfem(props); }
void  mfemExitHandler(void* obj) { mfem* m = static_cast<mfem*>(obj); delete m; }
  
mfemLayoutHandlerInstantiator::mfemLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["mfem"] = &mfemEnterHandler;
  (*layoutExitHandlers) ["mfem"] = &mfemExitHandler;
}
mfemLayoutHandlerInstantiator mfemLayoutHandlerInstance;

mfem::mfem(properties::iterator props) : block(properties::next(props)) {
  #if REMOTE_ENABLED
  dbg.ownerAccessing();
  string divName = txt()<<"mfem_mesh_container_"<<properties::getInt(props, "widgetID");
  
  dbg << "<iframe id=\""<<divName<<"\" width=0 height=0></iframe>\n";
  
//    dbg << "<a href=\"#\" onclick=\"javascript:setGDBLink(this, ':"<<GDB_PORT<<"/apps/mfem/mfemwrap.cgi?HOME="<<getenv("HOME")<<"&USER="<<getenv("USER")<<"&execFile="<<ROOT_PATH<<"/apps/mfem/glvis/glvis&mesh="<<meshPath<<"&soln="<<solnPath<<"')\"\">Mesh</a>\n";

  dbg << "<a href=\"#\" onclick=\""<<
           "javascript:document.getElementsByTagName('iframe')['"<<divName<<"'].width=1024; "<<
                      "document.getElementsByTagName('iframe')['"<<divName<<"'].height=768; "<<
                      "document.getElementsByTagName('iframe')['"<<divName<<"'].src=getHostLink"<<
                           "(':"<<GDB_PORT<<"/apps/mfem/mfemwrap.cgi?HOME="<<getenv("HOME")<<"&"<<
                                                                    "USER="<<getenv("USER")<<"&"<<
                                                                    "execFile="<<ROOT_PATH<<"/apps/mfem/glvis/glvis&"<<
                                                                    "mesh="<<dbg.getWorkDir()<<"/html/"<<properties::get(props, "meshFName")<<"&"<<
                                                                    "soln="<<dbg.getWorkDir()<<"/html/"<<properties::get(props, "solnFName")<<"'); this.innerHTML=''; return false;\">Mesh</a>"<<endl;
  dbg.userAccessing();
  #endif
}

mfem::~mfem() {
}

// Called to notify this block that a sub-block was started/completed inside of it. 
// Returns true of this notification should be propagated to the blocks 
// that contain this block and false otherwise.
bool mfem::subBlockEnterNotify(block* subBlock) { return true; }

bool mfem::subBlockExitNotify(block* subBlock) { return true; }
} // namespace layout
} // namespace sight
