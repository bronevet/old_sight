all: ${LLVM32_BUILD_PATH}/lib/Transforms/kulfi/Release/faults.d

${LLVM32_BUILD_PATH}/lib/Transforms/kulfi/Release/faults.d: faults.cpp Makefile.LLVM
	mkdir -p ${LLVM32_SRC_PATH}/lib/Transforms/kulfi
	cp Makefile.LLVM ${LLVM32_SRC_PATH}/lib/Transforms/kulfi
	cp faults.cpp ${LLVM32_SRC_PATH}/lib/Transforms/kulfi
	cd ${LLVM32_BUILD_PATH}; ${MAKE} -f Makefile
