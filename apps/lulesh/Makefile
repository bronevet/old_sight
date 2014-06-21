include ../../Makefile.extern

#default build suggestion of MPI + OPENMP with gcc on Livermore machines you might have to change the compiler name

SHELL = /bin/sh

LULESH_EXEC = lulesh2.0.real4.aos lulesh2.0.real8.aos lulesh2.0.real16.aos \
              lulesh2.0.real4.soa lulesh2.0.real8.soa lulesh2.0.real16.soa

MPI_INC = /opt/local/include/openmpi
MPI_LIB = /opt/local/lib

SERCXX = ${CCC} -DUSE_MPI=0
MPICXX = mpi${CCC} -DUSE_MPI=1
CXX = $(SERCXX)

SILO_INC = -I/usr/local/tools/silo-4.8/include -I/usr/local/tools/hdf5-gnu-serial-1.8.10/include
SILO_LIB = -L/usr/local/tools/silo-4.8/lib -lsiloh5 -L/usr/local/tools/hdf5-gnu-serial-1.8.10/lib -lhdf5

SOURCES2.0 = \
	lulesh.cc \
	lulesh-comm.cc \
	lulesh-viz.cc \
	lulesh-util.cc \
	lulesh-init.cc
OBJECTS2.0.REAL4.AOS  = $(SOURCES2.0:.cc=.real4.aos.o)
OBJECTS2.0.REAL8.AOS  = $(SOURCES2.0:.cc=.real8.aos.o)
OBJECTS2.0.REAL16.AOS = $(SOURCES2.0:.cc=.real16.aos.o)
OBJECTS2.0.REAL4.SOA  = $(SOURCES2.0:.cc=.real4.soa.o)
OBJECTS2.0.REAL8.SOA  = $(SOURCES2.0:.cc=.real8.soa.o)
OBJECTS2.0.REAL16.SOA = $(SOURCES2.0:.cc=.real16.soa.o)

#Default build suggestions with OpenMP for g++
CXXFLAGS = -g -O3 -DVIZ_MESH -DCOMP -I. -Wall ${SIGHT_CFLAGS} ${SILO_INC} -Wno-unknown-pragmas #-fopenmp
LDFLAGS = -g -O3 ${SIGHT_LINKFLAGS} ${SILO_LIB} #-fopenmp

#Below are reasonable default flags for a serial build
#CXXFLAGS = -g -O3 -I. -Wall
#LDFLAGS = -g -O3 

#common places you might find silo on the Livermore machines.
#SILO_INCDIR = /opt/local/include
#SILO_LIBDIR = /opt/local/lib
#SILO_INCDIR = ./silo/4.9/1.8.10.1/include
#SILO_LIBDIR = ./silo/4.9/1.8.10.1/lib

#If you do not have silo and visit you can get them at:
#silo:  https://wci.llnl.gov/codes/silo/downloads.html
#visit: https://wci.llnl.gov/codes/visit/download.html

#below is and example of how to make with silo, hdf5 to get vizulization by default all this is turned off.  All paths are Livermore specific.
#CXXFLAGS = -g -DVIZ_MESH -I${SILO_INCDIR} -Wall -Wno-pragmas
#LDFLAGS = -g -L${SILO_LIBDIR} -Wl,-rpath -Wl,${SILO_LIBDIR} -lsiloh5 -lhdf5

%.real4.aos.o: %.cc lulesh.h
	@echo "Building $<"
	$(CXX) -c $(CXXFLAGS) -DREAL4 -DAOS -o $@  $<

%.real8.aos.o: %.cc lulesh.h
	@echo "Building $<"
	$(CXX) -c $(CXXFLAGS) -DREAL8 -DAOS -o $@  $<

%.real16.aos.o: %.cc lulesh.h
	@echo "Building $<"
	$(CXX) -c $(CXXFLAGS) -DREAL10 -DAOS -o $@  $<

%.real4.soa.o: %.cc lulesh.h
	@echo "Building $<"
	$(CXX) -c $(CXXFLAGS) -DREAL4 -DSOA -o $@  $<

%.real8.soa.o: %.cc lulesh.h
	@echo "Building $<"
	$(CXX) -c $(CXXFLAGS) -DREAL8 -DSOA -o $@  $<

%.real16.soa.o: %.cc lulesh.h
	@echo "Building $<"
	$(CXX) -c $(CXXFLAGS) -DREAL10 -DSOA -o $@  $<

all: $(LULESH_EXEC)

lulesh2.0.real4.aos: $(OBJECTS2.0.REAL4.AOS)
	@echo "Linking"
	$(CXX) $(OBJECTS2.0.REAL4.AOS) $(LDFLAGS) -lm -o $@

lulesh2.0.real8.aos: $(OBJECTS2.0.REAL8.AOS)
	@echo "Linking"
	$(CXX) $(OBJECTS2.0.REAL8.AOS) $(LDFLAGS) -lm -o $@

lulesh2.0.real16.aos: $(OBJECTS2.0.REAL16.AOS)
	@echo "Linking"
	$(CXX) $(OBJECTS2.0.REAL16.AOS) $(LDFLAGS) -lm -o $@

lulesh2.0.real4.soa: $(OBJECTS2.0.REAL4.SOA)
	@echo "Linking"
	$(CXX) $(OBJECTS2.0.REAL4.SOA) $(LDFLAGS) -lm -o $@

lulesh2.0.real8.soa: $(OBJECTS2.0.REAL8.SOA)
	@echo "Linking"
	$(CXX) $(OBJECTS2.0.REAL8.SOA) $(LDFLAGS) -lm -o $@

lulesh2.0.real16.soa: $(OBJECTS2.0.REAL16.SOA)
	@echo "Linking"
	$(CXX) $(OBJECTS2.0.REAL16.SOA) $(LDFLAGS) -lm -o $@

clean:
	/bin/rm -f *.o *~ $(OBJECTS) $(LULESH_EXEC)
	/bin/rm -rf *.dSYM

tar: clean
	cd .. ; tar cvf lulesh-2.0.tar LULESH-2.0 ; mv lulesh-2.0.tar LULESH-2.0

