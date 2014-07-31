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

#ifndef _CD_GLOBAL_H
#define _CD_GLOBAL_H
#include <cstdint>
#include <vector>
#include <map>
#include <iostream>
//#include "cd_id.h"
//#include "node_id.h"
namespace cd {
  class CD;
  class MasterCD;
  class CDHandle;
  class CDEntry;
  class DataHandle;
  class Serializable;
  class NodeID;
  class CDID;
  class Regen;
  class Packer; 
  class Unpacker; 
  class Util;
  class CDEvent;
  class RegenObject;	
  class RecoverObject;

  enum CDErrT       { kOK=0, kAlreadyInit, kError };
  enum SysErrT      { kWhat=1 };
  enum SysErrLoc    { kIntraCore=1, kCore=2, kProc=4, kNode=8, kModule=16, kCabinet=32, kCabinetGroup=64, kSystem=128 };
  enum SysErrName   { kSoftMem=1, kDegradedMem=2, kSoftComm=4, kDegradedComm=8, kSoftComp=16, kDegradedResource=32, kHardResource=64, kFileSys=128 };
  enum CDPGASUsageT { kShared=1, KPrivate };
  enum CDPreserveT  { kCopy=0, kReference };
  enum CDType       { kStrict=0, kRelaxed };
  enum PreserveUseT { kUnsure=0, kReadOnly=1, kReadWrite=2 };

	/// Profile-related enumerator
	enum ProfileType      { LOOP_COUNT, EXEC_CYCLE, PRV_COPY_DATA, PRV_REF_DATA, OVERLAPPED_DATA, SYSTEM_BIT_VECTOR, MAX_PROFILE_DATA };
	enum ProfileFormat    { PRV, REC, BODY, MAX_FORMAT };

  //typedef CDID CDName;
  typedef CDType CDModeT;
  class CDNameT;

  // Local CDHandle object and CD object are managed by CDPath (Local means the current process)
  extern std::vector<CDHandle*> CDPath;
  extern std::vector<MasterCD*> MasterCDPath;
  extern bool is_visualized;
  extern int myTaskID;
  // This could be different from MPI program to PGAS program
  // key is the unique ID from 0 for each CD node.
  // value is the unique ID for mpi communication group or thread group.
  // For MPI, there is a communicator number so it is the group ID,
  // For PGAS, there should be a group ID, or we should generate it. 
  extern std::map<uint64_t, int> nodeMap;
  extern int status;
  extern void SetStatus(int flag);
  

  extern CDHandle* GetCurrentCD(void);
  extern CDHandle* GetRootCD(void);

  extern CDHandle* CD_Init(int numproc, int myrank);
  extern void CD_Finalize();
}



#define DATA_MALLOC malloc
#define DATA_FREE free
#define ERROR_MESSAGE(X) printf(X);
#define MAX_FILE_PATH 2048
/*
#define CD_BEGIN(X) (X)->Begin(); \
                    if((X)->context_preservation_mode_ ==CD::kExcludeStack) setjmp((X)->jump_buffer_); \
                    else getcontext(&(X)->context_); \
                    (X)->CommitPreserveBuff()

#define CD_COMPLETE(X) (X)->Complete()

//#define CD_BEGIN(X) (X).Begin(); \
//if((X).context_preservation_mode() ==CD::kExcludeStack) setjmp((X).jump_buffer_);  \
//else getcontext(&(X).context_) ; \
//(X).CommitPreserveBuff()
//#define CD_COMPLETE(X) (X).Complete()   

//#define CD_BEGIN(X) (X).Begin(); if((X).context_preservation_mode() ==CD::kExcludeStack) setjmp(*(X).jump_buffer()); else getcontext((X).context()) 
//#define CD_COMPLETE(X) (X).Complete()   
*/
#endif
