
include_directories (${QWT_SOURCE_DIR}
                     ${QTPROPERTYBROWSER_SOURCE_DIR}
                     ${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR}
                     ${LIBSAXSVIEW_SOURCE_DIR})

set (svplot_header svplotmainwindow.h
                   svplotsubwindow.h
                   svplotfiledockwidget.h
                   svplotfilebrowserdockwidget.h
                   svplotpropertydockwidget.h
                   svplotproject.h)

set (svplot_sources main.cpp
                    svplotmainwindow.cpp
                    svplotsubwindow.cpp
                    svplotfiledockwidget.cpp
                    svplotfilebrowserdockwidget.cpp
                    svplotpropertydockwidget.cpp
                    svplotproject.cpp)

qt4_wrap_cpp (svplot_moc ${svplot_header})


add_executable (svplot
                ${svplot_moc}
                ${svplot_uic}
                ${svplot_header}
                ${svplot_sources})

if (WIN32)
  set_target_properties (svplot PROPERTIES
                         WIN32_EXECUTABLE TRUE)
endif (WIN32)

target_link_libraries (svplot
                       saxsview
                       saxsimage)

install (TARGETS svplot
         RUNTIME DESTINATION bin)
