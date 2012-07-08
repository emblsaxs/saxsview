
find_package (TIFF)

if (TIFF_FOUND)
  set (LIBTIFF_LIBRARIES   ${TIFF_LIBRARY}     CACHE FILEPATH "")
  set (LIBTIFF_INCLUDE_DIR ${TIFF_INCLUDE_DIR} CACHE PATH     "")

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

  set (LIBTIFF_HEADERS t4.h	tif_dir.h tiff.h tiffio.h uvcode.h
                       tif_fax3.h tiffiop.h tif_predict.h tiffvers.h)

  set (LIBTIFF_EXTRA_LIBS)

  if (JPEG_FOUND)
    set (LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_ojpeg.c tif_jpeg.c)
  endif (JPEG_FOUND)

  if (ZLIB_FOUND)
    set (LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_zip.c tif_pixarlog.c)
    set (LIBTIFF_EXTRA_LIBS ${LIBTIFF_EXTRA_LIBS} ${ZLIB_LIBRARIES})
  endif (ZLIB_FOUND)

  if (WIN32)
    set(LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_win32.c)

  elseif (APPLE)
    # Carbon header files.
    include_directories (/Developer/Headers/FlatCarbon)

    configure_file (tif_config.apple.h tif_config.h COPYONLY)
    configure_file (tiffconf.apple.h tiffconf.h COPYONLY)

    set(LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_unix.c)
    set (LIBTIFF_HEADERS ${LIBTIFF_HEADERS} tif_config.h)

  elseif (UNIX)
    set (LIBTIFF_SOURCES ${LIBTIFF_SOURCES} tif_unix.c)
  endif(WIN32)


  include_directories (${LIBTIFF_BINARY_DIR})

  #
  # This target should be called 'tiff', but on MacOS, while there is no system
  # libtiff cmake could find, there is libTIFF in the ApplicationServices framework
  # ImageIO. Trying to use that libTIFF and this libtiff together results in all
  # kinds of linker errors. Instead of resolving those, it's much easier to name
  # this libtiff libtiff3 instead and all works.
  #
  add_shared_library (tiff3
                      SOURCES ${LIBTIFF_HEADERS} ${LIBTIFF_SOURCES}
                      LIBRARIES ${LIBTIFF_EXTRA_LIBS}
                      VERSION 3.9.6)

  install_library (TARGETS tiff3)

  set (LIBTIFF_LIBRARIES   tiff3                                       CACHE FILEPATH "")
  set (LIBTIFF_INCLUDE_DIR ${LIBTIFF_SOURCE_DIR} ${LIBTIFF_BINARY_DIR} CACHE PATH     "")

endif (TIFF_FOUND)

mark_as_advanced (LIBTIFF_LIBRARIES LIBTIFF_INCLUDE_DIR)

