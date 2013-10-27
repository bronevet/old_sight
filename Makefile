SIGHT_COMMON_O := sight_common.o attributes_common.o binreloc.o getAllHostnames.o utils.o
SIGHT_COMMON_H := sight.h sight_common_internal.h attributes_common.h binreloc.h getAllHostnames.h utils.h
SIGHT_STRUCTURE_O := sight_structure.o attributes_structure.o
SIGHT_STRUCTURE_H := sight.h sight_structure_internal.h attributes_structure.h
SIGHT_LAYOUT_O := sight_layout.o attributes_layout.o 
SIGHT_LAYOUT_H := sight.h sight_layout_internal.h attributes_layout.h 
sight := ${sight_O} ${sight_H} gdbLineNum.pl sightDefines.pl

OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXE := .exe
endif

all: sightDefines.pl gdbLineNum.pl libsight_structure.a slayout${EXE} widgets_post allExamples script/taffydb
	chmod 755 html img script
	chmod 644 html/* img/* script/*
	chmod 755 script/taffydb

ROOT_PATH = ${CURDIR}

# Set to "1" if we wish gdb support to be enabled and otherwise set to 0
ifeq (${OS}, Cygwin)
REMOTE_ENABLED := 0
else
REMOTE_ENABLED := 1
endif

ifneq (${OS}, Cygwin)
# The port on which sight sets up a daemon that invokes gdb so that it runs upto a particular point
# in the target application's execution
GDB_PORT := 17501
endif

.PHONY: apps
apps:
	cd apps/mfem;  make ROOT_PATH=${ROOT_PATH} REMOTE_ENABLED=${REMOTE_ENABLED} GDB_PORT=${GDB_PORT} OS=${OS}
ifneq (${OS}, Cygwin)
	cd apps/mcbench; ./build-linux-x86_64.sh ${ROOT_PATH}
endif

allExamples: libsight_structure.a
	cd examples; make ROOT_PATH=${ROOT_PATH} OS=${OS}

runExamples: libsight_structure.a slayout${EXE} apps
	cd examples; make ROOT_PATH=${ROOT_PATH} OS=${OS} run
	apps/mfem/mfem/examples/ex1 apps/mfem/mfem/data/beam-quad.mesh
	apps/mfem/mfem/examples/ex2 apps/mfem/mfem/data/beam-tet.mesh 2
	apps/mfem/mfem/examples/ex3 apps/mfem/mfem/data/ball-nurbs.mesh
	apps/mfem/mfem/examples/ex4 apps/mfem/mfem/data/fichera-q3.mesh
ifneq (${OS}, Cygwin)
	apps/mcbench/src/MCBenchmark.exe --nCores=1 --distributedSource --numParticles=13107 --nZonesX=256 --nZonesY=256 --xDim=16 --yDim=16 --mirrorBoundary --multiSigma --nThreadCore=1
endif

#pattern${EXE}: pattern.C pattern.h libsight.a ${sight_H}
#	g++ -g pattern.C -L. -lsight  -o pattern${EXE}

slayout${EXE}: slayout.C process.o process.h libsight_layout.a apps
	g++ slayout.C libsight_layout.a -DMFEM -I. -Iapps/mfem apps/mfem/mfem_layout.o -o slayout${EXE}

#	g++ -c slayout.C -o slayout.o
#	g++ slayout.o libsight_layout.a --relocateable -Ur --whole-archive -o slayout${EXE}
#	g++ slayout.C libsight_layout.a -o slayout${EXE}
#	g++ -c slayout.C -o slayout.o
#	ld slayout.o libsight_layout.a --relocateable -Ur --whole-archive -o slayout${EXE}

process.o: process.C process.h sight_common_internal.h
	g++ process.C -c -o process.o

#libsight_common.a: ${SIGHT_COMMON_O} ${SIGHT_COMMON_H} widgets_pre
#	ar -r libsight_common.a ${SIGHT_COMMON_O} widgets/*_common.o

libsight_structure.a: ${SIGHT_STRUCTURE_O} ${SIGHT_STRUCTURE_H} ${SIGHT_COMMON_O} ${SIGHT_COMMON_H} widgets_pre
	ar -r libsight_structure.a ${SIGHT_STRUCTURE_O} ${SIGHT_COMMON_O} widgets/*_structure.o widgets/*_common.o

libsight_layout.a: ${SIGHT_LAYOUT_O} ${SIGHT_LAYOUT_H} ${SIGHT_COMMON_O} ${SIGHT_COMMON_H} widgets_pre
	ar -r libsight_layout.a    ${SIGHT_LAYOUT_O}    ${SIGHT_COMMON_O} widgets/*_layout.o     widgets/*_common.o
#	ld libsight_layout.a    ${SIGHT_LAYOUT_O}    ${SIGHT_COMMON_O} widgets/*_layout.o     widgets/*_common.o


sight_common.o: sight_common.C sight_common_internal.h attributes_common.h
	g++ -g sight_common.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o sight_common.o

sight_structure.o: sight_structure.C sight_structure_internal.h attributes_structure.h sight_common_internal.h attributes_common.h
	g++ -g sight_structure.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o sight_structure.o

sight_layout.o: sight_layout.C sight_layout_internal.h attributes_layout.h sight_common_internal.h attributes_common.h
	g++ -g sight_layout.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o sight_layout.o


attributes_common.o: attributes_common.C  sight_common_internal.h attributes_common.h
	g++ -g attributes_common.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o attributes_common.o

attributes_structure.o: attributes_structure.C attributes_structure.h sight_common_internal.h attributes_common.h
	g++ -g attributes_structure.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o attributes_structure.o

attributes_layout.o: attributes_layout.C attributes_layout.h sight_common_internal.h attributes_common.h
	g++ -g attributes_layout.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o attributes_layout.o


# Rule for compiling the aspects of widgets that libsight.a requires
.PHONY: widgets_pre
widgets_pre:
	cd widgets; make -f Makefile_pre ROOT_PATH=${ROOT_PATH} REMOTE_ENABLED=${REMOTE_ENABLED} GDB_PORT=${GDB_PORT} OS=${OS}

# Rule for compiling the aspects of widgets that require libsight.a
widgets_post: libsight_layout.a libsight_structure.a
	cd widgets; make -f Makefile_post ROOT_PATH=${ROOT_PATH} REMOTE_ENABLED=${REMOTE_ENABLED} GDB_PORT=${GDB_PORT} OS=${OS}

binreloc.o: binreloc.c binreloc.h
	g++ -g binreloc.c -c -o binreloc.o

utils.o: utils.C utils.h
	g++ -g utils.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o utils.o

HOSTNAME_ARG=$(shell getHostnameArg.pl)
getAllHostnames.o: getAllHostnames.C getAllHostnames.h
	g++ -g getAllHostnames.C -DHOSTNAME_ARG="\"${HOSTNAME_ARG}\"" -c -o getAllHostnames.o

gdbLineNum.pl: setupGDBWrap.pl sight_structure.C
	./setupGDBWrap.pl

sightDefines.pl:
	printf "\$$main::sightPath = \"${ROOT_PATH}\"; return 1;" > sightDefines.pl

clean:
	cd widgets; make -f Makefile_pre clean
	cd widgets; make -f Makefile_post clean
	cd examples; make clean
	cd apps/mcbench; ./clean-linux-x86_64.sh
	cd apps/mfem; make clean
	rm -rf dbg dbg.* libsight.a *.o widgets/shellinabox* widgets/mongoose* widgets/graphviz* gdbLineNum.pl
	rm -rf script/taffydb sightDefines.pl gdbscript

script/taffydb:
	cd script; wget --no-check-certificate https://github.com/typicaljoe/taffydb/archive/master.zip
	cd script; mv master master.zip; unzip master.zip
	mv script/taffydb-master script/taffydb
	rm script/master*
	chmod 755 script/taffydb
	chmod 644 script/taffydb/*

