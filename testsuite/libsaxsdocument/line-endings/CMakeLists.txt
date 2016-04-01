# Test that files with different line endings can be read properly
set (LINE_ENDINGS cr lf crlf)
foreach (test ${LINE_ENDINGS})
  add_test (NAME read-dat-${test}-line-endings
            COMMAND $<TARGET_FILE:doctest> ${CMAKE_CURRENT_SOURCE_DIR}/${test}.csv
                                           ${CMAKE_CURRENT_SOURCE_DIR}/data.exp)
endforeach (test)
