all: mfem mfem_layout.o mfem_structure.o mfem_examples meshFile2Socket glvis

.PHONY: mfem
mfem:
	cd mfem; make ROOT_PATH=${ROOT_PATH} SIGHT_CFLAGS="${SIGHT_CFLAGS}"  CC=${CCC}

mfem_examples:
	cd mfem/examples; make ROOT_PATH=${ROOT_PATH} SIGHT_CFLAGS="${SIGHT_CFLAGS}" SIGHT_LINKFLAGS="${SIGHT_LINKFLAGS}" CC=${CCC} VNC_ENABLED=${VNC_ENABLED}
#	wget --no-check-certificate https://mfem.googlecode.com/files/mfem-2.0.tgz
#	tar -xf mfem-2.0.tgz
#	rm mfem-2.0.tgz
#	cd mfem-2.0; make ROOT_PATH=${ROOT_PATH}
#	mv mfem-2.0 mfem

mfem_layout.o: mfem_layout.C mfem_layout.h ../../sight_layout_internal.h mfem/mfem.hpp
	${CCC} ${SIGHT_CFLAGS} mfem_layout.C -c -I${ROOT_PATH} -Imfem -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -o mfem_layout.o

mfem_structure.o: mfem_structure.C mfem_structure.h ../../sight_structure_internal.h mfem/mfem.hpp
	${CCC} ${SIGHT_CFLAGS} mfem_structure.C -c -I${ROOT_PATH} -Imfem -DROOT_PATH="\"${ROOT_PATH}\"" -DREMOTE_ENABLED=${REMOTE_ENABLED} -DGDB_PORT=${GDB_PORT} -o mfem_structure.o

meshFile2Socket: meshFile2Socket.cpp mfem/mfem.hpp 
	${CCC} ${SIGHT_CFLAGS} meshFile2Socket.cpp -Imfem -Lmfem -lmfem -I${ROOT_PATH} -L${ROOT_PATH} -lsight_structure -DROOT_PATH=${ROOT_PATH} ${SIGHT_LINKFLAGS} -o meshFile2Socket

ifeq (${REMOTE_ENABLED}, 1)
glvis:
#	cd glvis; make ROOT_PATH=${ROOT_PATH}
	wget --no-check-certificate https://glvis.googlecode.com/files/glvis-2.0.tgz 
	tar -xf glvis-2.0.tgz
	rm glvis-2.0.tgz
	cp glvismakefile glvis-2.0/makefile
	cd glvis-2.0; make ROOT_PATH=${ROOT_PATH} SIGHT_CFLAGS="${SIGHT_CFLAGS}" SIGHT_LINKFLAGS="${SIGHT_LINKFLAGS}" CC=${CCC}
	mv glvis-2.0 glvis
else
glvis:
endif

clean:
	rm -fr meshFile2Socket
	cd mfem; make clean
	rm -f mfem_layout.o mfem_structure.o
	../../killP glvis
	rm -rf glvis
#	cd glvis; make clean

