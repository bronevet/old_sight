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

#ifndef _CD_ID_H
#define _CD_ID_H
#include "cd_global.h"
#include "node_id.h"
using namespace cd;

namespace cd{

extern uint64_t object_id;

class CDID {
  public:
    uint64_t domain_id_;          // Some physical information is desired in CDID to maximize locality when needed
    uint64_t level_;       // Level in the CD hierarhcy. It increases at Create() and destroys at Destroy.
    uint64_t sibling_id_;
    NodeID   node_id_; // Unique ID for each CD. It can be a communicator number. It increases at Create().
                                  // node_id_.first means node (color)
                                  // node_id_.second means ID in that node (color)
                                  // For now, node_id_.second==0 is always MASTER.
                                  // But it can be more nicely managed distribuing MASTER for one process.
//    static uint64_t task_id_;     // MPI rank ID / Thread ID. It is not increased at runtime but given at Init() in the beginning.
    static uint64_t object_id_;   // This will be local and unique within a process. It increases at creator or Create().
    uint64_t sequential_id_;      // # of Begin/Complete pairs of each CD object. It increases at Begin()
    
    CDID(); // TODO Initialize member values to zero or something, for now I will put just zero but this is less efficient.

    CDID(uint64_t level, const NodeID& new_node_id);

    CDID(const CDID& that);
    // should be in CDID.h
    // old_cd_id should be passed by value
    void UpdateCDID(uint64_t parent_level, const NodeID& new_node_id);

    void SetCDID(const NodeID& new_node_id);

    CDID& operator=(const CDID& that);
};


//std::ostream& operator<<(std::ostream& str, const NodeID& node_id);
std::ostream& operator<<(std::ostream& str, const CDID& cd_id);

}

#endif 
