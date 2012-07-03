
enable_language (Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  saxsview_add_library(fsaxsimage SOURCES fsaxsimage.f90
                                  LIBRARIES saxsimage
                                  STATIC)

endif (CMAKE_Fortran_COMPILER_WORKS)

