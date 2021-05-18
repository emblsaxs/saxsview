
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

set (RES resources/saxsview.qrc)

# out-of-source builds need to include the ${CMAKE_CURRENT_*_DIR}
include_directories (${LIBSAXSDOCUMENT_SOURCE_DIR}
                     ${LIBSAXSIMAGE_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${QWT_SOURCE_DIR}
                     ${QTPROPERTYBROWSER_SOURCE_DIR}
                     ${MUPARSER_SOURCE_DIR})

add_shared_library (saxsview
                    SOURCES ${SOURCES} ${HEADERS} ${RES}
                    LIBRARIES saxsdocument saxsimage muparser
                              qwt qtpropertybrowser Qt5::Widgets Qt5::PrintSupport
                    VERSION 5)

install_library (TARGETS saxsview)

