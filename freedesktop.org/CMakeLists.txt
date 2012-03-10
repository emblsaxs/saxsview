
if (UNIX)
  # These have no meaning on Windows.
  install(FILES saxsview.xml
          DESTINATION share/mime/packages)

  install(FILES svimage.desktop svplot.desktop
          DESTINATION share/applications)
endif (UNIX)
