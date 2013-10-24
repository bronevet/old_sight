all: mfem mfem.o mfem_examples meshFile2Socket glvis

.PHONY: mfem
mfem:
	cd mfem; make ROOT_PATH=${ROOT_PATH}

mfem_examples:
	cd mfem/examples; make ROOT_PATH=${ROOT_PATH}
#	wget --no-check-certificate https://mfem.googlecode.com/files/mfem-2.0.tgz
#	tar -xf mfem-2.0.tgz
#	rm mfem-2.0.tgz
#	cd mfem-2.0; make ROOT_PATH=${ROOT_PATH}
#	mv mfem-2.0 mfem

mfem.o: mfem.C mfem.h ../../dbglog_structure_internal.h mfem/mfem.hpp
	g++ mfem.C -c -I${ROOT_PATH} -Imfem -DROOT_PATH="\"${ROOT_PATH}\"" -DGDB_PORT=${GDB_PORT} -o mfem.o

meshFile2Socket: meshFile2Socket.cpp mfem/mfem.hpp 
	g++ meshFile2Socket.cpp -Imfem -Lmfem -lmfem -I${ROOT_PATH} -L${ROOT_PATH} -ldbglog_structure -DROOT_PATH=${ROOT_PATH} -o meshFile2Socket

glvis:
#	cd glvis; make ROOT_PATH=${ROOT_PATH}
	wget --no-check-certificate https://glvis.googlecode.com/files/glvis-2.0.tgz 
	tar -xf glvis-2.0.tgz
	rm glvis-2.0.tgz
	cp glvismakefile glvis-2.0/makefile
	cd glvis-2.0; make
	mv glvis-2.0 glvis

clean:
	rm -fr meshFile2Socket
	cd mfem; make clean
	rm -f mfem.o
	../../killP glvis
	rm -rf glvis
#	cd glvis; make clean

