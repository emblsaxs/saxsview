
include (GNUInstallDirs)

macro (saxsview_install)
  foreach (target ${ARGN})
    if (TARGET ${target})
      install (TARGETS               ${target}
               ARCHIVE DESTINATION   ${CMAKE_INSTALL_LIBDIR}
               LIBRARY DESTINATION   ${CMAKE_INSTALL_LIBDIR}
               RUNTIME DESTINATION   ${CMAKE_INSTALL_BINDIR}
               BUNDLE DESTINATION    .
               FRAMEWORK DESTINATION Frameworks)
    endif (TARGET ${target})
  endforeach (target)
endmacro (saxsview_install)

#
# System specific installation commands ...
#
include (${CMAKE_SYSTEM_NAME}/install)
