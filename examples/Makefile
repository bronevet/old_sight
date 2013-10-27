OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXE := .exe
endif

sight_H := ../*.h ../widgets/*.h
TESTERS = 1.StructuredFormatting${EXE} 2.ConditionalFormatting${EXE} 3.Navigation${EXE} \
          4.AttributeAnnotationFiltering${EXE} 5.Tracing${EXE} 6.PerfAnalysis${EXE} 

all: ${TESTERS}

run: ${TESTERS}
	./1.StructuredFormatting${EXE};         
	./2.ConditionalFormatting${EXE};        
	./3.Navigation${EXE};                   
	./4.AttributeAnnotationFiltering${EXE}; 
	./5.Tracing${EXE};                      
	./6.PerfAnalysis${EXE};                 

1.StructuredFormatting${EXE}: 1.StructuredFormatting.C ../libsight_structure.a ${sight_H}
	g++ -g 1.StructuredFormatting.C -I.. -I../widgets -L.. -lsight_structure -o 1.StructuredFormatting${EXE}

2.ConditionalFormatting${EXE}: 2.ConditionalFormatting.C ../libsight_structure.a ${sight_H}
	g++ -g 2.ConditionalFormatting.C -I.. -I../widgets -L.. -lsight_structure -o 2.ConditionalFormatting${EXE}

3.Navigation${EXE}: 3.Navigation.C ../libsight_structure.a ${sight_H}
	g++ -g 3.Navigation.C -I.. -I../widgets -L.. -lsight_structure -o 3.Navigation${EXE}

4.AttributeAnnotationFiltering${EXE}: 4.AttributeAnnotationFiltering.C ../libsight_structure.a ${sight_H}
	g++ -g 4.AttributeAnnotationFiltering.C -I.. -I../widgets -L.. -lsight_structure  -o 4.AttributeAnnotationFiltering${EXE}

5.Tracing${EXE}: 5.Tracing.C ../libsight_structure.a ${sight_H}
	g++ -g 5.Tracing.C -I.. -I../widgets -L.. -lsight_structure  -o 5.Tracing${EXE}

6.PerfAnalysis${EXE}: 6.PerfAnalysis.C ../libsight_structure.a ${sight_H}
	g++ -g 6.PerfAnalysis.C -I.. -I../widgets -L.. -lsight_structure  -o 6.PerfAnalysis${EXE}

clean:
	rm -rf ${TESTERS} dbg.*
