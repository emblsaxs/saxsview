
enable_language(Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  add_static_library (fsaxsdocument
                      SOURCES fsaxsdocument.f90
                      LIBRARIES saxsdocument)

  install_library (TARGETS fsaxsdocument)
endif (CMAKE_Fortran_COMPILER_WORKS)
