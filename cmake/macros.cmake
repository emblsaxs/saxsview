
find_package (Python COMPONENTS Interpreter Development)


function (add_application)
  cmake_parse_arguments (APP "GUI" "" "SOURCES;LIBRARIES" ${ARGN})

  set (APP_NAME ${APP_UNPARSED_ARGUMENTS})

  add_executable (${APP_NAME} ${APP_SOURCES})

  set_target_properties(${APP_NAME} PROPERTIES
                        AUTOMOC On
                        AUTORCC On)

  if (APP_LIBRARIES)
    target_link_libraries (${APP_NAME} ${APP_LIBRARIES})
  endif (APP_LIBRARIES)

  if (APP_GUI)
    set_target_properties(${APP_NAME} PROPERTIES
                          WIN32_EXECUTABLE         TRUE)
  endif (APP_GUI)
endfunction (add_application)


function (add_shared_library)
  cmake_parse_arguments (LIB "STATIC;SHARED" "VERSION" "SOURCES;LIBRARIES" ${ARGN})

  add_library (${LIB_UNPARSED_ARGUMENTS} SHARED ${LIB_SOURCES})

  set_target_properties(${LIB_UNPARSED_ARGUMENTS} PROPERTIES
                        AUTOMOC On
                        AUTORCC On)

  if (LIB_LIBRARIES)
    target_link_libraries(${LIB_UNPARSED_ARGUMENTS} ${LIB_LIBRARIES})
  endif (LIB_LIBRARIES)

  if (LIB_VERSION)
    set_target_properties (${LIB_UNPARSED_ARGUMENTS} PROPERTIES
                                                     VERSION ${LIB_VERSION})
  endif (LIB_VERSION)
endfunction (add_shared_library)


function (add_static_library)
  cmake_parse_arguments (LIB "" "" "SOURCES;LIBRARIES" ${ARGN})

  add_library (${LIB_UNPARSED_ARGUMENTS} STATIC ${LIB_SOURCES})

  if (LIB_LIBRARIES)
    target_link_libraries(${LIB_UNPARSED_ARGUMENTS} ${LIB_LIBRARIES})
  endif (LIB_LIBRARIES)
endfunction (add_static_library)

function (add_python_module)
  if (Python_Interpreter_FOUND AND Python_Development_FOUND)
    cmake_parse_arguments (MOD "" "COMPONENT;MODULE" "SOURCES;LIBRARIES" ${ARGN})

    set (PYTHON_MODULE_PREFIX "")
    set (PYTHON_MODULE_SUFFIX "")
    if (Python_VERSION_MAJOR EQUAL 3)
      string (APPEND PYTHON_MODULE_SUFFIX ".${Python_SOABI}")
    endif (Python_VERSION_MAJOR EQUAL 3)

    if (WIN32 AND NOT CYGWIN)
      string (APPEND PYTHON_MODULE_SUFFIX ".pyd")    # default is .dll
    elseif (APPLE)
      string (APPEND PYTHON_MODULE_SUFFIX ".so")     # default is .dylib
    else ()
      string (APPEND PYTHON_MODULE_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif (WIN32 AND NOT CYGWIN)

    ## In theory this should be MODULE and not SHARED, but for
    ## some reason SHARED works on Mac and Win and MODULE does not
    add_library (${MOD_UNPARSED_ARGUMENTS} SHARED ${MOD_SOURCES})

    target_include_directories(${MOD_UNPARSED_ARGUMENTS} PUBLIC
                               "${Python_INCLUDE_DIRS}")

    if (MOD_LIBRARIES)
      target_link_libraries (${MOD_UNPARSED_ARGUMENTS} ${MOD_LIBRARIES} ${Python_LIBRARIES})
    endif (MOD_LIBRARIES)

    if (MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 8)
      # Workaround for http://bugs.python.org/issue11722
      set_target_properties (${MOD_UNPARSED_ARGUMENTS} PROPERTIES
                             COMPILE_DEFINITIONS "MS_WIN64")
    endif (MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 8)

    set_target_properties (${MOD_UNPARSED_ARGUMENTS} PROPERTIES
                           PREFIX "${PYTHON_MODULE_PREFIX}"
                           SUFFIX "${PYTHON_MODULE_SUFFIX}")

  endif (Python_Interpreter_FOUND AND Python_Development_FOUND)
endfunction (add_python_module)
