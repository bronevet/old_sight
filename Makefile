DBGLOG_O := dbglog.o widgets.o attributes.o binreloc.o getAllHostnames.o widgets/valSelector.o
DBGLOG_H := dbglog.h dbglog_internal.h widgets.h attributes.h binreloc.h getAllHostnames.h widgets/valSelector.h
DBGLOG := ${DBGLOG_O} ${DBGLOG_H} gdbLineNum.pl

OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXT := .exe
endif

all: dbgLogTester${EXT} dbgLogGraphTester${EXT} dbgLogAttrTester${EXT} dbgLogClientServerTester${EXT} \
     ${DBGLOG} \
     widgets/shellinabox/bin/shellinaboxd${EXT} widgets/mongoose/mongoose${EXT} widgets/graphviz/bin/dot${EXT} script/taffydb
	chmod 755 html img script
	chmod 644 html/* img/* script/*
	chmod 755 widgets/canviz-0.1 script/taffydb
	chmod 644 widgets/canviz-0.1/* script/taffydb/*
	chmod 755 widgets/canviz-0.1/excanvas widgets/canviz-0.1/lib widgets/canviz-0.1/path widgets/canviz-0.1/prototype
	chmod 644 widgets/canviz-0.1/*/*

# Set this to the current Operating System (needed by the Mongoose web server). 
# Choices: linux|bsd|solaris|mac|windows|mingw|cygwin
ifeq (${OS}, Cygwin)
OS_MONGOOSE := cygwin
else
OS_MONGOOSE := linux
endif

DBGLOG_PATH = `pwd`

# The port on which dbglog sets up a daemon that invokes gdb so that it runs upto a particular point
# in the target application's execution
GDB_PORT := 17500

dbgLogTester${EXT}: dbgLogTester.C libdbglog.a ${DBGLOG_H}
	g++ -g dbgLogTester.C -L. -ldbglog -o dbgLogTester${EXT}

dbgLogGraphTester${EXT}: dbgLogGraphTester.C libdbglog.a ${DBGLOG_H}
	g++ -g dbgLogGraphTester.C -L. -ldbglog -o dbgLogGraphTester${EXT}

dbgLogAttrTester${EXT}: dbgLogAttrTester.C libdbglog.a ${DBGLOG_H}
	g++ -g dbgLogAttrTester.C -L. -ldbglog  -o dbgLogAttrTester${EXT}

dbgLogClientServerTester${EXT}: dbgLogClientServerTester.C libdbglog.a ${DBGLOG_H}
	g++ -g dbgLogClientServerTester.C -L. -ldbglog  -o dbgLogClientServerTester${EXT}

libdbglog.a: ${DBGLOG_O} ${DBGLOG_H}
	ar -r libdbglog.a ${DBGLOG_O}
	
dbglog.o: dbglog.C dbglog.h attributes.h
	g++ -g dbglog.C -DROOT_PATH="\"${CURDIR}\"" -DGDB_PORT=${GDB_PORT} -c -o dbglog.o
	
widgets.o: widgets.C widgets.h attributes.h
	g++ -g widgets.C -DROOT_PATH="\"${CURDIR}\"" -DGDB_PORT=${GDB_PORT} -c -o widgets.o
	
attributes.o: attributes.C attributes.h
	g++ -g attributes.C -DROOT_PATH="\"${CURDIR}\"" -DGDB_PORT=${GDB_PORT} -c -o attributes.o

widgets/valSelector.o: widgets/valSelector.C widgets/valSelector.h
	g++ -g widgets/valSelector.C -DROOT_PATH="\"${CURDIR}\"" -DGDB_PORT=${GDB_PORT} -c -o widgets/valSelector.o

binreloc.o: binreloc.c binreloc.h
	g++ -g binreloc.c -c -o binreloc.o

getAllHostnames.o: getAllHostnames.C getAllHostnames.h
	g++ -g getAllHostnames.C -c -o getAllHostnames.o

gdbLineNum.pl: setupGDBWrap.pl
	./setupGDBWrap.pl

clean:
	killP widgets/mongoose/mongoose
	killP widgets/shellinabox/bin/shellinaboxd
	rm -rf dbg *.o dbgLogTester${EXT} dbgLogGraphTester${EXT} dbgLogAttrTester${EXT} *.exe widgets/shellinabox* widgets/mongoose* widgets/graphviz* gdbLineNum.pl

widgets/shellinabox/bin/shellinaboxd${EXT}:
	rm -f widgets/shellinabox-2.14.tar.gz
	cd widgets; wget --no-check-certificate https://shellinabox.googlecode.com/files/shellinabox-2.14.tar.gz
	cd widgets; tar -xf shellinabox-2.14.tar.gz
	cd widgets/shellinabox-2.14; ./configure --prefix=${CURDIR}/widgets/shellinabox
	cd widgets/shellinabox-2.14; make
	cd widgets/shellinabox-2.14; make install
	rm -r widgets/shellinabox-2.14 widgets/shellinabox-2.14.tar.gz

widgets/mongoose/mongoose${EXT}:
	rm -f widgets/mongoose-3.8.tgz
	cd widgets; wget --no-check-certificate https://mongoose.googlecode.com/files/mongoose-3.8.tgz
	cd widgets; tar -xf mongoose-3.8.tgz
	cd widgets; rm mongoose-3.8.tgz
	cd widgets/mongoose; make ${OS_MONGOOSE}

widgets/graphviz/bin/dot${EXT}:
ifeq (${OS},Cygwin)
	rm -rf widgets/graphviz-2.32.zip widgets/release
	cd widgets; wget http://www.graphviz.org/pub/graphviz/stable/windows/graphviz-2.32.zip
	cd widgets; unzip graphviz-2.32.zip
	mkdir widgets/graphviz
	mv widgets/release/* widgets/graphviz
	rm -r widgets/graphviz-2.32.zip widgets/release
	chmod 755 widgets/graphviz/bin/*
else
	cd widgets; wget http://www.graphviz.org/pub/graphviz/stable/SOURCES/graphviz-2.32.0.tar.gz
	cd widgets; tar -xf graphviz-2.32.0.tar.gz
	cd widgets/graphviz-2.32.0; export CC=gcc; export CXX=g++; ./configure --prefix=${CURDIR}/widgets/graphviz --disable-swig --disable-sharp --disable-go --disable-io --disable-java --disable-lua --disable-ocaml --disable-perl --disable-php --disable-python --disable-r --disable-ruby --disable-tcl --without-pic --without-efence --without-expat --without-devil --without-webp --without-poppler --without-ghostscript --without-visio --without-pangocairo --without-lasi --without-glitz --without-freetype2 --without-fontconfig --without-gdk-pixbuf --without-gtk --without-gtkgl --without-gtkglext --without-gts --without-glade --without-ming --without-qt --without-quartz --without-gdiplus --without-libgd --without-glut --without-sfdp --without-smyrna --without-ortho --without-digcola --without-ipsepcola --enable-static --disable-shared
	cd widgets/graphviz-2.32.0; make
	cd widgets/graphviz-2.32.0; make install
	rm -r widgets/graphviz-2.32.0 widgets/graphviz-2.32.0.tar.gz
endif

script/taffydb:
	cd script; wget --no-check-certificate https://github.com/typicaljoe/taffydb/archive/master.zip
	cd script; unzip master.zip
	mv script/taffydb-master script/taffydb
	chmod 755 script/taffydb
	chmod 644 script/taffydb/*
