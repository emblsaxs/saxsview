
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
set_property (TEST read-nosuchfile PROPERTY PASS_REGULAR_EXPRESSION "Result: 2 ")

# Trying to read a file which is obviously not SAXS data should fail (but not crash)
add_test (NAME read-notsaxsdata
          COMMAND $<TARGET_FILE:readtest> ${CMAKE_CURRENT_SOURCE_DIR}/doctest.c)
set_property (TEST read-notsaxsdata PROPERTY FAIL_REGULAR_EXPRESSION "Result: 0 ")

# Trying to read a file which is obviously not SAXS data -- forcing the use of a particular format
add_test (NAME read-notsaxsdata-force
          COMMAND $<TARGET_FILE:readtest> ${CMAKE_CURRENT_SOURCE_DIR}/doctest.c atsas-int)
set_property (TEST read-notsaxsdata-force PROPERTY PASS_REGULAR_EXPRESSION "Result: 0 ")

# Trying to read a file which is obviously not SAXS data -- forcing the use of a particular format
add_test (NAME read-atsas-out-nofooter
          COMMAND $<TARGET_FILE:readtest> ${CMAKE_CURRENT_SOURCE_DIR}/gi-sasdak6.out)
set_property (TEST read-atsas-out-nofooter PROPERTY PASS_REGULAR_EXPRESSION "Result: 0 \\((Success|No error)\\)\nFormat ID: 'atsas-out'\nNumber of properties: 12\nNumber of curves: 4")

# read-only tests for .dat-files
set (DATTESTS "empty;whitespace;columns;bsa;bsa-sub;long-lines")
foreach (test ${DATTESTS})
  add_test (NAME read-dat-${test}
            COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/${test}.dat
                                           ${CMAKE_CURRENT_SOURCE_DIR}/${test}.exp)
endforeach (test)


# read-write tests
set (DATTESTS "columns;bsa;")
foreach (test ${DATTESTS})
  add_test (NAME read-write-dat-${test}
            COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/${test}.dat
                                           ${CMAKE_CURRENT_BINARY_DIR}/${test}.dat
                                           ${CMAKE_CURRENT_SOURCE_DIR}/${test}.exp)
endforeach (test)

# read-only tests for .out-files
set (OUTTESTS "lyzexp;")
foreach (test ${OUTTESTS})
  add_test (NAME read-out-${test}
            COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/${test}.out
                                           ${CMAKE_CURRENT_SOURCE_DIR}/${test}.exp)
endforeach (test)


#
# Test Fortran bindings
#
# read-only tests for .dat-files
# set (DATTESTS "empty;whitespace;columns;bsa;")
# foreach (test ${DATTESTS})
#   add_test (read-dat-${test}-fortran fdoctest ${CMAKE_CURRENT_SOURCE_DIR}/${test}.dat
#                                               ${CMAKE_CURRENT_SOURCE_DIR}/${test}.exp)
# endforeach (test)
# 
# # read-write tests
# set (DATTESTS "columns;bsa;")
# foreach (test ${DATTESTS})
#   add_test (read-write-dat-${test}-fortran fdoctest ${CMAKE_CURRENT_SOURCE_DIR}/${test}.dat
#                                                     ${CMAKE_CURRENT_BINARY_DIR}/${test}.dat
#                                                     ${CMAKE_CURRENT_SOURCE_DIR}/${test}.exp)
# endforeach (test)

add_subdirectory(crashtests)
