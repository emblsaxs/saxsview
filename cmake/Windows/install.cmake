
include (GNUInstallDirs)

macro (saxsview_install_dependencies)
  file (GLOB PLUGINS "${QT_PLUGINS_DIR}/imageformats/*.dll")
  install (CODE "set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

                 #
                 # It is necessary to evaluate this expression at runtime, otherwise
                 # `make install`would work, but not `make packge`.
                 #
                 set (PREFIX \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

                 # 
                 # The local copy of BundleUtilities.cmake includes a workaround for:
                 #   http://www.cmake.org/Bug/view.php?id=13052
                 #
                 include(BundleUtilities)

                 # 
                 # The local copy of DeployQt4.cmake includes fixes for:
                 #   http://www.cmake.org/Bug/view.php?id=13051
                 #   http://www.cmake.org/Bug/view.php?id=13129
                 #
                 include(DeployQt4)

				 #
				 # There is a difference between ´make install` and
				 # `make package` - the latter installs into a different
				 # location ...
				 #
                 FIXUP_QT4_EXECUTABLE (\"\${PREFIX}/${ARGV0}.exe\"
                                       \"${PLUGINS}\"
                                       \"\"
                                       \"\${PREFIX}\"
                                       \"plugins\")")
endmacro (saxsview_install_dependencies)
