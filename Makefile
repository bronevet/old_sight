all: dbgLogTester

# Set this if graphviz is installed. If graphviz is on the path, set it to the empty string "\"\"".
# Otherwise, place the full path, inside the quotes.
DOT_PATH := "\"\""

dbgLogTester: dbgLogTester.C dbglog.o dbglog.h
	g++ dbgLogTester.C dbglog.o -o dbgLogTester

dbglog.o: dbglog.C dbglog.h
	g++ dbglog.C -DROOT_PATH="\"${CURDIR}\"" -DDOT_PATH=${DOT_PATH} -c -o dbglog.o
	
clean:
	rm -rf dbg *.o dbgLogTester