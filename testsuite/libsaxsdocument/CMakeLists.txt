
include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR})

add_executable (doctest doctest.c)
target_link_libraries (doctest saxsdocument)

add_executable (readtest readtest.c)
target_link_libraries (readtest saxsdocument)

# add_executable (fdoctest doctest.f90)
# target_link_libraries (fdoctest fsaxsdocument)


#
# Tests
#

# a non-existant file should be flagged as such
add_test (NAME read-nosuchfile
          COMMAND $<TARGET_FILE:readtest> nosuchfile)
set_property (TEST read-nosuchfile PROPERTY PASS_REGULAR_EXPRESSION "lines_read returned 2 ")

# Trying to read a file which is obviously not SAXS data should fail (but not crash)
add_test (NAME read-notsaxsdata
          COMMAND $<TARGET_FILE:readtest> ${CMAKE_CURRENT_SOURCE_DIR}/doctest.c)
set_property (TEST read-notsaxsdata PROPERTY FAIL_REGULAR_EXPRESSION "Number of curves: [1-9]")

# Trying to read a file which is obviously not SAXS data -- forcing the use of a particular format
add_test (NAME read-notsaxsdata-force
          COMMAND $<TARGET_FILE:readtest> ${CMAKE_CURRENT_SOURCE_DIR}/doctest.c atsas-int)
set_property (TEST read-notsaxsdata-force PROPERTY PASS_REGULAR_EXPRESSION "Result: 0 ")



add_subdirectory(testdata)
add_subdirectory(crashtests)
add_subdirectory(unittests)
add_subdirectory(line-endings)
