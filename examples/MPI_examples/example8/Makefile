include ../../pro_sight/sight/Makefile.extern

CXX=mpi${CCC}
OPTS=-O3
LD_OPTS=
INCS=-I../../pnmpi-main/src/
LIBS=

all: ex

ex: ex.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -lX11 -o $@ $< -L ../../INSTALL/pnmpi/lib/ -lpnmpi -Wl,-rpath,../../INSTALL/pnmpi/lib/ -lX11 -lm

ex.o: example.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $< -lX11 -lm

clean:
	rm  ex ex.o
