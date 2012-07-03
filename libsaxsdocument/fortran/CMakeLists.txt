
enable_language(Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  saxsview_add_library(fsaxsdocument SOURCES fsaxsdocument.f90
                                     LIBRARIES saxsdocument
                                     STATIC)
endif (CMAKE_Fortran_COMPILER_WORKS)

