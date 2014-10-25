/******************************************************************************
* FILE: mpi_helloBsend.c
* DESCRIPTION:
*   MPI tutorial example code: Simple hello world program that uses blocking
*   send/receive routines.
* AUTHOR: Blaise Barney
* LAST REVISED: 07/09/12
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define  MASTER		0

#include <pnmpimod.h>

#include "sight.h"
using namespace sight;
using namespace std;

int main (int argc, char *argv[])
{
int  numtasks, taskid, len;
char hostname[MPI_MAX_PROCESSOR_NAME];
int  partner, message;
MPI_Status status;
PNMPI_modHandle_t handle;


MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Get_processor_name(hostname, &len);
{ scope s(txt()<<taskid);
printf ("Hello from task %d on %s!\n", taskid, hostname);
dbg << "Hello from task " << taskid << " on "<< hostname << "!" << endl;
if (taskid == MASTER){
   { scope s("MASTER!");
   printf("MASTER: Number of MPI tasks is: %d\n",numtasks);
   dbg << "MASTER: Number of MPI tasks is: " << numtasks << endl;
   }
}
}

/* determine partner and then send/receive with partner */
if (taskid < numtasks/2) {
  {  scope s(txt()<<"First Group");
  dbg<<"We are the first!";
  	  partner = numtasks/2 + taskid;
  	  MPI_Send(&taskid, 1, MPI_INT, partner, 1, MPI_COMM_WORLD);
  	  MPI_Recv(&message, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &status);
  }
}
else if (taskid >= numtasks/2) {
  { scope s(txt()<<"Second Group");
	  partner = taskid - numtasks/2;
	  MPI_Recv(&message, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &status);
	  MPI_Send(&taskid, 1, MPI_INT, partner, 1, MPI_COMM_WORLD);
  }
}

/* print partner info and exit*/
printf("Task %d is partner with %d\n",taskid,message);

MPI_Finalize();

}
