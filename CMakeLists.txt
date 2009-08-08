cmake_minimum_required (VERSION 2.6)
project (SAXSVIEW C CXX Fortran)

set (CMAKE_VERBOSE_MAKEFILE  OFF)
set (CMAKE_BUILD_TYPE        DEBUG)

find_package (Qt4 4.4.0)
set (QT_USE_QTSVG TRUE)
include (${QT_USE_FILE})

add_subdirectory (external/qwt)
add_subdirectory (libsaxsdocument)
add_subdirectory (libsaxsview)
add_subdirectory (saxsview)

ENABLE_TESTING()
add_subdirectory (testsuite)
