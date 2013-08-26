#include "dbglog.h"
#include "widgets/valSelector.h"
#include "widgets/trace.h"
#include <map>
#include <assert.h>

using namespace std;
using namespace dbglog;

#define idx(i, j, dim) (i*dim + j)

#define ijk 0
#define ikj 1
#define jik 2
#define jki 3
#define kij 4
#define kji 5
#define MMM_LOOP_TYPES 6

#define ij 0
#define ji 1
#define MVM_LOOP_TYPES 2

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

string mvmLoopTypeStr(int loop) {
  if(loop == ij) return "ij";
  if(loop == ji) return "ji";
  return "???";
}

int main(int argc, char** argv) {
  initializeDebug(argc, argv);
  
  // Create a trace. Every call to traceAttr() while this object is in scope will send data to this trace.
  // The values of the traced items will be shown as a function of attribute "i".
  // The visualization of the trace will be shown at the spot in the debug output where t goes into of scope.
  list<string> contextAttrs;
  contextAttrs.push_back("MtxSize");
  contextAttrs.push_back("mmmLoop");
  // Trace execution times of matrix-matrix multiplication
  trace mmmT("MMM Loop Elapsed", contextAttrs, trace::showBegin, trace::lines);
  
  // Trace execution times of matrix-vector multiplication
  contextAttrs.push_back("mvmLoop");
  trace mvmT("MVM Loop Elapsed", contextAttrs, trace::showEnd, trace::decTree);
  
  // Color selectors to assign different colors to lines associated with different MMMs and that have different elapsed times
  colorSelector loopColor("mmmLoop",0,0,.3,0,0,1); // Blue gradient
  colorSelector elapsedColor(.3,0,0,1,0,0); // Red gradient
  
  for(int rep=0; rep<3; rep++) {
    scope s(txt()<<"Repetition "<<rep);
    for(int MtxSize=1; MtxSize<=100; MtxSize*=10) {
      attr iAttr("MtxSize", (long)MtxSize);
      
      double* A = (double*)malloc(sizeof(double) * MtxSize * MtxSize);
      double* B = (double*)malloc(sizeof(double) * MtxSize * MtxSize);
      double* C = (double*)calloc(sizeof(double),  MtxSize * MtxSize);
      
      double* vecIn  = (double*)malloc(sizeof(double) * MtxSize);
      double* vecOut = (double*)malloc(sizeof(double) * MtxSize);
      
      { scope sMMM("MMM");
      for(int mmmLoop=0; mmmLoop<MMM_LOOP_TYPES; mmmLoop++) {
        attr mmmLAttr("mmmLoop", mmmLoopTypeStr(mmmLoop));
      
        measure* mmm = startMeasure("MMM Loop Elapsed", "MMM Elapsed");
        
             if(mmmLoop==ijk) matmult_ijk(A, B, C, MtxSize);
        else if(mmmLoop==ikj) matmult_ikj(A, B, C, MtxSize);
        else if(mmmLoop==jik) matmult_jik(A, B, C, MtxSize);
        else if(mmmLoop==jki) matmult_jki(A, B, C, MtxSize);
        else if(mmmLoop==kij) matmult_kij(A, B, C, MtxSize);
        else if(mmmLoop==kji) matmult_kji(A, B, C, MtxSize);
        
        double elapsed = endMeasure(mmm);
        
        dbg << textColor::start(loopColor) << "MMM "<<mmmLoopTypeStr(mmmLoop)<<textColor::end()<<", MtxSize="<<MtxSize<<": "<<textColor::start(elapsedColor, attrValue(elapsed))<<"elapsed = "<<elapsed<<textColor::end()<<endl;
      } }
      
      { scope sMVM("MVM");
      for(int mmmLoop=0; mmmLoop<MMM_LOOP_TYPES; mmmLoop++) {
        attr mmmLAttr("mmmLoop", mmmLoopTypeStr(mmmLoop));
        
        for(int mvmLoop=0; mvmLoop<MVM_LOOP_TYPES; mvmLoop++) {
          attr mvmLAttr("mvmLoop", mvmLoopTypeStr(mvmLoop));
        
          measure* mvm = startMeasure("MVM Loop Elapsed", "MVM Elapsed");
          
               if(mvmLoop==ij) matvec_ij(A, vecIn, vecOut, MtxSize);
          else if(mvmLoop==ji) matvec_ji(A, vecIn, vecOut, MtxSize);
          
          double elapsed = endMeasure(mvm);
          
          dbg << textColor::start(loopColor) << "MMM "<<mmmLoopTypeStr(mmmLoop)<<"/MVM "<<mvmLoopTypeStr(mvmLoop)<<textColor::end()<<", MtxSize="<<MtxSize<<": "<<textColor::start(elapsedColor, attrValue(elapsed))<<"elapsed = "<<elapsed<<textColor::end()<<endl;
        }
      }}
      
      free(A);
      free(B);
      free(C);
      free(vecIn);
      free(vecOut);
    }
  }
  
  return 0;
}

