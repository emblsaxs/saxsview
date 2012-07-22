
include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR})

add_application (svconv
                 SOURCES svconv.cpp
                 LIBRARIES saxsdocument saxsimage)

install_application (TARGETS svconv)
install_dependencies (svconv)
