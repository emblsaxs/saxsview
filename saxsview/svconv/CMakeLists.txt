
include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR})

add_application (svconv
                 SOURCES svconv.cpp
                 LIBRARIES saxsdocument saxsimage m)

install_application (TARGETS svconv)
