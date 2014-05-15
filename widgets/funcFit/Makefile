SRC = functionSetup.C ga.C parseinput.C transformation.C
OBJ = functionSetup.o ga.o parseinput.o transformation.o

GSL_PATH = ../gsl

.SUFFIXES: .c .C .o 
.C.o:
	${CCC} ${SIGHT_CFLAGS} -I${GSL_PATH}/include $^ -c -o $@

all: funcFit #ga
	
funcFit: filter.o ${OBJ} ${ROOT_PATH}/libsight_layout.*
	${CCC} -o funcFit filter.o ${OBJ} ${SIGHT_LINKFLAGS} -L${ROOT_PATH} -lsight_layout -lm -L${GSL_PATH}/lib -lgsl -lpthread -lgslcblas

ga: main.o datainput.o ${OBJ}
	${CCC} -o ga main.o ${OBJ} datainput.o -lm -L${GSL_PATH}/lib -lgsl -lpthread -lgslcblas

clean:
	rm -f funcFit
	rm -f *.o
