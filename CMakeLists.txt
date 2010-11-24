project(nepomukshell)

##################  Nepomukshell version  ################################
set(CMAKE_NEPOMUKSHELL_VERSION_MAJOR 0)
set(CMAKE_NEPOMUKSHELL_VERSION_MINOR 7)
set(CMAKE_NEPOMUKSHELL_VERSION_RELEASE 60)
set(CMAKE_NEPOMUKSHELL_VERSION_STRING "${CMAKE_NEPOMUKSHELL_VERSION_MAJOR}.${CMAKE_NEPOMUKSHELL_VERSION_MINOR}.${CMAKE_NEPOMUKSHELL_VERSION_RELEASE}")


set(KDE_MIN_VERSION "4.5.80")  # for the < 4.2 macro
find_package(KDE4 4.5.80 REQUIRED)
find_package(Nepomuk REQUIRED)

include(KDE4Defaults)
include(SopranoAddOntology)

configure_file(nepomukshell-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/nepomukshell-config.h)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_INCLUDE_DIR}
  ${nepomukshell_SOURCE_DIR}/settings
  ${nepomukshell_BINARY_DIR}
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
