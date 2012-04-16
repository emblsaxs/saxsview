
foreach (plist svplot.plist svimage.plist)
  configure_file(${plist}.in ${CMAKE_CURRENT_BINARY_DIR}/${plist} @ONLY)
endforeach (plist)
