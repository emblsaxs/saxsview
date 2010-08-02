project (LIBSAXSDOCUMENT C)

find_package (LibXml2)

set (SOURCES saxsproperty.c
             saxsdocument.c
             saxsdocument_format.c
             columns.c
             atsas_dat.c
             atsas_fir_fit.c
             atsas_int.c
             atsas_out.c)

set (HEADERS saxsproperty.h
             saxsdocument.h
             saxsdocument_format.h
             columns.h)

# conditional sources
if (LIBXML2_FOUND)
  add_definitions (-DHAVE_LIBXML2 ${LIBXML2_DEFINITIONS})
  set (SOURCES ${SOURCES} cansas_xml.c)
endif (LIBXML2_FOUND)

add_library (saxsdocument SHARED ${SOURCES} ${HEADERS})

# conditional include directories and libraries
if (LIBXML2_FOUND)
  include_directories(${LIBXML2_INCLUDE_DIR})
  target_link_libraries (saxsdocument ${LIBXML2_LIBRARIES})
endif (LIBXML2_FOUND)


install (TARGETS saxsdocument
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)
