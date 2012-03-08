include_directories (${QWT_SOURCE_DIR}
                     ${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR})

set (saxsview_headers saxsviewmainwindow.h
                      saxsviewsubwindow.h
                      saxsviewimagewindow.h
                      saxsviewplotwindow.h
                      selectplotwindowdialog.h
                      saxsview_configdialog.h
                      saxsview_configpage.h
                      saxsview_configpage_curve.h
                      saxsview_configpage_defaultcolors.h
                      saxsview_configpage_curvetemplate.h
                      saxsview_symbolstylecombo.h
                      saxsview_legendpositioncombo.h
                      saxsview_linestylecombo.h
                      saxsview_colorbutton.h
                      saxsview_plot.h
                      saxsview_plotcurve.h
                      saxsview_image.h
                      saxsview_config.h)

set (saxsview_sources main.cpp
                      saxsviewmainwindow.cpp
                      saxsviewsubwindow.cpp
                      saxsviewplotwindow.cpp
                      saxsviewimagewindow.cpp
                      selectplotwindowdialog.cpp
                      saxsview_configdialog.cpp
                      saxsview_configpage.cpp
                      saxsview_configpage_plot.cpp
                      saxsview_configpage_curve.cpp
                      saxsview_configpage_legend.cpp
                      saxsview_configpage_curvetemplate.cpp
                      saxsview_configpage_defaultcolors.cpp
                      saxsview_symbolstylecombo.cpp
                      saxsview_legendpositioncombo.cpp
                      saxsview_linestylecombo.cpp
                      saxsview_colorbutton.cpp
                      saxsview_plot.cpp
                      saxsview_plotcurve.cpp
                      saxsview_image.cpp
                      saxsview_config.cpp)

# run moc
qt4_wrap_cpp (saxsview_moc ${saxsview_headers})

qt4_wrap_ui (saxsview_uic selectplotwindowdialog.ui
                          saxsview_configpage_plot.ui
                          saxsview_configpage_curve.ui
                          saxsview_configpage_legend.ui
                          saxsview_configpage_curvetemplate.ui
                          saxsview_configpage_defaultcolors.ui)

qt4_add_resources (saxsview_res saxsview.qrc)

add_executable (saxsviewapp
                ${saxsview_moc}
                ${saxsview_uic}
                ${saxsview_res}
                ${saxsview_sources}
                ${saxsview_headers})

if (WIN32)
  set_target_properties (saxsviewapp PROPERTIES
                         WIN32_EXECUTABLE TRUE)
endif (WIN32)

#
# renaming a target on install does not work, hence redefine the output
# name to get a saxsview[.exe] application.
#
set_target_properties (saxsviewapp PROPERTIES
                       OUTPUT_NAME saxsview)


target_link_libraries (saxsviewapp
                       saxsdocument
                       saxsimage 
                       qwt ${QT_LIBRARIES})

install (TARGETS saxsviewapp
         RUNTIME DESTINATION bin)
