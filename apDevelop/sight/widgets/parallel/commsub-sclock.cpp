/***  trying to have graphics from sight!   ***/

/***  Scalar Clock  ***/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "commsub.h"
#include "pnmpimod.h"

#include "sight.h"
using namespace sight;
using namespace std;

/* Global variables */

int myid, group_myid;
int tasks, group_tasks;
long long bar_count; // Barrier counter
MPI_Group orig_group; //group for MPI_WORLD
scalarCausalClock* mpic=NULL;

std::string getName( int i );

class channel{
  friend ostream &operator<<(ostream &, const channel &);

public :
  MPI_Comm comm; // communicator
  int tag;     	 // tag, if collective operation:-1
  int sender;    // process
  int recvr;	 // receiver

  channel();
  channel(const MPI_Comm& comm, int tag, int sender, int recvr) : comm(comm), tag(tag), sender(sender), recvr(recvr) {}
  channel(const channel &);
  ~channel(){};
  channel &operator=(const channel &cpin);
  int operator==(const channel &cpin) const;
  int operator<(const channel &cpin) const;

  std::string ID(std::string prefix, int count) const {
	  ostringstream s;
	  s << prefix <<": ";
	  s << /*x << ' ' << */ "tag_"<<tag << ' ' << sender << "=>" << recvr << count;
	return s.str();
  }
};

ostream &operator<<(ostream &output, const channel &c)
{
  output << c.comm << ' ' << c.tag << ' ' << c.sender << ' ' << c.recvr << endl;
  return output;
}

channel::channel( )
{
	comm=MPI_COMM_WORLD;
	tag=0;
	sender=0;
	recvr=0;
}

channel::channel( const channel &cpin)
{
  comm=cpin.comm;
  tag=cpin.tag;
  sender=cpin.sender;
  recvr=cpin.recvr;
}

channel& channel::operator=(const channel &cpin)
{
  this->comm=cpin.comm;
  this->tag=cpin.tag;
  this->sender=cpin.sender;
  this->recvr=cpin.recvr;
  return *this;
}

int channel::operator==(const channel &cpin) const
{
  if(this->comm!=cpin.comm) return 0;
  if(this->tag!=cpin.tag) return 0;
  if(this->sender!=cpin.sender) return 0;
  if(this->recvr!=cpin.recvr) return 0;
  return 1;
}

int channel::operator<(const channel &cpin) const
{
  if( this->comm == cpin.comm && this->tag == cpin.tag && this->sender == cpin.sender && this->recvr < cpin.recvr) return 1;
  if( this->comm == cpin.comm && this->tag == cpin.tag && this->sender < cpin.sender ) return 1;
  if( this->comm == cpin.comm && this->tag < cpin.tag) return 1;
  if( this->comm < cpin.comm ) return 1;
  return 0;
}

map<channel, int> c_send;
map<channel, int> c_recv;


// Attribute that holds the rank of the current MPI process
attr* rankAttr=NULL;

void COMM_ALL_INIT(int argc, char**argv) 
{
  pnmpimod_comm_collective_support=PNMPIMOD_COMM_KEEPCOLLECTIVE;
  PMPI_Comm_size(MPI_COMM_WORLD, &tasks);
  PMPI_Comm_rank(MPI_COMM_WORLD, &myid);

  int i;
  int *ids ;
  ids = (int *)malloc(tasks * sizeof(int));
  int err;
  PNMPI_Service_descriptor_t service;

  /* register this module and its services */

  err=PNMPI_Service_RegisterModule("SClock-module");
  if (err!=PNMPI_SUCCESS)
    return;

  SightInit(argc, argv, "Test", txt()<<"dbg.rank_"<<myid);
  mpic = new scalarCausalClock();

  MPI_Comm_group(MPI_COMM_WORLD, &orig_group);

  rankAttr = new attr("MPIRank", myid);
}

void COMM_ALL_PREINIT(int argc, char**argv)
{
}

void COMM_ALL_FINALIZE() 
{
  // Destroy the MPI rank attribute
  assert(rankAttr);
  delete rankAttr;

  assert(mpic);
  delete mpic;
}

void SEND_P2P_START(void *buf, int count, MPI_Datatype dt, int node, int tag, 
		    MPI_Comm comm, void **ptr, int type) 
{
}

void SEND_P2P_ASYNC_MID1(void *buf, int count, MPI_Datatype dt, int node, int tag, 
			 MPI_Comm comm, void **ptr, int type) 
{
   long long sTime = mpic->send();
   PMPI_Send(&sTime,1,MPI_LONG_LONG_INT,node,tag,comm);

   { scope s(txt()<<"Send (to "<<node<<")" , scope::minimum);   }

   int ranks_out;
   MPI_Group new_group;
   MPI_Comm_group(comm, &new_group);
   MPI_Group_translate_ranks( new_group, 1, &node, orig_group, &ranks_out );

   channel c(comm, tag, myid, ranks_out);
   c_send[c]++;
   commSend("", c.ID("S", c_send[c]), "");
}

void SEND_P2P_END(void *buf, int count, MPI_Datatype dt, int node, int tag, 
		  MPI_Comm comm, int err, void **ptr, void **midptr, int type) 
{
  
  if (type!=PNMPIMOD_COMM_ASYNC_P2P)
    {
      SEND_P2P_ASYNC_MID1(buf,count,dt,node,tag,comm,ptr,type);
    }
}

void RECV_P2P_START(void *buf, int count, MPI_Datatype dt, int node, int tag, 
		    MPI_Comm comm, void **ptr, int type) 
{
}

void COMM_P2P_ASYNC_MID2(int count, MPI_Request *requests, int flag, void **midptr) 
{
}

void RECV_P2P_ASYNC_MID1(void *buf, int count, MPI_Datatype dt, int node, int tag, 
			 MPI_Comm comm, int err, void **ptr, int type) 
{
}

void COMM_P2P_ASYNC_COMPLETION(int flag) 
{
}

void RECV_P2P_END(void *buf, int count, MPI_Datatype dt, int node, int tag,
  MPI_Comm comm, int err, void **ptr, void **midptr, int type,
  MPI_Status *statusarray, int numindex, int index)
{
  long long recvsTime;
  MPI_Status status;
  PMPI_Recv(&recvsTime,1,MPI_LONG_LONG_INT,node,tag,comm,&status);
  //sTime = (sTime>recvsTime? sTime: recvsTime) + 1;
  mpic->recv(recvsTime);

  int ranks_out;
  MPI_Group new_group;
  MPI_Comm_group(comm, &new_group);
  MPI_Group_translate_ranks( new_group, 1, &node, orig_group, &ranks_out );

  channel c(comm, tag, ranks_out, myid);
  c_recv[c]++;
  commRecv("", c.ID("S", c_recv[c]), c.ID("R", c_recv[c]));

  { scope s(txt()<<"Receive (from "<<node<<")", scope::minimum);  }
}

void COMM_COLL_REDUCE(void *buf, int count, MPI_Datatype dt, MPI_Op ops, int size, void** ptr) 
{
}

void COMM_COLL_START(MPI_Comm comm,int root,int type, void **ptr) 
{ 
  int i;
  type = type&0xf0;
  MPI_Group group;
  MPI_Comm_group(comm, &group);
  MPI_Group_size(group, &group_tasks);
  MPI_Group_rank(group, &group_myid);

  string type_name = getName(type);


  if(type==PNMPIMOD_COMM_BCAST||type==PNMPIMOD_COMM_SCATTER||type==PNMPIMOD_COMM_SCATTERV){
    if(root==group_myid){
      { scope s(type_name, scope::minimum);

      int *ranks, *ranks_out;
      ranks = (int *)malloc( group_tasks * sizeof(int) );
      ranks_out = (int *)malloc( group_tasks * sizeof(int) );
      MPI_Group new_group;
	  MPI_Comm_group(comm, &new_group);
	  for(i=0;i<group_tasks;i++)
	  	ranks[i]=i;
 	  MPI_Group_translate_ranks( new_group, group_tasks, ranks, orig_group, ranks_out );

      for(i=0;i<group_tasks;i++){
    	   channel c(comm, -1, myid, ranks_out[i]);
    	   c_send[c]++;
    	   commSend("", c.ID("S", c_send[c]), "");
      }
     }
   }
  }
  else if(type!=PNMPIMOD_COMM_BARRIER){
    //if clause in order to not output the "barrier"
    if(type==PNMPIMOD_COMM_REDUCE || type==PNMPIMOD_COMM_GATHERV ||type==PNMPIMOD_COMM_GATHER ){
		{ scope s(type_name, scope::minimum);

		int ranks_out;
		MPI_Group new_group;
		MPI_Comm_group(comm, &new_group);
		MPI_Group_translate_ranks( new_group, 1, &root, orig_group, &ranks_out );

		channel c(comm, -2, myid, ranks_out);
		c_send[c]++;
		commSend("", c.ID("S", c_send[c]), "");
		}
    }
  }

}


void COMM_COLL_END(MPI_Comm comm,int root,int type, void **ptr) 
{
  int i;
  MPI_Group group;
  MPI_Comm_group(comm, &group);
  MPI_Group_size(group, &group_tasks);
  MPI_Group_rank(group, &group_myid);

  long long recvTime;

  type=type&0xf0;
  string type_name = getName(type);

  if(type==PNMPIMOD_COMM_SCATTER||type==PNMPIMOD_COMM_SCATTERV){ //One to All
    recvTime=mpic->send();
    PMPI_Bcast(&recvTime,1,MPI_LONG_LONG_INT, root,comm);
    mpic->recv(recvTime);
    { scope s(type_name, scope::minimum);

    int ranks_out;
	MPI_Group new_group;
	MPI_Comm_group(comm, &new_group);
	MPI_Group_translate_ranks( new_group, 1, &root, orig_group, &ranks_out );

    channel c(comm, -1, ranks_out, myid);
    c_recv[c]++;
    commRecv("", c.ID("S", c_recv[c]), c.ID("R", c_recv[c]));
    }
  }
  else if(type==PNMPIMOD_COMM_BCAST){ //Broadcast
      recvTime=mpic->send();
      PMPI_Bcast(&recvTime,1,MPI_LONG_LONG_INT, root,comm);
      mpic->recv(recvTime);


      int ranks_out;
	  MPI_Group new_group;
	  MPI_Comm_group(comm, &new_group);
	  MPI_Group_translate_ranks( new_group, 1, &root, orig_group, &ranks_out );
	  if(myid!=ranks_out){
		  { scope s(type_name, scope::minimum);
		  channel c(comm, -1, ranks_out, myid);
		  c_recv[c]++;
		  commRecv("", c.ID("S", c_recv[c]), c.ID("R", c_recv[c]));
      	  }
      }
    }
  else if(type==PNMPIMOD_COMM_GATHER||type==PNMPIMOD_COMM_GATHERV||type==PNMPIMOD_COMM_REDUCE) { //All to One
    PMPI_Reduce(&(mpic->send()), &recvTime, 1, MPI_LONG_LONG_INT, MPI_MAX, root, comm);
    if(group_myid==root){
      mpic->recv(recvTime);
      { scope s(type_name, scope::minimum);

      int *ranks, *ranks_out;
      ranks = (int *)malloc( group_tasks * sizeof(int) );
	  ranks_out = (int *)malloc( group_tasks * sizeof(int) );
      MPI_Group new_group;
      MPI_Comm_group(comm, &new_group);
	  for(i=0;i<group_tasks;i++)
		ranks[i]=i;
	  MPI_Group_translate_ranks( new_group, group_tasks, ranks, orig_group, ranks_out );


      for(i=0;i<group_tasks;i++){ //draw arrows! sends
    	  channel c(comm, -2, ranks_out[i], myid);
		  c_recv[c]++;
		  commRecv("", c.ID("S", c_recv[c]), c.ID("R", c_recv[c]));
      }
      }
    }
  }
  else if(type==PNMPIMOD_COMM_SCAN) { //Scan
    PMPI_Scan(&(mpic->send()), &recvTime, 1, MPI_LONG_LONG_INT, MPI_MAX, comm);
    mpic->recv(recvTime);
    {scope(txt()<< "Scan", scope::minimum);
    }
  }else{ //AllToAll
    PMPI_Allreduce(&(mpic->send()), &recvTime, 1, MPI_LONG_LONG_INT, MPI_MAX, comm);

    mpic->recv(recvTime);
   {scope(txt()<<type_name, scope::minimum);
   }
  }
}


std::string getName( int i ) {
	switch(i){
	      case PNMPIMOD_COMM_BARRIER:
	    	  return "";
	      case PNMPIMOD_COMM_ALLGATHER:
	    	  return "AllGather";
	      case PNMPIMOD_COMM_ALLGATHERV:
	    	  return "AllGatherv";
	      case PNMPIMOD_COMM_ALL2ALL:
	    	  return "All2All";
	      case PNMPIMOD_COMM_ALL2ALLV:
	    	  return "All2Allv";
	      case PNMPIMOD_COMM_REDUCESCATTER:
	    	  return "ReduceScatter";
	      case PNMPIMOD_COMM_ALLREDUCE:
	    	  return "AllReduce";
	      case PNMPIMOD_COMM_BCAST:
	    	  return "Broadcast";
	      case PNMPIMOD_COMM_GATHER:
	    	  return "Gather";
	      case PNMPIMOD_COMM_GATHERV:
	    	  return "Gatherv";
	      case PNMPIMOD_COMM_SCATTER:
			return "Scatter";
	      case PNMPIMOD_COMM_SCATTERV:
			return "Scatterv";
	      case PNMPIMOD_COMM_REDUCE:
	    	return "Reduce";
	  }
}
