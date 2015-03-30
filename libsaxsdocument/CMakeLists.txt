option(LIBSAXSDOCUMENT_HEAVY_ASSERTS
       "Use extensive asserts to check for bugs in the parsing process. Slows down parsing considerably"
       OFF)

add_subdirectory (src)
add_subdirectory (fortran)
add_subdirectory (python)

