
enable_language(Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  add_static_library (fsaxsdocument
                      SOURCES fsaxsdocument.f90
                      LIBRARIES saxsdocument)

endif (CMAKE_Fortran_COMPILER_WORKS)

