project (LIBSAXSIMAGE C)

set (SOURCES saxsimage.c
             saxsimage_format.c
             cbf.c
             msk.c
             tiff.c)

set (LIBRARIES saxsdocument cbf)


# We always have TIFF.
add_definitions (-DHAVE_TIFF)
set (LIBRARIES ${LIBRARIES} ${LIBTIFF_LIBRARIES})

# conditional sources
if (TARGET edf)
  add_definitions (-DHAVE_EDF)
  set (LIBRARIES ${LIBRARIES} edf)
  set (SOURCES ${SOURCES} edf.c)
endif (TARGET edf)

if (TARGET h5zlz4)
  add_definitions (-DHAVE_HDF5)
  set (SOURCES ${SOURCES} hdf5.c)
  # N.B. not sure if this is the right thing to do
  # perhaps h5zlz4 should not be linked but rather loaded as a plugin
  set (LIBRARIES ${LIBRARIES} h5zlz4)
endif (TARGET h5zlz4)


add_shared_library (saxsimage
                    SOURCES ${SOURCES}
                    LIBRARIES ${LIBRARIES}
                    VERSION 1)
target_include_directories(saxsimage PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install_library (TARGETS saxsimage)
