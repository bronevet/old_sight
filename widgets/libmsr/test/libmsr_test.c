#include <unistd.h>
#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"
#ifdef MPI
#include <mpi.h>
#endif

void rapl_set_test(double limit) {
	struct rapl_limit CPULimit, DRAMLimit;
	int s;
	CPULimit.watts = limit;
	CPULimit.seconds = 1;
	DRAMLimit.watts = limit;
	DRAMLimit.seconds = 1;
	fprintf(stdout, "Capping power at %f\n", limit);
	for(s=0; s<NUM_SOCKETS; s++) {
		set_rapl_limit(s, &CPULimit, NULL, &DRAMLimit);
	}
}

void perform_rapl_measurement(struct rapl_data* r) {
	int socket;
	for(socket=0; socket<NUM_SOCKETS; socket++) {
		read_rapl_data(socket, r);
		fprintf(stdout,"%8.6lf\t%8.6lf\t%8.6lf\t", r->pkg_watts, r->dram_watts, r->elapsed);
	}
	#ifdef LINEBREAK
	fprintf(stdout, "\n");
	#endif
}

int main(int argc, char** argv){
	init_msr();

	struct rapl_data r; r.flags = RDF_REENTRANT;
	int power;
	read_rapl_data(0, &r);  // Initialize r*/
	for(power=75; power>=25; power-=25) {
		rapl_set_test(power);
		sleep(2);
		perform_rapl_measurement(&r);
	}

	finalize_msr();
	#ifdef MPI
	MPI_Finalize();
	#endif

	return 0;
}
