/******************************************************************************
* FILE: mpi_ping.c
* DESCRIPTION:
*   MPI tutorial example code: Blocking Send/Receive
* AUTHOR: Blaise Barney
* LAST REVISED: 04/02/05
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <pnmpimod.h>

#include "sight.h"
using namespace sight;
using namespace std;
int main (int argc, char *argv[])
{
int numtasks, rank, dest, source, rc, count, tag=1;
char inmsg, outmsg='x';
MPI_Status Stat;
PNMPI_modHandle_t handle;

MPI_Init(&argc,&argv);

MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
if (rank==0) { scope s("Ping - Pong with two processes");
}
if (rank == 0) {
  if (numtasks > 2)
    printf("Numtasks=%d. Only 2 needed. Ignoring extra...\n",numtasks);
  dest = 1;
  source = 1;
  { scope s(txt()<<"Send & Receive");
	  rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
	  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
  }
}

else if (rank == 1) {
  dest = 0;
  source = 0;
  { scope s(txt() << "Receive & Send");
	  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
	  rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  }
}

if (rank < 2) {
  rc = MPI_Get_count(&Stat, MPI_CHAR, &count);
  printf("Task %d: Received %d char(s) from task %d with tag %d \n",
         rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);
  { scope s(txt() << "Received " << count << " char(s) from task " << Stat.MPI_SOURCE << " with tag " << Stat.MPI_TAG, scope::minimum);
  }
}

MPI_Finalize();

}
