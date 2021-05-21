
#
# Set RPATH on install to make sure that the binaries will
# find the private libraries.
#
# Here "@executable_path" is the equivalent to "$ORIGIN" on Linux.
#
SET(CMAKE_INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}/saxsview/")

