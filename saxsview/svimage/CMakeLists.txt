
set (svimage_sources main.cpp
                     svimagemainwindow.cpp
                     svimagesubwindow.cpp
                     svimagepropertydockwidget.cpp
                     svimagemaskthresholdsdialog.cpp)

add_application (svimage
                 SOURCES ${svimage_sources}
                 LIBRARIES saxsview ${QT_LIBRARIES} m
                 GUI)
set_target_properties(svimage PROPERTIES AUTOMOC On)

install_application (TARGETS svimage)
install_dependencies (svimage)
