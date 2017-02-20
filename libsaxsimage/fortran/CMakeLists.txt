
enable_language (Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  add_static_library (fsaxsimage
                      SOURCES fsaxsimage.f90
                      LIBRARIES saxsimage)

  target_include_directories(fsaxsimage INTERFACE "${CMAKE_Fortran_MODULE_DIRECTORY}")
endif (CMAKE_Fortran_COMPILER_WORKS)

