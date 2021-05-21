
set (POSTINSTALL_SH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Darwin/postinstall.sh)
install (CODE "execute_process (COMMAND bash ${POSTINSTALL_SH} \$ENV{DESTDIR}/\${CMAKE_INSTALL_PREFIX})")
