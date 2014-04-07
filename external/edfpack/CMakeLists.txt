
find_package (ZLIB QUIET)

if (ZLIB_FOUND)
  project (EDFPACK C)

  set (EDFPACK_HEADERS angle.h arc.h bslio.h edfio.h filename.h gamma.h
                       gauss.h ipol.h isotime.h numio.h poisson.h polarization.h
                       project.h readascii.h reference.h waxs.h statfunc.h
                       rot3d.h tilt3d.h sx.h strlib.h raster.h cmpr.h r2t.h)


  set (EDFPACK_SOURCES angle.c arc.c bslio.c edfio.c filename.c gamma.c
                       gauss.c ipol.c isotime.c numio.c poisson.c polarization.c
                       project.c readascii.c waxs.c r2t.c rot3d.c tilt3d.c
                       sx.c strlib.c raster.c cmpr.c statfunc.c)

  include_directories(${ZLIB_INCLUDE_DIRS})

  add_shared_library (edf
                      SOURCES ${EDFPACK_HEADERS} ${EDFPACK_SOURCES}
                      LIBRARIES m ${ZLIB_LIBRARIES}
                      VERSION 2.233)
  install_library (TARGETS edf)

else (ZLIB_FOUND)
  message (STATUS "Optional package zlib not found, EDF image format disabled.")

endif (ZLIB_FOUND)

