
set (CPACK_GENERATOR                 "PackageMaker")

# Required, otherwise the package will install itself in '/usr' by default.
set (CPACK_PACKAGING_INSTALL_PREFIX  ${CMAKE_INSTALL_PREFIX})