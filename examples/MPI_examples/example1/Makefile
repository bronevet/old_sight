include ../../pro_sight/sight/Makefile.extern

CXX=mpi${CCC}
OPTS=-O3
LD_OPTS=
INCS=-I../../pnmpi-main/src/
LIBS=

all: ex

ex: ex.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ../../../widget/PnMPI/INSTALL/lib/ -lpnmpi -Wl,-rpath,../../../widget/PnMPI/INSTALL/lib/

ex.o: example.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<

clean:
	rm  ex ex.o
