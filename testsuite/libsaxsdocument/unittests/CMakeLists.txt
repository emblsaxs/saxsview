
include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR})

add_executable (test_columns test_columns.c)
target_link_libraries (test_columns saxsdocument)

add_test(NAME test_columns
         COMMAND $<TARGET_FILE:test_columns>)
set_tests_properties(test_columns PROPERTIES
                     TIMEOUT 1) # should finish in under 1 second
