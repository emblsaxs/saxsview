
add_python_module (api
                   SOURCES api.c
                   LIBRARIES saxsdocument)


install_python_module (TARGETS api
                       FILES __init__.py
                            implementation.py 
	                   COMPONENT saxsdocument)
