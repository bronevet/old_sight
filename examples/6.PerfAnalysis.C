#include "sight.h"
#include <map>
#include <assert.h>

using namespace std;
using namespace sight;

#define idx(i, j, dim) (i*dim + j)

// We analyze the performance of 6 loop orderings for Matrix-Matrix Multiplication (MMM)

#define ijk 0
#define ikj 1
#define jik 2
#define jki 3
#define kij 4
#define kji 5
#define MMM_LOOP_TYPES 6

string mmmLoopTypeStr(int loop) {
  if(loop == ijk) return "ijk";
  if(loop == ikj) return "ikj";
  if(loop == jik) return "jik";
  if(loop == jki) return "jki";
  if(loop == kij) return "kij";
  if(loop == kji) return "kji";
  return "???";
}

void matmult_ijk(double* A, double* B, double* C, int n) {
  for(int i = 0 ; i < n ; i++ ) {
    for(int j = 0 ; j < n ; j++ ) {
      for(int k = 0 ; k < n ; k++ ) {
        C[idx(i,j,n)] += A[idx(i,k,n)]*B[idx(k,j,n)];
  } } }
}

void matmult_ikj(double* A, double* B, double* C, int n) {
  for(int i = 0 ; i < n ; i++ ) {
    for(int k = 0 ; k < n ; k++ ) {
      for(int j = 0 ; j < n ; j++ ) {
        C[idx(i,j,n)] += A[idx(i,k,n)]*B[idx(k,j,n)];
  } } }
}

void matmult_jik(double* A, double* B, double* C, int n) {
  for(int j = 0 ; j < n ; j++ ) {
    for(int i = 0 ; i < n ; i++ ) {
      for(int k = 0 ; k < n ; k++ ) {
        C[idx(i,j,n)] += A[idx(i,k,n)]*B[idx(k,j,n)];
  } } }
}

void matmult_jki(double* A, double* B, double* C, int n) {
  for(int j = 0 ; j < n ; j++ ) {
    for(int k = 0 ; k < n ; k++ ) {
      for(int i = 0 ; i < n ; i++ ) {
        C[idx(i,j,n)] += A[idx(i,k,n)]*B[idx(k,j,n)];
  } } }
}

void matmult_kij(double* A, double* B, double* C, int n) {
  for(int k = 0 ; k < n ; k++ ) {
    for(int i = 0 ; i < n ; i++ ) {
      for(int j = 0 ; j < n ; j++ ) {
        C[idx(i,j,n)] += A[idx(i,k,n)]*B[idx(k,j,n)];
  } } }
}

void matmult_kji(double* A, double* B, double* C, int n) {
  for(int k = 0 ; k < n ; k++ ) {
    for(int j = 0 ; j < n ; j++ ) {
      for(int i = 0 ; i < n ; i++ ) {
        C[idx(i,j,n)] += A[idx(i,k,n)]*B[idx(k,j,n)];
  } } }
}

// We analyze the performance of 2 loop orderings for Matrix-Vector Multiplication (MVM)

#define ij 0
#define ji 1
#define MVM_LOOP_TYPES 2

string mvmLoopTypeStr(int loop) {
  if(loop == ij) return "ij";
  if(loop == ji) return "ji";
  return "???";
}

void matvec_ij(double *mtx, double* vecIn, double* vecOut, int n) {
  for(int i=0; i<n; i++) {
    vecOut[i] = 0;
    for(int j=0; j<n; j++) {
      vecOut[i] += mtx[idx(i, j, n)] * vecIn[j];
    }
  }
}

void matvec_ji(double *mtx, double* vecIn, double* vecOut, int n) {
  for(int i=0; i<n; i++)
    vecOut[i] = 0;
    
  for(int j=0; j<n; j++) {
    for(int i=0; i<n; i++) {
      vecOut[i] += mtx[idx(i, j, n)] * vecIn[j];
    }
  }
}

int main(int argc, char** argv) {
  SightInit(argc, argv, "6.PerfAnalysis", "dbg.6.PerfAnalysis");
  
  dbg << "<h1>Example 6: Performance Analysis</h1>" << endl;
  
  dbg << "This example shows how to do basic performance profiling and analysis. It looks at 6 different loop "<<
         "iteration orders for the Matrix-Matrix Multiplication (MMM) algorithm and 2 loop orders for the "<<
         "Matrix-Vector Multiplication (MVM) algorithm, considering the impact of following each MMM loop ordering "<<
         "by each MVM loop ordering. The key sight mechanisms for performance analysis are traces and metrics. A "<<
         "trace makes it possible to collect many observations of some quantity (e.g. function execution time) and "<<
         "choose a useful way to visualize them. Metrics measure performance properties and automatically add them "<<
         "to the trace of the user's choice."<<endl;
  
  dbg << "The MMM performance trace will use the size of the matrix and the MMM loop ordering as context variables. "<<
         "It will be placed at the spot in the output where the trace variable comes into scope and will use a lines "<<
         "visualization."<<endl;
  list<string> contextAttrs;
  contextAttrs.push_back("MtxSize");
  contextAttrs.push_back("mmmLoop");
  trace mmmT("MMM Loop Elapsed", contextAttrs, trace::showBegin, trace::lines);
  
  dbg << "The MVM performance trace will use the size of the matrix and the MMM and MVM loop ordering as context variables."<<
         "It will be placed at the spot int the output where the trace variable exited scope and will use a Decision "<<
         "Tree visualization."<<endl;
  contextAttrs.push_back("mvmLoop");
  trace mvmT("MVM Loop Elapsed", contextAttrs, trace::showEnd, trace::table);
    
  // Color selectors to assign different colors to lines associated with different MMMs and that have different elapsed times
  colorSelector loopColor("mmmLoop",0,0,.3,0,0,1); // Blue gradient
  colorSelector elapsedColor(.3,0,0,1,0,0); // Red gradient
  
  int iter=0;
  for(int MtxSize=1; MtxSize<=100; MtxSize*=10) {
    attr sizeAttr("MtxSize", (long)MtxSize);
    
    // Allocate the matrixes and vectors we'll be operating on
    double* A = (double*)malloc(sizeof(double) * MtxSize * MtxSize);
    double* B = (double*)malloc(sizeof(double) * MtxSize * MtxSize);
    double* C = (double*)calloc(sizeof(double),  MtxSize * MtxSize);
    
    double* vecIn  = (double*)malloc(sizeof(double) * MtxSize);
    double* vecOut = (double*)malloc(sizeof(double) * MtxSize);
    
    if(MtxSize==1) 
      dbg << "Iterate over all the MMM loop orders, considering the effect of loop order and matrix size on performance"<<endl;
    
    { scope sMMM("MMM");
    for(int mmmLoop=0; mmmLoop<MMM_LOOP_TYPES; mmmLoop++) {
      attr mmmLAttr("mmmLoop", mmmLoopTypeStr(mmmLoop));
    
      if(MtxSize==1 && mmmLoop==0)
        dbg << "Begin a performance measurement by calling startMeasure(), providing as arguments the name of the target "<<
               "trace and the name of the name with which we'll associate the result of the performance measurement. "<<
               "startMeasure() returns a pointer to a measure object. To complete the measurement, call endMeasure(), "<<
               "providing this object as an argument. endMeasure() automatically adds the elapsed time to the previously "<<
               "chosen trace and also returns the elapsed time if the user wishes to do something with it."<<endl;
      measure* mmm = startMeasure<timeMeasure>("MMM Loop Elapsed", "MMM Elapsed");
      
           if(mmmLoop==ijk) matmult_ijk(A, B, C, MtxSize);
      else if(mmmLoop==ikj) matmult_ikj(A, B, C, MtxSize);
      else if(mmmLoop==jik) matmult_jik(A, B, C, MtxSize);
      else if(mmmLoop==jki) matmult_jki(A, B, C, MtxSize);
      else if(mmmLoop==kij) matmult_kij(A, B, C, MtxSize);
      else if(mmmLoop==kji) matmult_kji(A, B, C, MtxSize);
      
      list<pair<string, attrValue> > elapsedL = endGetMeasure(mmm, true);
      double elapsed = elapsedL.begin()->second.getFloat();
      
      dbg << textColor::start(loopColor) << "MMM "<<mmmLoopTypeStr(mmmLoop)<<textColor::end()<<", MtxSize="<<MtxSize<<": "<<textColor::start(elapsedColor, attrValue(elapsed))<<"elapsed = "<<elapsed<<textColor::end()<<endl;
    } }
    
    if(MtxSize==1) 
      dbg << "Iterate over all MVM loop orders, considering the influence of each given MMM loop order and the performance "<<
             "of a subsequent MVM loop order."<<endl;
    
    { scope sMVM("MVM");
    for(int mmmLoop=0; mmmLoop<MMM_LOOP_TYPES; mmmLoop++) {
      attr mmmLAttr("mmmLoop", mmmLoopTypeStr(mmmLoop));
      
      for(int mvmLoop=0; mvmLoop<MVM_LOOP_TYPES; mvmLoop++, iter++) {
        attr mvmLAttr("mvmLoop", mvmLoopTypeStr(mvmLoop));
      
        measure* mvm = startMeasure<timeMeasure>("MVM Loop Elapsed", "MVM Elapsed");
        
             if(mvmLoop==ij) matvec_ij(A, vecIn, vecOut, MtxSize);
        else if(mvmLoop==ji) matvec_ji(A, vecIn, vecOut, MtxSize);
        
        list<pair<string, attrValue> > elapsedL = endGetMeasure(mvm, true);
        double elapsed = elapsedL.begin()->second.getFloat();
        
        dbg << textColor::start(loopColor) << "MMM "<<mmmLoopTypeStr(mmmLoop)<<"/MVM "<<mvmLoopTypeStr(mvmLoop)<<textColor::end()<<", MtxSize="<<MtxSize<<": "<<textColor::start(elapsedColor, attrValue(elapsed))<<"elapsed = "<<elapsed<<textColor::end()<<endl;
      }
    }}
    
    // Deallocate all the matrixes and vectors MMM and MVM operated on
    free(A);
    free(B);
    free(C);
    free(vecIn);
    free(vecOut);
  }
  
  return 0;
}

