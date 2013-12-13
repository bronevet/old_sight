OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXE := .exe
endif

sight_H := ../*.h ../*/*.h ../widgets/*/*.h
TESTERS = 8.Modules${EXE} 
#demo${EXE} \
#          1.StructuredFormatting${EXE} 2.ConditionalFormatting${EXE} 3.Navigation${EXE} \
#          4.AttributeAnnotationFiltering${EXE} 5.Tracing${EXE} 6.PerfAnalysis${EXE} \
#          7.Merging${EXE} 8.Modules${EXE}

all: ${TESTERS}

run: ${TESTERS}
	./1.StructuredFormatting${EXE};         
	./2.ConditionalFormatting${EXE};        
	./3.Navigation${EXE};                   
	./4.AttributeAnnotationFiltering${EXE}; 
	export SIGHT_FILE_OUT=1; ./4.AttributeAnnotationFiltering${EXE} 0 20; ./4.AttributeAnnotationFiltering${EXE} 1 20; ./4.AttributeAnnotationFiltering${EXE} 2 20; ./4.AttributeAnnotationFiltering${EXE} 3 20
	export SIGHT_FILE_OUT=1; rm -rf dbg.4.AttributeAnnotationFiltering; ../hier_merge dbg.4.AttributeAnnotationFiltering zipper dbg.4.AttributeAnnotationFiltering.client_0.numIters_20/structure dbg.4.AttributeAnnotationFiltering.client_1.numIters_20/structure dbg.4.AttributeAnnotationFiltering.client_2.numIters_20/structure dbg.4.AttributeAnnotationFiltering.client_3.numIters_20/structure
	rm -rf dbg.4.AttributeAnnotationFiltering.*;
	../slayout${EXE} dbg.4.AttributeAnnotationFiltering/structure;	
	./5.Tracing${EXE};
	./6.PerfAnalysis${EXE};
	export SIGHT_FILE_OUT=1; ./7.Merging${EXE} 1; ./7.Merging${EXE} 2; ./7.Merging${EXE} 3; ./7.Merging${EXE} 4; ./7.Merging${EXE} 5; ./7.Merging${EXE} 6; 
	export SIGHT_FILE_OUT=1; rm -rf dbg.7.Merging; ../hier_merge${EXE} dbg.7.Merging common dbg.7.Merging.numIters_1/structure dbg.7.Merging.numIters_2/structure dbg.7.Merging.numIters_3/structure dbg.7.Merging.numIters_4/structure dbg.7.Merging.numIters_5/structure dbg.7.Merging.numIters_6/structure; 
	rm -rf dbg.7.Merging.numIters*;
	../slayout${EXE} dbg.7.Merging/structure;	
	export SIGHT_FILE_OUT=1; ./8.Modules${EXE} 1 10; ./8.Modules${EXE} 1 30; ./8.Modules${EXE} 1 100; ./8.Modules${EXE} 2 10; ./8.Modules${EXE} 2 30; ./8.Modules${EXE} 2 100; ./8.Modules${EXE} 3 10; ./8.Modules${EXE} 3 30; ./8.Modules${EXE} 3 100;
	export SIGHT_FILE_OUT=1; rm -rf dbg.8.Modules; ../hier_merge dbg.8.Modules common dbg.8.Modules.1_dims.10_particles/structure dbg.8.Modules.1_dims.30_particles/structure dbg.8.Modules.1_dims.100_particles/structure dbg.8.Modules.2_dims.10_particles/structure dbg.8.Modules.2_dims.30_particles/structure dbg.8.Modules.2_dims.100_particles/structure dbg.8.Modules.3_dims.10_particles/structure dbg.8.Modules.3_dims.30_particles/structure dbg.8.Modules.3_dims.100_particles/structure
	rm -rf dbg.8.Modules.*;
	../slayout${EXE} dbg.8.Modules/structure;	

demo${EXE}: demo.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} -DROOT_PATH="\"${ROOT_PATH}\"" demo.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o demo${EXE}

1.StructuredFormatting${EXE}: 1.StructuredFormatting.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} 1.StructuredFormatting.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 1.StructuredFormatting${EXE}

2.ConditionalFormatting${EXE}: 2.ConditionalFormatting.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} 2.ConditionalFormatting.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 2.ConditionalFormatting${EXE}

3.Navigation${EXE}: 3.Navigation.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} 3.Navigation.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 3.Navigation${EXE}

4.AttributeAnnotationFiltering${EXE}: 4.AttributeAnnotationFiltering.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} 4.AttributeAnnotationFiltering.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 4.AttributeAnnotationFiltering${EXE}

5.Tracing${EXE}: 5.Tracing.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} 5.Tracing.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 5.Tracing${EXE}

6.PerfAnalysis${EXE}: 6.PerfAnalysis.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} 6.PerfAnalysis.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 6.PerfAnalysis${EXE}

7.Merging${EXE}: 7.Merging.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} 7.Merging.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 7.Merging${EXE}

8.Modules${EXE}: 8.Modules.C ../libsight_structure.a ${sight_H}
	g++ ${SIGHT_CFLAGS} 8.Modules.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 8.Modules${EXE}

clean:
	rm -rf ${TESTERS} dbg.*
