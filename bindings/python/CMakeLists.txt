
find_package (PythonLibs)
find_package (PythonInterp)

#
# Older versions of the FindPythonInterp.cmake module
# do not define PYTHON_VERSION_STRING.
#
if (PYTHONINTERP_FOUND AND NOT PYTHON_VERSION_STRING)
  execute_process(COMMAND "${PYTHON_EXECUTABLE}" --version ERROR_VARIABLE _VERSION OUTPUT_QUIET ERROR_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "Python " "" PYTHON_VERSION_STRING "${_VERSION}")
  string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" PYTHON_VERSION_MAJOR "${PYTHON_VERSION_STRING}")
  string(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" PYTHON_VERSION_MINOR "${PYTHON_VERSION_STRING}")
  string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" PYTHON_VERSION_PATCH "${PYTHON_VERSION_STRING}")
endif (PYTHONINTERP_FOUND AND NOT PYTHON_VERSION_STRING)


#
# FindPythonLibs.cmake defines PYTHON_ADD_MODULE, but as of 2.8.7 it still
# lacks some special case handling. Do it manually instead.
#
# TODO: The platform specific bits need to go the platform directories?!
#
if (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)

  if (WIN32)
    add_definitions(-DPy_ENABLE_SHARED -DHAVE_DECLSPEC_DLL)
  endif (WIN32)

  include_directories(${PYTHON_INCLUDE_DIR}
                      ${LIBSAXSDOCUMENT_SOURCE_DIR})

  add_library (pysaxsdocument SHARED pysaxsdocument.c pysaxsdocument.h)
  target_link_libraries (pysaxsdocument saxsdocument)
  set_target_properties (pysaxsdocument PROPERTIES
                         PREFIX ""
                         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                         OUTPUT_NAME saxsdocument)

  #
  # Default suffix on Windows is ".dll", but somebody thought it funny to
  # name extension modules with a ".pyd" extension.
  # 
  # Default suffix on Mac is ".dylib", but Python expects ".so".
  #
  if (WIN32 AND NOT CYGWIN)
    set_target_properties(${TARGET_DEFAULT_ARGS} PROPERTIES SUFFIX ".pyd")
  elseif (APPLE)
    set_target_properties (${TARGET_DEFAULT_ARGS} PROPERTIES SUFFIX ".so")
  endif (WIN32 AND NOT CYGWIN)

  install (TARGETS pysaxsdocument
           DESTINATION lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)

else (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  message (STATUS "Note: Python not found, extension module 'saxsdocument' will not be built.")

endif (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
