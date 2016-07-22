
find_package (HDF5)
# Check for a sufficiently recent version of HDF5
# i.e. with the H5PLextern.h header file
find_file (HDF5_PLUGIN_HEADER
           "H5PLextern.h"
           PATHS ${HDF5_INCLUDE_DIRS}
           NO_DEFAULT_PATH)
if (HDF5_FOUND AND (NOT HDF5_PLUGIN_HEADER STREQUAL "HDF5_PLUGIN_HEADER-NOTFOUND"))
  project (H5ZLZ4 C)
  include_directories(${HDF5_INCLUDE_DIRS})

  set (H5ZLZ4_HEADERS lz4.h)
  set (H5ZLZ4_SOURCES lz4.c h5zlz4.c)

  add_definitions (-DHAVE_HDF5 -std=gnu99)
  add_shared_library (h5zlz4
                      SOURCES ${H5ZLZ4_HEADERS} ${H5ZLZ4_SOURCES}
                      LIBRARIES ${HDF5_LIBRARIES}
                      VERSION 0.2)

  install_library (TARGETS h5zlz4)
endif (HDF5_FOUND AND (NOT HDF5_PLUGIN_HEADER STREQUAL "HDF5_PLUGIN_HEADER-NOTFOUND"))
