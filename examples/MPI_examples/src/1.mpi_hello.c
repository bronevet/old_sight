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
int   numtasks, taskid, len;
char hostname[MPI_MAX_PROCESSOR_NAME];

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
MPI_Finalize();

}
