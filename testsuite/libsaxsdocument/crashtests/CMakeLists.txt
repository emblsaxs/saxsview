# This directory contains files that caused previous versions of libsaxsdocument to crash
# when trying to read them

add_test (NAME libsaxsdocument-crashtest-01
          COMMAND $<TARGET_FILE:readtest> crash01.dat)
