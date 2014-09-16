all: box_api_cpp.o box_merge.o box_layout.o

box_api_cpp.o: box_api_cpp.o: box_api_cpp.C box_api_cpp.h ../../attribut../../attributes_common.h ../../sight_structure.h
	${CCC} ${SIGHT_CFLAGS} box_api_cpp.C -I.. -DROOT_PATH="\"${ROOT_PATH}\"" -c -o box_api_cpp.o

box_merge.o: box_merge.C box_merge.h ../../attribut../../attributes_structure.h ../../sight_structure.h
	${CCC} ${SIGHT_CFLAGS} box_merge.C -I.. -DROOT_PATH="\"${ROOT_PATH}\"" -c -o box_merge.o

box_layout.o: box_layout.C box_layout.h ../../attribut../../attributes_layout.h ../../sight_layout.h
	${CCC} ${SIGHT_CFLAGS} box_layout.C -I.. -DROOT_PATH="\"${ROOT_PATH}\"" -c -o box_layout.o


