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
#include <assert.h>
#include <utility>
#include <math.h>
#include "cd_handle.h"

//#include "util.h"
#include "cd_id.h"
#include "cd_entry.h"
#include "node_id.h"
#include "cd.h"

#include <mpi.h>

#if _PROFILER_OLD
#include "rdtsc.h"
#include "sight.h"
using namespace sight;
#endif

//#include "cd_name_t.h"
using namespace cd;
using namespace std;

ostream& cd::operator<<(ostream& str, const NodeID& node_id)
{
  return str<< '(' << node_id.color_ << ", " << node_id.task_ << "/" << node_id.size_ << ')';
}

std::map<uint64_t, int> cd::nodeMap;
std::vector<CDHandle*> cd::CDPath;
int cd::status = 0;

int&    CDHandle::GetNodeID(void)  { return node_id_.color_; }
int     CDHandle::GetTaskID(void)  { return node_id_.task_;  }
int     CDHandle::GetTaskSize(void){ return node_id_.size_;  }
CD*     CDHandle::ptr_cd(void)     { return ptr_cd_;         }
NodeID& CDHandle::node_id(void)    { return node_id_;        }
void    CDHandle::SetCD(CD* ptr_cd){ ptr_cd_=ptr_cd;         }

bool CDHandle::operator==(const CDHandle &other) const 
{
  bool ptr_cd = (other.ptr_cd_ == this->ptr_cd_);
  bool color  = (other.node_id_.color_  == this->node_id_.color_);
  bool task   = (other.node_id_.task_   == this->node_id_.task_);
  bool size   = (other.node_id_.size_   == this->node_id_.size_);
  return (ptr_cd && color && task && size);
}

void cd::SetStatus(int flag)
{
  cd::status = flag;
}

#if _PROFILER_OLD
list<scope*>      CDHandle::sStack;
list<module*>     CDHandle::mStack;
//list<CDNode*>   CDHandle::cdStack;
list<comparison*> CDHandle::compStack;

modularApp*       CDHandle::ma;
graph*            CDHandle::scopeGraph;
#endif

CDHandle* cd::CD_Init(int numproc, int myrank)
{
  /// Create the data structure that CD object and CDHandle object are going to use.
  /// For example, CDEntry object or CDHandle object or CD object itself.
  /// These objects are very important data for CDs and 
  /// managed separately from the user-level data structure
  /// All the meta data objects and preserved data are managed internally.
  /// Register Root CD
 
  cout<<"CD_Init, MPI_COMM_WORLD : "<<MPI_COMM_WORLD<<endl;
 
  CDHandle* root_cd = new CDHandle(NULL, "Root", NodeID(MPI_COMM_WORLD, myrank, numproc, 0), kStrict, 0);

  CDPath.push_back(root_cd);
//  if(CDPath.empty())
//    cout<<"huh??empty?"<<endl;
//  else
//    cout<<"not empty but ?"<<endl;

#if _PROFILER
  SightInit(txt()<<"CDs", txt()<<"dbg_CDs_"<<myrank);

  /// Title
  dbg << "<h1>Containment Domains Profiling and Visualization</h1>" << endl;

  /// Explanation
  dbg << "Some explanation on CD runtime "<<endl<<endl;

  /// modularApp exists only one, and is created at init stage
    root_cd->ptr_cd()->SetViz();
//    root_cd->ptr_cd()->ma = new modularApp("CD Modular App");
//  module* m   = new module(instance("Root", 1, 0), 
//                           inputs(port(context("sequential_id", (int)(root_cd->node_id().task_)))),
//                           namedMeasures("time", new timeMeasure()));
//  root_cd->mStack.push_back(m);

    /// graph exists only one. It is created at init stage
//    root_cd->scopeGraph = new graph();
#else
#if _PROFILER_OLD

  SightInit(txt()<<"CDs", txt()<<"dbg_CDs_"<<myrank);

  /// Title
  dbg << "<h1>Containment Domains Profiling and Visualization</h1>" << endl;

  /// Explanation
  dbg << "Some explanation on CD runtime "<<endl<<endl;

  /// modularApp exists only one, and is created at init stage
  root_cd->ma = new modularApp("CD Modular App");
//  module* m   = new module(instance("Root", 1, 0), 
//                           inputs(port(context("sequential_id", (int)(root_cd->node_id().task_)))),
//                           namedMeasures("time", new timeMeasure()));
//  root_cd->mStack.push_back(m);

    /// graph exists only one. It is created at init stage
//    root_cd->scopeGraph = new graph();
#endif

#endif

  return root_cd;

}



void cd::CD_Finalize(void)
{

//    std::cout << "Finalize call" << std::endl;
//    for(auto it = cd_info.begin(); it != cd_info.end(); ++it) {
//      cout << "level (map size) : "
//           << cd_info.size() 
//           << "\tsibling (map size) : "
//           << it->second.size() << endl;
//    }

#if _PROFILER
//    assert(this->ma);
//    delete this->ma;

//      assert(this->scopeGraph);
//      delete this->scopeGraph;
//    GetRootCD()->Print_Profile();
//    GetRootCD()->Print_Graph();
#endif

}


CDHandle* cd::GetCurrentCD(void) 
{
//  if(cd::CDPath.empty()) {
//    cout<<"empty CDPath?"<<endl<<endl;
//    getchar();
//  }
  return cd::CDPath.back(); 
}

CDHandle* cd::GetRootCD(void)    
{ return cd::CDPath.front(); }


CDHandle::CDHandle()
  : ptr_cd_(0), node_id_()
{ 
#if _PROFILER_OLD
  sibling_id_ = 0;
  level_      = 0;

  profile_data_[MAX_PROFILE_DATA] = {0, };
  is_child_destroyed = false;
  usr_profile_enable = false;

  this_point_ = 0;
  that_point_ = 0;

#endif

  IsMaster_ = true;
  
}

CDHandle::CDHandle( CDHandle* parent, 
                    const char* name, 
                    const NodeID& node_id, 
                    CDModeT cd_type, 
                    uint64_t sys_bit_vector)
{
  // Design decision when we are receiving ptr_cd, do we assume that pointer is usable? 
  // That means is the handle is being created at local? 
  // Perhaps, we should not assume this 
  // as we will sometimes create a handle for an object that is located at remote.. right?
  //
  // CDID set up. ptr_cd_ set up. parent_cd_ set up. children_cd_ will be set up later by children's Create call.
  // sibling ID set up, cd_info set up
  // clear children list
  // request to add me as a children to parent (to MASTER CD object)

#if _PROFILER_OLD
  sibling_id_ = 0;
  level_      = 0;

  profile_data_[MAX_PROFILE_DATA] = {0, };
  is_child_destroyed = false;
  usr_profile_enable = false;

  this_point_ = 0;
  that_point_ = 0;
#endif
  IsMaster_ = false;
  cout<<"My task is : " <<node_id.task_<<endl;
  //getchar();
  node_id_ = node_id;
  SetMaster(node_id.task_);
  if(parent != NULL) { 
    CDID new_cd_id(parent->ptr_cd_->GetCDID().level_ + 1, node_id);

    if( !IsMaster() ) {
      cout<<"I am Slave :-( \n"<<endl;
      ptr_cd_  = new CD(parent, name, new_cd_id, cd_type, sys_bit_vector);
    }
    else {
      cout<<"I am Master :-) \n"<<node_id_<<endl;
      ptr_cd_  = new MasterCD(parent, name, new_cd_id, cd_type, sys_bit_vector);
    }
  }
  else { // Root CD
    cout<<"-------------- This is wrong for this app -----------"<<endl;
    getchar();
    CDID new_cd_id(0, node_id);
    if( !IsMaster() )
      ptr_cd_  = new CD(NULL, name, new_cd_id, cd_type, sys_bit_vector);
    else {
      cout<<"-------------- This is wrong for this app -----------"<<endl;
      ptr_cd_  = new MasterCD(NULL, name, new_cd_id, cd_type, sys_bit_vector);
    }
  }
  
}

CDHandle::CDHandle( CDHandle* parent, 
                    const char* name, 
                    NodeID&& node_id, 
                    CDModeT cd_type, 
                    uint64_t sys_bit_vector)
{
  // Design decision when we are receiving ptr_cd, do we assume that pointer is usable? 
  // That means is the handle is being created at local? 
  // Perhaps, we should not assume this 
  // as we will sometimes create a handle for an object that is located at remote.. right?
  //
  // CDID set up. ptr_cd_ set up. parent_cd_ set up. children_cd_ will be set up later by children's Create call.
  // sibling ID set up, cd_info set up
  // clear children list
  // request to add me as a children to parent (to MASTER CD object)

#if _PROFILER_OLD
  sibling_id_ = 0;
  level_      = 0;

  profile_data_[MAX_PROFILE_DATA] = {0, };
  is_child_destroyed = false;

  this_point_ = 0;
  that_point_ = 0;
#endif

  IsMaster_ = false;

  //cout<<"My task is : " <<node_id.task_<<endl;
  //getchar();

  node_id_ = std::move(node_id);
  SetMaster(node_id.task_);

  if(parent != NULL) { 
    CDID new_cd_id(parent->ptr_cd_->GetCDID().level_ + 1, node_id);

    if( !IsMaster() )
      ptr_cd_  = new CD(parent, name, new_cd_id, cd_type, sys_bit_vector);
    else
      ptr_cd_  = new MasterCD(parent, name, new_cd_id, cd_type, sys_bit_vector);
  }
  else { // Root CD
    cout<<"-------------- This is wrong for this app -----------"<<endl;
    getchar();
    CDID new_cd_id(0, node_id);
    if( !IsMaster() )
      ptr_cd_  = new CD(NULL, name, new_cd_id, cd_type, sys_bit_vector);
    else
      ptr_cd_  = new MasterCD(NULL, name, new_cd_id, cd_type, sys_bit_vector);
  }
  
}

CDHandle::~CDHandle()
{
  // request to delete me in the children list to parent (to MASTER CD object)
  if(ptr_cd_ != NULL) {
    // We should do this at Destroy(), not creator?
//    RemoveChild(this);
  } 
  else {  // There is no CD for this CDHandle!!

  }

}

void CDHandle::Init(CD* ptr_cd, NodeID node_id)
{ 
  // CDHandle *parent
  ptr_cd_	  = ptr_cd;
  node_id_  = node_id;
}

// Non-collective
CDHandle* CDHandle::Create( const char* name, 
                            CDModeT type, 
                            uint32_t error_name_mask, 
                            uint32_t error_loc_mask, 
                            CDErrT *error )
{
  // Create CDHandle for a local node
  // Create a new CDHandle and CD object
  // and populate its data structure correctly (CDID, etc...)
  uint64_t sys_bit_vec = SetSystemBitVector(error_name_mask, error_loc_mask);

  CDHandle* new_cd = new CDHandle(this, name, node_id_, type, sys_bit_vec);

  CDPath.push_back(new_cd);
  cout<<"push back cd"<<ptr_cd()->GetCDID().level_<<endl;;
  // Request to add me as a child to my parent
  // It could be a local execution or a remote one. 
  ptr_cd_->AddChild(ptr_cd_);

  
  return new_cd;
}

// Collective
CDHandle* CDHandle::Create( int color, 
                            uint32_t num_children, 
                            const char* name, 
                            CDModeT type, 
                            uint32_t error_name_mask, 
                            uint32_t error_loc_mask, 
                            CDErrT *error )
{
  // Create a new CDHandle and CD object
  // and populate its data structure correctly (CDID, etc...)
  //  This is an extremely powerful mechanism for dividing a single communicating group of processes into k subgroups, 
  //  with k chosen implicitly by the user (by the number of colors asserted over all the processes). 
  //  Each resulting communicator will be non-overlapping. 
  //  Such a division could be useful for defining a hierarchy of computations, 
  //  such as for multigrid, or linear algebra.
  //  
  //  Multiple calls to MPI_COMM_SPLIT can be used to overcome the requirement 
  //  that any call have no overlap of the resulting communicators (each process is of only one color per call). 
  //  In this way, multiple overlapping communication structures can be created. 
  //  Creative use of the color and key in such splitting operations is encouraged.
  //  
  //  Note that, for a fixed color, the keys need not be unique. 
  //  It is MPI_COMM_SPLIT's responsibility to sort processes in ascending order according to this key, 
  //  and to break ties in a consistent way. 
  //  If all the keys are specified in the same way, 
  //  then all the processes in a given color will have the relative rank order 
  //  as they did in their parent group. 
  //  (In general, they will have different ranks.)
  //  
  //  Essentially, making the key value zero for all processes of a given color means that 
  //  one doesn't really care about the rank-order of the processes in the new communicator.  
  Sync();
  uint64_t sys_bit_vec = SetSystemBitVector(error_name_mask, error_loc_mask);


  NodeID new_node(MPI_UNDEFINED, 0, 0, 0);
/*
  // Split the node
  uint32_t num_children_per_dim = (uint32_t)pow(num_children, 1/3.);
  double scale_k = pow(GetRootCD()->GetTaskSize(), 1/3.);
  uint32_t scale_j = (uint32_t)scale_k;
  uint32_t scale_i = (uint32_t)pow(scale_k, 2);
  std::cout << "num_children_per_dim = "<< num_children_per_dim << std::cout;
  for(int i=0; i < num_children_per_dim; ++i) {
    
    for(int j=0; j < num_children_per_dim; ++j) {
      
      for(int k=0; k < num_children_per_dim; ++k) {


//    if(i*GetTaskSize()/num_children =< GetTaskID()
//      && GetTaskID() < (i+1)*GetTaskSize()/num_children)
//    {
        new_node.color_ = i*scale_i + j*scale_j + k;
        new_node.size_  = GetTaskSize() / num_children;
        new_node.task_  = GetTaskID() % new_node.size_;;
//    }


      }
    }
  }
*/
  
  // Create CDHandle for multiple tasks (MPI rank or threads)

  MPI_Comm_split(node_id_.color_, color, num_children, &(new_node.color_));
  MPI_Comm_size(new_node.color_, &(new_node.size_));
  MPI_Comm_rank(new_node.color_, &(new_node.task_));
  // Then children CD get new MPI rank ID. (task ID) I think level&taskID should be also pair.

  CDHandle* new_cd = new CDHandle(this, name, new_node, type, sys_bit_vec);

  CDPath.push_back(new_cd);
  cout<<"push back cd"<<ptr_cd()->GetCDID().level_<<endl;;
  // Request to add me as a child to my parent
  // It could be a local execution or a remote one. 
  ptr_cd_->AddChild(ptr_cd_);

  return new_cd;

}

// Collective
CDHandle* CDHandle::Create (uint32_t  numchildren,
                            const char* name, 
                            CDModeT type, 
                            uint32_t error_name_mask, 
                            uint32_t error_loc_mask, 
                            CDErrT *error )
{
  uint64_t sys_bit_vec = SetSystemBitVector(error_name_mask, error_loc_mask);

  NodeID new_node_tmp, new_node;
  SetColorAndTask(new_node_tmp, numchildren);

  MPI_Comm_split(node_id_.color_, new_node_tmp.color_, new_node_tmp.task_, &(new_node.color_));
  MPI_Comm_size(new_node.color_, &(new_node.size_));
  MPI_Comm_rank(new_node.color_, &(new_node.task_));
  CDHandle* new_cd = new CDHandle(this, name, new_node, type, sys_bit_vec);
  CDPath.push_back(new_cd);
  cout<<"push back cd"<<ptr_cd()->GetCDID().level_<<endl;;

  ptr_cd_->AddChild(ptr_cd_);

  return new_cd;

}

void CDHandle::SetColorAndTask(NodeID& new_node, const int& numchildren) 
{
  // mytask = a*x+b;
  // a : color
  // x : children node size (# of tasks in it)
  // b : task ID of each children node
  for(int i = 0; i < numchildren; ++i) {
    if( i * node_id_.size_/numchildren <= node_id_.task_ && 
        node_id_.task_ < (i + 1)* node_id_.size_/numchildren) {
      new_node.color_ = i;
      new_node.task_  = node_id_.task_ % numchildren;
      new_node.size_  = node_id_.size_ / numchildren;
      break;
    }
  }
}

CDHandle* CDHandle::CreateAndBegin( uint32_t color, 
                                    uint32_t num_tasks_in_color, 
                                    const char* name, 
                                    CDModeT type, 
                                    uint32_t error_name_mask, 
                                    uint32_t error_loc_mask, 
                                    CDErrT *error )
{

  return NULL;
}

CDErrT CDHandle::Destroy (bool collective)
{
  CDErrT err;
 
  if ( collective ) {
//    if( Sync() != 0 ) {
    if( true ) {
      printf("------------------ Sync Success in Destroy %d ------------------\n", node_id_.task_);
    }
    else {
      printf("------------------ Sync Failure in Destroy %d ------------------\n", node_id_.task_);
    }
  }

  // It could be a local execution or a remote one.
  // These two should be atomic 

  if(this != GetRootCD()) { 

    // Mark that this is destroyed
    // this->parent_->is_child_destroyed = true;

  } 
  else {

#if _PROFILER_OLD 
    assert(this->ma);
    delete this->ma;

//    assert(this->scopeGraph);
//    delete this->scopeGraph;
#endif
  }


  if(CDPath.size() > 1)
    GetParent()->RemoveChild(ptr_cd_);
  
  assert(CDPath.size()>0);
  assert(CDPath.back() != NULL);
  err = ptr_cd_->Destroy();
  CDPath.pop_back();


  return err;
}

#if _PROFILER
void CDHandle::SetUsrProfileInput(std::pair<std::string, long> name_list)
{
  ptr_cd_->AddUsrProfile(name_list.first, name_list.second, 0);
}
void CDHandle::SetUsrProfileInput(std::initializer_list<std::pair<std::string, long>> name_list)
{
  for(auto il = name_list.begin() ;
      il != name_list.end(); ++il) {
    ptr_cd_->AddUsrProfile(il->first, il->second, 0);
  }
}

void CDHandle::SetUsrProfileOutput(std::pair<std::string, long> name_list)
{
  ptr_cd_->AddUsrProfile(name_list.first, name_list.second, 1);
}
void CDHandle::SetUsrProfileOutput(std::initializer_list<std::pair<std::string, long>> name_list)
{
  for(auto il = name_list.begin() ;
      il != name_list.end(); ++il) {
  
    ptr_cd_->AddUsrProfile(il->first, il->second, 1);
  }
}

#else
#if _PROFILER_OLD

void CDHandle::SetUsrProfileInput(std::pair<std::string, long> name_list)
{
if(IsMaster()) {
  usr_profile_input.add(name_list.first, name_list.second);
  usr_profile_enable = true;
}
}
void CDHandle::SetUsrProfileInput(std::initializer_list<std::pair<std::string, long>> name_list)
{

if(IsMaster()) {
  for(auto il = name_list.begin() ;
      il != name_list.end(); ++il) {
    usr_profile_input.add(il->first, il->second);
  }
  usr_profile_enable = true;
}
}

void CDHandle::SetUsrProfileOutput(std::pair<std::string, long> name_list)
{

if(IsMaster()) {
  usr_profile_output.add(name_list.first, name_list.second);
  usr_profile_enable = true;
}
}
void CDHandle::SetUsrProfileOutput(std::initializer_list<std::pair<std::string, long>> name_list)
{
{
  for(auto il = name_list.begin() ;
      il != name_list.end(); ++il) {
    usr_profile_output.add(il->first, il->second);
  }

  usr_profile_enable = true;
}
}
#endif
#endif

CDErrT CDHandle::Begin (bool collective, const char* label)
{
  SetStatus(1);

#if _PROFILER_OLD
  /// Timer on
  this->this_point_ = rdtsc();

  //-- Sight-related -------------------------------------------------------------------------------

//  CDNode* cdn = new CDNode(txt()<<label, txt()<<this->this_cd_->GetCDID()); 
//  this->cdStack.push_back(cdn);
//  dbg << "{{{ CDNode Test -- "<<this->this_cd_->cd_id_<<", #cdStack="<<cdStack.size()<<endl;

//  comparison* comp = new comparison(node_id().color_);
//  compStack.push_back(comp);


//  if(usr_profile_enable) {
    module* m = new module( instance(txt()<<label, 1, 1), 
                            inputs(port(context("cd_id", txt()<<this->node_id().task_, 
                                                "sequential_id", (int)(GetSeqID())))),
                            namedMeasures("time", new timeMeasure()) );
    this->mStack.push_back(m);
//  }
//  else {
//  
//    module* m = new module( instance(txt()<<label, 2, 2), 
//                            inputs(port(context("cd_id", txt()<<this->node_id().task_, 
//                                                "sequential_id", (int)(GetSeqID()))),
//                                   port(usr_profile_input)),
//                            namedMeasures("time", new timeMeasure()) );
//    this->mStack.push_back(m);
//  }



//  dbg << "[[[ Module Test -- "<<this->this_cd_->cd_id_<<", #mStack="<<mStack.size()<<endl;

//  /// create a new scope at each Begin() call
//  scope* s = new scope(txt()<<label<<", cd_id="<<node_id().task_);
//
//  /// Connect edge between previous node to newly created node
//  if(this->sStack.size()>0)
//    this->scopeGraph->addDirEdge(this->sStack.back()->getAnchor(), s->getAnchor());
//
//  /// push back this node into sStack to manage scopes
//  this->sStack.push_back(s);
//  dbg << "<<< Scope  Test -- "<<this->this_cd_->cd_id_<<", #sStack="<<sStack.size()<<endl;
//------------------------------------------------------------------------------------------------
#endif

  //TODO It is illegal to call a collective Begin() on a CD that was created without a collective Create()??
  if ( collective ) {
    int ret = Sync();
//    if( ret != 0 ) {
    if( true ) {
      printf("------------------ Sync Success in Begin %d (%d) ------------------\n", node_id_.task_, ret);
      //getchar();
    }
    else {
      //printf("------------------ Sync Failure in Begin %d (%d) ------------------\n", node_id_.task_, ret);
      //getchar();
      //exit(-1);
    }
  }
  cout << label << endl;
  assert(ptr_cd_ != 0);

  return ptr_cd_->Begin(collective, label);
}

CDErrT CDHandle::Complete (bool collective, bool update_preservations)
{
  SetStatus(0);

#if _PROFILER_OLD
  // outputs the preservation / detection info
//  if(cdStack.back() != nullptr){
//    cout<<"add new info"<<endl;
///*
//    cdStack.back()->setStageNode( "preserve", "data_copy", profile_data_[PRV_COPY_DATA]    );
//    cdStack.back()->setStageNode( "preserve", "data_overlapped", profile_data_[OVERLAPPED_DATA]  );
//    cdStack.back()->setStageNode( "preserve", "data_ref", profile_data_[PRV_REF_DATA]     );
//*/
//    
//    //scope s("Preserved Stats");
//    PreserveStageNode psn(txt()<<"Preserve Stage");
//    dbg << "data_copy="<<profile_data_[PRV_COPY_DATA]<<endl;
//    dbg << "data_overlapped="<<profile_data_[OVERLAPPED_DATA]<<endl;
//    dbg << "data_ref="<<profile_data_[PRV_REF_DATA]<<endl;
//
//  }


//  dbg << " >>> Scope  Test -- "<<this->this_cd_->cd_id_<<", #sStack="<<sStack.size()<<endl;
//  assert(sStack.size()>0);
//  assert(sStack.back() != NULL);
//  delete sStack.back();
//  sStack.pop_back();

//  dbg << " ]]] Module Test -- "<<this->this_cd_->cd_id_<<", #mStack="<<mStack.size()<<endl;
  assert(mStack.size()>0);
  assert(mStack.back() != NULL);
  mStack.back()->setOutCtxt(0, context("data_copy=", (long)profile_data_[PRV_COPY_DATA],
                                       "data_overlapped=", (long)profile_data_[OVERLAPPED_DATA],
                                       "data_ref=" , (long)profile_data_[PRV_REF_DATA]));
//  if(usr_profile_enable) {
//    mStack.back()->setOutCtxt(1, usr_profile_output);
//  }
/*
  mStack.back()->setOutCtxt(1, context("sequential id =" , (long)profile_data_[PRV_REF_DATA],
                                       "execution cycle=", (long)profile_data_[PRV_COPY_DATA],
                                       "estimated error rate=", (long)profile_data_[OVERLAPPED_DATA]));
*/
  delete mStack.back();
  mStack.pop_back();

//  assert(compStack.size()>0);
//  assert(compStack.back() != NULL);
//  delete compStack.back();
//  compStack.pop_back();


//  dbg << " }}} CDNode Test -- "<<this->this_cd_->cd_id_<<", #cdStack="<<cdStack.size()<<endl;
//  assert(cdStack.size()>0);
//  assert(cdStack.back() != nullptr);
//  delete cdStack.back();
//  cdStack.pop_back();

  /// Timer off
  uint64_t tmp_point = rdtsc();
  that_point_ = tmp_point;

  /// Loop Count (# of seq. CDs) + 1
//  (this->profile_data_)[LOOP_COUNT] += 1;
  (profile_data_)[LOOP_COUNT] = ptr_cd_->GetCDID().sequential_id_;

  /// Calcualate the execution time
  (profile_data_)[EXEC_CYCLE] += (that_point_) - (this_point_);

#endif

//FIXME
  /// It deletes entry directory in the CD (for every Complete() call). 
  /// We might modify this in the profiler to support the overlapped data among sequential CDs.
  ptr_cd_->DeleteEntryDirectory();

  if ( collective ) {
    int ret = Sync();
//    if( ret != 0 ) {
    if( true ) {
      printf("------------------ Sync Success in Complete %d (%d) ------------------\n", node_id_.task_, ret);
      //getchar();
    }
    else {
      //printf("------------------ Sync Failure in Complete %d (%d) ------------------\n", node_id_.task_, ret);
      //getchar();
      //exit(-1);
    }
  }
  assert(ptr_cd_ != 0);

  return ptr_cd_->Complete();

}










CDErrT CDHandle::Preserve ( void *data_ptr, 
                            uint64_t len, 
                            uint32_t preserve_mask, 
                            const char *my_name, 
                            const char *ref_name, 
                            uint64_t ref_offset, 
                            const RegenObject *regen_object, 
                            PreserveUseT data_usage )
{


  /// Preserve meta-data
  /// Accumulated volume of data to be preserved for Sequential CDs. 
  /// It will be averaged out with the number of seq. CDs.
#if _PROFILER_OLD
  if(preserve_mask == kCopy){

    profile_data_[PRV_COPY_DATA] += len;
//    if( (this->parent_ != nullptr) && check_overlap(this->parent_, ref_name) ){
      /// Sequential overlapped accumulated data. It will be calculated to OVERLAPPED_DATA/PRV_COPY_DATA
      /// overlapped data: overlapped data that is preserved only via copy method.
//      profile_data_[OVERLAPPED_DATA] += len_in_bytes;
//      cout<<"something is weird  "<<"level is "<<this->this_cd_->level_ << "length : "<<len_in_bytes<<endl;

  } else if(preserve_mask == kReference) {

    profile_data_[PRV_REF_DATA] += len;

  } else {
                                                                                                                                  
    cout<<"prvTy is not supported currently : "<< preserve_mask<<endl;          
    exit(-1);

  }
#endif
  if( IsMaster() ) {
    assert(ptr_cd_ != 0);
    return ptr_cd_->Preserve(data_ptr, len, preserve_mask, my_name, ref_name, ref_offset, regen_object, data_usage);
  }
  else {
    // It is at remote node so do something for that.

  }

  return kError;
}

CDErrT CDHandle::Preserve ( CDEvent &cd_event, 
                            void *data_ptr, 
                            uint64_t len, 
                            uint32_t preserve_mask, 
                            const char *my_name, 
                            const char *ref_name, 
                            uint64_t ref_offset, 
                            const RegenObject *regen_object, 
                            PreserveUseT data_usage )
{
  if( IsMaster() ) {
    assert(ptr_cd_ != 0);
    // TODO CDEvent object need to be handled separately, 
    // this is essentially shared object among multiple nodes.
    return ptr_cd_->Preserve( data_ptr, len, preserve_mask, 
                              my_name, ref_name, ref_offset, 
                              regen_object, data_usage );
  }
  else {
    // It is at remote node so do something for that.
  }

  return kError;
}

CDErrT CDHandle::CDAssert (bool test_true, const SysErrT *error_to_report)
{
  if( IsMaster() ) {
    assert(ptr_cd_ != 0);
    return ptr_cd_->Assert(test_true); 
  }
  else {
    // It is at remote node so do something for that.
  }

  return kOK;
}

CDErrT CDHandle::CDAssertFail (bool test_true, const SysErrT *error_to_report)
{
  if( IsMaster() ) {

  }
  else {
    // It is at remote node so do something for that.
  }

  return kOK;
}

CDErrT CDHandle::CDAssertNotify (bool test_true, const SysErrT *error_to_report)
{
  if( IsMaster() ) {
    // STUB
  }
  else {
    // It is at remote node so do something for that.
  }

  return kOK;
}

std::vector< SysErrT > CDHandle::Detect (CDErrT *err_ret_val)
{

  std::vector< SysErrT > ret_prepare;


  return ret_prepare;

}

CDErrT CDHandle::RegisterRecovery (uint32_t error_name_mask, uint32_t error_loc_mask, RecoverObject *recover_object)
{
  if( IsMaster() ) {
    // STUB
  }
  else {
    // It is at remote node so do something for that.
  }
  return kOK;
}

CDErrT CDHandle::RegisterDetection (uint32_t system_name_mask, uint32_t system_loc_mask)
{
  if( IsMaster() ) {
    // STUB
  }
  else {
    // It is at remote node so do something for that.

  }

  return kOK;
}

float CDHandle::GetErrorProbability (SysErrT error_type, uint32_t error_num)
{

  return 0;
}

float CDHandle::RequireErrorProbability (SysErrT error_type, uint32_t error_num, float probability, bool fail_over)
{


  return 0;
}

char* CDHandle::GetName(void)
{
  if(ptr_cd_ != NULL)
    return ptr_cd_->name_;	
  else
    assert(0);
}

CDHandle* CDHandle::GetParent()
{
  if(ptr_cd_ != NULL) {
    cout<<"level : " <<ptr_cd_->GetCDID().level_<<endl;
    //getchar();
    
    return CDPath.at(ptr_cd_->GetCDID().level_ - 1);
  }
  else {
    cout<< "ERROR in GetParent" <<endl;
    assert(0);
  }
}

int CDHandle::GetSeqID()
{
  return ptr_cd_->GetCDID().sequential_id_;

}

// FIXME
// For now task_id_==0 is always MASTER which is not good!
bool CDHandle::IsMaster(void)
{
  return IsMaster_;
/*
  if( node_id_.second == 0 )
    return true;
  else 
    return false;
*/
}

// FIXME
// For now task_id_==0 is always MASTER which is not good!
void CDHandle::SetMaster(int task)
{
  cout<<"In SetMaster, Newly born CDHandle's Task# is "<<task<<endl;
  if(task == 0)
    IsMaster_ = true;
  else
    IsMaster_ = false;
}

bool CDHandle::IsLocalObject()
{
/* RELEASE
  // Check whether we already have this current node info, if not then get one.
  if( current_task_id_ == -1 || current_process_id_ == -1 ) {
    current_task_id_    = cd::Util::GetCurrentTaskID(); 
    current_process_id_ = cd::Util::GetCurrentProcessID();
  }

  if( current_task_id_ == task_id_ && current_process_id_ == process_id_ )
    return true;
  else 
    return false;
*/
return true;
}

/// Synchronize the CD object in every task of that CD.
bool CDHandle::Sync() 
{
  int err;
#if _CD_MPI
  err = MPI_Barrier(node_id_.color_);
#endif
  return err;
}

CDErrT CDHandle::AddChild(CD* cd_child)
{
  CDErrT err;
  ptr_cd_->AddChild(cd_child);
  return err;
}

CDErrT CDHandle::RemoveChild(CD* cd_child)	
{
  CDErrT err;
  ptr_cd_->RemoveChild(cd_child);
  return err;
}

/*
int CDHandle::StopAllChildren()
{
  if( IsMaster() ) {
    ptr_cd_->StopAllChildren();
  }
  else {

  }
  //FIXME return value should be taken care of 
  return kOK; 
}
*/

int CDHandle::Stop()
{
  return ptr_cd_->Stop();

  //FIXME return value should be taken care of 
}

/* RELEASE
CDEntry* CDHandle::InternalGetEntry(std::string entry_name)
{
  return ptr_cd_->InternalGetEntry(entry_name);
  //FIXME need some way to ask to accomplish this functionality...  // Remote request
}
*/
void CDHandle::Recover (uint32_t error_name_mask, 
              uint32_t error_loc_mask, 
              std::vector< SysErrT > errors)
{
  // STUB
}

CDErrT CDHandle::SetPGASType (void *data_ptr, uint64_t len, CDPGASUsageT region_type)
{


  return kOK;
}

int CDHandle::context_preservation_mode()
{
/* RELEASE
  if( IsMaster() )
  {

    return (int)ptr_cd_->context_preservation_mode_;
  }
  else
  {
    //FIXME: need to get the flag from remote

  }
 */

  return 0;
}

void CDHandle::CommitPreserveBuff()
{
/*  RELEASE 
  if( IsMaster() )
  {
    if( ptr_cd_->context_preservation_mode_ == CD::kExcludeStack) 
     {
        memcpy(ptr_cd_->jump_buffer_, jump_buffer_, sizeof(jmp_buf));
     }
     else
     {
        ptr_cd_->context_ = this->context_;
     }
 
  } 
  else
  {
    //FIXME: need to transfer the buffers to remote

  }
*/
}




uint64_t CDHandle::SetSystemBitVector(uint64_t error_name_mask, uint64_t error_loc_mask)
{
  uint64_t sys_bit_vec = 0;
  if(error_name_mask = 0) {
  }
  else {
  }

  if(error_loc_mask = 0) {
  }
  else {
  }
  return sys_bit_vec;
}


/*
ucontext_t* CDHandle::context()
{
  if( IsMaster() ) {

    return &ptr_cd_->context_;
  }
  else {
    //FIXME: need to get the flag from remote

  }
}

jmp_buf* CDHandle::jump_buffer()
{
  if( IsMaster() ) {

    return &ptr_cd_->jump_buffer_;
  }
  else {
    //FIXME: need to get the flag from remote

  }
} 
*/


