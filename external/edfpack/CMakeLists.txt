project (EDFPACK C)

set (EDFPACK_HEADERS angle.h arc.h bslio.h edfio.h filename.h gamma.h
                     gauss.h ipol.h isotime.h numio.h poisson.h polarization.h
                     project.h readascii.h reference.h waxs.h)

set (EDFPACK_SOURCES angle.c arc.c bslio.c edfio.c filename.c gamma.c
                     gauss.c ipol.c isotime.c numio.c poisson.c polarization.c
                     project.c readascii.c waxs.c)

add_library (edf SHARED ${EDFPACK_SOURCES} ${EDFPACK_HEADERS})
target_link_libraries(edf m)

set_target_properties (edf PROPERTIES VERSION 2.178)

install (TARGETS edf
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)




