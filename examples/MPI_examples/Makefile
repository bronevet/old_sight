include ../../Makefile.extern

CXX=mpi${CCC}
OPTS=-O3
LD_OPTS=
INCS=-I../../widgets/PnMPI/INSTALL/include/
LIBS=
PNMPI=../../widgets/PnMPI/INSTALL/lib/

all: bin/1.mpi_hello bin/2.blocking_send_recv

bin/1.mpi_hello: bin/1.mpi_hello.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI} ; rm bin/1.mpi_hello.o;

bin/1.mpi_hello.o: src/1.mpi_hello.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<

bin/2.blocking_send_recv: bin/2.blocking_send_recv.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/2.blocking_send_recv.o: src/2.blocking_send_recv.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<



#ex: ex.o
#	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}
#
#ex.o: example.c
#	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
clean:
	rm -rf bin/*