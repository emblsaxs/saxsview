
if (PYTHONLIBS_FOUND)
  include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR})

  add_subdirectory(saxsdocument)

else (PYTHONLIBS_FOUND)
  message (STATUS "Note: Python not found, extension module 'saxsdocument' will not be built.")
endif (PYTHONLIBS_FOUND)
