project (LIBSAXSDOCUMENT C CXX Fortran)
set (SOURCES saxsproperty.c
             saxsdocument.c
             formats.c
             columns_reader.c
             dat_reader.c
             fir_fit_reader.c
             int_reader.c
             out_reader.c
             dat_writer.c)

set (HEADERS saxsproperty.h
             saxsdocument.h
             formats.h
             columns.h)

add_library (saxsdocument SHARED ${SOURCES} ${HEADERS})

install (TARGETS saxsdocument
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (FILES saxsdocument.h
         DESTINATION include/saxsdocument)

