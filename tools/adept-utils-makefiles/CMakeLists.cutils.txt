#
# C utility library -- this is in straight C for greater accessibility
#
add_library(adept_cutils link_utils.c)
use_c99_for_target(adept_cutils)

install(TARGETS adept_cutils DESTINATION lib EXPORT adept_utils-libs)
install(FILES link_utils.h DESTINATION include)

add_executable(test test.c)
target_link_libraries(test adept_cutils dl)

if (CMAKE_SYSTEM_NAME STREQUAL BlueGeneL)
  include_directories(/bgl/BlueLight/ppcfloor/bglsys/include)
endif()

include_directories(${PROJECT_BINARY_DIR})
