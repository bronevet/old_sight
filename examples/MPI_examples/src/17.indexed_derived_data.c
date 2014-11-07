/******************************************************************************
* FILE: mpi_indexed.c
* DESCRIPTION:
*   MPI tutorial example code: Indexed Derived Datatype
* AUTHOR: Blaise Barney
* LAST REVISED: 04/13/05
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define NELEMENTS 6
#include <pnmpimod.h>

#include "sight.h"
using namespace sight;
using namespace std;

int main (int argc, char *argv[])
{
int numtasks, rank, source=0, dest, tag=1, i;
int blocklengths[2], displacements[2];
float a[16] =
  {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
   9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
float b[NELEMENTS];

MPI_Status stat;
MPI_Datatype indextype;
PNMPI_modHandle_t handle;

MPI_Init(&argc,&argv);

MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
if (rank ==0)	{ scope s(txt() << "MPI Example : Indexed Derived Datatype"); }

blocklengths[0] = 4;
blocklengths[1] = 2;
displacements[0] = 5;
displacements[1] = 12;

MPI_Type_indexed(2, blocklengths, displacements, MPI_FLOAT, &indextype);
MPI_Type_commit(&indextype);

if (rank == 0) {
	{ scope s(txt() << "Sending to processes");
	  for (i=0; i<numtasks; i++)
		 MPI_Send(a, 1, indextype, i, tag, MPI_COMM_WORLD);
	}
}
{ scope s(txt() << "Receiving from task 0");
	MPI_Recv(b, NELEMENTS, MPI_FLOAT, source, tag, MPI_COMM_WORLD, &stat);
	printf("rank= %d  b= %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f\n",
		  rank,b[0],b[1],b[2],b[3],b[4],b[5]);
}
scope(txt() << "b = " << b[0] << " " << b[1] << " " << b[2] << " " << b[3] << " " << b[4] << " " << b[5], scope::minimum);

MPI_Type_free(&indextype);
MPI_Finalize();
}
