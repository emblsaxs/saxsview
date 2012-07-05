
include (CMakeParseArguments)
include (GNUInstallDirs)

function (install_application)
  cmake_parse_arguments (INSTALL "" "" "TARGETS" ${ARGN})

  install (TARGETS               ${INSTALL_TARGETS}
           ARCHIVE DESTINATION   ${CMAKE_INSTALL_LIBDIR}
           LIBRARY DESTINATION   ${CMAKE_INSTALL_LIBDIR}
           RUNTIME DESTINATION   ${CMAKE_INSTALL_BINDIR}
           BUNDLE DESTINATION    ".")
endfunction (install_application)


function (install_library)
  cmake_parse_arguments (INSTALL "" "" "TARGETS" ${ARGN})

  install (TARGETS               ${INSTALL_TARGETS}
           ARCHIVE DESTINATION   ${CMAKE_INSTALL_LIBDIR}
           LIBRARY DESTINATION   ${CMAKE_INSTALL_LIBDIR}
           RUNTIME DESTINATION   ${CMAKE_INSTALL_BINDIR}
           FRAMEWORK DESTINATION "Frameworks")
endfunction (install_library)


function (install_python_module)
  cmake_parse_arguments (INSTALL "" "" "TARGETS" ${ARGN})

  if (PYTHON_VERSION_MAJOR AND PYTHON_VERSION_MINOR)
    install (TARGETS ${INSTALL_TARGETS}
             DESTINATION lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
  endif (PYTHON_VERSION_MAJOR AND PYTHON_VERSION_MINOR)

endfunction (install_python_module)


#
# System specific installation commands ...
#
include (${CMAKE_SYSTEM_NAME}/install)
