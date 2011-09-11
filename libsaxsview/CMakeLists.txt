project (LIBSAXSVIEW CXX)

set (SOURCES saxsview_plot.cpp
             saxsview_plotcurve.cpp
             saxsview_image.cpp
             saxsview_colorbutton.cpp
             saxsview_legendpositioncombo.cpp
             saxsview_linestylecombo.cpp
             saxsview_symbolstylecombo.cpp
             saxsview_config.cpp
             saxsview_configdialog.cpp
             saxsview_configpage.cpp
             saxsview_configpage_plot.cpp
             saxsview_configpage_curve.cpp
             saxsview_configpage_legend.cpp
             saxsview_configpage_curvetemplate.cpp
             saxsview_configpage_defaultcolors.cpp)

set (HEADERS saxsview_plot.h
             saxsview_plotcurve.h
             saxsview_image.h
             saxsview_colorbutton.h
             saxsview_legendpositioncombo.h
             saxsview_linestylecombo.h
             saxsview_symbolstylecombo.h
             saxsview_config.h
             saxsview_configdialog.h
             saxsview_configpage.h
             saxsview_configpage_curvetemplate.h
             saxsview_configpage_defaultcolors.h)

qt4_wrap_cpp (MOC saxsview_plot.h
                  saxsview_plotcurve.h
                  saxsview_image.h
                  saxsview_colorbutton.h
                  saxsview_legendpositioncombo.h
                  saxsview_linestylecombo.h
                  saxsview_symbolstylecombo.h
                  saxsview_configdialog.h
                  saxsview_configpage_curve.h
                  saxsview_configpage_curvetemplate.h)

qt4_wrap_ui (UIC saxsview_configpage_plot.ui
                 saxsview_configpage_curve.ui
                 saxsview_configpage_legend.ui
                 saxsview_configpage_curvetemplate.ui
                 saxsview_configpage_defaultcolors.ui)

qt4_add_resources (RES resources/saxsview.qrc)

# out-of-source builds need to include the ${CMAKE_CURRENT_*_DIR}
include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${QWT_SOURCE_DIR})

add_library (saxsview SHARED
             ${SOURCES}
             ${HEADERS}
             ${MOC}
             ${UIC}
             ${RES})

link_directories (${LIBSAXSDOCUMENT_BINARY_DIR})

target_link_libraries (saxsview
                       saxsdocument
                       qwt
                       ${QT_LIBRARIES})

install (TARGETS saxsview
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)
