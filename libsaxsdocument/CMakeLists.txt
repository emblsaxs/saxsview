project (LIBSAXSDOCUMENT C CXX Fortran)
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

include_directories(${CBFLIB_SOURCE_DIR})

add_library (saxsdocument SHARED ${SOURCES} ${HEADERS})
target_link_libraries (saxsdocument cbf)

install (TARGETS saxsdocument
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (FILES saxsdocument.h
         DESTINATION include/saxsdocument)

