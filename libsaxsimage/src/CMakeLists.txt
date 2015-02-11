project (LIBSAXSIMAGE C)

# This is used for saxsproperty - is this absolutely necessary?
include_directories(${LIBSAXSDOCUMENT_SOURCE_DIR})

include_directories (${LIBTIFF_INCLUDE_DIR}
                     ${CBFLIB_SOURCE_DIR})

set (SOURCES saxsimage.c
             saxsimage_format.c
             cbf.c
             msk.c
             tiff.c)

set (HEADERS saxsimage.h
             saxsimage_format.h)

set (LIBRARIES saxsdocument cbf)


# We always have TIFF.
add_definitions (-DHAVE_TIFF)
set (LIBRARIES ${LIBRARIES} ${LIBTIFF_LIBRARIES})

# conditional sources
if (TARGET edf)
  add_definitions (-DHAVE_EDF)
  include_directories (${EDFPACK_SOURCE_DIR})
  set (LIBRARIES ${LIBRARIES} edf)
  set (SOURCES ${SOURCES} edf.c)
endif (TARGET edf)

find_package (HDF5)
if (HDF5_FOUND)
  add_definitions (-DHAVE_HDF5)
  include_directories (${HDF5_SOURCE_DIR})
  set (LIBRARIES ${LIBRARIES} ${HDF5_LIBRARIES})
  set (SOURCES ${SOURCES} hdf5.c)
endif (HDF5_FOUND)


add_shared_library (saxsimage
                    SOURCES ${HEADERS} ${SOURCES}
                    LIBRARIES ${LIBRARIES}
                    VERSION 1)

install_library (TARGETS saxsimage)
