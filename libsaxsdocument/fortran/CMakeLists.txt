
enable_language(Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  add_static_library (fsaxsdocument
                      SOURCES fsaxsdocument.f90
                      LIBRARIES saxsdocument)

  if (DEFINED CMAKE_Fortran_MODULE_DIRECTORY)
    target_include_directories(fsaxsdocument INTERFACE "${CMAKE_Fortran_MODULE_DIRECTORY}")
  else()
    target_include_directories(fsaxsdocument INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")
  endif()
endif (CMAKE_Fortran_COMPILER_WORKS)

