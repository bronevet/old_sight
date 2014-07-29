OS := $(shell uname -o)
ifeq (${OS}, Cygwin)
EXE := .exe
endif

sight_H := ../*.h ../*/*.h ../widgets/*/*.h
TESTERS = 8.Modules${EXE} 9.CompModules.merged${EXE} 9.CompModules.single${EXE} 5.Tracing${EXE} 0.Demo${EXE} 11.ExternTraceProcess${EXE} 10.SpringModules${EXE} \
          1.StructuredFormatting${EXE} 2.ConditionalFormatting${EXE} 3.Navigation${EXE} \
          4.AttributeAnnotationFiltering${EXE} 6.PerfAnalysis${EXE} \
          7.Merging${EXE} 

all: ${TESTERS}

run: ${TESTERS}
	# 0.Demo
	export SIGHT_FILE_OUT=1; ./0.Demo${EXE} 5; ./0.Demo${EXE} 6; ./0.Demo${EXE} 7
	export SIGHT_FILE_OUT=1; rm -rf dbg.0.Demo; ../hier_merge dbg.0.Demo common dbg.0.Demo.maxDepth_5/structure dbg.0.Demo.maxDepth_6/structure dbg.0.Demo.maxDepth_7/structure
	rm -rf dbg.0.Demo.*;
	../slayout${EXE} dbg.0.Demo/structure;
	# 1.StructuredFormatting
	./1.StructuredFormatting${EXE};
	# 2.ConditionalFormatting
	./2.ConditionalFormatting${EXE};
	# 3.Navigation
	./3.Navigation${EXE};
	# 4.AttributeAnnotationFiltering        
	./4.AttributeAnnotationFiltering${EXE}; 
	export SIGHT_FILE_OUT=1; ./4.AttributeAnnotationFiltering${EXE} 0 20; ./4.AttributeAnnotationFiltering${EXE} 1 20; ./4.AttributeAnnotationFiltering${EXE} 2 20; ./4.AttributeAnnotationFiltering${EXE} 3 20
	export SIGHT_FILE_OUT=1; rm -rf dbg.4.AttributeAnnotationFiltering; ../hier_merge dbg.4.AttributeAnnotationFiltering zipper dbg.4.AttributeAnnotationFiltering.client_0.numIters_20/structure dbg.4.AttributeAnnotationFiltering.client_1.numIters_20/structure dbg.4.AttributeAnnotationFiltering.client_2.numIters_20/structure dbg.4.AttributeAnnotationFiltering.client_3.numIters_20/structure
	rm -rf dbg.4.AttributeAnnotationFiltering.*;
	../slayout${EXE} dbg.4.AttributeAnnotationFiltering/structure;
	# 5.Tracing
	./5.Tracing${EXE};
	# 6. PerfAnalysis
	./6.PerfAnalysis${EXE};
	# 7.Merging
	export SIGHT_FILE_OUT=1; rm -rf dbg.7.Merging*; ./7.Merging${EXE} 1; ./7.Merging${EXE} 2; ./7.Merging${EXE} 3; ./7.Merging${EXE} 4; ./7.Merging${EXE} 5; ./7.Merging${EXE} 6; 
	export SIGHT_FILE_OUT=1; ../hier_merge${EXE} dbg.7.Merging common dbg.7.Merging.*/structure
	rm -rf dbg.7.Merging.*;
	../slayout${EXE} dbg.7.Merging/structure;
	# 8.Modules
	export SIGHT_FILE_OUT=1; rm -rf dbg.8.Modules*; ./8.Modules${EXE} 1 10; ./8.Modules${EXE} 1 30; ./8.Modules${EXE} 1 100; ./8.Modules${EXE} 2 10; ./8.Modules${EXE} 2 30; ./8.Modules${EXE} 2 100; ./8.Modules${EXE} 3 10; ./8.Modules${EXE} 3 30; ./8.Modules${EXE} 3 100;
	export SIGHT_FILE_OUT=1; ../hier_merge dbg.8.Modules common dbg.8.Modules.*/structure
	rm -rf dbg.8.Modules.*;
	../slayout${EXE} dbg.8.Modules/structure;
	# 9.CompModules
	export SIGHT_FILE_OUT=1; ./9.CompModules.merged${EXE} 1 .025 .025 .0001 .05; ./9.CompModules.merged${EXE} 0 .025 .025 .0005 .05; ./9.CompModules.merged${EXE} 0 .025 .025 .001 .05; ./9.CompModules.merged${EXE} 0 .025 .025 .005 .05;
	export SIGHT_FILE_OUT=1; ./9.CompModules.merged${EXE} 0 .05  .05  .0001 .05; ./9.CompModules.merged${EXE} 0 .05  .05  .0005 .05; ./9.CompModules.merged${EXE} 0 .05  .05  .001 .05; ./9.CompModules.merged${EXE} 0 .05  .05  .005 .05;
	export SIGHT_FILE_OUT=1; ./9.CompModules.merged${EXE} 0 .1   .1   .0001 .05; ./9.CompModules.merged${EXE} 0 .1   .1   .0005 .05; ./9.CompModules.merged${EXE} 0 .1   .1   .001 .05; ./9.CompModules.merged${EXE} 0 .1   .1   .005 .05;
	export SIGHT_FILE_OUT=1; ../hier_merge dbg.9.CompModules zipper dbg.9.CompModules.*/structure
	rm -rf dbg.9.CompModules.*;
	../slayout${EXE} dbg.9.CompModules/structure;
	./9.CompModules.single${EXE} 0 .1 .1 .05 .01
	# 10.SpringModules
	rm -rf dbg.10.SpringModules*; 
	export SIGHT_FILE_OUT=1; export SPRING_BUF_SIZE=0;       ./10.SpringModules${EXE} 0 .1 .1 .01 .05; 
	export SIGHT_FILE_OUT=1; export SPRING_BUF_SIZE=1024;    ./10.SpringModules${EXE} 0 .1 .1 .01 .05; ./10.SpringModules${EXE} 1 .1 .1 .01 .05; ./10.SpringModules${EXE} 2 .1 .1 .01 .05; ./10.SpringModules${EXE} 3 .1 .1 .01 .05;  ./10.SpringModules${EXE} 4 .1 .1 .01 .05;
	export SIGHT_FILE_OUT=1; export SPRING_BUF_SIZE=8096;    ./10.SpringModules${EXE} 0 .1 .1 .01 .05; ./10.SpringModules${EXE} 1 .1 .1 .01 .05; ./10.SpringModules${EXE} 2 .1 .1 .01 .05; ./10.SpringModules${EXE} 3 .1 .1 .01 .05;  ./10.SpringModules${EXE} 4 .1 .1 .01 .05;
	export SIGHT_FILE_OUT=1; export SPRING_BUF_SIZE=524288;  ./10.SpringModules${EXE} 0 .1 .1 .01 .05; ./10.SpringModules${EXE} 1 .1 .1 .01 .05; ./10.SpringModules${EXE} 2 .1 .1 .01 .05; ./10.SpringModules${EXE} 3 .1 .1 .01 .05;  ./10.SpringModules${EXE} 4 .1 .1 .01 .05;
#	export SIGHT_FILE_OUT=1; export SPRING_BUF_SIZE=4194304; ./10.SpringModules${EXE} 0 .1 .1 .01 .05; ./10.SpringModules${EXE} 1 .1 .1 .01 .05; ./10.SpringModules${EXE} 2 .1 .1 .01 .05; ./10.SpringModules${EXE} 3 .1 .1 .01 .05;  ./10.SpringModules${EXE} 4 .1 .1 .01 .05;
	export SIGHT_FILE_OUT=1; ../hier_merge dbg.10.SpringModules zipper dbg.10.SpringModules.*/structure
	rm -rf dbg.10.SpringModules.*;
	../slayout${EXE} dbg.10.SpringModules/structure;
	# 11.ExternTraceProcess
	./11.ExternTraceProcess

0.Demo${EXE}: 0.Demo.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} -DROOT_PATH="\"${ROOT_PATH}\"" 0.Demo.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 0.Demo${EXE}

1.StructuredFormatting${EXE}: 1.StructuredFormatting.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 1.StructuredFormatting.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 1.StructuredFormatting${EXE}

2.ConditionalFormatting${EXE}: 2.ConditionalFormatting.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 2.ConditionalFormatting.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 2.ConditionalFormatting${EXE}

3.Navigation${EXE}: 3.Navigation.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 3.Navigation.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 3.Navigation${EXE}

4.AttributeAnnotationFiltering${EXE}: 4.AttributeAnnotationFiltering.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 4.AttributeAnnotationFiltering.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 4.AttributeAnnotationFiltering${EXE}

5.Tracing${EXE}: 5.Tracing.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 5.Tracing.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 5.Tracing${EXE}

6.PerfAnalysis${EXE}: 6.PerfAnalysis.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 6.PerfAnalysis.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 6.PerfAnalysis${EXE}

7.Merging${EXE}: 7.Merging.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 7.Merging.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 7.Merging${EXE}

8.Modules${EXE}: 8.Modules.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 8.Modules.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 8.Modules${EXE}

9.CompModules.single${EXE}: 9.CompModules.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 9.CompModules.C -DTRACE -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 9.CompModules.single${EXE}

9.CompModules.merged${EXE}: 9.CompModules.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 9.CompModules.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 9.CompModules.merged${EXE}

10.SpringModules${EXE}: 10.SpringModules.C ../libsight_structure.so ${sight_H}
	${CCC} ${SIGHT_CFLAGS} 10.SpringModules.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 10.SpringModules${EXE}

11.ExternTraceProcess${EXE}: 11.ExternTraceProcess.C ../libsight_structure.so ${sight_H}
	${CCC} -g 11.ExternTraceProcess.windowing.C -I.. -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 11.ExternTraceProcess.windowing${EXE}
	${CCC} -g ${SIGHT_CFLAGS} 11.ExternTraceProcess.C -I.. -I../widgets -L.. -lsight_structure ${SIGHT_LINKFLAGS} -o 11.ExternTraceProcess${EXE}

clean:
	rm -rf ${TESTERS} dbg.* *.core
