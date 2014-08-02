#include <mpi.h>
#include <iostream>
#include <omp.h>
#include "cd.h"
#include "cd_handle.h"

#define MSG_COMM_SBN      1024
#define MSG_MONOQ         3072
#define MSG_SYNC_POS_VEL  2048
#define MAX_FIELDS_PER_MPI_COMM 6

#define CACHE_COHERENCE_PAD_REAL (128 / sizeof(double))

#define CACHE_ALIGN_REAL(n) \
   (((n) + (CACHE_COHERENCE_PAD_REAL - 1)) & ~(CACHE_COHERENCE_PAD_REAL-1))

using namespace cd;

#define NUM_CDS_IN_LEVEL_1   8
#define NUM_CDS_IN_LEVEL_2   1
#define NUM_CDS_IN_LEVEL_3_0 1
#define NUM_CDS_IN_LEVEL_3_1 1 
#define NUM_CDS_IN_LEVEL_3_2 1
#define NUM_CDS_IN_LEVEL_3_3 1
#define NUM_CDS_IN_LEVEL_3_4 1
#define NUM_CDS_IN_LEVEL_3_5 1
#define NUM_CDS_IN_LEVEL_3_6 1
#define NUM_CDS_IN_LEVEL_4_0 1
#define NUM_CDS_IN_LEVEL_4_1 1
#define NUM_CDS_IN_LEVEL_4_2 1
#define NUM_CDS_IN_LEVEL_4_3 1
#define NUM_CDS_IN_LEVEL_5_0 1
#define NUM_CDS_IN_LEVEL_5_1 1
#define NUM_CDS_IN_LEVEL_6_0 1
#define NUM_CDS_IN_LEVEL_6_1 1
#define NUM_CDS_IN_LEVEL_7   1


struct DataT {
   
   int cycle;
   int mA;
   int mB;
   int mC;
   int mD;
   int mE;

   int numRanks ;
   int col      ;
   int row      ;
   int plane    ;
   int side     ;
   int datasize ;
   int numRegion;

   int sizeX_;
   int sizeY_;
   int sizeZ_;

   DataT() 
   {
      Init();
   }


   DataT(int _numRanks, int _col, int _row, int _plane, int _side, int _datasize, int _numRegion)
   {
      Init();
      numRanks  = _numRanks ;
      col       = _col      ;
      row       = _row      ;
      plane     = _plane    ;
      side      = _side     ;
      datasize  = _datasize ;
      numRegion = _numRegion;
   }

   void Init()
   {
      cycle = 0;
      mA = 0;
      mB = 0;
      mC = 0;
      mD = 0;
      mE = 0;
      sizeX_ = 0;
      sizeY_ = 0;
      sizeZ_ = 0;
   }

   int sizeX() { return sizeX_; }
   int sizeY() { return sizeY_; }
   int sizeZ() { return sizeZ_; }
};

void CommRecv(DataT& data, int msgType, int xferFields,                    int dx, int dy, int dz, bool doRecv, bool planeOnly);
void CommSend(DataT& data, int msgType, int xferFields, DataT *fieldData,  int dx, int dy, int dz, bool doSend, bool planeOnly);
void CommSBN(DataT& data,               int xferFields, DataT *fieldData);
void CommSyncPosVel(DataT& data);
void CommMonoQ(DataT& data);

void InitMeshDecomp(int numRanks, int myRank, int *col, int *row, int *plane, int *side);
