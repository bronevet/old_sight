PKG_INFO=${shell which pkg-config > /dev/null; echo $$?}

#check if lib ($1) is in system (non-empty) and return Cflags else return $2
define COND_INC
$(if $($(1)_str), $($(1)_cflags), $(2))
endef

define COND_LIBS
$(if $($(1)_str), $($(1)_L_PATH) $($(1)_lib_PATH), $(2))
endef

ifeq ($(PKG_INFO),0)
PACKAGES=mpich boost #adept-utils

#packages info for mpich
MPICH_L_PATH=${shell pkg-config mpich --silence-errors --libs-only-L}
MPICH_exist=${shell pkg-config mpich --silence-errors --exists ; echo $$?}
MPICH_lib_PATH=${shell pkg-config mpich --silence-errors --libs-only-l}
MPICH_cflags=${shell pkg-config mpich --silence-errors --cflags}
MPICH_str=${shell pkg-config mpich --modversion --silence-errors}

#packages info for boost
BOOST_L_PATH=${shell pkg-config boost --silence-errors --libs-only-L}
BOOST_exist=${shell pkg-config boost --silence-errors --exists ; echo $$?}
BOOST_lib_PATH=${shell pkg-config boost --silence-errors --libs-only-l}
BOOST_cflags=${shell pkg-config boost --silence-errors --cflags}
BOOST_str=${shell pkg-config boost --modversion --silence-errors}

#test functions
pkg_info_check: ${PACKAGES}
	
#test functions
mpich: 
	#@echo "$@"
	#cc ab.c -o a.out $(call COND_LIBS,MPICH,-L/home/usw/a.so)
	@echo $(MPICH_exist)
	@echo $(MPICH_L_PATH)
	@echo $(MPICH_lib_PATH)
	@echo $(MPICH_cflags)


boost: 
	#@echo "$@"
	#cc ab.c -o a.out $(call COND_LIBS,BOOST,-L/home/usw/a.so)
	@echo $(BOOST_exist)
	@echo $(BOOST_L_PATH)
	@echo $(BOOST_lib_PATH)
	@echo $(BOOST_cflags)
else
#default to non exist(1) when pkg-info not avaialble
MPICH_exist=1
BOOST_exist=1

pkg_info_check:
	@echo "ERROR! pkg-config command not found.."

endif







