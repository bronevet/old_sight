SIGHT_COMMON_O := sight_common.o attributes/attributes_common.o binreloc.o getAllHostnames.o utils.o
SIGHT_COMMON_H := sight.h sight_common_internal.h attributes/attributes_common.h binreloc.h getAllHostnames.h utils.h
SIGHT_STRUCTURE_O := sight_structure.o attributes/attributes_structure.o
SIGHT_STRUCTURE_H := sight.h sight_structure_internal.h attributes/attributes_structure.h
SIGHT_LAYOUT_O := sight_layout.o attributes/attributes_layout.o slayout.o variant_layout.o 
SIGHT_LAYOUT_H := sight.h sight_layout_internal.h attributes/attributes_layout.h variant_layout.h
sight := ${sight_O} ${sight_H} gdbLineNum.pl sightDefines.pl

ROOT_PATH = ${CURDIR}

SIGHT_CFLAGS = -g -fPIC -I${ROOT_PATH} -I${ROOT_PATH}/attributes -I${ROOT_PATH}/widgets/* \
                -I${ROOT_PATH}/tools/callpath/src -I${ROOT_PATH}/tools/adept-utils/include \
                -I${ROOT_PATH}/tools/boost_1_55_0 \
                -I${ROOT_PATH}/widgets/papi/include \
                -I${ROOT_PATH}/widgets/libmsr/include

SIGHT_LINKFLAGS = \
                  -Wl,-rpath ${ROOT_PATH} \
                  ${ROOT_PATH}/tools/adept-utils/lib/libadept_cutils.so \
                  ${ROOT_PATH}/tools/adept-utils/lib/libadept_timing.so \
                  ${ROOT_PATH}/tools/adept-utils/lib/libadept_utils.so \
                  -Wl,-rpath ${ROOT_PATH}/tools/adept-utils/lib \
                  ${ROOT_PATH}/tools/callpath/src/src/libcallpath.so \
                  -Wl,-rpath ${ROOT_PATH}/tools/callpath/src/src \
                  ${ROOT_PATH}/widgets/papi/lib/libpapi.a \
                  ${ROOT_PATH}/widgets/gsl/lib/libgsl.so \
                  ${ROOT_PATH}/widgets/gsl/lib/libgslcblas.so \
                  -Wl,-rpath ${ROOT_PATH}/widgets/gsl/lib \
	          -lpthread
RAPL_ENABLED = 0
ifeq (${RAPL_ENABLED}, 1)
SIGHT_LINKFLAGS += ${ROOT_PATH}/widgets/libmsr/lib/libmsr.so \
                    -Wl,-rpath ${ROOT_PATH}/widgets/libmsr/lib
endif
	                
	                #-Wl,-rpath ${ROOT_PATH}/widgets/papi/lib 
override CC=gcc
override CCC=g++
MPICC = mpi${CC}
MPICCC = mpi${CCC}

OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXE := .exe
endif

all: core allExamples
	
core: sightDefines.pl gdbLineNum.pl Makefile.extern definitions.h maketools libsight_common.a libsight_structure.so slayout${EXE} libsight_layout.so hier_merge${EXE} widgets_post script/taffydb 
	chmod 755 html img script
	chmod 644 html/* img/* script/*
	chmod 755 script/taffydb

# Remakes only the object files
refresh: clean_objects all

# Set to "1" if we wish to enable support for services running on the node on which the original computation was 
# performed in addition to running code in the browser. The includes gdb and VNC integration. Otherwise set to 0
ifeq (${OS}, Cygwin)
REMOTE_ENABLED := 0
else
# Default distribution disables remote access since this capability requires us to run a web server
# and many compute centers disallow this
#REMOTE_ENABLED := 1
REMOTE_ENABLED := 0
endif

ifneq (${OS}, Cygwin)
# The port on which sight sets up a daemon that invokes gdb so that it runs upto a particular point
# in the target application's execution
GDB_PORT := 17501
endif

# Set to "1" if we wish to enable VNC integration
ifeq (${OS}, Cygwin)
VNC_ENABLED := 0
else
# Default distribution disables VNC 
#VNC_ENABLED := 0
# VNC is only enabled if remote services are enabled
ifeq (${REMOTE_ENABLED}, 1)
 VNC_ENABLED := 1 
endif
endif

# By default we disable KULFI-based fault injection since it requires LLVM
KULFI_ENABLED := 0
	
ifeq (${KULFI_ENABLED}, 1)
# Sight must use the same LLVM Clang compiler as KULFI does
# The path to the source, build and install of LLVM 3.2 that Kulfi will use
LLVM32_SRC_PATH := /g/g15/bronevet/apps/llvm-3.2
LLVM32_BUILD_PATH := /g/g15/bronevet/apps/llvm-3.2-build
LLVM32_INSTALL_PATH := /g/g15/bronevet/apps/llvm
	
$(info Setting compiler to be the same LLVM Clang as KULFI, from path ${LLVM32_INSTALL_PATH})
CC = ${LLVM32_INSTALL_PATH}/bin/clang
CCC = ${LLVM32_INSTALL_PATH}/bin/clang++
MPICC = ${ROOT_PATH}/tools/mpiclang
MPICCC = ${ROOT_PATH}/tools/mpiclang++
endif

MAKE_DEFINES = ROOT_PATH=${ROOT_PATH} REMOTE_ENABLED=${REMOTE_ENABLED} GDB_PORT=${GDB_PORT} VNC_ENABLED=${VNC_ENABLED} MPI_ENABLED=${MPI_ENABLED} OS=${OS} SIGHT_CFLAGS="${SIGHT_CFLAGS}" SIGHT_LINKFLAGS="${SIGHT_LINKFLAGS}" CC=${CC} CCC=${CCC} KULFI_ENABLED=${KULFI_ENABLED} LLVM32_SRC_PATH=${LLVM32_SRC_PATH} LLVM32_BUILD_PATH=${LLVM32_BUILD_PATH} LLVM32_INSTALL_PATH=${LLVM32_INSTALL_PATH}

# Set to "!" if we wish to enable examples that use MPI
MPI_ENABLED = 0
#MPI_ENABLED = 1

.PHONY: apps
ifeq (${MPI_ENABLED}, 1)
apps: mfem CoMD #mcbench
else
apps: mfem
endif

mfem: libsight_structure.so
	cd apps/mfem; make ${MAKE_DEFINES}

CoMD: 
	cd apps/CoMD/src-mpi; make ${MAKE_DEFINES}
	
#mcbench:
#ifneq (${OS}, Cygwin)
#	cd apps/mcbench; ./build-linux-x86_64.sh ${ROOT_PATH}
#endif

allExamples: libsight_structure.so
	cd examples; make ${MAKE_DEFINES}

run: all runExamples runApps

runExamples: core
	cd examples; make ${MAKE_DEFINES} run

runApps: libsight_structure.so slayout${EXE} hier_merge${EXE} apps
	cd examples; ../apps/mfem/mfem/examples/mfemComp.pl
	cd examples; ../apps/mfem/mfem/examples/ex2 ../apps/mfem/mfem/data/beam-tet.mesh 2
	cd examples; ../apps/mfem/mfem/examples/ex3 ../apps/mfem/mfem/data/ball-nurbs.mesh
	cd examples; ../apps/mfem/mfem/examples/ex4 ../apps/mfem/mfem/data/fichera-q3.mesh
ifeq (${MPI_ENABLED}, 1)
	cd examples; ../apps/CoMD/bin/CoMD-mpi.modules
	cd examples; ../apps/CoMD/bin/CoMD-mpi.tracepath
	cd examples; ../apps/CoMD/bin/CoMD-mpi.tracepos
	cd examples; ../apps/CoMD/CoMDCompare.pl
endif
ifeq (${REMOTE_ENABLED}, 1)
	cd examples; ../apps/mfem/mfem/examples/ex1 ../apps/mfem/mfem/data/beam-quad.mesh
endif

#runMCBench:
#	apps/mcbench/src/MCBenchmark.exe --nCores=1 --distributedSource --numParticles=13107 --nZonesX=256 --nZonesY=256 --xDim=16 --yDim=16 --mirrorBoundary --multiSigma --nThreadCore=1

runApps: libsight_structure.so slayout${EXE} hier_merge${EXE} apps runMFEM runCoMD #runMCBench


slayout.o: slayout.C process.C process.h
	${CCC} ${SIGHT_CFLAGS} slayout.C -I. -c -o slayout.o

slayout${EXE}: mfem libsight_layout.so widgets_post
	${CCC} -Wl,-rpath ${ROOT_PATH} -Wl,--whole-archive libsight_layout.so apps/mfem/mfem_layout.o -Wl,-no-whole-archive -o slayout${EXE}
#slayout${EXE}: mfem libsight_layout.so
#	${CCC} libsight_layout.so -Wl,-rpath ${ROOT_PATH} -Wl,-rpath ${ROOT_PATH}/widgets/gsl/lib -Lwidgets/gsl/lib -lgsl -lgslcblas apps/mfem/mfem_layout.o -o slayout${EXE}
#slayout${EXE}: mfem libsight_layout.a
#	${CCC} -Wl,--whole-archive libsight_layout.a apps/mfem/mfem_layout.o -Wl,-no-whole-archive -o slayout${EXE}
#	ld --whole-archive slayout.o libsight_layout.a apps/mfem/mfem_layout.o -o slayout${EXE}
#	${CCC} ${SIGHT_CFLAGS} slayout.C -Wl,--whole-archive libsight_layout.a -DMFEM -I. -Iapps/mfem apps/mfem/mfem_layout.o -Wl,-no-whole-archive -o slayout${EXE}

hier_merge${EXE}: hier_merge.C process.C process.h libsight_structure.so 
	${CCC} ${SIGHT_CFLAGS} hier_merge.C -Wl,--whole-archive libsight_structure.so -Wl,-no-whole-archive \
	                                 -DMFEM -I. ${SIGHT_LINKFLAGS} -o hier_merge${EXE}

libsight_common.a: ${SIGHT_COMMON_O} ${SIGHT_COMMON_H} widgets_pre
	ar -r libsight_common.a ${SIGHT_COMMON_O} widgets/*/*_common.o

libsight_structure.so: ${SIGHT_STRUCTURE_O} ${SIGHT_STRUCTURE_H} ${SIGHT_COMMON_O} ${SIGHT_COMMON_H} widgets_pre
	${CC} -shared  -Wl,-soname,libsight_structure.so -o libsight_structure.so  ${SIGHT_STRUCTURE_O} ${SIGHT_COMMON_O} widgets/*/*_structure.o widgets/*/*_common.o

#libsight_structure.a: ${SIGHT_STRUCTURE_O} ${SIGHT_STRUCTURE_H} ${SIGHT_COMMON_O} ${SIGHT_COMMON_H} widgets_pre
#	ar -r libsight_structure.a ${SIGHT_STRUCTURE_O} ${SIGHT_COMMON_O} widgets/*/*_structure.o widgets/*/*_common.o

libsight_layout.so: ${SIGHT_LAYOUT_O} ${SIGHT_LAYOUT_H} ${SIGHT_COMMON_O} ${SIGHT_COMMON_H} widgets_pre widgets/gsl/lib/libgsl.so widgets/gsl/lib/libgslcblas.so
	${CC} -shared -Wl,-soname,libsight_layout.so -o libsight_layout.so ${SIGHT_LAYOUT_O} ${SIGHT_COMMON_O} widgets/*/*_layout.o widgets/*/*_common.o -Lwidgets/gsl/lib -lgsl -lgslcblas
#widgets/gsl/lib/libgsl.a widgets/gsl/lib/libgslcblas.a
#-Wl,-rpath widgets/gsl/lib -Wl,--whole-archive widgets/gsl/lib/libgsl.so widgets/gsl/lib/libgslcblas.so -Wl,--no-whole-archive

#libsight_layout.a: ${SIGHT_LAYOUT_O} ${SIGHT_LAYOUT_H} ${SIGHT_COMMON_O} ${SIGHT_COMMON_H} widgets_pre widgets/gsl/lib/libgsl.so widgets/gsl/lib/libgslcblas.so
#	mkdir -p tmp
#	cd tmp; ar -x ../widgets/gsl/lib/libgsl.a; ar -x ../widgets/gsl/lib/libgslcblas.a
#	ar -r libsight_layout.a    ${SIGHT_LAYOUT_O}    ${SIGHT_COMMON_O} widgets/*/*_layout.o widgets/*/*_common.o tmp/*.o
#	rm -fr tmp
	
#libaz.a: libabc.a(*.o) libxyz.a(*.o)
#    ar rcs $@ $^
#	ld libsight_layout.a    ${SIGHT_LAYOUT_O}    ${SIGHT_COMMON_O} widgets/*_layout.o     widgets/*_common.o


sight_common.o: sight_common.C sight_common_internal.h attributes/attributes_common.h
	${CCC} ${SIGHT_CFLAGS} sight_common.C -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -c -o sight_common.o

sight_structure.o: sight_structure.C sight_structure_internal.h attributes/attributes_structure.h sight_common_internal.h attributes/attributes_common.h maketools
	${CCC} ${SIGHT_CFLAGS} sight_structure.C -Itools/dtl -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -c -o sight_structure.o

sight_layout.o: sight_layout.C sight_layout_internal.h attributes/attributes_layout.h sight_common_internal.h attributes/attributes_common.h
	${CCC} ${SIGHT_CFLAGS} sight_layout.C -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -c -o sight_layout.o

variant_layout.o: variant_layout.C variant_layout.h sight_layout_internal.h attributes/attributes_layout.h sight_common_internal.h attributes/attributes_common.h
	${CCC} ${SIGHT_CFLAGS} variant_layout.C -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -c -o variant_layout.o


attributes/attributes_common.o: attributes/attributes_common.C  sight_common_internal.h attributes/attributes_common.h
	${CCC} ${SIGHT_CFLAGS} attributes/attributes_common.C -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -c -o attributes/attributes_common.o

attributes/attributes_structure.o: attributes/attributes_structure.C attributes/attributes_structure.h sight_common_internal.h attributes/attributes_common.h
	${CCC} ${SIGHT_CFLAGS} attributes/attributes_structure.C -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -c -o attributes/attributes_structure.o

attributes/attributes_layout.o: attributes/attributes_layout.C attributes/attributes_layout.h sight_common_internal.h attributes/attributes_common.h
	${CCC} ${SIGHT_CFLAGS} attributes/attributes_layout.C -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -c -o attributes/attributes_layout.o


# Rule for compiling the aspects of widgets that libsight.a requires
.PHONY: widgets_pre
widgets_pre: maketools
	cd widgets; make -f Makefile_pre ${MAKE_DEFINES}

# Rule for compiling the aspects of widgets that require libsight.a
widgets_post: libsight_layout.so libsight_structure.so
	cd widgets; make -f Makefile_post ${MAKE_DEFINES}

maketools: 
	cd tools; make -f Makefile ${MAKE_DEFINES}

binreloc.o: binreloc.c binreloc.h
	${CCC} ${SIGHT_CFLAGS} binreloc.c -c -o binreloc.o

utils.o: utils.C utils.h
	${CCC} ${SIGHT_CFLAGS} utils.C -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -c -o utils.o

HOSTNAME_ARG=$(shell ./getHostnameArg.pl)
getAllHostnames.o: getAllHostnames.C getAllHostnames.h
	${CCC} ${SIGHT_CFLAGS} getAllHostnames.C -DHOSTNAME_ARG="\"${HOSTNAME_ARG}\"" -c -o getAllHostnames.o

gdbLineNum.pl: setupGDBWrap.pl sight_structure.C
	./setupGDBWrap.pl

sightDefines.pl:
	printf "\$$main::sightPath = \"${ROOT_PATH}\"; return 1;" > sightDefines.pl

Makefile.extern: initMakefile.extern
	chmod 755 initMakefile.extern
	./initMakefile.extern ${CC} ${CCC} ${RAPL_ENABLED} ${LLVM32_SRC_PATH} ${LLVM32_BUILD_PATH} ${LLVM32_INSTALL_PATH}

definitions.h: initDefinitionsH
	chmod 755 initDefinitionsH
	./initDefinitionsH ${RAPL_ENABLED}

clean:
	cd widgets; make -f Makefile_pre clean
	cd widgets; make -f Makefile_post clean
	cd tools; make -f Makefile clean
	cd tools make clean
	cd examples; make clean
#	cd apps/mcbench; ./clean-linux-x86_64.sh
	cd apps/mfem; make clean
	rm -rf Makefile.extern definitions.h dbg dbg.* *.a *.so *.o widgets/shellinabox* widgets/mongoose* widgets/graphviz* gdbLineNum.pl
	rm -rf script/taffydb sightDefines.pl gdbscript
	rm -f slayout hier_merge

clean_objects:
	rm -f Makefile.extern definitions.h *.a *.so *.o attributes/*.o widgets/*.o widgets/*/*.o hier_merge slayout
	cd widgets/kulfi; make clean

script/taffydb:
	#cd script; wget --no-check-certificate https://github.com/typicaljoe/taffydb/archive/master.zip
	#cd script; mv master master.zip; unzip master.zip
	#rm script/master*
	cd script; ../getGithub https://github.com/typicaljoe/taffydb/archive/master.zip zip unzip
	mv script/taffydb-master script/taffydb
	chmod 755 script/taffydb
	chmod 644 script/taffydb/*
