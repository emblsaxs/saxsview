# This directory contains files that caused previous versions of libsaxsdocument to crash
# when trying to read them

add_test (NAME libsaxsdocument-crashtest-01
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash01.dat")

add_test (NAME libsaxsdocument-crashtest-02
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash02.dat")

add_test (NAME libsaxsdocument-crashtest-03
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash03.dat")

add_test (NAME libsaxsdocument-crashtest-04
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash04.txt")

add_test (NAME libsaxsdocument-crashtest-05
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash05.txt")

add_test (NAME libsaxsdocument-crashtest-06
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash06.txt")

add_test (NAME libsaxsdocument-crashtest-07
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash07.fit")

add_test (NAME libsaxsdocument-crashtest-08
          COMMAND $<TARGET_FILE:readtest> "${CMAKE_CURRENT_SOURCE_DIR}/crash08.fit")
