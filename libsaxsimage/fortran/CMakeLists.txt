
enable_language (Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  add_static_library (fsaxsimage
                      SOURCES fsaxsimage.f90
                      LIBRARIES saxsimage)

endif (CMAKE_Fortran_COMPILER_WORKS)

