#include "test_cd_mpi.h"

CDErrT err = kOK;

//struct DataT {
//   
//   cycle;
//   sizeX;
//   sizeY
//   sizeZ;
//   mA;
//   mB;
//   mC;
//   mD;
//   mE;
//
//   numRanks;
//   col;
//   row;
//   plane;
//   side;
//   datasize;
//   numRegion;
//
//   DataT() 
//   {
//      Init();
//   }
//
//   Data(int _numRanks, int _col, int _row, int _plane, int _side, int _datasize, int _numRegion)
//   {
//      Init();
//      numRanks  = _numRanks ;
//      col       = _col      ;
//      row       = _row      ;
//      plane     = _plane    ;
//      side      = _side     ;
//      datasize  = _datasize ;
//      numRegion = _numRegion;
//   }
//
//   void Init()
//   {
//      cycle = 0;
//      mA = 0;
//      mB = 0;
//      mC = 0;
//      mD = 0;
//      mE = 0;
//      sizeX = 0;
//      sizeY = 0;
//      sizeZ = 0;
//   }
//
//};
//


/******************************************/

static inline
void MainFunc1(DataT& data)
{

   GetCurrentCD()->Begin(true, "MainFunc1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   double sendBuf = 1.0;
   double recvBuf = 0.0;
   MPI_Allreduce(&sendBuf, &recvBuf, 1, 
                 ((sizeof(double) == 4) ? MPI_FLOAT : MPI_DOUBLE),
                 MPI_MIN, MPI_COMM_WORLD);

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void FuncAACA_FFA_Leaf(DataT &data)
{

   GetCurrentCD()->Begin(true, "FuncAACA_FFA_Leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void FuncAAA_Leaf(DataT &data)
{
   
   GetCurrentCD()->Begin(true, "FuncAAA_Leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void FuncFAC_Leaf( DataT& data )
{

   GetCurrentCD()->Begin(true, "FuncFAC_Leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}


/******************************************/

static inline
void FuncAAB_Leaf( DataT &data)
{


   GetCurrentCD()->Begin(true, "FuncAAB_Leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/


static inline
void FuncAAC_Leaf(DataT& data)
{

   GetCurrentCD()->Begin(true, "FuncAAC_Leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

   return ;
}

/******************************************/

static inline
void FuncAA(DataT& data)
{


   GetCurrentCD()->Begin(true, "FuncAA");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   CDHandle* cd = GetCurrentCD()->Create("CD5_0", kStrict, 0, 0, &err);

      FuncAAA_Leaf(data);

      FuncAAB_Leaf( data );

      FuncAAC_Leaf(data);

   cd->Destroy();    // CD5_0 is destroyed
   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline void FuncA(DataT& data)
{
  
   CommRecv(data, MSG_COMM_SBN, 3,
           data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
           true, false) ;

   GetCurrentCD()->Begin(true, "FuncA");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   // Seems possible to start fine-grain CDs from here
   // Spawn multiple children
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_4_0, "CD4_0", kStrict, 0, 0, &err);

      FuncAA(data) ;

   cd->Destroy();    // CD4_0 is destroyed

//  fine-grain CD ends
   DataT* fieldData;
 
   CommSend(data, MSG_COMM_SBN, 3, fieldData,
           data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() +  1,
           true, false) ;
   CommSBN(data, 3, fieldData) ;

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void FuncB(DataT& data)
{ 

   GetCurrentCD()->Begin(true, "FuncB");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void FuncC(DataT& data)
{

   GetCurrentCD()->Begin(true, "FuncC");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void FuncD(DataT &data)
{

   GetCurrentCD()->Begin(true, "FuncD");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void FuncE(DataT &data)
{

   GetCurrentCD()->Begin(true, "FuncE");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void NestedInFunc2_1(DataT& data)
{

   GetCurrentCD()->Begin(true, "NestedInFunc2_1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   CDHandle* cd = GetCurrentCD()->Create("CD3_0", kStrict, 0, 0, &err);

      FuncA(data);  

   cd->Destroy(); // CD3_0 is destroyed

   CommRecv(data, MSG_SYNC_POS_VEL, 6,
            data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
            false, false) ;

   // fine-grain CDs
   cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_1, "CD3_1", kStrict, 0, 0, &err);


      FuncB( data );
      FuncC( data );
      FuncD( data );
      FuncE( data );


   cd->Destroy(); // CD3_1 is destroyed

   DataT* fieldData;
   CommSend(data, MSG_SYNC_POS_VEL, 6, fieldData,
            data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
            false, false) ;
   CommSyncPosVel(data) ;

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

   return;
}


/******************************************/

static inline
void FuncFAB_Leaf( DataT& data )
{


   GetCurrentCD()->Begin(true, "FuncFAB_Leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void FuncFAD_Leaf( DataT& data )
{

   GetCurrentCD()->Begin(true, "FuncFAD_Leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

//static inline
void FuncFA( DataT &data, int count )
{

 
   GetCurrentCD()->Begin(true, "FuncFA");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   CDHandle* cd = GetCurrentCD()->Create("CD4_1", kStrict, 0, 0, &err);


     for( int k=0 ; k<count ; ++k )
     {
   
       FuncAACA_FFA_Leaf(data);
   
       FuncFAB_Leaf(data);
   
       FuncFAC_Leaf(data);
   
       FuncFAD_Leaf(data);
   
     }   // loop ends

   cd->Destroy(); // CD4_1 is destroyed
   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void FuncF(DataT data, int count)
{

   GetCurrentCD()->Begin(true, "FuncF");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   // Fine-grain CD
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_2, "CD3_2", kStrict, 0, 0, &err);



      FuncFA(data, count) ;


   cd->Destroy();  // CD3_2 is destroyed
   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void FuncGA(DataT data, int count)
{

   for (int i = 0 ; i < count ; ++i ) {

      GetCurrentCD()->Begin(true, "FuncGA");
      GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

      GetCurrentCD()->Detect();
      GetCurrentCD()->Complete();

   }  // loop ends

}



/******************************************/

static inline
void FuncGB(DataT data, int count)
{  

   for (int r=0 ; r<count ; ++r) {


      GetCurrentCD()->Begin(true, "FuncGB");
      GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




      GetCurrentCD()->Detect();
      GetCurrentCD()->Complete();


   }



}

/******************************************/
// NOTE for CD
static inline
void FuncG(DataT data, int count)
{

   GetCurrentCD()->Begin(true, "FuncG");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   
   CommRecv(data, MSG_MONOQ, 3,
            data.sizeX(), data.sizeY(), data.sizeZ(),
            true, true) ;
   
   // fine-grain CD 
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_3, "CD3_3", kStrict, 0, 0, &err);

      FuncGA(data, count);

   cd->Destroy(); // CD3_3 is destroyed
   // fine-grain CD ends

   DataT* fieldData;
   CommSend(data, MSG_MONOQ, 3, fieldData,
            data.sizeX(), data.sizeY(), data.sizeZ(),
            true, true) ;

   CommMonoQ(data) ;
   
   // fine-grain CD
   cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_4, "CD3_4", kStrict, 0, 0, &err);

      FuncGB(data, count) ;

   cd->Destroy();    // CD3_4 is destroyed
      // fine-grain CD ends

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void FuncHAAA_Leaf(DataT& data)
{

   GetCurrentCD()->Begin(true, "FuncHAAA_Leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void FuncHAA(DataT data, int count)
{

   FuncHAAA_Leaf(data);

   GetCurrentCD()->Begin(true, "FuncHAAA_leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   for (int i = 0 ; i < count ; ++i) {
      //
   }

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

   FuncHAAA_Leaf(data);

   GetCurrentCD()->Begin(true, "FuncHAAA_leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   for (int i = 0 ; i < count ; ++i) {
      //
   }

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

   FuncHAAA_Leaf(data);

   GetCurrentCD()->Begin(true, "FuncHAAA_leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   for (int i = 0 ; i < count ; ++i) {
      //
   }

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

   return ;
}

/******************************************/
// FIXME : CD
static inline
void FuncHAB_leaf(DataT &data)
{

   GetCurrentCD()->Begin(true, "FuncHAB_leaf");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/
//FIXME : CD
static inline
void FuncHA(DataT data, int count)
{

   GetCurrentCD()->Begin(true, "FuncHA");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   CDHandle* cd = GetCurrentCD()->Create("CD5_1", kStrict, 0, 0, &err);


      for(int j = 0; j < count; j++) {
   
         GetCurrentCD()->Begin(true, "loop 0 in FuncHA");
         GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   
          // Body
   
         GetCurrentCD()->Detect();
         GetCurrentCD()->Complete();
   
         FuncHAA(data, count);
   
      }  // loop ends


   cd->Destroy();    // CD5_1 is destroyed
   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

   FuncHAB_leaf(data);

}

/******************************************/
// NOTE : CD
static inline
void FuncH(DataT data, int count)
{
   GetCurrentCD()->Begin(true, "FuncH");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   // fine-grain CD start
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_5, "CD3_5", kStrict, 0, 0, &err);
  
  
      for (int r=0 ; r<count ; r++) {
  
         GetCurrentCD()->Begin(true, "Loop in FuncH");
         GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
  
  
            CDHandle* cd2 = GetCurrentCD()->Create("CD4_3", kStrict, 0, 0, &err);
     
            FuncHA(data, count);
     
            cd2->Destroy();  // CD4_3 is destroyed
  
         GetCurrentCD()->Detect();
         GetCurrentCD()->Complete();
  
      }
  
   cd->Destroy();   // CD3_5 is destroyed


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void FuncI(DataT &data, int count)
{

   GetCurrentCD()->Begin(true, "FuncI");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


   return ;
}

/******************************************/

static inline
void NestedInFunc2_2(DataT data, int count)
{

  FuncF(data, count) ;

  FuncG(data, count) ;

  FuncH(data, count) ;

  FuncI(data, count) ;

}

/******************************************/

static inline
void FuncY(DataT &data, int count)
{


   GetCurrentCD()->Begin(true, "FuncY");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);






   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();



   return ;

}

/******************************************/

static inline
void FuncZ(DataT &data, int count)
{


   GetCurrentCD()->Begin(true, "FuncZ");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


   return ;
}

/******************************************/

static inline
void NestedInFunc2_3(DataT& data, int count) {

   // CD2
   GetCurrentCD()->Begin(true, "NestedInFunc2_3");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   
   // Fine-grain CDs start
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_6, "CD3_6", kStrict, 0, 0, &err);


      for (int r=0 ; r < count ; ++r) {
         
         FuncY(data, count);
   
         FuncZ(data, count);
      
      }


   cd->Destroy(); // CD3_6 is destroyed

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void MainFunc2(DataT& data, int count)
{

   GetCurrentCD()->Begin(true, "MainFunc2");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   CDHandle* cd = GetCurrentCD()->Create("CD2", kStrict, 0, 0, &err);

      NestedInFunc2_1(data);  
      NestedInFunc2_2(data, count);
   
      CommRecv(data, MSG_SYNC_POS_VEL, 6,
               data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
               false, false) ;
      
   DataT* fieldData;
      CommSend(data, MSG_SYNC_POS_VEL, 6, fieldData,
               data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
               false, false) ;
   
      NestedInFunc2_3(data, count);
   
      CommSyncPosVel(data) ;

   cd->Destroy(); // CD2 is destroyed

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}



/******************************************/


int main(int argc, char *argv[])
{
   DataT *data ;
   int numRanks ;
   int myRank ;
   int datasize = 10;
   int numRegion = 11;
   int iter_count = 10;
   int innerloopcount = 20;

   MPI_Init(&argc, &argv) ;
   MPI_Comm_size(MPI_COMM_WORLD, &numRanks) ;
   MPI_Comm_rank(MPI_COMM_WORLD, &myRank) ;



   // Set up the mesh and decompose. Assumes regular cubes for now
   int col, row, plane, side;
   InitMeshDecomp(numRanks, myRank, &col, &row, &plane, &side);

   // Build the main data structure and initialize it
   data = new DataT(numRanks, col, row, plane, side, datasize, numRegion) ;


   CDHandle* root_cd = CD_Init(numRanks, myRank);

   root_cd->Begin(true, "Root Begin");
   root_cd->Preserve(data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   // Initial data boundary communication 
   //DataT fieldData = &DataT::nodalMass ;
   DataT fieldData;
   CommRecv(*data, MSG_COMM_SBN, 1,
            data->sizeX() + 1, data->sizeY() + 1, data->sizeZ() + 1,
            true, false) ;
   CommSend(*data, MSG_COMM_SBN, 1, &fieldData,
            data->sizeX() + 1, data->sizeY() + 1, data->sizeZ() +  1,
            true, false) ;
   CommSBN(*data, 1, &fieldData) ;

   // End initialization
   MPI_Barrier(MPI_COMM_WORLD);
   
   
   // BEGIN timestep to solution */
   double start;
   start = MPI_Wtime();
   start = clock();

   root_cd->Detect();
   root_cd->Complete();

   root_cd->Begin(true, "Root Before Main Loop");
   root_cd->Preserve(data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   CDHandle* cd1_for_main_loop = root_cd->Create(root_cd->GetNodeID(), NUM_CDS_IN_LEVEL_1, "CD1", kStrict, 0, 0, &err);
   printf("\n\n<-------------------------------- Main Loop start --------------------------------->\n\n");

   // Main loop start
//   while( data->cycle < iter_count ) {

      // Main functions in the loop
      MainFunc1(*data) ;
      MainFunc2(*data, innerloopcount) ;
      data->cycle++;

      printf("\n\t-------------------------------- Loop %d --------------------------------->\t\n");
//   }

   printf("\n\n<-------------------------------- Main Loop ends --------------------------------->\n\n");
   cd1_for_main_loop->Destroy();

   double elapsed_time = MPI_Wtime() - start;   
   printf("Elapsed time for test program is %f\n", elapsed_time);
   
   root_cd->Detect();
   root_cd->Complete();

   CD_Finalize();
   MPI_Finalize() ;

   return 0 ;
}
