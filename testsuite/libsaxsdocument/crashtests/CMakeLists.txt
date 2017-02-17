# This directory contains files that caused previous versions of libsaxsdocument to crash
# when trying to read them

foreach(i RANGE 1 10)
  if (${i} LESS 10)
    set(i "0${i}")
  endif()

  add_test(NAME libsaxsdoc-crashtest-${i}
           COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash${i}.txt")
  set_tests_properties(libsaxsdoc-crashtest-${i}
                       PROPERTIES TIMEOUT 5)
endforeach()
