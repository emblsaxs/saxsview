
project (LIBSAXSVIEW CXX)

set (SOURCES saxsview_plot.cpp
             saxsview_plotcurve.cpp
             saxsview_image.cpp
             saxsview_colormap.cpp
             saxsview_config.cpp
             saxsview_property.cpp
             saxsview_scaledraw.cpp
             saxsview_transformation.cpp)

set (HEADERS saxsview.h
             saxsview_plot.h
             saxsview_plotcurve.h
             saxsview_image.h
             saxsview_colormap.h
             saxsview_config.h
             saxsview_property.h
             saxsview_scaledraw.h
             saxsview_transformation.h)

qt4_wrap_cpp (MOC ${HEADERS})

qt4_add_resources (RES resources/saxsview.qrc)



add_shared_library (saxsview
                    SOURCES ${SOURCES}  ${MOC} ${RES}
                    LIBRARIES saxsdocument saxsimage muparser
                              qwt qtpropertybrowser ${QT_LIBRARIES}
                    VERSION 1)
target_include_directories(saxsview PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
                                    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

install_library (TARGETS saxsview)

