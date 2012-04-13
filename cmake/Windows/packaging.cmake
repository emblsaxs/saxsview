
set (CPACK_GENERATOR              NSIS ZIP)
set (CPACK_STRIP_FILES            TRUE)

set (CPACK_NSIS_DISPLAY_NAME      "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
set (CPACK_NSIS_PACKAGE_NAME      "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
set (CPACK_NSIS_CONTACT           ${CPACK_PACKAGE_CONTACT})
set (CPACK_NSIS_URL_INFO_ABOUT    "http://saxsview.sourceforge.net")

# set (CPACK_NSIS_MUI_ICON        ${CMAKE_CURRENT_SOURCE_DIR}/saxsview.ico)
# set (CPACK_NSIS_MUI_UNIICON     ${CMAKE_CURRENT_SOURCE_DIR}/saxsview.ico)

# Menu entries for binaries and links to external web pages in the Start menu
set (CPACK_PACKAGE_EXECUTABLES    "svplot;SAXS Data;svmage;SAXS Image;")
set (CPACK_NSIS_MENU_LINKS        "http://saxsview.sourceforge.net" "saxsview Homepage"
                                  COPYING.txt "saxsview License")

#
# Define file associations using a helper script taken from:
#   http://nsis.sourceforge.net/File_Association
#
# NSIS expects a path with native '\' delimiters, cmake usually
# works with '/'. Thanks to:
#    http://www.cmake.org/Bug/view.php?id=5939
#
# we can not use FILE(TO_NATIVE_PATH) to convert the '/' to '\'.
#
string (REPLACE "/" "\\\\" FILE_ASSOCIATION_NSH ${CMAKE_CURRENT_LIST_DIR}/FileAssociation.nsh)
set (CPACK_NSIS_DEFINES           "!include ${FILE_ASSOCIATION_NSH}") 
set (CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
     \\\${RegisterExtension} \\\"$INSTDIR\\\\bin\\\\svplot.exe\\\" \\\".dat\\\" \\\"SAXS Data\\\"
     \\\${RegisterExtension} \\\"$INSTDIR\\\\bin\\\\svplot.exe\\\" \\\".fir\\\" \\\"SAXS Data\\\"
     \\\${RegisterExtension} \\\"$INSTDIR\\\\bin\\\\svplot.exe\\\" \\\".fit\\\" \\\"SAXS Data\\\"
     \\\${RegisterExtension} \\\"$INSTDIR\\\\bin\\\\svimage.exe\\\" \\\".cbf\\\" \\\"CBF Image\\\"
     \\\${RegisterExtension} \\\"$INSTDIR\\\\bin\\\\svimage.exe\\\" \\\".edf\\\" \\\"EDF Image\\\"
     \\\${RegisterExtension} \\\"$INSTDIR\\\\bin\\\\svimage.exe\\\" \\\".msk\\\" \\\"MASK Image\\\"
     \\\${RegisterExtension} \\\"$INSTDIR\\\\bin\\\\svimage.exe\\\" \\\".tiff\\\" \\\"TIFF Image\\\"
     ")
set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
     \\\${UnRegisterExtension} \\\".dat\\\" \\\"SAXS Data\\\"
     \\\${UnRegisterExtension} \\\".fir\\\" \\\"SAXS Data\\\"
     \\\${UnRegisterExtension} \\\".fit\\\" \\\"SAXS Data\\\"
     \\\${UnRegisterExtension} \\\".cbf\\\" \\\"CBF Image\\\"
     \\\${UnRegisterExtension} \\\".edf\\\" \\\"EDF Image\\\"
     \\\${UnRegisterExtension} \\\".msk\\\" \\\"MASK Image\\\"
     \\\${UnRegisterExtension} \\\".tiff\\\" \\\"TIFF Image\\\"
     ")

# An extra page will appear in the installer that will allow the 
# user to choose whether the program directory should be added
# to the system PATH variable.
set (CPACK_NSIS_MODIFY_PATH       ON)
