/*
Copyright 2014, The University of Texas at Austin 
All rights reserved.

THIS FILE IS PART OF THE CONTAINMENT DOMAINS RUNTIME LIBRARY

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met: 

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer. 

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution. 

3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _CD_HANDLE_H 
#define _CD_HANDLE_H
#include "cd_global.h"
#include "node_id.h"
//#include "cd_name_t.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <setjmp.h>
#include <ucontext.h>
#include "sight.h"
//#include "../../../../../blcr/blcr-0.8.5/include/libcr.h"


// CDHandle is a global accessor to the CD object. 
// CDHandle must be valid regardless of MPI rank or threads
// That means that once I know CDHandle object, 
// I can access to the ptr_cd_ anyhow.
// Sometimes, CDHandle::node_id_.second and CDID::node_id_.second can be different. 
// (If node_id_.first is different, that means there is something wrong!)
// That means that CD object was newly given from somewhere,
// and in that case, we should be careful of synchorizing the CD object to CDHandle
using namespace cd;
using namespace sight;

class cd::CDHandle {
//  friend class cd::RegenObject;
  friend class cd::CD;

  private:
    CD*  ptr_cd_;

    // FIXME
    // In MPI context,
    // first : Communicator ID
    // second: ID in the current Communicator
    NodeID node_id_; 
    bool IsMaster_; 
    /// CD hierarchy information (level_ is important)
    /// We should think about why we put level_ here.
//    CDHandle* parent_cd_;
//    int task_id_;
//    int process_id_; 
//    int current_task_id_;
//    int current_process_id_;
    // pid_t is OS dependent. So let's use int instead
    //pid_t process_id_; 
  
  public:
#if _PROFILER
    uint64_t  sibling_id_;
    uint64_t  level_;
  
    /// Profile-related meta data
    uint64_t profile_data_[MAX_PROFILE_DATA];
    bool     is_child_destroyed;
    
    /// Timer-related meta data
    uint64_t this_point_;
    uint64_t that_point_;
  
    /// sight-related member data
    /// All scopes that are currently live
    static std::list<scope*> sStack;
    static graph* scopeGraph;
    
    /// All modules that are currently live
    static std::list<module*> mStack;
    static modularApp* ma;
  
    /// All modules that are currently live
//    static std::list<CDNode*> cdStack;
#endif

    //TODO copy these to CD async 
    ucontext_t context_;
    jmp_buf    jump_buffer_;
  
    CDHandle();

    CDHandle( CDHandle* parent, 
              const char* name, 
              const NodeID& node_id, 
              CDModeT cd_type, 
              uint64_t sys_bit_vector);
    CDHandle( CDHandle* parent, 
              const char* name, 
              NodeID&& node_id, 
              CDModeT cd_type, 
              uint64_t sys_bit_vector);

    void Init(CD* ptr_cd, NodeID node_id); 
   ~CDHandle(); 
    char*   GetName(); // for now just return the CDID
    CDHandle* GetParent();

  // API List 0.1
  // Should Create/Destroy, Begin/Complete be static??

    // Non-collective
    CDHandle* Create (const char* name=0, 
                      CDModeT type=kStrict, 
                      uint32_t error_name_mask=0, 
                      uint32_t error_loc_mask=0, 
                      CDErrT *error=0 );
  
    // Collective
    CDHandle* Create (uint32_t color=0, 
                      uint32_t num_tasks_in_color=0, 
                      const char* name=0, 
                      CDModeT type=kStrict, 
                      uint32_t error_name_mask=0, 
                      uint32_t error_loc_mask=0, 
                      CDErrT *error=0 );
     // Collective
    CDHandle* Create (uint32_t  numchildren=0,
                      const char* name=0, 
                      CDModeT type=kStrict, 
                      uint32_t error_name_mask=0, 
                      uint32_t error_loc_mask=0, 
                      CDErrT *error=0 );
 
    CDHandle* CreateAndBegin (uint32_t color=0, 
                              uint32_t num_tasks_in_color=0, 
                              const char* name=0, 
                              CDModeT type=kStrict, 
                              uint32_t error_name_mask=0, 
                              uint32_t error_loc_mask=0, 
                              CDErrT *error=0 );

    CDErrT Destroy (bool collective=false);
    
    void SetColorAndTask(NodeID& new_node, const int& numchildren);
    
    CDErrT Begin   (bool collective=true, const char* label=0);
    CDErrT Complete(bool collective=true, bool update_preservations=false);
  
    CDErrT Preserve ( void *data_ptr=0, 
                      uint64_t len=0, 
                      uint32_t preserve_mask=kCopy, 
                      const char *my_name=0, 
                      const char *ref_name=0, 
                      uint64_t ref_offset=0, 
                      const RegenObject *regen_object=0, 
                      PreserveUseT data_usage=kUnsure );
  
    CDErrT Preserve ( CDEvent &cd_event, 
                      void *data_ptr=0, 
                      uint64_t len=0, 
                      uint32_t preserve_mask=kCopy, 
                      const char *my_name=0, 
                      const char *ref_name=0, 
                      uint64_t ref_offset=0, 
                      const RegenObject *regen_object=0, 
                      PreserveUseT data_usage=kUnsure );
  
//    CDErrT Preserve ( void *data_ptr, 
//                      uint64_t len, 
//                      uint32_t preserve_mask=kCopy, 
//                      const char *ref_name=0, 
//                      uint64_t ref_offset=0, 
//                      const Regen *regen_object=0);
//    CDErrT Preserve ( CDEvent &cd_event, 
//                      void *data_ptr, 
//                      uint64_t len, 
//                      uint32_t preserve_mask=kCopy, 
//                      const char *ref_name=0, 
//                      uint64_t ref_offset=0, 
//                      const Regen *regen_object=0);

// if Regen were to registered from a remote node to a actual CD Object, 
// it will need to serialize the Regen object and then finally send the object wait... 
// we can't send the "binary" to the remote node.. this will be too much to support... 
// so we have to always assume this preservation happens local...  
// Basically preserve function will get called from local..
  
    CDErrT CDAssert( bool test_true, 
                     const SysErrT *error_to_report=0);

    CDErrT CDAssertFail( bool test_true, 
                         const SysErrT *error_to_report=0);

    CDErrT CDAssertNotify( bool test_true, 
                           const SysErrT *error_to_report=0);
  
    std::vector< SysErrT > Detect (CDErrT *err_ret_val=0);

    CDErrT RegisterDetection( uint32_t system_name_mask, 
                              uint32_t system_loc_mask);

    CDErrT RegisterRecovery( uint32_t error_name_mask, 
                             uint32_t error_loc_mask, 
                             RecoverObject *recover_object=0);
  
    CDErrT RegisterRecovery( uint32_t error_name_mask, 
                             uint32_t error_loc_mask, 
                             CDErrT(*recovery_func)(std::vector< SysErrT > errors)=0);

    float GetErrorProbability (SysErrT error_type, 
                               uint32_t error_num);

    float RequireErrorProbability ( SysErrT error_type, 
                                    uint32_t error_num, 
                                    float probability, 
                                    bool fail_over=true );
    virtual void Recover (uint32_t error_name_mask, 
                          uint32_t error_loc_mask, 
                          std::vector< SysErrT > errors);

    CDErrT SetPGASType (void *data_ptr, 
                        uint64_t len, 
                        CDPGASUsageT region_type=kShared);
  
// Internal use -------------------------------------------------------------
//    cd::CDEntry* InternalGetEntry(std::string entry_name);
    void CommitPreserveBuff();
  protected:
    void InternalReexecute ();
    void InternalEscalate ( uint32_t error_name_mask, 
                            uint32_t error_loc_mask, 
                            std::vector< SysErrT > errors);
    // It returns sibling ID
    // type can be just int because color and task are int type
    CDErrT AddChild(CD* cd_child);
    CDErrT RemoveChild(CD* cd_child);	

//    int StopAllChildren();
    int Stop();
  
    /// Synchronize the CD object in every task of that CD.
    bool Sync();

    uint64_t SetSystemBitVector(uint64_t error_name_mask, uint64_t error_loc_mask);
  public:
    /// Do we need this?
    bool IsLocalObject(void);

    /// Check if this CD object (*ptr_cd_) is the MASTER CD object
    bool IsMaster(void);

    void SetMaster(int task);
    // Accessors
    CD*    ptr_cd(); 
    NodeID& node_id();  
    void   SetCD(CD* ptr_cd);
    int&    GetNodeID();
    int    GetTaskID();
    int    GetTaskSize();
    int    context_preservation_mode();
    bool   operator==(const CDHandle &other) const ;
};

namespace cd {
  extern CDHandle* GetCurrentCD(void);
  extern CDHandle* GetRootCD(void);

  extern CDHandle* CD_Init(int numproc, int myrank);
  extern void CD_Finalize();
}
#endif
