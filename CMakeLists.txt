cmake_minimum_required (VERSION 2.6)
project (SAXSVIEW C CXX Fortran)

set (CMAKE_VERBOSE_MAKEFILE  OFF)
set (CMAKE_BUILD_TYPE        DEBUG)

set (CMAKE_MODULE_PATH       ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include (packaging)

if (WIN32)
  add_definitions (-DQT_DLL)

  set (MINGW_LINKER_FLAGS "-Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc -Wl,--allow-multiple-definition")
  set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${MINGW_LINKER_FLAGS}")
  set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${MINGW_LINKER_FLAGS}")
  set (CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} ${MINGW_LINKER_FLAGS}")
endif(WIN32)

find_package (Qt4 4.4.0)
set (QT_USE_QTSVG TRUE)
include (${QT_USE_FILE})


add_subdirectory (external/qwt)
add_subdirectory (external/cbflib)
add_subdirectory (external/edfpack)
add_subdirectory (libsaxsdocument)
add_subdirectory (libsaxsimage)
add_subdirectory (libsaxsview)
add_subdirectory (saxsview)

ENABLE_TESTING()
add_subdirectory (testsuite)
