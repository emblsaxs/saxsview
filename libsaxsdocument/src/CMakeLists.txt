project (LIBSAXSDOCUMENT C)

if (MINGW)
  message (STATUS "Linking to libxml2 fails on MinGW, disabling CANSAS-XML support")
else (MINGW)
  find_package (LibXml2 QUIET)
  if (NOT LIBXML2_FOUND)
    message (STATUS "Optional package libxml2 not found, CANSAS-XML document format disabled.")
  endif (NOT LIBXML2_FOUND)
endif (MINGW)

if (LIBSAXSDOCUMENT_HEAVY_ASSERTS)
  add_definitions(-DLIBSAXSDOCUMENT_HEAVY_ASSERTS)
endif(LIBSAXSDOCUMENT_HEAVY_ASSERTS)

set (SOURCES saxsproperty.c
             saxsdocument.c
             saxsdocument_format.c
             columns.c
             csv.c
             atsas_dat.c
             atsas_fir_fit.c
             atsas_int.c
             atsas_out.c
             maxlab_rad.c
             malvern_txt.c)

set (HEADERS saxsproperty.h
             saxsdocument.h
             saxsdocument_format.h
             columns.h)

# conditional sources
if (LIBXML2_FOUND)
  add_definitions (-DHAVE_LIBXML2 ${LIBXML2_DEFINITIONS})
  include_directories(${LIBXML2_INCLUDE_DIR})
  set (SOURCES ${SOURCES} cansas_xml.c)
endif (LIBXML2_FOUND)

add_shared_library (saxsdocument
                    SOURCES ${HEADERS} ${SOURCES}
                    LIBRARIES m ${LIBXML2_LIBRARIES}
                    VERSION 1)

install_library (TARGETS saxsdocument)
