
set (CPACK_GENERATOR   RPM DEB TGZ)
set (CPACK_STRIP_FILES TRUE)


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
set (CPACK_DEBIAN_PACKAGE_HOMEPAGE     "http://saxsview.sourceforge.net")
set (CPACK_DEBIAN_PACKAGE_SECTION      "science")
set (CPACK_DEBIAN_PACKAGE_DEPENDS      "libxml2 (>= 2.7.0), libtiff4 (>= 3.7.2), libqtgui4 (>= 4.4.0), zlib1g")
set (CPACK_DEBIAN_PACKAGE_SUGGESTS     "python-minimal")

#
# Specifics for RPM packages.
#   Verify files: `rpm -qpl <file>`
#   Verify description: `rpm -qpi <file>`
#
set (CPACK_RPM_PACKAGE_LICENSE         "GPLv3")
set (CPACK_RPM_PACKAGE_GROUP           "science")
set (CPACK_RPM_PACKAGE_REQUIRES        "libxml2 >= 2.7.0, libtiff >= 3.7.2, qt4 >= 4.4.0")
set (CPACK_RPM_PACKAGE_RELEASE         "1")
