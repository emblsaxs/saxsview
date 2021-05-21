#
# System specific settings ...
#

install (CODE "message(\"-- Post-intall script:\")")
include (${CMAKE_SYSTEM_NAME}/postinstall)

