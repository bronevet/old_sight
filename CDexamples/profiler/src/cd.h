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

#ifndef _CD_H 
#define _CD_H
#include "cd_global.h"
#include "cd_handle.h"
#include "cd_entry.h"
#include <list>
#include <vector>
#include <array>
#include <stdint.h>
//#include "util.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <setjmp.h>
#include <ucontext.h>
#include <unordered_map>

//class cd::CDEntry;
//class cd::CDHandle;
using namespace cd;

class cd::CD {
//    friend class cd::RegenObject;   
  public:
    // enumerators 
    enum CDExecutionMode  {kExecution=0, kReexecution, kSuspension};
    enum ContextPreservationMode {kExcludeStack=0, kIncludeStack};
    enum CDInternalErrT { kOK=0, kExecutionModeError };	
  //	enum ReturnValue { kOK = 0, kError};
  //	enum PreservationType { kCopy =0, kReference, kRegeneration };
    
  public:
    char* name_;
    std::string label_;
    ucontext_t context_;
    jmp_buf jump_buffer_;
    ContextPreservationMode context_preservation_mode_;
  protected: 
    CDID cd_id_;
//FIXME
//    CDID cd_id_parent_;
//
//  public:
//    cd::CDHandle cd_parent();
//    void set_cd_parent(cd::CDHandle parent);
    CDType              cd_type_;
    CDExecutionMode     cd_execution_mode_;
    uint64_t            sys_detect_bit_vector_;
    std::list<int*>     usr_detect_func_list_;  //custom_detect_func;
    uint64_t            option_save_context_; 

    // FIXME later this should be map or list, sometimes it might be better to have map structure 
    // and sometimes list would be beneficial depending on how frequently search functionality is required.
    // here define binary search tree structure... and then insert only the ones who has ref names.
    std::list<CDEntry> entry_directory_; 
    
    // only the CDEntries that has refname will be pushed into this data structure for later quick search.
    std::unordered_map<std::string, CDEntry> entry_directory_map_;   
    
    // This shall be used for re-execution. We will restore the value one by one.
    // Should not be CDEntry*. 
    std::list<CDEntry>::iterator iterator_entry_;   

    int num_reexecution_;
    // TODO: What if the pointer of the object has changed since it was created? 
    // This could be desaster if we are just relying on the address of the pointer to access CD object.
  
  
    //	std::vector<cd_log> log_directory_;
    //	pthread_mutex_t mutex_;			//
    //	pthread_mutex_t log_directory_mutex_;


  public:
    CD();
    CD(CDType cd_type, CDHandle &parent);
    CD(CDHandle* cd_parent, const char* name, CDID cd_id, CDModeT cd_type, uint64_t sys_bit_vector);

    void Init();
  
    //	int AddDetectFunc(void *() custom_detect_func);
    virtual ~CD();
  
    //TODO Implement serialize and deserialize of this instance
    CDID GetCDID() { return cd_id_; }

    // TODO This should be moved to outside and inside the namespace cd
//    static CDHandle Create(CDType cd_type, CDHandle &parent);
//    static void Destroy(CD *cd);
    CD* Create(CDHandle* cd_parent, 
               const char* name, 
               const CDID& cd_id, 
               CDModeT cd_type, 
               uint64_t sys_bit_vector, 
               CDErrT* cd_err);

    CDErrT Destroy();
    CDErrT Begin(bool collective=true, std::string label="");
    CDErrT Complete(bool collective=true, 
                    bool update_preservations=true);

//    CDErrT Peserve(int rank_id, 
//                   char* data, 
//                   int len_data, 
//                   enum PreserveType preserve_type, 
//                   enum MediumLevel medium_level);

//  	int Preserve(const void* data, uint64_t len_in_bytes); 

  //Jinsuk: Really simple preservation function. 
  //This will use default storage 
  //(can be set when creating this instance or later by calling set_ something function. 
  
    CDErrT Preserve(void *data,             // address in the local process 
                    uint64_t len_in_bytes,        // data size to preserve
                    uint32_t preserve_mask=kCopy, // preservation method
                    const char *my_name=0,        // data name
                    const char *ref_name=0,       // reference name
                    uint64_t ref_offset=0,        // reference offset
                    const RegenObject * regen_object=0, // regen object
                    PreserveUseT data_usage=kUnsure);   // for optimization
  
    // Collective version
    CDErrT Preserve(CDEvent &cd_event,            // Event object to synch
                    void *data_ptr, 
                    uint64_t len, 
                    uint32_t preserve_mask=kCopy, 
                    const char *my_name=0, 
                    const char *ref_name=0, 
                    uint64_t ref_offset=0, 
                    const RegenObject *regen_object=0, 
                    PreserveUseT data_usage=kUnsure);
  
    int Detect(); 
    int Restore();
  
//  DISCUSS: Jinsuk: About longjmp setjmp: By running some experiement, 
//  I confirm that this only works when stack is just there. 
//  That means, if we want to jump to a function context which does not exist in stack anymore, 
//  you will eventually get segfault. 
//  Thus, CD begin and complete should be in the same function context,
//  or at least complete should be deper in the stack so that we can jump back to the existing stack pointer. 
//  I think that Just always pairing them in the same function is much cleaner. 
  
    CDErrT Assert(bool test);
  
    int Reexecute();
  
    // Utilities -----------------------------------------------------
  
    virtual  int Stop();
    virtual  int Resume();
  
    CDInternalErrT InternalPreserve(void *data, 
                                    uint64_t len_in_bytes,
                                    uint32_t preserve_mask, 
                                    const char *my_name, 
                                    const char *ref_name, 
                                    uint64_t ref_offset, 
                                    const RegenObject * regen_object, 
                                    PreserveUseT data_usage);
  
    // copy should happen for the part that is needed.. 
    // so serializing entire CDEntry class does not make sense. 
    CDEntry InternalGetEntry(std::string entry_name); 
  
    // When Restore is called, it should be copied the only part ... smartly. 
    // It means we need to send a copy request to remote node. 
    // If it is in PGAS this might be a little easier to handle. 
    // For MPI, it could be a little tricky but anyways possible.
    // When it is serialized they should not do the deep copy for datahandle... 
    // so basically address information is transfered but not the actual data... 
    // FIXME for now let's just consider single process or single thread environment.. 
    // so we can always return CDEntry without worries. 
    // But later we will need to trasfer this object 
    // and then new CDEntry and then return with proper value. 
  

    void DeleteEntryDirectory(void);

    virtual int AddChild(CD* cd_child);
    virtual int RemoveChild(CD* cd_child);
    
    virtual void StartProfile(void);
    virtual void FinishProfile(void);
    virtual void FinalizeViz(void);
#if _PROFILER

    static graph* scopeGraph;
    static modularApp* ma;



    //void InitProfile(std::string label="");
    virtual void GetLocalAvg(void);
    virtual void GetPrvData(void *data, 
                            uint64_t len_in_bytes,
                            uint32_t preserve_mask, 
                            const char *my_name, 
                            const char *ref_name, 
                            uint64_t ref_offset, 
                            const RegenObject * regen_object, 
                            PreserveUseT data_usage);

    virtual void AddUsrProfile(std::string key, long val, int mode);
    virtual void InitViz();

// FIXME
    virtual bool CheckCollectProfile(void);
    virtual void SetCollectProfile(bool flag);
#endif
 };









class cd::MasterCD : public cd::CD {
  public:
    // Link information of CD hierarchy   
    // This is important data for MASTER among sibling mpi ranks (not sibling CDs) of each CD 
    // Therefore, CDTree is generated with CD object. 
    // parent CD object is always in the same process.
    // But MASTER's parent CD object is in the MASTER process of parent CD.
    // CDHandle cd_parent_ should be an accessor for this MASTER process.
    // Regarding children CDs, this MASTER's CD has actual copy of the MASTER children's CD object.
    // If children CD is gone, this MASTER CD sends children CD.
    // So, when we create CDs, we should send MASTER CDHandle and its CD to its parent CD
    std::list<cd::CDHandle*> cd_children_;
    cd::CDHandle*            cd_parent_;


#if _PROFILER

    uint64_t  sibling_id_;
    uint64_t  level_;
  
    /// Profile-related meta data
    std::map<std::string, std::array<uint64_t, MAX_PROFILE_DATA>> profile_data_;
    bool     is_child_destroyed;
    bool     collect_profile_;
    bool usr_profile_enable;
//    std::vector<std::pair<std::string, long>>  usr_profile;
    context usr_profile_input;
    context usr_profile_output;
    /// Timer-related meta data
    uint64_t this_point_;
    uint64_t that_point_;
  
    /// sight-related member data
    /// All scopes that are currently live
    static std::list<scope*> sStack;
//    static graph* scopeGraph;
    
    /// All modules that are currently live
    static std::list<module*> mStack;
//    static modularApp* ma;

    static std::list<comparison*> compStack;
 
    /// All modules that are currently live
//    static std::list<CDNode*> cdStack;

    void InitProfile(std::string label="INITIAL_LABEL");

    virtual void GetLocalAvg(void);
    virtual void GetPrvData(void *data, 
                            uint64_t len_in_bytes,
                            uint32_t preserve_mask, 
                            const char *my_name, 
                            const char *ref_name, 
                            uint64_t ref_offset, 
                            const RegenObject * regen_object, 
                            PreserveUseT data_usage);

    virtual void AddUsrProfile(std::string key, long val, int mode);

    virtual void InitViz();
// FIXME
    virtual bool CheckCollectProfile(void);
    virtual void SetCollectProfile(bool flag);



  void CreateCDNode(void);
  void CreateScope(void);
  void CreateModule(void);
  void CreateComparison(void);
  void DestroyCDNode(void);
  void DestroyScope(void);
  void DestroyModule(void);
  void DestroyComparison(void);
#endif








    MasterCD();
    MasterCD( CDHandle* cd_parent, 
              const char* name, 
              CDID cd_id, 
              CDModeT cd_type, 
              uint64_t sys_bit_vector);
    virtual ~MasterCD();
   
    virtual void StartProfile(void);
    virtual void FinishProfile(void);
    virtual void FinalizeViz(void);
    virtual int Stop(CDHandle cd);
    virtual int Resume(); // Does this make any sense?
    virtual int AddChild(CDHandle* cd_child); 
    virtual int RemoveChild(CDHandle* cd_child); 
    CDHandle* cd_parent();
    void set_cd_parent(CDHandle* cd_parent);
};

namespace cd {
//  extern CDHandle* GetCurrentCD(void);
//  extern CDHandle* GetRootCD(void);
//
//  extern CDHandle* CD_Init(int numproc, int myrank);
//  extern void CD_Finalize();
}

#endif
