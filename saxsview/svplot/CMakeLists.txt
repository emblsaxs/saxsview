
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

qt4_wrap_cpp (svplot_moc ${svplot_headers})

add_application (svplot
                 SOURCES ${svplot_headers} ${svplot_moc} ${svplot_sources}
                 LIBRARIES saxsview saxsdocument m
                 GUI)

install_application (TARGETS svplot)
install_dependencies (svplot)

