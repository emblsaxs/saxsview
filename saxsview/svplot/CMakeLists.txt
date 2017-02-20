
set (svplot_sources main.cpp
                    svplotmainwindow.cpp
                    svplotsubwindow.cpp
                    svplotfiledockwidget.cpp
                    svplotfilebrowserdockwidget.cpp
                    svplotpropertydockwidget.cpp
                    svplotproject.cpp)

add_application (svplot
                 SOURCES ${svplot_sources}
                 LIBRARIES saxsview saxsdocument m
                 GUI)
set_target_properties(svplot PROPERTIES AUTOMOC On)

install_application (TARGETS svplot)
install_dependencies (svplot)

