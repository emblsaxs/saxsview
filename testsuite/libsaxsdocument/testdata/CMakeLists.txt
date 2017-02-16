# Test reading a GNOM .out file with no footer
add_test (NAME read-atsas-out-nofooter
          COMMAND $<TARGET_FILE:readtest> ${CMAKE_CURRENT_SOURCE_DIR}/gi-sasdak6.out)
set_property (TEST read-atsas-out-nofooter PROPERTY PASS_REGULAR_EXPRESSION "Trying format 'atsas-out'\\.\\.\\.\nResult: 0 \\((Success|No error)\\)\nNumber of properties: 12\nNumber of curves: 4")

# Test reading an ESRF .dat file
add_test (NAME read-dat-ESRF
          COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/SASDB76-cropped.dat
                                         ${CMAKE_CURRENT_SOURCE_DIR}/SASDB76.exp)

# Test reading an autosub .dat file (old pipeline at X33)
add_test (NAME read-dat-autosub
          COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/SASDAB2.dat
                                         ${CMAKE_CURRENT_SOURCE_DIR}/SASDAB2.exp)

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

SET (FIRTESTS "bodies-0196-hollow-sphere;bodies-lyz-ellipsoid;dammif-lyz;gasbori-abcdef;gasborp-gnlyzfu")
SET (FITTESTS "bunch-gstdh2;crysol-4mld00;cryson-tr00;dammif-p92;dammin-lyz;em2dam-rubisco;gajoe-profiles_001;gasbormx-gts;globsymm-pdc01;mixture-test2;mixture-test3;oligomer-m01;sasrefmx-mical1mx15")
foreach (test ${FIRTESTS})
  add_test (NAME read-fir-${test}
            COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/${test}.fir
                                           ${CMAKE_CURRENT_SOURCE_DIR}/${test}.exp)
endforeach (test)
foreach (test ${FITTESTS})
  add_test (NAME read-fit-${test}
            COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/${test}.fit
                                           ${CMAKE_CURRENT_SOURCE_DIR}/${test}.exp)
endforeach (test)

# For the time being, expect these tests to fail
# TODO: add code to read these headers too
set_tests_properties(read-fir-gasborp-gnlyzfu
                     read-fit-em2dam-rubisco
                     read-fit-globsymm-pdc01
                     read-fit-mixture-test2
                     read-fit-mixture-test3
                     read-fit-oligomer-m01
                     PROPERTIES WILL_FAIL 1)

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
