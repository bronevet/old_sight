#include <iostream>
#include <mpi.h>
#define NN 1000000
#include "cd.h"
#include "cd_handle.h"

using namespace cd;
using namespace std;
CDErrT err;
ucontext_t context;

int min(int a, int b)
{
  int res = 0;
  if(a >= b) res = b;
  else res = a;

  return res;
}


void para_range(int n1, int n2, int nprocs, int myrank, int *ista, int *iend)
{
  int iwork1, iwork2;
  iwork1 = (n2-n1+1)/nprocs;
  iwork2 = (n2-n1+1) % nprocs;
  *ista = myrank*iwork1 + n1 + min(myrank, iwork2);
  *iend = *ista + iwork1 - 1;
  if(iwork2 > myrank) *iend = *iend + 1;
}


void get_sum_linear()
{
  double a[NN], sum;
  int i;
  for( i=0; i<NN; i++) {
    a[i] = i + 1;
  }
  sum = 0.0;
  for(i=0; i<NN; i++) {
    sum = sum + a[i];
  }
//  printf("sum = %f\n", sum);
}


int main(int argc, char* argv[])
{
  int i, nprocs, myrank;
  int ista, iend;
  double a[NN], sum, tmp;

  double varA[100];
  double varB[200];
  double varC[300];
  double varD[400];
  double varE[500];
  double varF[600];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  CDHandle* cd_root = CD_Init(nprocs, myrank);
  cd_root->Begin(true, "Root begun");

  para_range(1, NN, nprocs, myrank, &ista, &iend);
  
  CDHandle* cd1 = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), 8, "CD1", kStrict, 0, 0, &err);
  
    cd1->Begin(true, "CD 1A begun");
  
    cd1->Preserve(varA, sizeof(varA), kCopy, "varA");
    cd1->Preserve(varB, sizeof(varB), kCopy, "varB");
    cd1->Preserve(varC, sizeof(varC), kReference, "varC");
    cd1->Preserve(varD, sizeof(varD), kReference, "varD");
    cd1->Preserve(varF, sizeof(varF), kReference, "varF");
  
    cd1->Detect();
    cd1->Complete();
  
    cd1->Begin(true, "CD 1B begun");
  
    cd1->Preserve(varA, sizeof(varA), kCopy, "varA");
    cd1->Preserve(varB, sizeof(varB), kCopy, "varB");
    cd1->Preserve(varC, sizeof(varC), kReference, "varC");
    cd1->Preserve(varD, sizeof(varD), kReference, "varD");
    cd1->Preserve(varF, sizeof(varF), kReference, "varF");

      CDHandle* cd2 = GetCurrentCD()->Create(GetCurrentCD()->GetNodeID(), 1, "CD2", kStrict, 0, 0, &err);
      cout << "CD 2 created \n"<<endl;

      cd2->Begin(true, "CD 2A begun");

      cd2->Preserve(varA, sizeof(varA), kReference, "varA");
      cd2->Preserve(varB, sizeof(varB), kReference, "varB");
      cd2->Preserve(varC, sizeof(varC), kCopy, "varC");
      cd2->Preserve(varE, sizeof(varE), kCopy, "varE");	// it is not inclusive
  
      cd2->Detect();
      cd2->Complete();
  
      cd2->Begin(true, "CD 2B begun");
  
      cd2->Preserve(varA, sizeof(varA), kReference, "varA");
      cd2->Preserve(varB, sizeof(varB), kReference, "varB");
      cd2->Preserve(varC, sizeof(varC), kCopy, "varC");
      cd2->Preserve(varE, sizeof(varE), kCopy, "varE");	// it is not inclusive
  
      cd2->Detect();
      cd2->Complete();
 
//      cd2->Begin(true, "CD 2A begun");
//  
//      cd2->Preserve(varA, sizeof(varA), kReference, "varA");
//      cd2->Preserve(varB, sizeof(varB), kReference, "varB");
//      cd2->Preserve(varC, sizeof(varC), kCopy, "varC");
//      cd2->Preserve(varE, sizeof(varE), kCopy, "varE");	// it is not inclusive
//      cd2->Detect();
//      cd2->Complete();
//      cd2->Begin(true, "CD 2B begun");
//  
//      cd2->Preserve(varA, sizeof(varA), kReference, "varA");
//      cd2->Preserve(varB, sizeof(varB), kReference, "varB");
//      cd2->Preserve(varC, sizeof(varC), kCopy, "varC");
//      cd2->Preserve(varE, sizeof(varE), kCopy, "varE");	// it is not inclusive
//
//      cd2->Detect();
//      cd2->Complete();
 

       cd2->Destroy();

    //getchar();
    cout<< "before cd1 destroyed"<<endl;

		cd1->Detect();
    cd1->Complete();
    cd1->Destroy();

  //getchar();
  cout<<"before root destoryed"<<endl;

  cd_root->Detect();
  cd_root->Complete();




/*

  for(i = ista-1; i<iend; i++) {
    a[i] = i + 1;
  }

  sum = 0.0;

  for(i = ista-1; i<iend; i++) {
    sum = sum + a[i];
  }

  MPI_Reduce(&sum, &tmp, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  sum = tmp;

//  if(myrank == 0) 

    printf("sum = %f\n", sum);
*/



  CD_Finalize();
  MPI_Finalize();
  return 0;
}


