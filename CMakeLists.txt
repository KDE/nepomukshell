project(nepomukshell)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
  ${SOPRANO_INCLUDE_DIR}/soprano
  ${NEPOMUK_INCLUDE_DIR}
  ${NEPOMUK_INCLUDE_DIR}/nepomuk
  ${pimolib_SOURCE_DIR}
  ${pimolib_BINARY_DIR}
  ${nepomuk_visualization_SOURCE_DIR}
  ${kcompleter_SOURCE_DIR}
  ${nepomukcompletion_SOURCE_DIR}
)

set( nepomukshell_BIN_SRCS
  main.cpp
  mainwindow.cpp
  newclassdialog.cpp
  resourcepropertymodel.cpp
  resourceinputwidget.cpp
  resourcepropertydelegate.cpp
  asyncquery.cpp
  asyncloadingresourcemodel.cpp
)

kde4_add_ui_files(nepomukshell_BIN_SRCS mainwindow.ui)
kde4_add_ui_files(nepomukshell_BIN_SRCS newclassdialog.ui)

kde4_add_executable(nepomukshell ${nepomukshell_BIN_SRCS})

target_link_libraries(nepomukshell
  nepomukvisualization
  pimo
  nepomukcompletion
  kcompleter
  ${NEPOMUK_LIBRARIES}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${SOPRANO_LIBRARIES}
  ${SOPRANO_INDEX_LIBRARIES}
)

install(TARGETS nepomukshell DESTINATION ${BIN_INSTALL_DIR})
install(FILES nepomukshell.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES nepomukshellui.rc DESTINATION  ${DATA_INSTALL_DIR}/nepomukshell)

#add_subdirectory(test)
