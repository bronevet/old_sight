#include ../../Makefile.extern
#CCC=clang++

all: kulfi_structure.bc faultInj

.PHONY: faultInj
faultInj:
	cd faultInj; ${MAKE} -f Makefile

kulfi_structure.bc: kulfi_structure.C
	${CCC} -emit-llvm kulfi_structure.C ${SIGHT_CFLAGS} -c -o kulfi_structure.bc

clean:
	rm -f kulfi_structure.bc
