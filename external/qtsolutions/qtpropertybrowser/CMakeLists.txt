project (QTPROPERTYBROWSER CXX)

find_package(Qt5 5.12.0 COMPONENTS Widgets Concurrent PrintSupport Svg OpenGL REQUIRED)

include_directories (${QTPROPERTYBROWSER_BINARY_DIR})

set (HEADERS qtbuttonpropertybrowser.h
             qtpropertybrowser.h
             qttreepropertybrowser.h
             qteditorfactory.h
             qtpropertybrowserutils_p.h
             qtvariantproperty.h
             qtgroupboxpropertybrowser.h
             qtpropertymanager.h)

set (SOURCES qtbuttonpropertybrowser.cpp
             qtpropertybrowser.cpp
             qttreepropertybrowser.cpp
             qteditorfactory.cpp
             qtpropertybrowserutils.cpp
             qtvariantproperty.cpp
             qtgroupboxpropertybrowser.cpp
             qtpropertymanager.cpp)


add_library(qtpropertybrowser SHARED ${HEADRS} ${SOURCES} qtpropertybrowser.qrc)
target_link_libraries(qtpropertybrowser Qt5::Widgets Qt5::Concurrent Qt5::PrintSupport Qt5::Svg Qt5::OpenGL)

set_target_properties(qtpropertybrowser PROPERTIES
                      AUTOMOC On
                      VERSION 2.5.1)

install_library (TARGETS qtpropertybrowser)
