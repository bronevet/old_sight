include ../../pro_sight/sight/Makefile.extern

CXX=mpi${CCC}
OPTS=-O3
LD_OPTS=
INCS=-I../../pnmpi-main/src/
LIBS=

all: ex

ex: ex.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ../../INSTALL/pnmpi/lib/ -lpnmpi -Wl,-rpath,../../INSTALL/pnmpi/lib/

ex.o: example.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<

clean:
	rm  ex ex.o
