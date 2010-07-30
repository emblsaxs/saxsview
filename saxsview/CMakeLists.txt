include_directories (${QWT_SOURCE_DIR}
                     ${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSVIEW_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR})

link_directories (${LIBSAXSDOCUMENT_BINARY_DIR}
                  ${LIBSAXSVIEW_BINARY_DIR})

# run moc
qt4_wrap_cpp (saxsview_moc saxsviewmainwindow.h
                           saxsviewsubwindow.h
                           saxsviewimagewindow.h
                           saxsviewplotwindow.h
                           selectplotwindowdialog.h)

qt4_wrap_ui (saxsview_uic selectplotwindowdialog.ui)

qt4_add_resources (saxsview_res saxsview.qrc)


configure_file (config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_executable (saxsviewapp
                main.cpp
                ${saxsview_moc}
                ${saxsview_uic}
                ${saxsview_res}
                saxsviewmainwindow.cpp saxsviewmainwindow.h
                saxsviewsubwindow.cpp saxsviewsubwindow.h
                saxsviewplotwindow.cpp saxsviewplotwindow.h
                saxsviewimagewindow.cpp saxsviewimagewindow.h
                selectplotwindowdialog.cpp selectplotwindowdialog.h)

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
                       saxsview
                       saxsdocument
                       saxsimage)

install (TARGETS saxsviewapp
         RUNTIME DESTINATION bin)
