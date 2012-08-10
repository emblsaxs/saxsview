
#
# System specific settings ...
#
include (${CMAKE_SYSTEM_NAME}/install)
include (CMakeParseArguments)
include (GNUInstallDirs)

function (install_application)
  cmake_parse_arguments (INSTALL "" "" "TARGETS" ${ARGN})

  install (TARGETS               ${INSTALL_TARGETS}
           RUNTIME DESTINATION   ${SAXSVIEW_INSTALL_BINDIR}
           BUNDLE DESTINATION    ".")
endfunction (install_application)


function (install_library)
  cmake_parse_arguments (INSTALL "" "" "TARGETS" ${ARGN})

  install (TARGETS               ${INSTALL_TARGETS}
           ARCHIVE DESTINATION   ${SAXSVIEW_INSTALL_LIBDIR}
           LIBRARY DESTINATION   ${SAXSVIEW_INSTALL_LIBDIR})
endfunction (install_library)


function (install_python_module)
  cmake_parse_arguments (INSTALL "" "" "TARGETS" ${ARGN})

  if (PYTHON_VERSION_MAJOR AND PYTHON_VERSION_MINOR)
    install (TARGETS ${INSTALL_TARGETS}
             DESTINATION ${SAXSVIEW_INSTALL_PYPKGDIR})
  endif (PYTHON_VERSION_MAJOR AND PYTHON_VERSION_MINOR)
endfunction (install_python_module)
