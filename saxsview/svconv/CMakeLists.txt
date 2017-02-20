
add_application (svconv
                 SOURCES svconv.cpp
                 LIBRARIES saxsdocument saxsimage m)

install_application (TARGETS svconv)
install_dependencies (svconv)
