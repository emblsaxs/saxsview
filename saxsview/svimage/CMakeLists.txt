
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

add_application (svimage
                 SOURCES ${svimage_headers} ${svimage_sources}
                 LIBRARIES saxsview
                 GUI)

install_application (TARGETS svimage)
