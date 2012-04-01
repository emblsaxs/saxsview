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
  set (SOURCES ${SOURCES} edf.c)
endif (TARGET edf)

if (TIFF_FOUND)
  add_definitions (-DHAVE_TIFF)
  set (SOURCES ${SOURCES} tiff.c)
endif (TIFF_FOUND)

add_library (saxsimage SHARED ${SOURCES} ${HEADERS})
set_target_properties (saxsimage PROPERTIES
                       VERSION 1)

include_directories(${CBFLIB_SOURCE_DIR})
target_link_libraries (saxsimage saxsdocument cbf)

# conditional include directories and libraries
if (TIFF_FOUND)
  include_directories (${TIFF_INCLUDE_DIR})
  target_link_libraries (saxsimage ${TIFF_LIBRARIES})
endif (TIFF_FOUND)

if (TARGET edf)
  include_directories (${EDFPACK_SOURCE_DIR})
  target_link_libraries (saxsimage edf)
endif (TARGET edf)

# Fortran bindings for libsaxsimage
add_library (fsaxsimage STATIC saxsimage.f90)
target_link_libraries (fsaxsimage saxsimage)


install (TARGETS saxsimage
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (TARGETS fsaxsimage
         ARCHIVE DESTINATION lib)
