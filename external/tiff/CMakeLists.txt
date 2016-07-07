
find_package (TIFF)

if (TIFF_FOUND)
  set (LIBTIFF_LIBRARIES   ${TIFF_LIBRARY}     CACHE FILEPATH "")

else (TIFF_FOUND)

  find_package (JPEG QUIET)
  find_package (ZLIB QUIET)

  project(LIBTIFF C)

  set (LIBTIFF_SOURCES tif_write.c tif_warning.c
                       tif_version.c tif_tile.c tif_thunder.c tif_swab.c
                       tif_strip.c tif_read.c tif_print.c tif_predict.c
                       tif_packbits.c tif_open.c 
                       tif_next.c tif_lzw.c tif_luv.c 
                       tif_getimage.c tif_flush.c tif_fax3sm.c tif_fax3.c
                       tif_extension.c tif_error.c tif_dumpmode.c tif_dirwrite.c
                       tif_dirread.c tif_dirinfo.c tif_dir.c tif_compress.c
                       tif_color.c tif_codec.c tif_close.c tif_aux.c)

  set (LIBTIFF_EXTRA_LIBS)

  if (JPEG_FOUND)
    set (LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_ojpeg.c tif_jpeg.c)
  endif (JPEG_FOUND)

  if (ZLIB_FOUND)
    set (LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_zip.c tif_pixarlog.c)
    set (LIBTIFF_EXTRA_LIBS ${LIBTIFF_EXTRA_LIBS} ${ZLIB_LIBRARIES})
  endif (ZLIB_FOUND)

  if (WIN32)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
      configure_file (tif_config.win32.h tif_config.h COPYONLY)
      configure_file (tiffconf.win32.h tiffconf.h COPYONLY)

    elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      configure_file (tif_config.win64.h tif_config.h COPYONLY)
      configure_file (tiffconf.win64.h tiffconf.h COPYONLY)
    endif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)

    set(LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_win32.c)

  elseif (APPLE)
    configure_file (tif_config.apple.h tif_config.h COPYONLY)
    configure_file (tiffconf.apple.h tiffconf.h COPYONLY)

    # Use tif_unix.c, not tif_apple.c, the latter results in
    # some strange compile errors?! Works anyway.
    set(LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_unix.c)
    set (LIBTIFF_HEADERS ${LIBTIFF_HEADERS} tif_config.h)

  elseif (UNIX)
    set (LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_unix.c)
  endif(WIN32)

  #
  # This target should be called 'tiff', but on MacOS, while there is no system
  # libtiff cmake could find, there is libTIFF in the ApplicationServices framework
  # ImageIO. Trying to use that libTIFF and this libtiff together results in all
  # kinds of linker errors. Instead of resolving those, it's much easier to name
  # this libtiff libtiff3 instead and all works.
  #
  add_shared_library (tiff3
                      SOURCES   ${LIBTIFF_SOURCES}
                      LIBRARIES ${LIBTIFF_EXTRA_LIBS}
                      VERSION 3.9.6)
  target_include_directories(tiff3 PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}"
                                   PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

  install_library (TARGETS tiff3)

  set (LIBTIFF_LIBRARIES   tiff3               CACHE FILEPATH "")

endif (TIFF_FOUND)

mark_as_advanced (LIBTIFF_LIBRARIES LIBTIFF_INCLUDE_DIR)

