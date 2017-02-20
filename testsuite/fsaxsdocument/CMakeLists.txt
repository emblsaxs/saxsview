
enable_language(Fortran OPTIONAL)

if (TARGET fsaxsdocument)

  add_executable(ftnreadtest readtest.f90)
  target_link_libraries(ftnreadtest fsaxsdocument)

  ## compiler flags to enable SIGFPE exceptions
  target_compile_options(ftnreadtest PRIVATE "-ffpe-trap=invalid,zero,overflow")

  add_test (NAME read-columns-nan-inf
            COMMAND $<TARGET_FILE:ftnreadtest> "${CMAKE_CURRENT_SOURCE_DIR}/columns-bad.dat")
  set_property (TEST read-columns-nan-inf PROPERTY PASS_REGULAR_EXPRESSION "Result: *0")
endif(TARGET fsaxsdocument)
