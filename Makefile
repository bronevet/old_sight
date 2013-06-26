all: dbgLogTester dbgLogGraphTester

# Set this if graphviz is installed. If graphviz is on the path, set it to the empty string "\"\"".
# Otherwise, place the full path, inside the quotes.
DOT_PATH := "\"\""

dbgLogTester: dbgLogTester.C dbglog.o dbglog.h widgets.o widgets.h
	g++ -g dbgLogTester.C dbglog.o widgets.o -o dbgLogTester

dbgLogGraphTester: dbgLogGraphTester.C dbglog.o dbglog.h widgets.o widgets.h
	g++ -g dbgLogGraphTester.C dbglog.o widgets.o -o dbgLogGraphTester

dbglog.o: dbglog.C dbglog.h
	g++ -g dbglog.C -DROOT_PATH="\"${CURDIR}\"" -DDOT_PATH=${DOT_PATH} -c -o dbglog.o
	
widgets.o: widgets.C widgets.h
	g++ -g widgets.C -DROOT_PATH="\"${CURDIR}\"" -DDOT_PATH=${DOT_PATH} -c -o widgets.o
	
clean:
	rm -rf dbg *.o dbgLogTester *.exe
