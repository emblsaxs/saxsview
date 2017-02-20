
find_package (HDF5)
# Check for a sufficiently recent version of HDF5
# i.e. with the H5PLextern.h header file
find_file (HDF5_PLUGIN_HEADER
           "H5PLextern.h"
           PATHS ${HDF5_INCLUDE_DIRS}
           NO_DEFAULT_PATH)
if (HDF5_FOUND AND (NOT HDF5_PLUGIN_HEADER STREQUAL "HDF5_PLUGIN_HEADER-NOTFOUND"))
  project (H5ZLZ4 C)

  set (H5ZLZ4_SOURCES lz4.c h5zlz4.c)

  add_library(h5zlz4 SHARED ${H5ZLZ4_SOURCES})
  set_target_properties(h5zlz4 PROPERTIES VERSION 0.2)
  if (CMAKE_VERSION VERSION_LESS 3.1)
    target_compile_options(h5zlz4 PRIVATE --std=gnu99)
  else()
    set_target_properties(h5zlz4 PROPERTIES C_STANDARD 99)
  endif()

  target_link_libraries(h5zlz4 PUBLIC ${HDF5_LIBRARIES})
  target_include_directories(h5zlz4 PUBLIC ${HDF5_INCLUDE_DIRS})

  install_library (TARGETS h5zlz4)
endif (HDF5_FOUND AND (NOT HDF5_PLUGIN_HEADER STREQUAL "HDF5_PLUGIN_HEADER-NOTFOUND"))
