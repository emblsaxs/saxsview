#
# Everything related to packaging of saxsview.
#

#
# General information
#
set (CPACK_PACKAGE_NAME                "saxsview")
set (CPACK_PACKAGE_CONTACT             "dfranke@users.sourceforge.net")
set (CPACK_PACKAGE_DESCRIPTION_FILE    "${SAXSVIEW_SOURCE_DIR}/README.txt")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "saxsview; read, convert and view 1D and 2D-files related to Small Angle X-ray Scattering (SAXS).")
set (CPACK_PACKAGE_VERSION             "0.3.0")
set (CPACK_PACKAGE_VERSION_MAJOR       "0")
set (CPACK_PACKAGE_VERSION_MINOR       "3")
set (CPACK_PACKAGE_VERSION_PATCH       "0")
set (CPACK_PACKAGE_FILE_NAME           "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_RESOURCE_FILE_LICENSE       "${SAXSVIEW_SOURCE_DIR}/COPYING.txt")

#
# System specific settings
#
include (${CMAKE_SYSTEM_NAME}/packaging)

include (CPack)
