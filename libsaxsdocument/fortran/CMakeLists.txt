
enable_language(Fortran OPTIONAL)

if (CMAKE_Fortran_COMPILER_WORKS)
  add_static_library (fsaxsdocument
                      SOURCES fsaxsdocument.f90
                      LIBRARIES saxsdocument)
  target_compile_options(fsaxsdocument PRIVATE $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-frecursive>)
endif (CMAKE_Fortran_COMPILER_WORKS)

