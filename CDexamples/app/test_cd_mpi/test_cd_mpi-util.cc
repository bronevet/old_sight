#include "test_cd_mpi.h"
#include <mpi.h>
#include <string.h>
#include <cmath>

/* Comm Routines */

#define ALLOW_UNPACKED_PLANE false
#define ALLOW_UNPACKED_ROW   false
#define ALLOW_UNPACKED_COL   false



/******************************************/


/* doRecv flag only works with regular block structure */
void CommRecv(DataT& data, int msgType, int xferFields,
              int dx, int dy, int dz, bool doRecv, bool planeOnly) {
}

//void OrigCommRecv(DataT& data, int msgType, int xferFields,
//              int dx, int dy, int dz, bool doRecv, bool planeOnly) {
//   if (data.numRanks() == 1)
//      return ;
//
//   /* post recieve buffers for all incoming messages */
//   int myRank ;
//   int maxPlaneComm = xferFields * data.maxPlaneSize() ;
//   int maxEdgeComm  = xferFields * data.maxEdgeSize() ;
//   int pmsg = 0 ; /* plane comm msg */
//   int emsg = 0 ; /* edge comm msg */
//   int cmsg = 0 ; /* corner comm msg */
//   MPI_Datatype baseType = ((sizeof(double) == 4) ? MPI_FLOAT : MPI_DOUBLE) ;
//   bool rowMin, rowMax, colMin, colMax, planeMin, planeMax ;
//
//   /* assume communication to 6 neighbors by default */
//   rowMin = rowMax = colMin = colMax = planeMin = planeMax = true ;
//
//   if (data.rowLoc() == 0) {
//      rowMin = false ;
//   }
//   if (data.rowLoc() == (data.tp()-1)) {
//      rowMax = false ;
//   }
//   if (data.colLoc() == 0) {
//      colMin = false ;
//   }
//   if (data.colLoc() == (data.tp()-1)) {
//      colMax = false ;
//   }
//   if (data.planeLoc() == 0) {
//      planeMin = false ;
//   }
//   if (data.planeLoc() == (data.tp()-1)) {
//      planeMax = false ;
//   }
//
//   for (int i=0; i<26; ++i) {
//      data.recvRequest[i] = MPI_REQUEST_NULL ;
//   }
//
//   MPI_Comm_rank(MPI_COMM_WORLD, &myRank) ;
//
//   /* post receives */
//
//   /* receive data from neighboring data faces */
//   if (planeMin && doRecv) {
//      /* contiguous memory */
//      int fromRank = myRank - data.tp()*data.tp() ;
//      int recvCount = dx * dy * xferFields ;
//      MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm],
//                recvCount, baseType, fromRank, msgType,
//                MPI_COMM_WORLD, &data.recvRequest[pmsg]) ;
//      ++pmsg ;
//   }
//   if (planeMax) {
//      /* contiguous memory */
//      int fromRank = myRank + data.tp()*data.tp() ;
//      int recvCount = dx * dy * xferFields ;
//      MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm],
//                recvCount, baseType, fromRank, msgType,
//                MPI_COMM_WORLD, &data.recvRequest[pmsg]) ;
//      ++pmsg ;
//   }
//   if (rowMin && doRecv) {
//      /* semi-contiguous memory */
//      int fromRank = myRank - data.tp() ;
//      int recvCount = dx * dz * xferFields ;
//      MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm],
//                recvCount, baseType, fromRank, msgType,
//                MPI_COMM_WORLD, &data.recvRequest[pmsg]) ;
//      ++pmsg ;
//   }
//   if (rowMax) {
//      /* semi-contiguous memory */
//      int fromRank = myRank + data.tp() ;
//      int recvCount = dx * dz * xferFields ;
//      MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm],
//                recvCount, baseType, fromRank, msgType,
//                MPI_COMM_WORLD, &data.recvRequest[pmsg]) ;
//      ++pmsg ;
//   }
//   if (colMin && doRecv) {
//      /* scattered memory */
//      int fromRank = myRank - 1 ;
//      int recvCount = dy * dz * xferFields ;
//      MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm],
//                recvCount, baseType, fromRank, msgType,
//                MPI_COMM_WORLD, &data.recvRequest[pmsg]) ;
//      ++pmsg ;
//   }
//   if (colMax) {
//      /* scattered memory */
//      int fromRank = myRank + 1 ;
//      int recvCount = dy * dz * xferFields ;
//      MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm],
//                recvCount, baseType, fromRank, msgType,
//                MPI_COMM_WORLD, &data.recvRequest[pmsg]) ;
//      ++pmsg ;
//   }
//
//   if (!planeOnly) {
//      /* receive data from datas connected only by an edge */
//      if (rowMin && colMin && doRecv) {
//         int fromRank = myRank - data.tp() - 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dz * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMin && planeMin && doRecv) {
//         int fromRank = myRank - data.tp()*data.tp() - data.tp() ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dx * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (colMin && planeMin && doRecv) {
//         int fromRank = myRank - data.tp()*data.tp() - 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dy * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMax && colMax) {
//         int fromRank = myRank + data.tp() + 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dz * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMax && planeMax) {
//         int fromRank = myRank + data.tp()*data.tp() + data.tp() ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dx * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (colMax && planeMax) {
//         int fromRank = myRank + data.tp()*data.tp() + 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dy * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMax && colMin) {
//         int fromRank = myRank + data.tp() - 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dz * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMin && planeMax) {
//         int fromRank = myRank + data.tp()*data.tp() - data.tp() ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dx * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (colMin && planeMax) {
//         int fromRank = myRank + data.tp()*data.tp() - 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dy * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMin && colMax && doRecv) {
//         int fromRank = myRank - data.tp() + 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dz * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMax && planeMin && doRecv) {
//         int fromRank = myRank - data.tp()*data.tp() + data.tp() ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dx * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (colMax && planeMin && doRecv) {
//         int fromRank = myRank - data.tp()*data.tp() + 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm],
//                   dy * xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      /* receive data from datas connected only by a corner */
//      if (rowMin && colMin && planeMin && doRecv) {
//         /* corner at data logical coord (0, 0, 0) */
//         int fromRank = myRank - data.tp()*data.tp() - data.tp() - 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL],
//                   xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMin && colMin && planeMax) {
//         /* corner at data logical coord (0, 0, 1) */
//         int fromRank = myRank + data.tp()*data.tp() - data.tp() - 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL],
//                   xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMin && colMax && planeMin && doRecv) {
//         /* corner at data logical coord (1, 0, 0) */
//         int fromRank = myRank - data.tp()*data.tp() - data.tp() + 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL],
//                   xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMin && colMax && planeMax) {
//         /* corner at data logical coord (1, 0, 1) */
//         int fromRank = myRank + data.tp()*data.tp() - data.tp() + 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL],
//                   xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMax && colMin && planeMin && doRecv) {
//         /* corner at data logical coord (0, 1, 0) */
//         int fromRank = myRank - data.tp()*data.tp() + data.tp() - 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL],
//                   xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMax && colMin && planeMax) {
//         /* corner at data logical coord (0, 1, 1) */
//         int fromRank = myRank + data.tp()*data.tp() + data.tp() - 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL],
//                   xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMax && colMax && planeMin && doRecv) {
//         /* corner at data logical coord (1, 1, 0) */
//         int fromRank = myRank - data.tp()*data.tp() + data.tp() + 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL],
//                   xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMax && colMax && planeMax) {
//         /* corner at data logical coord (1, 1, 1) */
//         int fromRank = myRank + data.tp()*data.tp() + data.tp() + 1 ;
//         MPI_Irecv(&data.commDataRecv[pmsg * maxPlaneComm +
//                                         emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL],
//                   xferFields, baseType, fromRank, msgType,
//                   MPI_COMM_WORLD, &data.recvRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//   }
//}

/******************************************/

void CommSend(DataT& data, int msgType,
              int xferFields, DataT *fieldData,
              int dx, int dy, int dz, bool doSend, bool planeOnly)
{}

//void OrigCommSend(DataT& data, int msgType,
//              int xferFields, DataT_member *fieldData,
//              int dx, int dy, int dz, bool doSend, bool planeOnly)
//{
//   if (data.numRanks() == 1)
//      return ;
//
//   /* post recieve buffers for all incoming messages */
//   int myRank ;
//   int maxPlaneComm = xferFields * data.maxPlaneSize() ;
//   int maxEdgeComm  = xferFields * data.maxEdgeSize() ;
//   int pmsg = 0 ; /* plane comm msg */
//   int emsg = 0 ; /* edge comm msg */
//   int cmsg = 0 ; /* corner comm msg */
//   MPI_Datatype baseType = ((sizeof(double) == 4) ? MPI_FLOAT : MPI_DOUBLE) ;
//   MPI_Status status[26] ;
//   double *destAddr ;
//   bool rowMin, rowMax, colMin, colMax, planeMin, planeMax ;
//   /* assume communication to 6 neighbors by default */
//   rowMin = rowMax = colMin = colMax = planeMin = planeMax = true ;
//   if (data.rowLoc() == 0) {
//      rowMin = false ;
//   }
//   if (data.rowLoc() == (data.tp()-1)) {
//      rowMax = false ;
//   }
//   if (data.colLoc() == 0) {
//      colMin = false ;
//   }
//   if (data.colLoc() == (data.tp()-1)) {
//      colMax = false ;
//   }
//   if (data.planeLoc() == 0) {
//      planeMin = false ;
//   }
//   if (data.planeLoc() == (data.tp()-1)) {
//      planeMax = false ;
//   }
//
//   for (int i=0; i<26; ++i) {
//      data.sendRequest[i] = MPI_REQUEST_NULL ;
//   }
//
//   MPI_Comm_rank(MPI_COMM_WORLD, &myRank) ;
//
//   /* post sends */
//
//   if (planeMin | planeMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int sendCount = dx * dy ;
//
//      if (planeMin) {
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm] ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<sendCount; ++i) {
//               destAddr[i] = (data.*src)(i) ;
//            }
//            destAddr += sendCount ;
//         }
//         destAddr -= xferFields*sendCount ;
//
//         MPI_Isend(destAddr, xferFields*sendCount, baseType,
//                   myRank - data.tp()*data.tp(), msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg]) ;
//         ++pmsg ;
//      }
//      if (planeMax && doSend) {
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm] ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<sendCount; ++i) {
//               destAddr[i] = (data.*src)(dx*dy*(dz - 1) + i) ;
//            }
//            destAddr += sendCount ;
//         }
//         destAddr -= xferFields*sendCount ;
//
//         MPI_Isend(destAddr, xferFields*sendCount, baseType,
//                   myRank + data.tp()*data.tp(), msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg]) ;
//         ++pmsg ;
//      }
//   }
//   if (rowMin | rowMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int sendCount = dx * dz ;
//
//      if (rowMin) {
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dx; ++j) {
//                  destAddr[i*dx+j] = (data.*src)(i*dx*dy + j) ;
//               }
//            }
//            destAddr += sendCount ;
//         }
//         destAddr -= xferFields*sendCount ;
//
//         MPI_Isend(destAddr, xferFields*sendCount, baseType,
//                   myRank - data.tp(), msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg]) ;
//         ++pmsg ;
//      }
//      if (rowMax && doSend) {
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dx; ++j) {
//                  destAddr[i*dx+j] = (data.*src)(dx*(dy - 1) + i*dx*dy + j) ;
//               }
//            }
//            destAddr += sendCount ;
//         }
//         destAddr -= xferFields*sendCount ;
//
//         MPI_Isend(destAddr, xferFields*sendCount, baseType,
//                   myRank + data.tp(), msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg]) ;
//         ++pmsg ;
//      }
//   }
//   if (colMin | colMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int sendCount = dy * dz ;
//
//      if (colMin) {
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dy; ++j) {
//                  destAddr[i*dy + j] = (data.*src)(i*dx*dy + j*dx) ;
//               }
//            }
//            destAddr += sendCount ;
//         }
//         destAddr -= xferFields*sendCount ;
//
//         MPI_Isend(destAddr, xferFields*sendCount, baseType,
//                   myRank - 1, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg]) ;
//         ++pmsg ;
//      }
//      if (colMax && doSend) {
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dy; ++j) {
//                  destAddr[i*dy + j] = (data.*src)(dx - 1 + i*dx*dy + j*dx) ;
//               }
//            }
//            destAddr += sendCount ;
//         }
//         destAddr -= xferFields*sendCount ;
//
//         MPI_Isend(destAddr, xferFields*sendCount, baseType,
//                   myRank + 1, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg]) ;
//         ++pmsg ;
//      }
//   }
//
//   if (!planeOnly) {
//      if (rowMin && colMin) {
//         int toRank = myRank - data.tp() - 1 ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               destAddr[i] = (data.*src)(i*dx*dy) ;
//            }
//            destAddr += dz ;
//         }
//         destAddr -= xferFields*dz ;
//         MPI_Isend(destAddr, xferFields*dz, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMin && planeMin) {
//         int toRank = myRank - data.tp()*data.tp() - data.tp() ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dx; ++i) {
//               destAddr[i] = (data.*src)(i) ;
//            }
//            destAddr += dx ;
//         }
//         destAddr -= xferFields*dx ;
//         MPI_Isend(destAddr, xferFields*dx, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (colMin && planeMin) {
//         int toRank = myRank - data.tp()*data.tp() - 1 ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dy; ++i) {
//               destAddr[i] = (data.*src)(i*dx) ;
//            }
//            destAddr += dy ;
//         }
//         destAddr -= xferFields*dy ;
//         MPI_Isend(destAddr, xferFields*dy, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMax && colMax && doSend) {
//         int toRank = myRank + data.tp() + 1 ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               destAddr[i] = (data.*src)(dx*dy - 1 + i*dx*dy) ;
//            }
//            destAddr += dz ;
//         }
//         destAddr -= xferFields*dz ;
//         MPI_Isend(destAddr, xferFields*dz, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMax && planeMax && doSend) {
//         int toRank = myRank + data.tp()*data.tp() + data.tp() ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dx; ++i) {
//              destAddr[i] = (data.*src)(dx*(dy-1) + dx*dy*(dz-1) + i) ;
//            }
//            destAddr += dx ;
//         }
//         destAddr -= xferFields*dx ;
//         MPI_Isend(destAddr, xferFields*dx, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (colMax && planeMax && doSend) {
//         int toRank = myRank + data.tp()*data.tp() + 1 ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dy; ++i) {
//               destAddr[i] = (data.*src)(dx*dy*(dz-1) + dx - 1 + i*dx) ;
//            }
//            destAddr += dy ;
//         }
//         destAddr -= xferFields*dy ;
//         MPI_Isend(destAddr, xferFields*dy, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMax && colMin && doSend) {
//         int toRank = myRank + data.tp() - 1 ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               destAddr[i] = (data.*src)(dx*(dy-1) + i*dx*dy) ;
//            }
//            destAddr += dz ;
//         }
//         destAddr -= xferFields*dz ;
//         MPI_Isend(destAddr, xferFields*dz, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMin && planeMax && doSend) {
//         int toRank = myRank + data.tp()*data.tp() - data.tp() ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dx; ++i) {
//               destAddr[i] = (data.*src)(dx*dy*(dz-1) + i) ;
//            }
//            destAddr += dx ;
//         }
//         destAddr -= xferFields*dx ;
//         MPI_Isend(destAddr, xferFields*dx, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (colMin && planeMax && doSend) {
//         int toRank = myRank + data.tp()*data.tp() - 1 ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dy; ++i) {
//               destAddr[i] = (data.*src)(dx*dy*(dz-1) + i*dx) ;
//            }
//            destAddr += dy ;
//         }
//         destAddr -= xferFields*dy ;
//         MPI_Isend(destAddr, xferFields*dy, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMin && colMax) {
//         int toRank = myRank - data.tp() + 1 ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               destAddr[i] = (data.*src)(dx - 1 + i*dx*dy) ;
//            }
//            destAddr += dz ;
//         }
//         destAddr -= xferFields*dz ;
//         MPI_Isend(destAddr, xferFields*dz, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMax && planeMin) {
//         int toRank = myRank - data.tp()*data.tp() + data.tp() ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dx; ++i) {
//               destAddr[i] = (data.*src)(dx*(dy - 1) + i) ;
//            }
//            destAddr += dx ;
//         }
//         destAddr -= xferFields*dx ;
//         MPI_Isend(destAddr, xferFields*dx, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (colMax && planeMin) {
//         int toRank = myRank - data.tp()*data.tp() + 1 ;
//         destAddr = &data.commDataSend[pmsg * maxPlaneComm +
//                                          emsg * maxEdgeComm] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            DataT_member src = fieldData[fi] ;
//            for (int i=0; i<dy; ++i) {
//               destAddr[i] = (data.*src)(dx - 1 + i*dx) ;
//            }
//            destAddr += dy ;
//         }
//         destAddr -= xferFields*dy ;
//         MPI_Isend(destAddr, xferFields*dy, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg]) ;
//         ++emsg ;
//      }
//
//      if (rowMin && colMin && planeMin) {
//         /* corner at data logical coord (0, 0, 0) */
//         int toRank = myRank - data.tp()*data.tp() - data.tp() - 1 ;
//         double *comBuf = &data.commDataSend[pmsg * maxPlaneComm +
//                                                emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            comBuf[fi] = (data.*fieldData[fi])(0) ;
//         }
//         MPI_Isend(comBuf, xferFields, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMin && colMin && planeMax && doSend) {
//         /* corner at data logical coord (0, 0, 1) */
//         int toRank = myRank + data.tp()*data.tp() - data.tp() - 1 ;
//         double *comBuf = &data.commDataSend[pmsg * maxPlaneComm +
//                                                emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL] ;
//         int idx = dx*dy*(dz - 1) ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            comBuf[fi] = (data.*fieldData[fi])(idx) ;
//         }
//         MPI_Isend(comBuf, xferFields, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMin && colMax && planeMin) {
//         /* corner at data logical coord (1, 0, 0) */
//         int toRank = myRank - data.tp()*data.tp() - data.tp() + 1 ;
//         double *comBuf = &data.commDataSend[pmsg * maxPlaneComm +
//                                                emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL] ;
//         int idx = dx - 1 ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            comBuf[fi] = (data.*fieldData[fi])(idx) ;
//         }
//         MPI_Isend(comBuf, xferFields, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMin && colMax && planeMax && doSend) {
//         /* corner at data logical coord (1, 0, 1) */
//         int toRank = myRank + data.tp()*data.tp() - data.tp() + 1 ;
//         double *comBuf = &data.commDataSend[pmsg * maxPlaneComm +
//                                                emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL] ;
//         int idx = dx*dy*(dz - 1) + (dx - 1) ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            comBuf[fi] = (data.*fieldData[fi])(idx) ;
//         }
//         MPI_Isend(comBuf, xferFields, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMax && colMin && planeMin) {
//         /* corner at data logical coord (0, 1, 0) */
//         int toRank = myRank - data.tp()*data.tp() + data.tp() - 1 ;
//         double *comBuf = &data.commDataSend[pmsg * maxPlaneComm +
//                                                emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL] ;
//         int idx = dx*(dy - 1) ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            comBuf[fi] = (data.*fieldData[fi])(idx) ;
//         }
//         MPI_Isend(comBuf, xferFields, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMax && colMin && planeMax && doSend) {
//         /* corner at data logical coord (0, 1, 1) */
//         int toRank = myRank + data.tp()*data.tp() + data.tp() - 1 ;
//         double *comBuf = &data.commDataSend[pmsg * maxPlaneComm +
//                                                emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL] ;
//         int idx = dx*dy*(dz - 1) + dx*(dy - 1) ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            comBuf[fi] = (data.*fieldData[fi])(idx) ;
//         }
//         MPI_Isend(comBuf, xferFields, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMax && colMax && planeMin) {
//         /* corner at data logical coord (1, 1, 0) */
//         int toRank = myRank - data.tp()*data.tp() + data.tp() + 1 ;
//         double *comBuf = &data.commDataSend[pmsg * maxPlaneComm +
//                                                emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL] ;
//         int idx = dx*dy - 1 ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            comBuf[fi] = (data.*fieldData[fi])(idx) ;
//         }
//         MPI_Isend(comBuf, xferFields, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//      if (rowMax && colMax && planeMax && doSend) {
//         /* corner at data logical coord (1, 1, 1) */
//         int toRank = myRank + data.tp()*data.tp() + data.tp() + 1 ;
//         double *comBuf = &data.commDataSend[pmsg * maxPlaneComm +
//                                                emsg * maxEdgeComm +
//                                         cmsg * CACHE_COHERENCE_PAD_REAL] ;
//         int idx = dx*dy*dz - 1 ;
//         for (int fi=0; fi<xferFields; ++fi) {
//            comBuf[fi] = (data.*fieldData[fi])(idx) ;
//         }
//         MPI_Isend(comBuf, xferFields, baseType, toRank, msgType,
//                   MPI_COMM_WORLD, &data.sendRequest[pmsg+emsg+cmsg]) ;
//         ++cmsg ;
//      }
//   }
//
//   MPI_Waitall(26, data.sendRequest, status) ;
//}

/******************************************/

void CommSBN(DataT& data, int xferFields, DataT *fieldData) 
{ }

//void CommSBN(DataT& data, int xferFields, DataT_member *fieldData) {
//
//   if (data.numRanks() == 1)
//      return ;
//
//   /* summation order should be from smallest value to largest */
//   /* or we could try out kahan summation! */
//
//   int myRank ;
//   int maxPlaneComm = xferFields * data.maxPlaneSize() ;
//   int maxEdgeComm  = xferFields * data.maxEdgeSize() ;
//   int pmsg = 0 ; /* plane comm msg */
//   int emsg = 0 ; /* edge comm msg */
//   int cmsg = 0 ; /* corner comm msg */
//   int dx = data.sizeX() + 1 ;
//   int dy = data.sizeY() + 1 ;
//   int dz = data.sizeZ() + 1 ;
//   MPI_Status status ;
//   double *srcAddr ;
//   int rowMin, rowMax, colMin, colMax, planeMin, planeMax ;
//   /* assume communication to 6 neighbors by default */
//   rowMin = rowMax = colMin = colMax = planeMin = planeMax = 1 ;
//   if (data.rowLoc() == 0) {
//      rowMin = 0 ;
//   }
//   if (data.rowLoc() == (data.tp()-1)) {
//      rowMax = 0 ;
//   }
//   if (data.colLoc() == 0) {
//      colMin = 0 ;
//   }
//   if (data.colLoc() == (data.tp()-1)) {
//      colMax = 0 ;
//   }
//   if (data.planeLoc() == 0) {
//      planeMin = 0 ;
//   }
//   if (data.planeLoc() == (data.tp()-1)) {
//      planeMax = 0 ;
//   }
//
//   MPI_Comm_rank(MPI_COMM_WORLD, &myRank) ;
//
//   if (planeMin | planeMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dx * dy ;
//
//      if (planeMin) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(i) += srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (planeMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(dx*dy*(dz - 1) + i) += srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//
//   if (rowMin | rowMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dx * dz ;
//
//      if (rowMin) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dx; ++j) {
//                  (data.*dest)(i*dx*dy + j) += srcAddr[i*dx + j] ;
//               }
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (rowMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dx; ++j) {
//                  (data.*dest)(dx*(dy - 1) + i*dx*dy + j) += srcAddr[i*dx + j] ;
//               }
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//   if (colMin | colMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dy * dz ;
//
//      if (colMin) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dy; ++j) {
//                  (data.*dest)(i*dx*dy + j*dx) += srcAddr[i*dy + j] ;
//               }
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (colMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dy; ++j) {
//                  (data.*dest)(dx - 1 + i*dx*dy + j*dx) += srcAddr[i*dy + j] ;
//               }
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//
//   if (rowMin & colMin) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dz; ++i) {
//            (data.*dest)(i*dx*dy) += srcAddr[i] ;
//         }
//         srcAddr += dz ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMin & planeMin) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dx; ++i) {
//            (data.*dest)(i) += srcAddr[i] ;
//         }
//         srcAddr += dx ;
//      }
//      ++emsg ;
//   }
//
//   if (colMin & planeMin) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dy; ++i) {
//            (data.*dest)(i*dx) += srcAddr[i] ;
//         }
//         srcAddr += dy ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMax & colMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dz; ++i) {
//            (data.*dest)(dx*dy - 1 + i*dx*dy) += srcAddr[i] ;
//         }
//         srcAddr += dz ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMax & planeMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dx; ++i) {
//            (data.*dest)(dx*(dy-1) + dx*dy*(dz-1) + i) += srcAddr[i] ;
//         }
//         srcAddr += dx ;
//      }
//      ++emsg ;
//   }
//
//   if (colMax & planeMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dy; ++i) {
//            (data.*dest)(dx*dy*(dz-1) + dx - 1 + i*dx) += srcAddr[i] ;
//         }
//         srcAddr += dy ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMax & colMin) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dz; ++i) {
//            (data.*dest)(dx*(dy-1) + i*dx*dy) += srcAddr[i] ;
//         }
//         srcAddr += dz ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMin & planeMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dx; ++i) {
//            (data.*dest)(dx*dy*(dz-1) + i) += srcAddr[i] ;
//         }
//         srcAddr += dx ;
//      }
//      ++emsg ;
//   }
//
//   if (colMin & planeMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dy; ++i) {
//            (data.*dest)(dx*dy*(dz-1) + i*dx) += srcAddr[i] ;
//         }
//         srcAddr += dy ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMin & colMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dz; ++i) {
//            (data.*dest)(dx - 1 + i*dx*dy) += srcAddr[i] ;
//         }
//         srcAddr += dz ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMax & planeMin) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dx; ++i) {
//            (data.*dest)(dx*(dy - 1) + i) += srcAddr[i] ;
//         }
//         srcAddr += dx ;
//      }
//      ++emsg ;
//   }
//
//   if (colMax & planeMin) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dy; ++i) {
//            (data.*dest)(dx - 1 + i*dx) += srcAddr[i] ;
//         }
//         srcAddr += dy ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMin & colMin & planeMin) {
//      /* corner at data logical coord (0, 0, 0) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(0) += comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMin & colMin & planeMax) {
//      /* corner at data logical coord (0, 0, 1) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy*(dz - 1) ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) += comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMin & colMax & planeMin) {
//      /* corner at data logical coord (1, 0, 0) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx - 1 ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) += comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMin & colMax & planeMax) {
//      /* corner at data logical coord (1, 0, 1) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy*(dz - 1) + (dx - 1) ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) += comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMax & colMin & planeMin) {
//      /* corner at data logical coord (0, 1, 0) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*(dy - 1) ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) += comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMax & colMin & planeMax) {
//      /* corner at data logical coord (0, 1, 1) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy*(dz - 1) + dx*(dy - 1) ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) += comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMax & colMax & planeMin) {
//      /* corner at data logical coord (1, 1, 0) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy - 1 ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) += comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMax & colMax & planeMax) {
//      /* corner at data logical coord (1, 1, 1) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy*dz - 1 ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) += comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//}

/******************************************/

void CommSyncPosVel(DataT& data) 
{

}
//void CommSyncPosVel(DataT& data) {
//
//   if (data.numRanks() == 1)
//      return ;
//
//   int myRank ;
//   bool doRecv = false ;
//   int xferFields = 6 ; /* x, y, z, xd, yd, zd */
//   DataT_member fieldData[6] ;
//   int maxPlaneComm = xferFields * data.maxPlaneSize() ;
//   int maxEdgeComm  = xferFields * data.maxEdgeSize() ;
//   int pmsg = 0 ; /* plane comm msg */
//   int emsg = 0 ; /* edge comm msg */
//   int cmsg = 0 ; /* corner comm msg */
//   int dx = data.sizeX() + 1 ;
//   int dy = data.sizeY() + 1 ;
//   int dz = data.sizeZ() + 1 ;
//   MPI_Status status ;
//   double *srcAddr ;
//   bool rowMin, rowMax, colMin, colMax, planeMin, planeMax ;
//
//   /* assume communication to 6 neighbors by default */
//   rowMin = rowMax = colMin = colMax = planeMin = planeMax = true ;
//   if (data.rowLoc() == 0) {
//      rowMin = false ;
//   }
//   if (data.rowLoc() == (data.tp()-1)) {
//      rowMax = false ;
//   }
//   if (data.colLoc() == 0) {
//      colMin = false ;
//   }
//   if (data.colLoc() == (data.tp()-1)) {
//      colMax = false ;
//   }
//   if (data.planeLoc() == 0) {
//      planeMin = false ;
//   }
//   if (data.planeLoc() == (data.tp()-1)) {
//      planeMax = false ;
//   }
//
//   fieldData[0] = &DataT::x ;
//   fieldData[1] = &DataT::y ;
//   fieldData[2] = &DataT::z ;
//   fieldData[3] = &DataT::xd ;
//   fieldData[4] = &DataT::yd ;
//   fieldData[5] = &DataT::zd ;
//
//   MPI_Comm_rank(MPI_COMM_WORLD, &myRank) ;
//
//   if (planeMin | planeMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dx * dy ;
//
//      if (planeMin && doRecv) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(i) = srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (planeMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(dx*dy*(dz - 1) + i) = srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//
//   if (rowMin | rowMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dx * dz ;
//
//      if (rowMin && doRecv) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dx; ++j) {
//                  (data.*dest)(i*dx*dy + j) = srcAddr[i*dx + j] ;
//               }
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (rowMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dx; ++j) {
//                  (data.*dest)(dx*(dy - 1) + i*dx*dy + j) = srcAddr[i*dx + j] ;
//               }
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//
//   if (colMin | colMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dy * dz ;
//
//      if (colMin && doRecv) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dy; ++j) {
//                  (data.*dest)(i*dx*dy + j*dx) = srcAddr[i*dy + j] ;
//               }
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (colMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<dz; ++i) {
//               for (int j=0; j<dy; ++j) {
//                  (data.*dest)(dx - 1 + i*dx*dy + j*dx) = srcAddr[i*dy + j] ;
//               }
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//
//   if (rowMin && colMin && doRecv) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dz; ++i) {
//            (data.*dest)(i*dx*dy) = srcAddr[i] ;
//         }
//         srcAddr += dz ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMin && planeMin && doRecv) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dx; ++i) {
//            (data.*dest)(i) = srcAddr[i] ;
//         }
//         srcAddr += dx ;
//      }
//      ++emsg ;
//   }
//
//   if (colMin && planeMin && doRecv) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dy; ++i) {
//            (data.*dest)(i*dx) = srcAddr[i] ;
//         }
//         srcAddr += dy ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMax && colMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dz; ++i) {
//            (data.*dest)(dx*dy - 1 + i*dx*dy) = srcAddr[i] ;
//         }
//         srcAddr += dz ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMax && planeMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dx; ++i) {
//            (data.*dest)(dx*(dy-1) + dx*dy*(dz-1) + i) = srcAddr[i] ;
//         }
//         srcAddr += dx ;
//      }
//      ++emsg ;
//   }
//
//   if (colMax && planeMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dy; ++i) {
//            (data.*dest)(dx*dy*(dz-1) + dx - 1 + i*dx) = srcAddr[i] ;
//         }
//         srcAddr += dy ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMax && colMin) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dz; ++i) {
//            (data.*dest)(dx*(dy-1) + i*dx*dy) = srcAddr[i] ;
//         }
//         srcAddr += dz ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMin && planeMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dx; ++i) {
//            (data.*dest)(dx*dy*(dz-1) + i) = srcAddr[i] ;
//         }
//         srcAddr += dx ;
//      }
//      ++emsg ;
//   }
//
//   if (colMin && planeMax) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dy; ++i) {
//            (data.*dest)(dx*dy*(dz-1) + i*dx) = srcAddr[i] ;
//         }
//         srcAddr += dy ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMin && colMax && doRecv) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dz; ++i) {
//            (data.*dest)(dx - 1 + i*dx*dy) = srcAddr[i] ;
//         }
//         srcAddr += dz ;
//      }
//      ++emsg ;
//   }
//
//   if (rowMax && planeMin && doRecv) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dx; ++i) {
//            (data.*dest)(dx*(dy - 1) + i) = srcAddr[i] ;
//         }
//         srcAddr += dx ;
//      }
//      ++emsg ;
//   }
//
//   if (colMax && planeMin && doRecv) {
//      srcAddr = &data.commDataRecv[pmsg * maxPlaneComm +
//                                       emsg * maxEdgeComm] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg], &status) ;
//      for (int fi=0 ; fi<xferFields; ++fi) {
//         DataT_member dest = fieldData[fi] ;
//         for (int i=0; i<dy; ++i) {
//            (data.*dest)(dx - 1 + i*dx) = srcAddr[i] ;
//         }
//         srcAddr += dy ;
//      }
//      ++emsg ;
//   }
//
//
//   if (rowMin && colMin && planeMin && doRecv) {
//      /* corner at data logical coord (0, 0, 0) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(0) = comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMin && colMin && planeMax) {
//      /* corner at data logical coord (0, 0, 1) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy*(dz - 1) ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) = comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMin && colMax && planeMin && doRecv) {
//      /* corner at data logical coord (1, 0, 0) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx - 1 ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) = comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMin && colMax && planeMax) {
//      /* corner at data logical coord (1, 0, 1) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy*(dz - 1) + (dx - 1) ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) = comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMax && colMin && planeMin && doRecv) {
//      /* corner at data logical coord (0, 1, 0) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*(dy - 1) ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) = comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMax && colMin && planeMax) {
//      /* corner at data logical coord (0, 1, 1) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy*(dz - 1) + dx*(dy - 1) ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) = comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMax && colMax && planeMin && doRecv) {
//      /* corner at data logical coord (1, 1, 0) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy - 1 ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) = comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//   if (rowMax && colMax && planeMax) {
//      /* corner at data logical coord (1, 1, 1) */
//      double *comBuf = &data.commDataRecv[pmsg * maxPlaneComm +
//                                             emsg * maxEdgeComm +
//                                      cmsg * CACHE_COHERENCE_PAD_REAL] ;
//      int idx = dx*dy*dz - 1 ;
//      MPI_Wait(&data.recvRequest[pmsg+emsg+cmsg], &status) ;
//      for (int fi=0; fi<xferFields; ++fi) {
//         (data.*fieldData[fi])(idx) = comBuf[fi] ;
//      }
//      ++cmsg ;
//   }
//}

/******************************************/

void CommMonoQ(DataT& data)
{}

//void CommMonoQ(DataT& data)
//{
//   if (data.numRanks() == 1)
//      return ;
//
//   int myRank ;
//   int xferFields = 3 ; /* delv_xi, delv_eta, delv_zeta */
//   DataT_member fieldData[3] ;
//   int fieldOffset[3] ;
//   int maxPlaneComm = xferFields * data.maxPlaneSize() ;
//   int pmsg = 0 ; /* plane comm msg */
//   int dx = data.sizeX() ;
//   int dy = data.sizeY() ;
//   int dz = data.sizeZ() ;
//   MPI_Status status ;
//   double *srcAddr ;
//   bool rowMin, rowMax, colMin, colMax, planeMin, planeMax ;
//   /* assume communication to 6 neighbors by default */
//   rowMin = rowMax = colMin = colMax = planeMin = planeMax = true ;
//   if (data.rowLoc() == 0) {
//      rowMin = false ;
//   }
//   if (data.rowLoc() == (data.tp()-1)) {
//      rowMax = false ;
//   }
//   if (data.colLoc() == 0) {
//      colMin = false ;
//   }
//   if (data.colLoc() == (data.tp()-1)) {
//      colMax = false ;
//   }
//   if (data.planeLoc() == 0) {
//      planeMin = false ;
//   }
//   if (data.planeLoc() == (data.tp()-1)) {
//      planeMax = false ;
//   }
//
//   /* point into ghost data area */
//   // fieldData[0] = &(data.delv_xi(data.numElem())) ;
//   // fieldData[1] = &(data.delv_eta(data.numElem())) ;
//   // fieldData[2] = &(data.delv_zeta(data.numElem())) ;
//   fieldData[0] = &DataT::delv_xi ;
//   fieldData[1] = &DataT::delv_eta ;
//   fieldData[2] = &DataT::delv_zeta ;
//   fieldOffset[0] = data.numElem() ;
//   fieldOffset[1] = data.numElem() ;
//   fieldOffset[2] = data.numElem() ;
//
//
//   MPI_Comm_rank(MPI_COMM_WORLD, &myRank) ;
//
//   if (planeMin | planeMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dx * dy ;
//
//      if (planeMin) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(fieldOffset[fi] + i) = srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//            fieldOffset[fi] += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (planeMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(fieldOffset[fi] + i) = srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//            fieldOffset[fi] += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//
//   if (rowMin | rowMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dx * dz ;
//
//      if (rowMin) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(fieldOffset[fi] + i) = srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//            fieldOffset[fi] += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (rowMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(fieldOffset[fi] + i) = srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//            fieldOffset[fi] += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//   if (colMin | colMax) {
//      /* ASSUMING ONE DOMAIN PER RANK, CONSTANT BLOCK SIZE HERE */
//      int opCount = dy * dz ;
//
//      if (colMin) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(fieldOffset[fi] + i) = srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//            fieldOffset[fi] += opCount ;
//         }
//         ++pmsg ;
//      }
//      if (colMax) {
//         /* contiguous memory */
//         srcAddr = &data.commDataRecv[pmsg * maxPlaneComm] ;
//         MPI_Wait(&data.recvRequest[pmsg], &status) ;
//         for (int fi=0 ; fi<xferFields; ++fi) {
//            DataT_member dest = fieldData[fi] ;
//            for (int i=0; i<opCount; ++i) {
//               (data.*dest)(fieldOffset[fi] + i) = srcAddr[i] ;
//            }
//            srcAddr += opCount ;
//         }
//         ++pmsg ;
//      }
//   }
//}

void InitMeshDecomp(int numRanks, int myRank,
                    int *col, int *row, int *plane, int *side)
{
   int testProcs;
   int dx, dy, dz;
   int myDom;
   
   // Assume cube processor layout for now 
   testProcs = int(cbrt(double(numRanks))+0.5) ;
   if (testProcs*testProcs*testProcs != numRanks) {
      printf("Num processors must be a cube of an integer (1, 8, 27, ...)\n") ;
      MPI_Abort(MPI_COMM_WORLD, -1) ;
   }
   if (sizeof(double) != 4 && sizeof(double) != 8) {
      printf("MPI operations only support float and double right now...\n");
      MPI_Abort(MPI_COMM_WORLD, -1) ;
   }
   if (MAX_FIELDS_PER_MPI_COMM > CACHE_COHERENCE_PAD_REAL) {
      printf("corner element comm buffers too small.  Fix code.\n") ;
      MPI_Abort(MPI_COMM_WORLD, -1) ;
   }

   dx = testProcs ;
   dy = testProcs ;
   dz = testProcs ;

   // temporary test
   if (dx*dy*dz != numRanks) {
      printf("error -- must have as many datas as procs\n") ;
      MPI_Abort(MPI_COMM_WORLD, -1) ;
   }
   int remainder = dx*dy*dz % numRanks ;
   if (myRank < remainder) {
      myDom = myRank*( 1+ (dx*dy*dz / numRanks)) ;
   }
   else {
      myDom = remainder*( 1+ (dx*dy*dz / numRanks)) +
         (myRank - remainder)*(dx*dy*dz/numRanks) ;
   }

   *col = myDom % dx ;
   *row = (myDom / dx) % dy ;
   *plane = myDom / (dx*dy) ;
   *side = testProcs;

   return;
}
