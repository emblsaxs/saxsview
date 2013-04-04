
include_directories (${QWT_SOURCE_DIR}
                     ${QTPROPERTYBROWSER_SOURCE_DIR}
                     ${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR}
                     ${LIBSAXSVIEW_SOURCE_DIR})

set (svimage_headers svimagemainwindow.h
                     svimagesubwindow.h
                     svimagepropertydockwidget.h
                     svimagemaskthresholdsdialog.h)

set (svimage_sources main.cpp
                     svimagemainwindow.cpp
                     svimagesubwindow.cpp
                     svimagepropertydockwidget.cpp
                     svimagemaskthresholdsdialog.cpp)

qt4_wrap_cpp (svimage_moc ${svimage_headers})

add_application (svimage
                 SOURCES ${svimage_headers} ${svimage_sources} ${svimage_moc}
                 LIBRARIES saxsview ${QT_LIBRARIES} m
                 GUI)

install_application (TARGETS svimage)
install_dependencies (svimage)
