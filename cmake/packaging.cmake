#
# Everything related to packaging of saxsview.
#

if (WIN32)
  set (CPACK_GENERATOR  "NSIS")

elseif (APPLE)
  set (CPACK_GENERATOR  "PackageMaker")
  set (CMAKE_OSX_ARCHITECTURES "i386")

else (WIN32)
  set (CPACK_GENERATOR  "RPM" "DEB" "TGZ")

endif (WIN32)

#
# General information
#
set (CPACK_PACKAGE_NAME                "saxsview")
set (CPACK_PACKAGE_CONTACT             "dfranke@users.sourceforge.net")
set (CPACK_PACKAGE_DESCRIPTION_FILE    "${SAXSVIEW_SOURCE_DIR}/README")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "saxsview; read, convert and view 1D and 2D-files related to Small Angle X-ray Scattering (SAXS).")
set (CPACK_PACKAGE_VERSION             "0.2.0")
set (CPACK_PACKAGE_VERSION_MAJOR       "0")
set (CPACK_PACKAGE_VERSION_MINOR       "2")
set (CPACK_PACKAGE_VERSION_PATCH       "0")
set (CPACK_PACKAGE_FILE_NAME           "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_RESOURCE_FILE_LICENSE       "${SAXSVIEW_SOURCE_DIR}/COPYING")
set (CPACK_STRIP_FILES                 TRUE)


#
# Specifics for SOURCE packages.
#
set (CPACK_SOURCE_PACKAGE_FILE_NAME    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_GENERATOR            "TGZ")
set (CPACK_SOURCE_IGNORE_FILES         "/.svn/;/build/;")


#
# Specifics for DEBIAN packages.
#   Verify files: `dpkg --contents <file>`
#   Verify description: `dpkg --info <file>`
#
set (CPACK_DEBIAN_PACKAGE_SECTION      "science")
set (CPACK_DEBIAN_PACKAGE_DEPENDS      "libxml2 (>= 2.7.0), libtiff4 (>= 3.7.2), libqt4-gui (>= 4.5.0)")


#
# Specifics for RPM packages.
#   Verify files: `rpm -qpl <file>`
#   Verify description: `rpm -qpi <file>`
#
set (CPACK_RPM_PACKAGE_LICENSE         "GPLv3")
set (CPACK_RPM_PACKAGE_GROUP           "science")
set (CPACK_RPM_PACKAGE_REQUIRES        "libxml2 >= 2.7.0, libtiff >= 3.7.2, qt4 >= 4.5.0")
set (CPACK_RPM_PACKAGE_RELEASE         "1")

INCLUDE (CPack)

