project (LIBSAXSVIEW CXX)

set (SOURCES saxsview_plot.cpp
             saxsview_plotcurve.cpp
             saxsview_plotconfigdialog.cpp
             saxsview_image.cpp
	     saxsview_colorbutton.cpp)

set (HEADERS saxsview_plot.h
             saxsview_plotcurve.h
             saxsview_plotconfigdialog.h
             saxsview_image.h
	     saxsview_colorbutton.h)

qt4_wrap_cpp (MOC saxsview_plot.h
                  saxsview_plotcurve.h
                  saxsview_plotconfigdialog.h
                  saxsview_image.h
	          saxsview_colorbutton.h)


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
install (FILES ${HEADERS}
         DESTINATION include/saxsview)
