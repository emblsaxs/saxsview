#
# There is a general convention for CMake commands that take optional flags and/or
# variable arguments. Optional flags are all caps and are added to the arguments to
# turn on. Variable arguments have an all caps identifier to determine where each
# variable argument list starts. The PARSE_ARGUMENTS macro, defined below, can be
# used by other macros to parse arguments defined in this way.
#
# Taken from:
#     http://www.cmake.org/Wiki/CMakeMacroParseArguments
#
MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})    
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})
    SET(larg_names ${arg_names})
    LIST(FIND larg_names "${arg}" is_arg_name)
    IF (is_arg_name GREATER -1)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name GREATER -1)
      SET(loption_names ${option_names})
      LIST(FIND loption_names "${arg}" is_option)
      IF (is_option GREATER -1)
             SET(${prefix}_${arg} TRUE)
      ELSE (is_option GREATER -1)
             SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option GREATER -1)
    ENDIF (is_arg_name GREATER -1)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)


macro (saxsview_add_executable)
  PARSE_ARGUMENTS (APP "SOURCES;LIBRARIES" "" ${ARGN})

  add_executable (${APP_DEFAULT_ARGS} WIN32 MACOSX_BUNDLE ${APP_SOURCES})
  if (APP_LIBRARIES)
    target_link_libraries (${APP_DEFAULT_ARGS} ${APP_LIBRARIES})
  endif (APP_LIBRARIES)
endmacro (saxsview_add_executable)

macro (saxsview_add_library)
  PARSE_ARGUMENTS (LIB "SOURCES;LIBRARIES;VERSION" "STATIC;SHARED" ${ARGN})

  if (LIB_STATIC)
    add_library (${LIB_DEFAULT_ARGS} STATIC ${LIB_SOURCES})
  elseif (LIB_SHARED)
    add_library (${LIB_DEFAULT_ARGS} SHARED ${LIB_SOURCES})
  else ()
    message (WARNING "saxsview_add_library: either SHARED or STATIC must be defined.")
  endif ()

  if (LIB_LIBRARIES)
    target_link_libraries(${LIB_DEFAULT_ARGS} ${LIB_LIBRARIES})
  endif (LIB_LIBRARIES)

  if (LIB_VERSION)
    set_target_properties (${LIB_DEFAULT_ARGS} PROPERTIES
                                               VERSION ${LIB_VERSION})
  endif (LIB_VERSION)
endmacro (saxsview_add_library)

