
find_package (HDF5)
if (HDF5_FOUND)

  project (H5ZLZ4 C)

  set (H5ZLZ4_HEADERS lz4.h)
  set (H5ZLZ4_SOURCES lz4.c h5zlz4.c)

  add_definitions (-DHAVE_HDF5 -std=gnu99)
  add_shared_library (h5zlz4
                      SOURCES ${H5ZLZ4_HEADERS} ${H5ZLZ4_SOURCES}
                      LIBRARIES 
                      VERSION 0.2)

  install_library (TARGETS h5zlz4)
endif (HDF5_FOUND)
