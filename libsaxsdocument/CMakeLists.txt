project (LIBSAXSDOCUMENT C Fortran)

find_package (LibXml2 QUIET)
if (NOT LIBXML2_FOUND)
  message (STATUS "Optional package libxml2 not found, CANSAS-XML document format disabled.")
endif (NOT LIBXML2_FOUND)

set (SOURCES saxsproperty.c
             saxsdocument.c
             saxsdocument_format.c
             columns.c
             csv.c
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
set_target_properties (saxsdocument PROPERTIES
                       VERSION 1)

# conditional include directories and libraries
if (LIBXML2_FOUND)
  include_directories(${LIBXML2_INCLUDE_DIR})
  target_link_libraries (saxsdocument ${LIBXML2_LIBRARIES})
endif (LIBXML2_FOUND)


# Fortran bindings for libsaxsdocument
add_library (fsaxsdocument STATIC saxsdocument.f90)
target_link_libraries (fsaxsdocument saxsdocument)

install (TARGETS saxsdocument fsaxsdocument
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)
