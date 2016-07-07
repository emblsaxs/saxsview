
find_package (ZLIB QUIET)

if (ZLIB_FOUND)
  project (EDFPACK C)

  set (EDFPACK_SOURCES angle.c arc.c bslio.c edfio.c filename.c gamma.c
                       gauss.c ipol.c isotime.c numio.c poisson.c polarization.c
                       project.c readascii.c waxs.c r2t.c rot3d.c tilt3d.c
                       sx.c strlib.c raster.c cmpr.c statfunc.c)

  add_library(edf SHARED ${EDFPACK_SOURCES})
  set_target_properties(edf PROPERTIES VERSION 2.233)
  target_link_libraries(edf PRIVATE m ${ZLIB_LIBRARIES})
  target_include_directories(edf PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

  install_library (TARGETS edf)

else (ZLIB_FOUND)
  message (STATUS "Optional package zlib not found, EDF image format disabled.")

endif (ZLIB_FOUND)

