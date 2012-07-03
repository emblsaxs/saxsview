cmake_minimum_required (VERSION 2.6)
project (SAXSVIEW C CXX Fortran)

set (CMAKE_VERBOSE_MAKEFILE         OFF)
set (CMAKE_BUILD_TYPE               DEBUG)

set (CMAKE_MODULE_PATH              ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include (flags)
include (macros)
include (install)
include (packaging)

find_package (Qt4 4.4.0)
set (QT_USE_QTSVG TRUE)
include (${QT_USE_FILE})

configure_file (config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${SAXSVIEW_BINARY_DIR})

add_subdirectory (admin)
add_subdirectory (external/qwt)
add_subdirectory (external/qtsolutions)
add_subdirectory (external/cbflib)
add_subdirectory (external/edfpack)
add_subdirectory (external/muparser)
add_subdirectory (libsaxsdocument)
add_subdirectory (libsaxsimage)
add_subdirectory (libsaxsview)
add_subdirectory (saxsview)

ENABLE_TESTING()
add_subdirectory (testsuite)
