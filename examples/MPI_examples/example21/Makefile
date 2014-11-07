include ../../pro_sight/sight/Makefile.extern

CXX=mpig++
#${CCC}
OPTS=-O3
LD_OPTS=
INCS=-I../../pnmpi-main/src/
LIBS=
PNMPI=../../pro_sight/sight/widget/PnMPI/INSTALL/

all: ex

ex: ex.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI}/lib/ -lpnmpi -Wl,-rpath,${PNMPI}/lib/

ex.o: example.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<

clean:
	rm  ex ex.o
