
include_directories (${QWT_SOURCE_DIR}
                     ${QTPROPERTYBROWSER_SOURCE_DIR}
                     ${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR}
                     ${LIBSAXSVIEW_SOURCE_DIR})

set (svplot_headers svplotmainwindow.h
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

add_application (svplot
                 SOURCES ${svplot_headers} ${svplot_sources}
                 LIBRARIES saxsview saxsdocument m
                 GUI)

install_application (TARGETS svplot)
install_dependencies (svplot)

