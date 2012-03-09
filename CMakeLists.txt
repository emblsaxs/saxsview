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

  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif(WIN32)

find_package (Qt4 4.4.0)
set (QT_USE_QTSVG TRUE)
include (${QT_USE_FILE})

configure_file (config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${SAXSVIEW_BINARY_DIR})

add_subdirectory (external/qwt)
add_subdirectory (external/qtsolutions)
add_subdirectory (external/cbflib)
add_subdirectory (external/edfpack)
add_subdirectory (libsaxsdocument)
add_subdirectory (libsaxsimage)
add_subdirectory (libsaxsview)
add_subdirectory (saxsview)
add_subdirectory (freedesktop.org)

ENABLE_TESTING()
add_subdirectory (testsuite)
