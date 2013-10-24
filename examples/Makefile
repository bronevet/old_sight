OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXE := .exe
endif

DBGLOG_H := ../*.h ../widgets/*.h
TESTERS = 1.StructuredFormatting${EXE} 2.ConditionalFormatting${EXE} 3.Navigation${EXE} \
          4.AttributeAnnotationFiltering${EXE} 5.Tracing${EXE} 6.PerfAnalysis${EXE} 

all: ${TESTERS}

run:
	./1.StructuredFormatting${EXE};         ../slayout${EXE} dbg.1.StructuredFormatting/structure
	./2.ConditionalFormatting${EXE};        ../slayout${EXE} dbg.2.ConditionalFormatting/structure
	./3.Navigation${EXE};                   ../slayout${EXE} dbg.3.Navigation/structure
	./4.AttributeAnnotationFiltering${EXE}; ../slayout${EXE} dbg.4.AttributeAnnotationFiltering/structure
	./5.Tracing${EXE};                      ../slayout${EXE} dbg.5.Tracing/structure
	./6.PerfAnalysis${EXE};                 ../slayout${EXE} dbg.6.PerfAnalysis/structure

1.StructuredFormatting${EXE}: 1.StructuredFormatting.C ../libdbglog_structure.a ${DBGLOG_H}
	g++ -g 1.StructuredFormatting.C -I.. -I../widgets -L.. -ldbglog_structure -o 1.StructuredFormatting${EXE}

2.ConditionalFormatting${EXE}: 2.ConditionalFormatting.C ../libdbglog_structure.a ${DBGLOG_H}
	g++ -g 2.ConditionalFormatting.C -I.. -I../widgets -L.. -ldbglog_structure -o 2.ConditionalFormatting${EXE}

3.Navigation${EXE}: 3.Navigation.C ../libdbglog_structure.a ${DBGLOG_H}
	g++ -g 3.Navigation.C -I.. -I../widgets -L.. -ldbglog_structure -o 3.Navigation${EXE}

4.AttributeAnnotationFiltering${EXE}: 4.AttributeAnnotationFiltering.C ../libdbglog_structure.a ${DBGLOG_H}
	g++ -g 4.AttributeAnnotationFiltering.C -I.. -I../widgets -L.. -ldbglog_structure  -o 4.AttributeAnnotationFiltering${EXE}

5.Tracing${EXE}: 5.Tracing.C ../libdbglog_structure.a ${DBGLOG_H}
	g++ -g 5.Tracing.C -I.. -I../widgets -L.. -ldbglog_structure  -o 5.Tracing${EXE}

6.PerfAnalysis${EXE}: 6.PerfAnalysis.C ../libdbglog_structure.a ${DBGLOG_H}
	g++ -g 6.PerfAnalysis.C -I.. -I../widgets -L.. -ldbglog_structure  -o 6.PerfAnalysis${EXE}

clean:
	rm -rf ${TESTERS} dbg.*
