
if (Python_Development_FOUND)
  include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR})

  add_subdirectory(saxsdocument)

else (Python_Development_FOUND)
  message (STATUS "Note: Python not found, extension module 'saxsdocument' will not be built.")
endif (Python_Development_FOUND)
