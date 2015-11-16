set(Python_ADDITIONAL_VERSIONS "2.7" "2.6")

find_package (PythonLibs)
find_package (PythonInterp)

include (CMakeParseArguments)


function (add_application)
  cmake_parse_arguments (APP "GUI" "" "SOURCES;LIBRARIES" ${ARGN})

  set (APP_NAME ${APP_UNPARSED_ARGUMENTS})

  add_executable (${APP_NAME} ${APP_SOURCES})

  if (APP_LIBRARIES)
    target_link_libraries (${APP_NAME} ${APP_LIBRARIES})
  endif (APP_LIBRARIES)

  if (APP_GUI)
    set_target_properties(${APP_NAME} PROPERTIES
                          WIN32_EXECUTABLE         TRUE
                          MACOSX_BUNDLE            TRUE
                          MACOSX_BUNDLE_INFO_PLIST ${SAXSVIEW_BINARY_DIR}/admin/Darwin/${APP_NAME}.plist)
  endif (APP_GUI)
endfunction (add_application)


function (add_shared_library)
  cmake_parse_arguments (LIB "STATIC;SHARED" "VERSION" "SOURCES;LIBRARIES" ${ARGN})

  add_library (${LIB_UNPARSED_ARGUMENTS} SHARED ${LIB_SOURCES})

  if (LIB_LIBRARIES)
    target_link_libraries(${LIB_UNPARSED_ARGUMENTS} ${LIB_LIBRARIES})
  endif (LIB_LIBRARIES)

  if (LIB_VERSION)
    set_target_properties (${LIB_UNPARSED_ARGUMENTS} PROPERTIES
                                                     VERSION ${LIB_VERSION})
  endif (LIB_VERSION)

  set_target_properties (${LIB_UNPARSED_ARGUMENTS} PROPERTIES
                                                   FRAMEWORK TRUE)
endfunction (add_shared_library)


function (add_static_library)
  cmake_parse_arguments (LIB "" "" "SOURCES;LIBRARIES" ${ARGN})

  add_library (${LIB_UNPARSED_ARGUMENTS} STATIC ${LIB_SOURCES})

  if (LIB_LIBRARIES)
    target_link_libraries(${LIB_UNPARSED_ARGUMENTS} ${LIB_LIBRARIES})
  endif (LIB_LIBRARIES)
endfunction (add_static_library)


# FindPythonInterpreter.cmake (python_add_module), is still incomplete as of cmake-2.8.7.
function (add_python_module)
  if (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
    cmake_parse_arguments (MOD "" "" "SOURCES;LIBRARIES" ${ARGN})

    set (PYTHON_MODULE_PREFIX "")
    if (WIN32 AND NOT CYGWIN)
      set (PYTHON_MODULE_SUFFIX ".pyd")    # default is .dll
    elseif (APPLE)
      set (PYTHON_MODULE_SUFFIX ".so")     # default is .dylib
    else ()
      set (PYTHON_MODULE_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif (WIN32 AND NOT CYGWIN)

    include_directories (${PYTHON_INCLUDE_DIR})

    add_library (${MOD_UNPARSED_ARGUMENTS} MODULE ${MOD_SOURCES})
    if (MOD_LIBRARIES)
      target_link_libraries (${MOD_UNPARSED_ARGUMENTS} ${MOD_LIBRARIES} ${PYTHON_LIBRARIES})
    endif (MOD_LIBRARIES)

	if (MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 8)
	  # Workaround for http://bugs.python.org/issue11722
      set_target_properties (${MOD_UNPARSED_ARGUMENTS} PROPERTIES
	                                                   COMPILE_DEFINITIONS "MS_WIN64")
	endif (MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 8)

    set_target_properties(${MOD_UNPARSED_ARGUMENTS} PROPERTIES
                                                    PREFIX "${PYTHON_MODULE_PREFIX}"
                                                    SUFFIX "${PYTHON_MODULE_SUFFIX}")
  endif (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
endfunction (add_python_module)
