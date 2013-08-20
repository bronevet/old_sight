#include "dbglog.h"
#include "widgets/valSelector.h"
#include "widgets/trace.h"
#include <map>
#include <assert.h>
#include <sys/time.h>

using namespace std;
using namespace dbglog;

#define idx(i, j, dim) (i*dim + j)

#define ijk 0
#define ikj 1
#define jik 2
#define jki 3
#define kij 4
#define kji 5
#define LOOP_TYPES 6

string loopTypeStr(int loop) {
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

int main(int argc, char** argv) {
  initializeDebug(argc, argv);
  
  // Create a trace. Every call to traceAttr() while this object is in scope will send data to this trace.
  // The values of the traced items will be shown as a function of attribute "i".
  // The visualization of the trace will be shown at the spot in the debug output where t goes into of scope.
  list<string> contextAttrs;
  contextAttrs.push_back("MtxSize");
  contextAttrs.push_back("loop");
  trace t("Loop Elapsed", contextAttrs, trace::showEnd, trace::decTree);
  colorSelector loopColor("loop",0,0,.3,0,0,1); // Green gradient
  colorSelector elapsedColor(.3,0,0,1,0,0); // Red gradient
  
  for(int rep=0; rep<3; rep++) {
    scope s(txt()<<"Repetition "<<rep);
    for(int MtxSize=1; MtxSize<=100; MtxSize*=10) {
      attr iAttr("MtxSize", (long)MtxSize);
      
      double* A = (double*)malloc(sizeof(double) * MtxSize * MtxSize);
      double* B = (double*)malloc(sizeof(double) * MtxSize * MtxSize);
      double* C = (double*)calloc(sizeof(double),  MtxSize * MtxSize);
      
      for(int loop=0; loop<LOOP_TYPES; loop++) {
        attr lAttr("loop", loopTypeStr(loop));
      
        struct timeval start;
        gettimeofday(&start, NULL);
        
             if(loop==ijk) matmult_ijk(A, B, C, MtxSize);
        else if(loop==ikj) matmult_ikj(A, B, C, MtxSize);
        else if(loop==jik) matmult_jik(A, B, C, MtxSize);
        else if(loop==jki) matmult_jki(A, B, C, MtxSize);
        else if(loop==kij) matmult_kij(A, B, C, MtxSize);
        else if(loop==kji) matmult_kji(A, B, C, MtxSize);
        
        struct timeval end;
        gettimeofday(&end, NULL);
        
        double elapsed = ((end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 + start.tv_usec)) / 1000000.0;
        dbg << textColor::start(loopColor) << "Loop "<<loopTypeStr(loop)<<textColor::end()<<", MtxSize="<<MtxSize<<": "<<textColor::start(elapsedColor, attrValue(elapsed))<<"elapsed = "<<elapsed<<textColor::end()<<endl;
        
        traceAttr("Elapsed", attrValue((double)elapsed));
      }
      
      free(A);
      free(B);
      free(C);
    }
  }
  
  return 0;
}

