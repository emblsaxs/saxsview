

macro (saxsview_install_dependencies)
  set (PREFIX $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX})
  file (GLOB PLUGINS "${QT_PLUGINS_DIR}/imageformats/*")

  install (CODE "set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
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
                 #   http://public.kitware.com/Bug/view.php?id=13052
                 #
                 include(BundleUtilities)

                 # 
                 # The local copy of DeployQt4.cmake includes the fix for:
                 #   http://public.kitware.com/Bug/view.php?id=13051
                 #
                 include(DeployQt4)

                 FIXUP_QT4_EXECUTABLE (\"${PREFIX}/${ARGV0}.app\"
                                       \"${PLUGINS}\"
                                       \"\"
                                       \"${PREFIX}/Frameworks\"
                                       \"${PREFIX}/Frameworks/Plugins\")")

endmacro (saxsview_install_dependencies)
