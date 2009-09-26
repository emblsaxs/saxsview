project (LIBSAXSDOCUMENT C CXX Fortran)

find_package (TIFF)
find_package (LibXml2)

set (SOURCES saxsproperty.c
             saxsdocument.c
             saxsdocument_format.c
             saxsimage.c
             saxsimage_format.c
             columns_reader.c
             atsas_dat.c
             atsas_fir_fit.c
             atsas_int.c
             atsas_out.c
             image_cbf.c)

set (HEADERS saxsproperty.h
             saxsdocument.h
             saxsdocument_format.h
             saxsimage.h
             saxsimage_format.h
             columns.h)

# conditional sources
if (TIFF_FOUND)
  add_definitions (-DHAVE_TIFF)
  set (SOURCES ${SOURCES} image_tiff.c)
endif (TIFF_FOUND)

if (LIBXML2_FOUND)
  add_definitions (-DHAVE_LIBXML2 ${LIBXML2_DEFINITIONS})
  set (SOURCES ${SOURCES} cansas_xml.c)
endif (LIBXML2_FOUND)

add_library (saxsdocument SHARED ${SOURCES} ${HEADERS})

include_directories(${CBFLIB_SOURCE_DIR})
target_link_libraries (saxsdocument cbf)

# conditional include directories and libraries
if (TIFF_FOUND)
  include_directories (${TIFF_INCLUDE_DIR})
  target_link_libraries (saxsdocument ${TIFF_LIBRARIES})
endif (TIFF_FOUND)

if (LIBXML2_FOUND)
  include_directories(${LIBXML2_INCLUDE_DIR})
  target_link_libraries (saxsdocument ${LIBXML2_LIBRARIES})
endif (LIBXML2_FOUND)


install (TARGETS saxsdocument
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (FILES saxsdocument.h
         DESTINATION include/saxsdocument)

