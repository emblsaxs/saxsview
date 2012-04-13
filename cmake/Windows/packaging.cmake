
set (CPACK_GENERATOR              NSIS ZIP)
set (CPACK_STRIP_FILES            TRUE)

set (CPACK_NSIS_DISPLAY_NAME      "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
set (CPACK_NSIS_PACKAGE_NAME      "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
set (CPACK_NSIS_CONTACT           ${CPACK_PACKAGE_CONTACT})
set (CPACK_NSIS_URL_INFO_ABOUT    "http://saxsview.sourceforge.net")

# set (CPACK_NSIS_MUI_ICON        ${CMAKE_CURRENT_SOURCE_DIR}/saxsview.ico)
# set (CPACK_NSIS_MUI_UNIICON     ${CMAKE_CURRENT_SOURCE_DIR}/saxsview.ico)

# Links to external web pages in the Start menu
set (CPACK_NSIS_MENU_LINKS        COPYING.txt "SaxsView License"
                                  "http://saxsview.sourceforge.net" "saxsview Homepage")

# An extra page will appear in the installer that will allow the 
# user to choose whether the program directory should be added
# to the system PATH variable.
set (CPACK_NSIS_MODIFY_PATH       ON)