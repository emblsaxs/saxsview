project (LIBSAXSDOCUMENT C CXX Fortran)
set (SOURCES saxsproperty.c
             saxsdocument.c
             saxsdocument_format.c
             columns_reader.c
             atsas_dat.c
             atsas_fir_fit.c
             atsas_int.c
             atsas_out.c)

set (HEADERS saxsproperty.h
             saxsdocument.h
             saxsdocument_format.h
             columns.h)

add_library (saxsdocument SHARED ${SOURCES} ${HEADERS})

install (TARGETS saxsdocument
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (FILES saxsdocument.h
         DESTINATION include/saxsdocument)

