project (LIBSAXSIMAGE C Fortran)

find_package (TIFF QUIET)
if (NOT TIFF_FOUND)
  message (STATUS "Optional package libtiff not found, TIFF image format disabled.")
endif (NOT TIFF_FOUND)


set (SOURCES saxsimage.c
             saxsimage_format.c
             cbf.c
             msk.c)

set (HEADERS saxsimage.h
             saxsimage_format.h)

# conditional sources
if (TARGET edf)
  add_definitions (-DHAVE_EDF)
  include_directories (${EDFPACK_SOURCE_DIR})
  set (EDF_LIBRARIES edf)
  set (SOURCES ${SOURCES} edf.c)
endif (TARGET edf)

if (TIFF_FOUND)
  add_definitions (-DHAVE_TIFF)
  include_directories (${TIFF_INCLUDE_DIR})
  set (SOURCES ${SOURCES} tiff.c)
endif (TIFF_FOUND)

include_directories(${CBFLIB_SOURCE_DIR})

saxsview_add_library(saxsimage SOURCES ${HEADERS} ${SOURCES}
                               LIBRARIES saxsdocument cbf
                                         ${EDF_LIBRARIES} ${TIFF_LIBRARIES} 
                               VERSION 1
                               SHARED)

# Fortran bindings for libsaxsimage
saxsview_add_library(fsaxsimage SOURCES saxsimage.f90
                                LIBRARIES saxsimage
                                STATIC)

saxsview_install(TARGETS saxsimage)

