
enable_language(Fortran OPTIONAL)

if (TARGET fsaxsdocument)
  ## Set up include_directories() for the Fortran modules
  ## TODO: do this with target_include_directories()
  include_directories("${SAXSVIEW_BINARY_DIR}/libsaxsdocument/fortran")

  ## compiler flags to enable SIGFPE exceptions
  ## TODO: use target_compile_options()
  set(CMAKE_Fortran_COMPILER_FLAGS "${CMAKE_Fortran_COMPILER_FLAGS} -ffpe-trap=invalid,zero,overflow")

  add_executable(ftnreadtest readtest.f90)
  target_link_libraries(ftnreadtest fsaxsdocument)

  add_test (NAME read-columns-nan-inf
            COMMAND $<TARGET_FILE:ftnreadtest> "${CMAKE_CURRENT_SOURCE_DIR}/columns-bad.dat")
  set_property (TEST read-columns-nan-inf PROPERTY PASS_REGULAR_EXPRESSION "Result: *0")
endif(TARGET fsaxsdocument)
