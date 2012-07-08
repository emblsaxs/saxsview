project (LIBSAXSIMAGE C)

# This is used for saxsproperty - is this absolutely necessary?
include_directories(${LIBSAXSDOCUMENT_SOURCE_DIR})

add_definitions (-DHAVE_TIFF)

set (SOURCES saxsimage.c
             saxsimage_format.c
             cbf.c
             msk.c
             tiff.c)

set (HEADERS saxsimage.h
             saxsimage_format.h)

# conditional sources
if (TARGET edf)
  add_definitions (-DHAVE_EDF)
  include_directories (${EDFPACK_SOURCE_DIR})
  set (EDF_LIBRARIES edf)
  set (SOURCES ${SOURCES} edf.c)
endif (TARGET edf)

include_directories (${LIBTIFF_INCLUDE_DIR}
                     ${CBFLIB_SOURCE_DIR})

add_shared_library (saxsimage
                    SOURCES ${HEADERS} ${SOURCES}
                    LIBRARIES saxsdocument cbf ${EDF_LIBRARIES} ${LIBTIFF_LIBRARIES} 
                    VERSION 1)

install_library (TARGETS saxsimage)
