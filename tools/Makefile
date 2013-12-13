all: dtl callpath

dtl-1.17.tar.gz:
	wget --no-check-certificate https://dtl-cpp.googlecode.com/files/dtl-1.17.tar.gz

dtl: dtl-1.17.tar.gz
	rm -fr dtl-1.17
	tar -xf dtl-1.17.tar.gz 
	mv dtl-1.17 dtl

boost_1_55_0.tar:
	wget http://downloads.sourceforge.net/project/boost/boost/1.55.0/boost_1_55_0.tar.bz2
	rm -f boost_1_55_0.tar
	bunzip2 boost_1_55_0.tar.bz2

boost_1_55_0: boost_1_55_0.tar
	tar -xf boost_1_55_0.tar

adept-utils.zip:
	wget --no-check-certificate https://github.com/tgamblin/adept-utils/archive/master.zip
	rm -fr adept-utils
	mv master.zip adept-utils.zip; unzip master.zip

adept-utils: boost_1_55_0 adept-utils.zip
	rm -fr adept-utils-master
	unzip adept-utils.zip
	mv adept-utils-master adept-utils
#	cp adept-utils-makefiles/CMakeLists.root.txt   adept-utils
#	cp adept-utils-makefiles/CMakeLists.cutils.txt adept-utils/cutils/CMakeLists.txt
#	cp adept-utils-makefiles/CMakeLists.utils.txt  adept-utils/utils/CMakeLists.txt
	cd adept-utils; cmake -DCMAKE_INSTALL_PREFIX=${ROOT_PATH}/tools/adept-utils \
	                      -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
	                      -DBOOST_ROOT:PATHNAME=${ROOT_PATH}/tools/boost_1_55_0 .
	cd adept-utils; make; make install
	
callpath.zip:
	wget --no-check-certificate https://github.com/tgamblin/callpath/archive/master.zip
	mv master.zip callpath.zip
	
callpath: callpath.zip
	rm -rf callpath-master
	unzip callpath.zip
	mv callpath-master callpath
	cd callpath/src; cmake -DCMAKE_INSTALL_PREFIX=${ROOT_PATH}/tools/callpath \
	                       -DCALLPATH_WALKER=backtrace -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	                       -DCMAKE_CXX_FLAGS=-fpermissive \
	                       -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
	                       -Dadept_utils_DIR=${ROOT_PATH}/tools/adept-utils/share/cmake/adept_utils ..
	cd callpath/src; make; make install

#-DCALLPATH_HAVE_MPI=false 

clean:
	rm -r *.tar *.tar.gz *.tar.bz2 dtl adept-utils callpath boost_1_55_0