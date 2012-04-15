

macro (saxsview_install_dependencies)
  file (GLOB PLUGINS "${QT_PLUGINS_DIR}/imageformats/*.dylib")
  install (CODE "set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

                 #
                 # It is necessary to evaluate this expression at runtime, otherwise
                 # `make install`would work, but not `make packge`.
                 #
                 set (PREFIX \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX})

                 #
                 # Implement a path override used in GetPrerequisites/BundleUtilities
                 # to specify the Framework search path relative to the location of the
                 # bundle executable.
                 #
                 function(gp_item_default_embedded_path_override item path)
                   set(path \"@executable_path/../../../Frameworks\" PARENT_SCOPE)
                 endfunction(gp_item_default_embedded_path_override)

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
                 # The location of the plugins is assumed to be relative 
                 # to the bundle contents. See also:
                 #   http://www.cmake.org/Bug/view.php?id=13133
                 #
                 FIXUP_QT4_EXECUTABLE (\"\${PREFIX}/${ARGV0}.app\"
                                       \"${PLUGINS}\"
                                       \"\"
                                       \"\${PREFIX}/Frameworks\"
                                       \"../../Frameworks/Plugins\")")

endmacro (saxsview_install_dependencies)
