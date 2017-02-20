project (QTPROPERTYBROWSER CXX)

set (SOURCES qtbuttonpropertybrowser.cpp
             qtpropertybrowser.cpp
             qttreepropertybrowser.cpp
             qteditorfactory.cpp
             qtpropertybrowserutils.cpp
             qtvariantproperty.cpp
             qtgroupboxpropertybrowser.cpp
             qtpropertymanager.cpp)

#
# The QtSolutions are written as if being an internal part
# of the Qt libraries and expect to be compiled with qmake.
#
# Thus, here we can not rely on `qt4_wrap_cpp` as usual, but
# need to put in a bit more effort and create the files the
# same way as qmake would.
#
# Note to self: never build Qt with cmake ;)
#
MACRO(special_moc infile outfile dependfile)
  qt4_generate_moc (${infile} ${outfile})

  get_source_file_property (DEPENDS ${dependfile} OBJECT_DEPENDS)
  if (DEPENDS)
    set (DEPENDS ${DEPENDS}
                 ${QTPROPERTYBROWSER_BINARY_DIR}/${outfile})
  else(DEPENDS)
    set (DEPENDS ${QTPROPERTYBROWSER_BINARY_DIR}/${outfile})
  endif(DEPENDS)

  set_source_files_properties (${dependfile} PROPERTIES
                               OBJECT_DEPENDS "${DEPENDS}")
ENDMACRO(special_moc infile outfile dependfile)

special_moc (qtbuttonpropertybrowser.h   moc_qtbuttonpropertybrowser.cpp   qtbuttonpropertybrowser.cpp)
special_moc (qtpropertybrowser.h         moc_qtpropertybrowser.cpp         qtpropertybrowser.cpp)
special_moc (qttreepropertybrowser.h     moc_qttreepropertybrowser.cpp     qttreepropertybrowser.cpp)
special_moc (qtpropertybrowserutils_p.h  moc_qtpropertybrowserutils_p.cpp  qtpropertybrowser.h)
special_moc (qteditorfactory.h           moc_qteditorfactory.cpp           qteditorfactory.cpp)
special_moc (qtvariantproperty.h         moc_qtvariantproperty.cpp         qtvariantproperty.cpp)
special_moc (qtgroupboxpropertybrowser.h moc_qtgroupboxpropertybrowser.cpp qtgroupboxpropertybrowser.cpp)
special_moc (qtpropertymanager.h         moc_qtpropertymanager.cpp         qtpropertymanager.cpp)

special_moc (qteditorfactory.cpp         qteditorfactory.moc               qteditorfactory.cpp)
special_moc (qtpropertymanager.cpp       qtpropertymanager.moc             qtpropertymanager.cpp)
special_moc (qttreepropertybrowser.cpp   qttreepropertybrowser.moc         qttreepropertybrowser.cpp)

qt4_add_resources (RES qtpropertybrowser.qrc)


add_shared_library (qtpropertybrowser
                    SOURCES ${SOURCES} ${RES} moc_qtpropertybrowserutils_p.cpp
                    LIBRARIES ${QT_LIBRARIES}
                    VERSION 2.5.1)
target_include_directories(qtpropertybrowser PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}"
                                             PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

install_library (TARGETS qtpropertybrowser)

