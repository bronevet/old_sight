OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXE := .exe
endif

DBGLOG_H := ../*.h ../widgets/*.h
TESTERS = 1.StructuredFormatting${EXE} 2.ConditionalFormatting${EXE} 3.Navigation${EXE} \
          4.AttributeAnnotationFiltering${EXE} 5.Tracing${EXE} 6.PerfAnalysis${EXE}

all: ${TESTERS}

run:
	rm -fr dbg; ./1.StructuredFormatting${EXE};         mv dbg dbg.1.StructuredFormatting        ; sleep 1
	rm -fr dbg; ./2.ConditionalFormatting${EXE};        mv dbg dbg.2.ConditionalFormatting       ; sleep 1
	rm -fr dbg; ./3.Navigation${EXE};                   mv dbg dbg.3.Navigation                  ; sleep 1
	rm -fr dbg; ./4.AttributeAnnotationFiltering${EXE}; mv dbg dbg.4.AttributeAnnotationFiltering; sleep 1
	rm -fr dbg; ./5.Tracing${EXE};                      mv dbg dbg.5.Tracing                     ; sleep 1
	rm -fr dbg; ./6.PerfAnalysis${EXE};                 mv dbg dbg.6.PerfAnalysis                ; sleep 1

1.StructuredFormatting${EXE}: 1.StructuredFormatting.C ../libdbglog.a ${DBGLOG_H}
	g++ -g 1.StructuredFormatting.C -I.. -I../widgets -L.. -ldbglog -o 1.StructuredFormatting${EXE}

2.ConditionalFormatting${EXE}: 2.ConditionalFormatting.C ../libdbglog.a ${DBGLOG_H}
	g++ -g 2.ConditionalFormatting.C -I.. -I../widgets -L.. -ldbglog -o 2.ConditionalFormatting${EXE}

3.Navigation${EXE}: 3.Navigation.C ../libdbglog.a ${DBGLOG_H}
	g++ -g 3.Navigation.C -I.. -I../widgets -L.. -ldbglog -o 3.Navigation${EXE}

4.AttributeAnnotationFiltering${EXE}: 4.AttributeAnnotationFiltering.C ../libdbglog.a ${DBGLOG_H}
	g++ -g 4.AttributeAnnotationFiltering.C -I.. -I../widgets -L.. -ldbglog  -o 4.AttributeAnnotationFiltering${EXE}

5.Tracing${EXE}: 5.Tracing.C ../libdbglog.a ${DBGLOG_H}
	g++ -g 5.Tracing.C -I.. -I../widgets -L.. -ldbglog  -o 5.Tracing${EXE}

6.PerfAnalysis${EXE}: 6.PerfAnalysis.C ../libdbglog.a ${DBGLOG_H}
	g++ -g 6.PerfAnalysis.C -I.. -I../widgets -L.. -ldbglog  -o 6.PerfAnalysis${EXE}

clean:
	rm -rf ${TESTERS}
