
if (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR})

  add_python_module (pysaxsdocument
                     SOURCES pysaxsdocument.c
                     LIBRARIES saxsdocument)

  #
  # If this would be dropped into the normal runtime output directory,
  # the actual libsaxsdocument would be overwritten.
  #
  set_target_properties (pysaxsdocument PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                         OUTPUT_NAME saxsdocument)

  install_python_module (TARGETS pysaxsdocument)

else (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  message (STATUS "Note: Python not found, extension module 'saxsdocument' will not be built.")

endif (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
