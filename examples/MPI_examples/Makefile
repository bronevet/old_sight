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

bin/3.non_blocking_send_recv: bin/3.non_blocking_send_recv.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/3.non_blocking_send_recv.o: src/3.non_blocking_send_recv.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/4.array_decomposition: bin/4.array_decomposition.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/4.array_decomposition.o: src/4.array_decomposition.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/5.matrix_multiply: bin/5.matrix_multiply.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/5.matrix_multiply.o: src/5.matrix_multiply.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/6.pi_calculation_p2p: bin/6.pi_calculation_p2p.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/6.pi_calculation_p2p.o: src/6.pi_calculation_p2p.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/7.pi_calculation_collectives: bin/7.pi_calculation_collectives.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/7.pi_calculation_collectives.o: src/7.pi_calculation_collectives.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/8.wave: bin/8.wave.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/8.wave.o: src/8.wave.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/9.2D_heat_equation: bin/9.2D_heat_equation.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/9.2D_heat_equation.o: src/9.2D_heat_equation.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/10.round_trip_latency: bin/10.round_trip_latency.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/10.round_trip_latency.o: src/10.round_trip_latency.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/11.bandwidth_timing_test_blocking: bin/11.bandwidth_timing_test_blocking.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/11.bandwidth_timing_test_blocking.o: src/11.bandwidth_timing_test_blocking.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/12.prime_nums: bin/12.prime_nums.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/12.prime_nums.o: src/12.prime_nums.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/13.ping_pong: bin/13.ping_pong.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/13.ping_pong.o: src/13.ping_pong.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/14.scatter_oper: bin/14.scatter_oper.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/14.scatter_oper.o: src/14.scatter_oper.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/15.contiguous_derived_data: bin/15.contiguous_derived_data.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/15.contiguous_derived_data.o: src/15.contiguous_derived_data.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/16.vector_derived_data: bin/16.vector_derived_data.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/16.vector_derived_data.o: src/16.vector_derived_data.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/17.indexed_derived_data: bin/17.indexed_derived_data.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/17.indexed_derived_data.o: src/17.indexed_derived_data.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/18.structured_derived_data: bin/18.structured_derived_data.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/18.structured_derived_data.o: src/18.structured_derived_data.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
bin/19.group_communications: bin/19.group_communications.o
	$(CXX) $(LD_OPTS) ${SIGHT_LINKFLAGS} -o $@ $< -L ${PNMPI} -lpnmpi -Wl,-rpath,${PNMPI}

bin/19.group_communications.o: src/19.group_communications.c
	$(CXX) $(OPTS) $(INCS) ${SIGHT_CFLAGS} -c -o $@ $<
	
clean exes:
	rm -rf bin/*
	
clean output:
	rm -rf output