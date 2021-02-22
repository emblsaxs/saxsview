
add_python_module (api
                   SOURCES api.c
                   LIBRARIES saxsdocument)


install_python_module (TARGETS api
	               COMPONENT saxsdocument)

install (FILES implementation.py __init__.py
         DESTINATION ${ATSAS_PYTHON_PACKAGE_DESTINATION}/saxsdocument)

