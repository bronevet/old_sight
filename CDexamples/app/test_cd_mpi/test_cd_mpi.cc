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
void Func_CD1_v0(DataT& data)
{

   GetCurrentCD()->Begin(true, "CD1_v0");
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
void Func_CD4_1_v0(DataT &data)
{

   GetCurrentCD()->Begin(true, "CD4_1_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void Func_CD5_0_v0(DataT &data)
{
   
   GetCurrentCD()->Begin(true, "CD5_0_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void Func_CD4_1_v2( DataT& data )
{

   GetCurrentCD()->Begin(true, "CD4_1_v2");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}


/******************************************/

static inline
void Func_CD5_0_v1( DataT &data)
{


   GetCurrentCD()->Begin(true, "CD5_0_v1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/


static inline
void Func_CD5_0_v2(DataT& data)
{

   GetCurrentCD()->Begin(true, "CD5_0_v2");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

   return ;
}

/******************************************/

static inline
void Func_CD4_0_v0(DataT& data)
{


   GetCurrentCD()->Begin(true, "CD4_0_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   CDHandle* cd = GetCurrentCD()->Create("CD5_0", kStrict, 0, 0, &err);

      Func_CD5_0_v0(data);

      Func_CD5_0_v1( data );

      Func_CD5_0_v2(data);

   cd->Destroy();    // CD5_0 is destroyed
   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline void Func_CD3_0_v0(DataT& data)
{
  
   CommRecv(data, MSG_COMM_SBN, 3,
           data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
           true, false) ;

   GetCurrentCD()->Begin(true, "CD3_0_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   // Seems possible to start fine-grain CDs from here
   // Spawn multiple children
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_4_0, "CD4_0", kStrict, 0, 0, &err);

      Func_CD4_0_v0(data) ;

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
void Func_CD3_1_v0(DataT& data)
{ 

   GetCurrentCD()->Begin(true, "CD3_1_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void Func_CD3_1_v1(DataT& data)
{

   GetCurrentCD()->Begin(true, "CD3_1_v1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void Func_CD3_1_v2(DataT &data)
{

   GetCurrentCD()->Begin(true, "CD3_1_v2");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

static inline
void Func_CD3_1_v3(DataT &data)
{

   GetCurrentCD()->Begin(true, "CD3_1_v3");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void Func_CD2_v0(DataT& data)
{

   GetCurrentCD()->Begin(true, "CD2_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   CDHandle* cd = GetCurrentCD()->Create("CD3_0", kStrict, 0, 0, &err);

      Func_CD3_0_v0(data);  

   cd->Destroy(); // CD3_0 is destroyed

   CommRecv(data, MSG_SYNC_POS_VEL, 6,
            data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
            false, false) ;

   // fine-grain CDs
   cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_1, "CD3_1", kStrict, 0, 0, &err);


      Func_CD3_1_v0( data );
      Func_CD3_1_v1( data );
      Func_CD3_1_v2( data );
      Func_CD3_1_v3( data );


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
void Func_CD4_1_v1( DataT& data )
{


   GetCurrentCD()->Begin(true, "CD4_1_v1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void Func_CD4_1_v3( DataT& data )
{

   GetCurrentCD()->Begin(true, "CD4_1_v3");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);



   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();

}

/******************************************/

//static inline
void Func_CD3_2_v0( DataT &data, int count )
{

 
   GetCurrentCD()->Begin(true, "CD3_2_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   CDHandle* cd = GetCurrentCD()->Create("CD4_1", kStrict, 0, 0, &err);


     for( int k=0 ; k<count ; ++k )
     {
   
       Func_CD4_1_v0(data);
   
       Func_CD4_1_v1(data);
   
       Func_CD4_1_v2(data);
   
       Func_CD4_1_v3(data);
   
     }   // loop ends

   cd->Destroy(); // CD4_1 is destroyed
   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void Func_CD2_v1_0(DataT data, int count)
{

   GetCurrentCD()->Begin(true, "CD2_v1_0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   // Fine-grain CD
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_2, "CD3_2", kStrict, 0, 0, &err);



      Func_CD3_2_v0(data, count) ;


   cd->Destroy();  // CD3_2 is destroyed
   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void Func_CD3_3_v0(DataT data, int count)
{

   for (int i = 0 ; i < count ; ++i ) {

      GetCurrentCD()->Begin(true, "CD3_3_v0");
      GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

      GetCurrentCD()->Detect();
      GetCurrentCD()->Complete();

   }  // loop ends

}



/******************************************/

static inline
void Func_CD3_4_v0(DataT data, int count)
{  

   for (int r=0 ; r<count ; ++r) {


      GetCurrentCD()->Begin(true, "CD3_4_v0");
      GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




      GetCurrentCD()->Detect();
      GetCurrentCD()->Complete();


   }



}

/******************************************/
// NOTE for CD
static inline
void Func_CD2_v1_1(DataT data, int count)
{

   GetCurrentCD()->Begin(true, "CD2_v1_1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   
   CommRecv(data, MSG_MONOQ, 3,
            data.sizeX(), data.sizeY(), data.sizeZ(),
            true, true) ;
   
   // fine-grain CD 
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_3, "CD3_3", kStrict, 0, 0, &err);

      Func_CD3_3_v0(data, count);

   cd->Destroy(); // CD3_3 is destroyed
   // fine-grain CD ends

   DataT* fieldData;
   CommSend(data, MSG_MONOQ, 3, fieldData,
            data.sizeX(), data.sizeY(), data.sizeZ(),
            true, true) ;

   CommMonoQ(data) ;
   
   // fine-grain CD
   cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_4, "CD3_4", kStrict, 0, 0, &err);

      Func_CD3_4_v0(data, count) ;

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
void Func_CD5_1_v1(DataT data, int count)
{


   GetCurrentCD()->Begin(true, "CD5_1_v1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);


   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


   return ;
}


/******************************************/
//FIXME : CD
static inline
void Func_CD4_3_v0(DataT data, int count)
{

   GetCurrentCD()->Begin(true, "CD4_3_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   CDHandle* cd = GetCurrentCD()->Create("CD5_1", kStrict, 0, 0, &err);


      for(int j = 0; j < count; j++) {
   
         GetCurrentCD()->Begin(true, "CD5_1_v0");
         GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   
          // Body
   
         GetCurrentCD()->Detect();
         GetCurrentCD()->Complete();
   
         Func_CD5_1_v1(data, count);
   
      }  // loop ends


   cd->Destroy();    // CD5_1 is destroyed
   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/
// NOTE : CD
static inline
void Func_CD2_v1_2(DataT data, int count)
{
   GetCurrentCD()->Begin(true, "CD2_v1_2");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   // fine-grain CD start
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_5, "CD3_5", kStrict, 0, 0, &err);
  
  
      for (int r=0 ; r<count ; r++) {
  
         GetCurrentCD()->Begin(true, "CD3_5_v0");
         GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
  
  
            CDHandle* cd2 = GetCurrentCD()->Create("CD4_3", kStrict, 0, 0, &err);
     
            Func_CD4_3_v0(data, count);
     
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
void Func_CD2_v1_3(DataT &data, int count)
{

   GetCurrentCD()->Begin(true, "CD2_v1_3");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


   return ;
}

/******************************************/

static inline
void Func_CD2_v1(DataT data, int count)
{

  Func_CD2_v1_0(data, count) ;

  Func_CD2_v1_1(data, count) ;

  Func_CD2_v1_2(data, count) ;

  Func_CD2_v1_3(data, count) ;

}

/******************************************/

static inline
void Func_CD3_6_v0(DataT &data, int count)
{


   GetCurrentCD()->Begin(true, "CD3_6_v0");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);






   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();



   return ;

}

/******************************************/

static inline
void Func_CD3_6_v1(DataT &data, int count)
{


   GetCurrentCD()->Begin(true, "CD3_6_v1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);




   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


   return ;
}

/******************************************/

static inline
void Func_CD2_v2(DataT& data, int count) {

   // CD2
   GetCurrentCD()->Begin(true, "CD2_v2");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);
   
   // Fine-grain CDs start
   CDHandle* cd = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), NUM_CDS_IN_LEVEL_3_6, "CD3_6", kStrict, 0, 0, &err);


      for (int r=0 ; r < count ; ++r) {
         
         Func_CD3_6_v0(data, count);
   
         Func_CD3_6_v1(data, count);
      
      }


   cd->Destroy(); // CD3_6 is destroyed

   GetCurrentCD()->Detect();
   GetCurrentCD()->Complete();


}

/******************************************/

static inline
void Func_CD1_v1(DataT& data, int count)
{

   GetCurrentCD()->Begin(true, "CD1_v1");
   GetCurrentCD()->Preserve(&data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   CDHandle* cd = GetCurrentCD()->Create("CD2", kStrict, 0, 0, &err);

      Func_CD2_v0(data);  
      Func_CD2_v1(data, count);
   
      CommRecv(data, MSG_SYNC_POS_VEL, 6,
               data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
               false, false) ;
      
   DataT* fieldData;
      CommSend(data, MSG_SYNC_POS_VEL, 6, fieldData,
               data.sizeX() + 1, data.sizeY() + 1, data.sizeZ() + 1,
               false, false) ;
   
      Func_CD2_v2(data, count);
   
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
   int datasize = 5;
   int numRegion = 5;
   int iter_count = 5;
   int innerloopcount = 5;

   MPI_Init(&argc, &argv) ;
   MPI_Comm_size(MPI_COMM_WORLD, &numRanks) ;
   MPI_Comm_rank(MPI_COMM_WORLD, &myRank) ;



   // Set up the mesh and decompose. Assumes regular cubes for now
   int col, row, plane, side;
   InitMeshDecomp(numRanks, myRank, &col, &row, &plane, &side);

   // Build the main data structure and initialize it
   data = new DataT(numRanks, col, row, plane, side, datasize, numRegion) ;


   CDHandle* root_cd = CD_Init(numRanks, myRank);

   root_cd->Begin(true, "CD0_v0");
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

   root_cd->Begin(true, "CD0_v1");
   root_cd->Preserve(data, sizeof(data), kCopy, "data", 0, 0, 0, kUnsure);

   CDHandle* cd1_for_main_loop = root_cd->Create(root_cd->GetNodeID(), NUM_CDS_IN_LEVEL_1, "CD1", kStrict, 0, 0, &err);
   printf("\n\n<-------------------------------- Main Loop start --------------------------------->\n\n");

   // Main loop start
//   while( data->cycle < iter_count ) {

      // Main functions in the loop
      Func_CD1_v0(*data) ;
      Func_CD1_v1(*data, innerloopcount) ;
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
