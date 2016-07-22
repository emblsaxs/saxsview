if (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)

  add_test(NAME "pysaxsdocument"
           WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
           COMMAND "${PYTHON_EXECUTABLE}" test_pysaxsdocument.py
  )

  set_tests_properties("pysaxsdocument" PROPERTIES
                       ENVIRONMENT "PYTHONPATH=${SAXSVIEW_BINARY_DIR}/libsaxsdocument/python")

endif (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)