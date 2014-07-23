SHELL = /bin/sh
.SUFFIXES: .cc .o

LULESH_EXEC = lulesh

MPI_INC = /opt/local/include/openmpi
MPI_LIB = /opt/local/lib

SERCXX = g++ -DUSE_MPI=0
MPICXX = mpic++ -DUSE_MPI=1
CXX = $(MPICXX)

CXXFLAGS = -g -openmp -I./ -I${SILO_INCDIR} -Wall -Wno-pragmas #-DVIZ_MESH
LDFLAGS = -g 

SOURCES = \
	lulesh.cc \
	lulesh-comm.cc \
	lulesh-viz.cc \
	lulesh-util.cc \
	lulesh-init.cc
OBJECTS = $(SOURCES:.cc=.o)


# Silo is only required if you want to test I/O and visualization.
# Add -DVIZ_MESH to the CXXFLAGS line
# The following paths provide some standard locations on LLNL platforms,
# or you can download a copy to build from 
# Silo: https://wci.llnl.gov/codes/silo/downloads.html
# HDF5: http://www.hdfgroup.org/ftp/HDF5/current/src/
# Visit: https://wci.llnl.gov/codes/visit/download.html

# Mac OSX (default location of build using macports (port install silo)
#SILO_INCDIR = /opt/local/include
#SILO_LIBDIR = /opt/local/lib
# Default location on LLNL linux clusters
#SILO_INCDIR = /usr/gapps/silo/current/chaos_5_x86_64/include
#SILO_LIBDIR = /usr/gapps/silo/current/chaos_5_x86_64/lib

# Uncomment these to build with silo/hdf5
#CXXFLAGS = ${CXXFLAGS} -DVIZ_MESH -I${SILO_INCDIR} 
#LDFLAGS = ${LDFLAGS} -L${SILO_LIBDIR} -Wl,-rpath -Wl,${SILO_LIBDIR} -lsiloh5 -lhdf5


.cc.o: 
	@echo "Building $<"
	$(CXX) -c $(CXXFLAGS) -o $@ $<

all: lulesh

lulesh: $(OBJECTS)
	@echo "Linking"
	$(CXX) $(OBJECTS) $(LDFLAGS) -lm -o $@

clean:
	/bin/rm -f *.o *~ $(OBJECTS) $(LULESH_EXEC)
	/bin/rm -rf *.dSYM

tar: clean
	cd .. ; tar cvf lulesh-2.0.tar LULESH-2.0 ; mv lulesh-2.0.tar LULESH-2.0

