project (LIBSAXSIMAGE C)

find_package (TIFF)

set (SOURCES saxsimage.c
             saxsimage_format.c
             image_cbf.c)

set (HEADERS saxsimage.h
             saxsimage_format.h)

# conditional sources
if (TIFF_FOUND)
  add_definitions (-DHAVE_TIFF)
  set (SOURCES ${SOURCES} image_tiff.c)
endif (TIFF_FOUND)

add_library (saxsimage SHARED ${SOURCES} ${HEADERS})

include_directories(${CBFLIB_SOURCE_DIR})
target_link_libraries (saxsimage cbf)

# conditional include directories and libraries
if (TIFF_FOUND)
  include_directories (${TIFF_INCLUDE_DIR})
  target_link_libraries (saxsimage ${TIFF_LIBRARIES})
endif (TIFF_FOUND)


install (TARGETS saxsimage
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (FILES saxsdocument.h
         DESTINATION include/saxsdocument)

