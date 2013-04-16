
include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR})

add_executable (doctest doctest.c)
target_link_libraries (doctest saxsdocument)

# add_executable (fdoctest doctest.f90)
# target_link_libraries (fdoctest fsaxsdocument)


#
# Tests
#

# a non-existant file should be flagged as such
add_test (NAME read-nosuchfile
          COMMAND $<TARGET_FILE:doctest> nosuchfile)
set_property (TEST read-nosuchfile PROPERTY WILL_FAIL TRUE)

# read-only tests for .dat-files
set (DATTESTS "empty;whitespace;columns;bsa;bsa-sub;long-lines")
foreach (test ${DATTESTS})
  add_test (NAME read-dat-${test}
            COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/${test}.dat
                                           ${CMAKE_CURRENT_SOURCE_DIR}/${test}.exp)
endforeach (test)

set_tests_properties (read-dat-whitespace PROPERTIES WILL_FAIL TRUE)

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

