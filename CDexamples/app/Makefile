include ./../Makefile.extern
SHELL = /bin/sh
.SUFFIXES: .cc .o

LULESH_EXEC   = lulesh_cd_profile
#TEST_EXEC     = test_profiler
TEST_EXEC     = test_mpi_profiler
EXEC_PROFILED = $(LULESH_EXEC)

SERCXX   = g++ -DUSE_MPI=0 
MPICXX   = mpig++ -DUSE_MPI=1 -D_CD=1 -D_DEBUG=1 
CXX      = $(MPICXX)
CXXFLAGS = -g -std=gnu++0x -openmp -I${SILO_INCDIR} -Wall -Wno-pragmas ${CD_PROFILER_CFLAGS} ${SIGHT_CFLAGS}
LDFLAGS  = ${CD_PROFILER_LINKFLAGS} ${SIGHT_LINKFLAGS}

#SILO_INCDIR = /usr/gapps/silo/current/chaos_5_x86_64/include
#SILO_LIBDIR = /usr/gapps/silo/current/chaos_5_x86_64/lib
#CXXFLAGS    = ${CXXFLAGS} -DVIZ_MESH -I${SILO_INCDIR} 
#LDFLAGS     = ${LDFLAGS} -L${SILO_LIBDIR} -Wl,-rpath -Wl,${SILO_LIBDIR} -lsiloh5 -lhdf5

SOURCE_LULESH = \
	./lulesh_v2/lulesh.cc \
	./lulesh_v2/lulesh-comm.cc \
	./lulesh_v2/lulesh-viz.cc \
	./lulesh_v2/lulesh-util.cc \
	./lulesh_v2/lulesh-init.cc
SOURCE_TEST     = ./test_mpi_profiler/mpi_profiler_test.cc
SOURCE_TEST_MPI = ./test_cd_mpi/test_cd_mpi.cc \
						./test_cd_mpi/test_cd_mpi-util.cc

LULESH_O      = $(SOURCE_LULESH:.cc=.o)
TEST_O        = $(SOURCE_TEST:.cc=.o)
TEST_MPI_O    = $(SOURCE_TEST_MPI:.cc=.o)

cd_profiler_H := ${CD_PROFILER}/src/*.h \
                 ${CD_PROFILER}/lib/libcd_profiler.so
all_H        :=  ${cd_profiler_H}

.cc.o:  
	@echo "\nBuilding $<\n\n"
	$(CXX) -c $(CXXFLAGS) -o $@ $<

all: test_cd_mpi_profiler

lulesh_cd_profile: $(LULESH_O) $(CD_PROFILER_O) 
	@echo "\nLinking"
	$(CXX) -o $@ $^ $(LDFLAGS) -lm

test_cd_profile: $(TEST_O) $(CD_PROFILER_O) 
	@echo "\nLinking"
	$(CXX) -o $@ $^ $(LDFLAGS) -lm

test_cd_mpi_profiler: $(TEST_MPI_O) $(CD_PROFILER_O) 
	@echo "\nLinking"
	$(CXX) -o $@ $^ $(LDFLAGS) -lm

clean:
	rm -rf ${CD_APP}/lulesh_v2/*.dSYM ./lulesh_v2/*.o;
	rm -rf test_cd_profile; rm -rf test_cd_mpi_profiler; rm -rf lulesh_cd_profile;  
	rm -rf ${CD_APP}/dbg*; 
	rm -rf ${CD_APP}/*.core; rm -rf ${CD_APP}/*.btr; 
	rm -rf ${CD_APP}/profile_data/*; rm -rf ${CD_APP}/test_mpi_profiler/*.o;
	rm -rf ${CD_APP}/test_cd_mpi/*.o;

clean_dbg:
	rm -rf ${CD_APP}/dbg*; 
	rm -rf ${CD_APP}/*.core; 
	rm -rf ${CD_APP}/*.btr; 
	rm -rf ${CD_APP}/profile_data/*.conf; 
