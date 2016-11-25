# This directory contains files that caused previous versions of libsaxsdocument to crash
# when trying to read them

add_test (NAME libsaxsdocument-crashtest-01
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash01.dat")

add_test (NAME libsaxsdocument-crashtest-02
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash02.dat")

add_test (NAME libsaxsdocument-crashtest-03
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash03.dat")
