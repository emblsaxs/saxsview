
enable_language (Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  add_static_library (fsaxsimage
                      SOURCES fsaxsimage.f90
                      LIBRARIES saxsimage)

  install_library (TARGETS fsaxsimage)
endif (CMAKE_Fortran_COMPILER_WORKS)
