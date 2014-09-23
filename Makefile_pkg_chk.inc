PKG_INFO=${shell which pkg-config > /dev/null; echo $$?}

ifeq ($(PKG_INFO),0)
PACKAGES=mpich boost #adept-utils

#packages info for mpich
MPICH_L_PATH=${shell pkg-config mpich --silence-errors --libs-only-L}
MPICH_exist=${shell pkg-config mpich --silence-errors --exists ; echo $$?}
MPICH_lib_PATH=${shell pkg-config mpich --silence-errors --libs-only-l}
MPICH_cflags=${shell pkg-config mpich --silence-errors --cflags}


#packages info for boost
BOOST_L_PATH=${shell pkg-config boost --silence-errors --libs-only-L}
BOOST_exist=${shell pkg-config boost --silence-errors --exists ; echo $$?}
BOOST_lib_PATH=${shell pkg-config boost --silence-errors --libs-only-l}
BOOST_cflags=${shell pkg-config boost --silence-errors --cflags}

pkg_info_check: ${PACKAGES}
	
mpich: 
	#@echo "$@"
	#@echo $(PKG_INFO)
	@echo $(MPICH_exist)
	@echo $(MPICH_L_PATH)
	@echo $(MPICH_lib_PATH)
	@echo $(MPICH_cflags)


boost: 
	#@echo "$@"
	@echo $(BOOST_exist)
	@echo $(BOOST_L_PATH)
	@echo $(BOOST_lib_PATH)
	@echo $(BOOST_cflags)
else
pkg_info_check:
	@echo "ERROR! pkg-config command not found.."

endif







