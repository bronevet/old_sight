# Makefile for src-parallel

SHELL = /bin/bash

### CoMD can be built in either double or single precision and with or
### without MPI.  Select desired precision and MPI here.

# double precision (ON/OFF)
DOUBLE_PRECISION = ON
# MPI for parallel (ON/OFF)
DO_MPI = ON

### Set your desired C compiler and any necessary flags.  Note that CoMD
### uses some c99 features.  You can also set flags for optimization and
### specify paths to include files that the compiler can't find on its
### own.  If you need any -L or -l switches to get C standard libraries
### (such as -lm for the math library) put them in C_LIB.
#CCC := mpi${CCC}
CFLAGS = -I../../.. ${SIGHT_CFLAGS}
OPTFLAGS = -g -O3
INCLUDES = 
C_LIB = -lm


### If you need to specify include paths, library paths, or link flags
### for MPI, put them here.  Put both -L and -l switches into MPI_LIB.
MPI_LIB =
MPI_INCLUDE =

### A place to specify any other include or library switches your
### platform requires.
OTHER_LIB =
OTHER_INCLUDE =




#########################################
### Below here, it is pitch black.  
### You are likely to be eaten by a grue.
##########################################

# clear all suffixes
.SUFFIXES:
# list only those that we use 
.SUFFIXES: .c .o

.PHONY: DEFAULT clean distclean depend

BIN_DIR=../bin

# Check for double precision
ifeq ($(DOUBLE_PRECISION), ON)
CFLAGS += -DDOUBLE
else
CFLAGS += -DSINGLE
endif

# Set executable name and add includes & libraries for MPI if needed.
CoMD_VARIANT = CoMD-serial
ifeq ($(DO_MPI), ON)
CoMD_VARIANT = CoMD-mpi
INCLUDES += ${MPI_INCLUDE}
CFLAGS += -DDO_MPI
LDFLAGS += ${MPI_LIB}
endif
CoMD_EXE = ${BIN_DIR}/${CoMD_VARIANT}

LDFLAGS += ${C_LIB} ${OTHER_LIB} -L../../.. -lsight_structure ${SIGHT_LINKFLAGS} 
CFLAGS  += ${OPTFLAGS} ${INCLUDES} ${OTHER_INCLUDE}


SOURCES=$(wildcard *.c)
OBJECTS_MODULES=$(SOURCES:.c=.modules.o)
OBJECTS_COMPMODULES=$(SOURCES:.c=.compmodules.o)
OBJECTS_DISTILLMODULES=$(SOURCES:.c=.distillmodules.o)
OBJECTS_TRACEPOS=$(SOURCES:.c=.tracepos.o)
OBJECTS_TRACEPATH=$(SOURCES:.c=.tracepath.o)
	
DEFAULT: ${CoMD_EXE}.modules ${CoMD_EXE}.compmodules ${CoMD_EXE}.distillmodules ${CoMD_EXE}.tracepos ${CoMD_EXE}.tracepath

%.modules.o: %.c
	${MPICCC} ${CFLAGS} -DMODULES -c $< -o $@

%.compmodules.o: %.c
	${MPICCC} ${CFLAGS} -DMODULES -DMOD_COMP -c $< -o $@
	
%.distillmodules.o: %.c
	${MPICCC} ${CFLAGS} -DMODULES -DDISTILL_MONITOR -c $< -o $@

%.tracepos.o: %.c
	${MPICCC} ${CFLAGS} -DTRACE_POS -c $< -o $@

%.tracepath.o: %.c
	${MPICCC} ${CFLAGS} -DTRACE_PATH -c $< -o $@


${CoMD_EXE}.modules: ${BIN_DIR} CoMD_info.h ${OBJECTS_MODULES} 
	${MPICCC} ${CFLAGS} -o ${CoMD_EXE}.modules ${OBJECTS_MODULES} ${LDFLAGS}

${CoMD_EXE}.compmodules: ${BIN_DIR} CoMD_info.h ${OBJECTS_COMPMODULES} 
	${MPICCC} ${CFLAGS} -o ${CoMD_EXE}.compmodules ${OBJECTS_COMPMODULES} ${LDFLAGS}

${CoMD_EXE}.distillmodules: ${BIN_DIR} CoMD_info.h ${OBJECTS_DISTILLMODULES} 
	${MPICCC} ${CFLAGS} -o ${CoMD_EXE}.distillmodules ${OBJECTS_DISTILLMODULES} ${LDFLAGS}

${CoMD_EXE}.tracepos: ${BIN_DIR} CoMD_info.h ${OBJECTS_TRACEPOS} 
	${MPICCC} ${CFLAGS} -o ${CoMD_EXE}.tracepos ${OBJECTS_TRACEPOS} ${LDFLAGS}

${CoMD_EXE}.tracepath: ${BIN_DIR} CoMD_info.h ${OBJECTS_TRACEPATH} 
	${MPICCC} ${CFLAGS} -o ${CoMD_EXE}.tracepath ${OBJECTS_TRACEPATH} ${LDFLAGS}

CoMD_info.h: Makefile
	./generate_info_header ${CoMD_VARIANT} "$(CC)" "$(CFLAGS)" "$(LDFLAGS)"

${BIN_DIR}:
	@if [ ! -d ${BIN_DIR} ]; then mkdir -p ${BIN_DIR} ; fi

clean:
	rm -f *.o CoMD_info.h .depend

distclean: clean
	rm -f ${CoMD_EXE} .depend.bak
	rm -rf html latex

.depend: $(SOURCES)
	@touch .depend
	@$(MAKE) --no-print-directory depend

depend:
	@echo "Rebuilding dependencies..."
	@$(MAKE) CoMD_info.h
	@makedepend -f .depend -Y. --$(CFLAGS)-- $(SOURCES) 2> /dev/null


-include .depend
