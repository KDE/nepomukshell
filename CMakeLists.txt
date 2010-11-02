project(nepomukshell)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
  ${SOPRANO_INCLUDE_DIR}/soprano
  ${NEPOMUK_INCLUDE_DIR}
  ${NEPOMUK_INCLUDE_DIR}/nepomuk
  ${nepomukextras_SOURCE_DIR}
  ${nepomuk_kde_SOURCE_DIR}/misc
  ${nepomukshell_SOURCE_DIR}/settings
)

set( nepomukshell_BIN_SRCS
  main.cpp
  mainwindow.cpp
  newclassdialog.cpp
  resourcepropertymodel.cpp
  resourceview.cpp
  resourcebrowserwidget.cpp
  resourceeditorwidget.cpp
  resourcequerywidget.cpp
  querymodel.cpp
  infosplash.cpp
  sparqlsyntaxhighlighter.cpp
  queryeditor.cpp
  classmodel.cpp
  pimomodel.cpp
)

soprano_add_ontology(nepomukshell_BIN_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig "PIMO" "Nepomuk::Vocabulary" "trig")

kde4_add_ui_files(nepomukshell_BIN_SRCS 
  mainwindow.ui
  resourceview.ui
  resourcebrowserwidget.ui
  resourceeditorwidget.ui
  resourcequerywidget.ui
  newclassdialog.ui
  settings/resourcebrowsersettingspage.ui)

kde4_add_kcfg_files(nepomukshell_BIN_SRCS
  settings/nepomukshellsettings.kcfgc)

kde4_add_executable(nepomukshell ${nepomukshell_BIN_SRCS})

target_link_libraries(nepomukshell
  ${NEPOMUK_LIBRARIES}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${SOPRANO_LIBRARIES}
  ${NEPOMUK_QUERY_LIBRARIES}
  nepomukutils
)

install(TARGETS nepomukshell ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES nepomukshell.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES nepomukshellui.rc DESTINATION  ${DATA_INSTALL_DIR}/nepomukshell)

add_subdirectory(doc)
