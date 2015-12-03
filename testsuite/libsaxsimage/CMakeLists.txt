include_directories(${LIBSAXSIMAGE_SOURCE_DIR})

add_executable (imgreadtest readtest.c)
target_link_libraries (imgreadtest saxsimage)
