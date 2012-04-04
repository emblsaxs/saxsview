
add_definitions (-DQT_DLL)

set (MINGW_LINKER_FLAGS "-Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc -Wl,--allow-multiple-definition")
set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${MINGW_LINKER_FLAGS}")
set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${MINGW_LINKER_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} ${MINGW_LINKER_FLAGS}")
