/****************************************************************************
* Copyright � Udayanga Wickramasinghe - Indiana University                 *
*                                                                          *
****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <string>
#include "mrnet/MRNet.h"
#include "mrnet_integration.h"
#include "AtomicSyncPrimitives.h"


//#define VERBOSE

/**
* This class is responsible for following
*   - initialize MRNet FE (for lazy init- will wait untill all BE nodes are available)
*     and MRNet Filters
*   - recv/read the final merged output as a character stream and output it
*/

using namespace MRN;
using namespace std;
using namespace atomiccontrols;

extern bool saw_failure ;
extern FILE* merge_output_descr;

extern int num_attach_callbacks;
extern int num_detach_callbacks;

extern atomic_mutex_t cb_lock ;
extern AtomicSync locker ;

void BE_Add_Callback( Event* evt, void* evt_data );
void BE_Remove_Callback( Event* evt, void* evt_data );


void printLeafInfo(std::vector< NetworkTopology::Node * >& leaves, int num_be, char* conn_info);

string createDir(string workDir, string dirName) ;


int startup(char* topology_file, char* so_file, char* structureFile_path, int num_commnodes, int num_backends);
