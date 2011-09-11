include_directories (${QWT_SOURCE_DIR}
                     ${QTPROPERTYBROWSER_SOURCE_DIR}
                     ${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR}
                     ${LIBSAXSVIEW_SOURCE_DIR})

set (svimage_header svimagemainwindow.h
                    svimagesubwindow.h
                    svimagepropertydockwidget.h
                    svimageproperty.h)

set (svimage_sources main.cpp
                     svimagemainwindow.cpp
                     svimagesubwindow.cpp
                     svimagepropertydockwidget.cpp
                     svimageproperty.cpp)

qt4_wrap_cpp (svimage_moc ${svimage_header})


add_executable (svimage
                ${svimage_moc}
                ${svimage_uic}
                ${svimage_header}
                ${svimage_sources})

if (WIN32)
  set_target_properties (svimage PROPERTIES
                         WIN32_EXECUTABLE TRUE)
endif (WIN32)

target_link_libraries (svimage
                       qtpropertybrowser
                       saxsview
                       saxsimage)

install (TARGETS svimage
         RUNTIME DESTINATION bin)

