PKG_INFO=${shell which pkg-config > /dev/null; echo $$?}

#check if lib ($1) is in system (non-empty) and return Cflags else return $2
define COND_INC
$(if $($(1)_str), $($(1)_cflags), $(2))
endef

define COND_LIBS
$(if $($(1)_str), $($(1)_L_PATH) $($(1)_lib_PATH), $(2))
endef

ifeq ($(PKG_INFO),0)
#PACKAGES=test_mpich test_boost test_papi test_libmsr test_gsl #adept-utils

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

#packages info for papi
PAPI_L_PATH=${shell pkg-config papi --silence-errors --libs-only-L}
PAPI_exist=${shell pkg-config papi --silence-errors --exists ; echo $$?}
PAPI_lib_PATH=${shell pkg-config papi --silence-errors --libs-only-l}
PAPI_cflags=${shell pkg-config papi --silence-errors --cflags}
PAPI_str=${shell pkg-config papi --modversion --silence-errors}

#packages info for gsl
GSL_L_PATH=${shell pkg-config gsl --silence-errors --libs-only-L}
GSL_exist=${shell pkg-config gsl --silence-errors --exists ; echo $$?}
GSL_lib_PATH=${shell pkg-config gsl --silence-errors --libs-only-l}
GSL_cflags=${shell pkg-config gsl --silence-errors --cflags}
GSL_str=${shell pkg-config gsl --modversion --silence-errors}

#packages info for libmsr
LIBMSR_L_PATH=${shell pkg-config libmsr --silence-errors --libs-only-L}
LIBMSR_exist=${shell pkg-config libmsr --silence-errors --exists ; echo $$?}
LIBMSR_lib_PATH=${shell pkg-config libmsr --silence-errors --libs-only-l}
LIBMSR_cflags=${shell pkg-config libmsr --silence-errors --cflags}
LIBMSR_str=${shell pkg-config libmsr --modversion --silence-errors}

#test functions
#pkg_info_check: ${PACKAGES}
	
#test functions
#test_mpich: 
	#@echo "$@"
	#cc ab.c -o a.out $(call COND_LIBS,MPICH,-L/home/usw/a.so)
	#@echo $(MPICH_exist)
	#@echo $(MPICH_L_PATH)
	#@echo $(MPICH_lib_PATH)
	#@echo $(MPICH_cflags)


#test_boost: 
	#@echo "$@"
	#cc ab.c -o a.out $(call COND_LIBS,BOOST,-L/home/usw/a.so)
	#@echo $(BOOST_exist)
	#@echo $(BOOST_L_PATH)
	#@echo $(BOOST_lib_PATH)
	#@echo $(BOOST_cflags)

#test_papi: 
	#@echo "$@"
	#cc ab.c -o a.out $(call COND_LIBS,PAPI,-L/home/usw/papi.so)
	#@echo $(PAPI_exist)
	#@echo $(PAPI_L_PATH)
	#@echo $(PAPI_lib_PATH)
	#@echo $(PAPI_cflags)

#test_gsl: 
	#@echo "$@"
	#cc ab.c -o a.out $(call COND_LIBS,GSL,-L/home/usw/gsl.so)
	#@echo $(GSL_exist)
	#@echo $(GSL_L_PATH)
	#@echo $(GSL_lib_PATH)
	#@echo $(GSL_cflags)

#test_libmsr: 
	#@echo "$@"
	#cc ab.c -o a.out $(call COND_LIBS,LIBMSR,-L/home/usw/libmsr.so)
	#@echo $(LIBMSR_exist)
	#@echo $(LIBMSR_L_PATH)
	#@echo $(LIBMSR_lib_PATH)
	#@echo $(LIBMSR_cflags)

else
#default to non exist(1) when pkg-info not avaialble
MPICH_exist=1
BOOST_exist=1
GSL_exist=1
PAPI_exist=1
LIBMSR_exist=1

pkg_info_check:
	@echo "ERROR! pkg-config command not found.."

endif







