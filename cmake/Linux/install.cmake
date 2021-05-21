
include (GNUInstallDirs)

set (SAXSVIEW_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})

# A private library directory for saxsview applications. This
# is to avoid issues with, e.g. qwt already installed in /usr/lib.
set (SAXSVIEW_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}/saxsview)

# Set RPATH on install to make sure that the binaries will
# find the private libraries.
set (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${SAXSVIEW_INSTALL_LIBDIR})

#
# Debian wants its python extension packages installed to "dist-packages",
# not the default "site-packages".
#
if (NOT EXISTS "/etc/debian_version")
  set (PYTHON_INSTALL_PACKAGEDIR "site-packages")
else ()
  set (PYTHON_INSTALL_PACKAGEDIR "dist-packages")
endif ()

set (SAXSVIEW_INSTALL_PYPKGDIR ${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/${PYTHON_INSTALL_PACKAGEDIR})

