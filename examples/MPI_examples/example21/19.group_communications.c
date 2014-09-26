/******************************************************************************
* FILE: mpi_group.c
* DESCRIPTION:
*  MPI tutorial example code: Groups/Communicators
* AUTHOR: Blaise Barney
* LAST REVISED: 04/13/05
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define NPROCS 8
#include <pnmpimod.h>

#include "sight.h"
using namespace sight;
using namespace std;

int main (int argc, char *argv[])
{
int        rank, new_rank, sendbuf, recvbuf, numtasks,
           ranks1[4]={0,1,2,3}, ranks2[4]={4,5,6,7},
           i, ii;
MPI_Group  orig_group, new_group;
MPI_Comm   new_comm;
PNMPI_modHandle_t handle;
MPI_Status status;
MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

if (numtasks != NPROCS) {
  printf("Must specify %d tasks. Terminating.\n",NPROCS);
  MPI_Finalize();
  exit(0);
  }


sendbuf = rank;

MPI_Comm_group(MPI_COMM_WORLD, &orig_group);


/* Divide tasks into two distinct groups based upon rank */
if (rank < NPROCS/2) {
  MPI_Group_incl(orig_group, NPROCS/2, ranks1, &new_group);
  }
else {
  MPI_Group_incl(orig_group, NPROCS/2, ranks2, &new_group);
  }


{ scope s(txt()<<"Create new new communicator");
/* Create new communicator and then perform collective communications */
MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);
}
int rank2;
MPI_Comm_rank(new_comm, &rank2);


int temp;
if(rank2==1)
	MPI_Send(&rank2, 1, MPI_INT, 2, 1, new_comm);
if(rank2==2)
	MPI_Recv(&temp, 1, MPI_INT, 1, 1, new_comm, &status);


int *ranks, *ranks_out;
MPI_Group_size(new_group, &i);
ranks = (int *)malloc( i * sizeof(int) );
ranks_out = (int *)malloc( i * sizeof(int) );
for(ii=0;ii<i;ii++)
	ranks[ii]=ii;
MPI_Group_translate_ranks( new_group, i, ranks, orig_group, ranks_out );


MPI_Allreduce(&sendbuf, &recvbuf, 1, MPI_INT, MPI_SUM, new_comm);
//MPI_Reduce(&sendbuf, &recvbuf, 1, MPI_INT, MPI_SUM, 3, new_comm);


MPI_Group_rank (new_group, &new_rank);
//printf("rank= %d newrank= %d recvbuf= %d\n",rank,new_rank,recvbuf);
MPI_Barrier(MPI_COMM_WORLD);

MPI_Finalize();
}
