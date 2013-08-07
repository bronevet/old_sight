DBGLOG := dbglog.o dbglog.h widgets.o widgets.h binreloc.o binreloc.h getAllHostnames.C getAllHostnames.h getAllHostnames.o gdbLineNum.pl

all: dbgLogTester dbgLogGraphTester ${DBGLOG} widgets/shellinabox/shellinaboxd widgets/mongoose/mongoose widgets/graphviz

# Set this to the current Operating System (needed by the Mongoose web server). 
# Choices: linux|bsd|solaris|mac|windows|mingw|cygwin
OS_MONGOOSE := linux

# Set this if graphviz is installed. If graphviz is on the path, set it to the empty string "\"\"".
# Otherwise, place the full path, inside the quotes.
#DOT_PATH := "\"\""
DBGLOG_PATH = `pwd`

# The port on which dbglog sets up a daemon that invokes gdb so that it runs upto a particular point
# in the target application's execution
GDB_PORT := 17500

dbgLogTester: dbgLogTester.C ${DBGLOG}
	g++ -g dbgLogTester.C dbglog.o widgets.o binreloc.o getAllHostnames.o -o dbgLogTester

dbgLogGraphTester: dbgLogGraphTester.C ${DBGLOG}
	g++ -g dbgLogGraphTester.C dbglog.o widgets.o binreloc.o getAllHostnames.o -o dbgLogGraphTester

dbglog.o: dbglog.C dbglog.h
	g++ -g dbglog.C -DROOT_PATH="\"${CURDIR}\"" -DGDB_PORT=${GDB_PORT} -c -o dbglog.o
	
widgets.o: widgets.C widgets.h
	g++ -g widgets.C -DROOT_PATH="\"${CURDIR}\"" -DGDB_PORT=${GDB_PORT} -c -o widgets.o
	
binreloc.o: binreloc.c binreloc.h
	g++ -g binreloc.c -c -o binreloc.o

getAllHostnames.o: getAllHostnames.C getAllHostnames.h
	g++ -g getAllHostnames.C -c -o getAllHostnames.o

gdbLineNum.pl: setupGDBWrap.pl
	./setupGDBWrap.pl

clean:
	killP widgets/mongoose/mongoose
	killP widgets/shellinabox/bin/shellinaboxd
	rm -rf dbg *.o dbgLogTester dbgLogGraphTester *.exe widgets/shellinabox* widgets/mongoose* widgets/graphviz* gdbLineNum.pl

widgets/shellinabox/shellinaboxd:
	cd widgets; wget --no-check-certificate https://shellinabox.googlecode.com/files/shellinabox-2.14.tar.gz
	cd widgets; tar -xf shellinabox-2.14.tar.gz
	cd widgets/shellinabox-2.14; ./configure --prefix=${CURDIR}/widgets/shellinabox
	cd widgets/shellinabox-2.14; make
	cd widgets/shellinabox-2.14; make install
	rm -r widgets/shellinabox-2.14 widgets/shellinabox-2.14.tar.gz

widgets/mongoose/mongoose:
	cd widgets; wget --no-check-certificate https://mongoose.googlecode.com/files/mongoose-3.8.tgz
	cd widgets; tar -xf mongoose-3.8.tgz
	cd widgets; rm mongoose-3.8.tgz
	cd widgets/mongoose; make ${OS_MONGOOSE}

widgets/graphviz:
	cd widgets; wget http://www.graphviz.org/pub/graphviz/stable/SOURCES/graphviz-2.32.0.tar.gz
	cd widgets; tar -xf graphviz-2.32.0.tar.gz
	cd widgets/graphviz-2.32.0; export CC=gcc; export CXX=g++; ./configure --prefix=${CURDIR}/widgets/graphviz --disable-swig --disable-sharp --disable-go --disable-io --disable-java --disable-lua --disable-ocaml --disable-perl --disable-php --disable-python --disable-r --disable-ruby --disable-tcl --without-pic --without-efence --without-expat --without-devil --without-webp --without-poppler --without-ghostscript --without-visio --without-pangocairo --without-lasi --without-glitz --without-freetype2 --without-fontconfig --without-gdk-pixbuf --without-gtk --without-gtkgl --without-gtkglext --without-gts --without-glade --without-ming --without-qt --without-quartz --without-gdiplus --without-libgd --without-glut --without-sfdp --without-smyrna --without-ortho --without-digcola --without-ipsepcola --enable-static --disable-shared
	cd widgets/graphviz-2.32.0; make
	cd widgets/graphviz-2.32.0; make install
	rm -r widgets/graphviz-2.32.0 widgets/graphviz-2.32.0.tar.gz
