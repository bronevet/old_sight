DBGLOG := dbglog.o dbglog.h widgets.o widgets.h binreloc.o binreloc.h getAllHostnames.C getAllHostnames.h getAllHostnames.o gdbLineNum.pl

all: dbgLogTester dbgLogGraphTester ${DBGLOG} widgets/shellinabox-2.14/shellinaboxd widgets/mongoose/mongoose

# Set this to the current Operating System (needed by the Mongoose web server). 
# Choices: linux|bsd|solaris|mac|windows|mingw|cygwin
OS_MONGOOSE := linux

# Set this if graphviz is installed. If graphviz is on the path, set it to the empty string "\"\"".
# Otherwise, place the full path, inside the quotes.
DOT_PATH := "\"\""

# The port on which dbglog sets up a daemon that invokes gdb so that it runs upto a particular point
# in the target application's execution
GDB_PORT := 17500

dbgLogTester: dbgLogTester.C ${DBGLOG}
	g++ -g dbgLogTester.C dbglog.o widgets.o binreloc.o getAllHostnames.o -o dbgLogTester

dbgLogGraphTester: dbgLogGraphTester.C ${DBGLOG}
	g++ -g dbgLogGraphTester.C dbglog.o widgets.o binreloc.o getAllHostnames.o -o dbgLogGraphTester

dbglog.o: dbglog.C dbglog.h
	g++ -g dbglog.C -DROOT_PATH="\"${CURDIR}\"" -DDOT_PATH=${DOT_PATH} -DGDB_PORT=${GDB_PORT} -c -o dbglog.o
	
widgets.o: widgets.C widgets.h
	g++ -g widgets.C -DROOT_PATH="\"${CURDIR}\"" -DDOT_PATH=${DOT_PATH} -DGDB_PORT=${GDB_PORT} -c -o widgets.o
	
binreloc.o: binreloc.c binreloc.h
	g++ -g binreloc.c -c -o binreloc.o

getAllHostnames.o: getAllHostnames.C getAllHostnames.h
	g++ -g getAllHostnames.C -c -o getAllHostnames.o

gdbLineNum.pl: setupGDBWrap.pl
	./setupGDBWrap.pl

clean:
	killP widgets/mongoose/mongoose
	killP widgets/shellinabox-2.14/shellinaboxd
	rm -rf dbg *.o dbgLogTester dbgLogGraphTester *.exe widgets/shellinabox-2.14* widgets/mongoose* gdbLineNum.pl

widgets/shellinabox-2.14/shellinaboxd:
	cd widgets; wget --no-check-certificate https://shellinabox.googlecode.com/files/shellinabox-2.14.tar.gz
	cd widgets; tar -xf shellinabox-2.14.tar.gz
	cd widgets/shellinabox-2.14; ./configure --prefix=`pwd`
	cd widgets/shellinabox-2.14; make

widgets/mongoose/mongoose:
	cd widgets; wget --no-check-certificate https://mongoose.googlecode.com/files/mongoose-3.8.tgz
	cd widgets; tar -xf mongoose-3.8.tgz
	cd widgets; rm mongoose-3.8.tgz
	cd widgets/mongoose; make ${OS_MONGOOSE}

