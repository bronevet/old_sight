include ./../../Makefile.extern
SHELL = /bin/sh
.SUFFIXES: .cc .o

APPFLAG = -D_PROFILER_OLD=0 \
					-D_PROFILER=1 \
					-D_ENABLE_MODULE=1 \
					-D_ENABLE_SCOPE=0 \
					-D_ENABLE_COMP=0 \
					-D_ENABLE_CDNODE=0
CXX=mpig++ ${APPFLAG} 
CXXFLAGS = -g -fPIC -std=gnu++0x -Wall ${SIGHT_CFLAGS}

LDFLAGS  = ${SIGHT_LINKFLAGS}
SOURCE_CD_PROFILER= $(wildcard *.cc)
CD_PROFILER_H= ./*.h
CD_PROFILER_O=$(SOURCE_CD_PROFILER:.cc=.o)

#LIBOUTPUT=../lib/libcd.a
#$(LIBOUTPUT): $(CD_PROFILER_O) 
#	 #$(CXX) $(LDFLAGS) $(CD_PROFILER_O) -o $@
#	 ar -cr $@ $(CD_PROFILER_O) 

LIBOUTPUT=../lib/libcd_profiler.so

.cc.o: 
	@echo "\n\n CD Profiler Building $<\n\n"
	$(CXX) -c $(CXXFLAGS) $< -o $@


all: $(LIBOUTPUT)

$(LIBOUTPUT): ${CD_PROFILER_O} ${CD_PROFILER_H}
	@echo "\n\n Building Shared Object File of CD Profiler...  ${CD_PROFILER_O} \n\n"
	${CXX} -shared -Wl,-soname,libcd_profiler.so -o ${LIBOUTPUT} ${CD_PROFILER_O} -L${SIGHT_ROOT} -lsight_structure ${LDFLAGS}

clean:
	rm $(LIBOUTPUT) $(CD_PROFILER_O)

