DBGLOG_O := dbglog.o attributes.o binreloc.o getAllHostnames.o
DBGLOG_H := dbglog.h dbglog_internal.h attributes.h binreloc.h getAllHostnames.h
DBGLOG := ${DBGLOG_O} ${DBGLOG_H} gdbLineNum.pl dbglogDefines.pl

OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXE := .exe
endif

all: libdbglog.a widgets_post allExamples script/taffydb
	chmod 755 html img script
	chmod 644 html/* img/* script/*
	chmod 755 script/taffydb

ROOT_PATH = ${CURDIR}

# The port on which dbglog sets up a daemon that invokes gdb so that it runs upto a particular point
# in the target application's execution
GDB_PORT := 17500

.PHONY: apps
apps:
	cd apps/mfem;  make ROOT_PATH=${ROOT_PATH} GDB_PORT=${GDB_PORT} OS=${OS}
	cd apps/mcbench; ./build-linux-x86_64.sh ${ROOT_PATH}

allExamples: libdbglog.a
	cd examples; make ROOT_PATH=${ROOT_PATH} OS=${OS}

runExamples: libdbglog.a apps
	cd examples; make ROOT_PATH=${ROOT_PATH} OS=${OS} run
	apps/mfem/mfem/examples/ex1 apps/mfem/mfem/data/beam-quad.mesh
	apps/mfem/mfem/examples/ex2 apps/mfem/mfem/data/beam-tet.mesh 2
	apps/mfem/mfem/examples/ex3 apps/mfem/mfem/data/ball-nurbs.mesh
	apps/mfem/mfem/examples/ex4 apps/mfem/mfem/data/fichera-q3.mesh
	apps/mcbench/src/MCBenchmark.exe --nCores=1 --distributedSource --numParticles=13107 --nZonesX=256 --nZonesY=256 --xDim=16 --yDim=16 --mirrorBoundary --multiSigma --nThreadCore=1

#pattern${EXE}: pattern.C pattern.h libdbglog.a ${DBGLOG_H}
#	g++ -g pattern.C -L. -ldbglog  -o pattern${EXE}

libdbglog.a: ${DBGLOG_O} ${DBGLOG_H} widgets_pre
	ar -r libdbglog.a ${DBGLOG_O} widgets/*.o
	
dbglog.o: dbglog.C dbglog.h attributes.h
	g++ -g dbglog.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o dbglog.o

attributes.o: attributes.C attributes.h
	g++ -g attributes.C -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -c -o attributes.o

# Rule for compiling the aspects of widgets that libdbglog.a requires
.PHONY: widgets_pre
widgets_pre:
	cd widgets; make -f Makefile_pre ROOT_PATH=${ROOT_PATH} GDB_PORT=${GDB_PORT} OS=${OS}

# Rule for compiling the aspects of widgets that require libdbglog.a
widgets_post: libdbglog.a
	cd widgets; make -f Makefile_post ROOT_PATH=${ROOT_PATH} GDB_PORT=${GDB_PORT} OS=${OS}

binreloc.o: binreloc.c binreloc.h
	g++ -g binreloc.c -c -o binreloc.o

getAllHostnames.o: getAllHostnames.C getAllHostnames.h
	g++ -g getAllHostnames.C -c -o getAllHostnames.o

gdbLineNum.pl: setupGDBWrap.pl
	./setupGDBWrap.pl

dbglogDefines.pl:
	printf "\$$main::dbglogPath = \"${DBGLOG_PATH}\";" > dbglogDefines.pl

clean:
	cd widgets; make -f Makefile_pre clean
	cd widgets; make -f Makefile_post clean
	cd examples; make clean
	cd apps/mcbench; ./clean-linux-x86_64.sh
	cd apps/mfem; make clean
	rm -rf dbg dbg.* libdbglog.a *.o widgets/shellinabox* widgets/mongoose* widgets/graphviz* gdbLineNum.pl

script/taffydb:
	cd script; wget --no-check-certificate https://github.com/typicaljoe/taffydb/archive/master.zip
	cd script; mv master master.zip; unzip master.zip
	mv script/taffydb-master script/taffydb
	rm script/master*
	chmod 755 script/taffydb
	chmod 644 script/taffydb/*

