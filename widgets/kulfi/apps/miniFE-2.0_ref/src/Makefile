include ../../../../../Makefile.extern

#-----------------------------------------------------------------------

MINIFE_TYPES =  \
        -DMINIFE_SCALAR=double   \
        -DMINIFE_LOCAL_ORDINAL=int      \
        -DMINIFE_GLOBAL_ORDINAL=int

MINIFE_MATRIX_TYPE = -DMINIFE_CSR_MATRIX
# MINIFE_MATRIX_TYPE = -DMINIFE_ELL_MATRIX

#-----------------------------------------------------------------------

CFLAGS = -O3 -g
CXXFLAGS = $(CFLAGS)

CPPFLAGS = -I. -I../utils -I../fem $(MINIFE_TYPES) $(MINIFE_MATRIX_TYPE) \
	-DHAVE_MPI -DMPICH_IGNORE_CXX_SEEK \
	-DMINIFE_REPORT_RUSAGE -DUSE_SIGHT -DKULFI ${SIGHT_CFLAGS} -I../../..
#	-DMINIFE_REPORT_RUSAGE

EXTERN_OBJ = ../../../kulfi_structure.bc
LDFLAGS= ${SIGHT_LINKFLAGS} -L/usr/lib/gcc/x86_64-redhat-linux6E/4.6.1 -lstdc++ -lsqlite3 
#LDFLAGS= -L/usr/lib/gcc/x86_64-redhat-linux6E/4.6.1 -lstdc++ -lsqlite3 
LIBS=
LLVM_INSTALL=/g/g15/bronevet/apps/llvm

CXX=./mpiclang++#mpig++#mpicxx
CC=./mpiclang#mpigcc#mpicc

#CXX=g++
#CC=gcc

include make_targets

