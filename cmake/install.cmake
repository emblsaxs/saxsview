
#
# System specific settings ...
#
include (${CMAKE_SYSTEM_NAME}/install)


function (install_application)
  cmake_parse_arguments (INSTALL "" "" "TARGETS" ${ARGN})

  install (TARGETS               ${INSTALL_TARGETS}
           RUNTIME DESTINATION   ${CMAKE_INSTALL_BINDIR})
endfunction (install_application)


function (install_library)
  cmake_parse_arguments (INSTALL "" "" "TARGETS" ${ARGN})

  install (TARGETS               ${INSTALL_TARGETS}
           ARCHIVE DESTINATION   ${CMAKE_INSTALL_LIBDIR}/saxsview
           LIBRARY DESTINATION   ${CMAKE_INSTALL_LIBDIR}/saxsview
		   RUNTIME DESTINATION   ${CMAKE_INSTALL_BINDIR})
endfunction (install_library)


function (install_python_module)
  cmake_parse_arguments (INSTALL "" "COMPONENT" "TARGETS;FILES" ${ARGN})

  #
  # cmake provides '${Python_SITEARCH}' which would be a convenient
  # install location, if it wouldn't break the package installation
  # paths on Mac.
  #

  set (INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/python/${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages/${INSTALL_COMPONENT})  

  if (INSTALL_TARGETS)
    install (TARGETS ${INSTALL_TARGETS}
             DESTINATION ${INSTALL_DIR})
  endif (INSTALL_TARGETS)

  if (INSTALL_FILES)
    install (FILES ${INSTALL_FILES}
             DESTINATION ${INSTALL_DIR})
  endif (INSTALL_FILES)
endfunction (install_python_module)
