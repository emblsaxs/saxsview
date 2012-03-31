project (EDFPACK C)

set (EDFPACK_HEADERS angle.h arc.h bslio.h edfio.h filename.h gamma.h
                     gauss.h ipol.h isotime.h numio.h poisson.h polarization.h
                     project.h readascii.h reference.h waxs.h
                     rot3d.h tilt3d.h sx.h strlib.h raster.h cmpr.h r2t.h)


set (EDFPACK_SOURCES angle.c arc.c bslio.c edfio.c filename.c gamma.c
                     gauss.c ipol.c isotime.c numio.c poisson.c polarization.c
                     project.c readascii.c waxs.c r2t.c rot3d.c tilt3d.c
                     sx.c strlib.c raster.c cmpr.c)

add_library (edf SHARED ${EDFPACK_SOURCES} ${EDFPACK_HEADERS})
target_link_libraries(edf m)

set_target_properties (edf PROPERTIES VERSION 2.216)

install (TARGETS edf
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)






