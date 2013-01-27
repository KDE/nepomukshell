project(nepomukshell)

##################  Nepomukshell version  ################################
set(CMAKE_NEPOMUKSHELL_VERSION_MAJOR 0)
set(CMAKE_NEPOMUKSHELL_VERSION_MINOR 8)
set(CMAKE_NEPOMUKSHELL_VERSION_RELEASE 60)
set(CMAKE_NEPOMUKSHELL_VERSION_STRING "${CMAKE_NEPOMUKSHELL_VERSION_MAJOR}.${CMAKE_NEPOMUKSHELL_VERSION_MINOR}.${CMAKE_NEPOMUKSHELL_VERSION_RELEASE}")


find_package(KDE4 4.10 REQUIRED)
find_package(NepomukCore REQUIRED)
find_package(Soprano REQUIRED)

include(KDE4Defaults)
include(SopranoAddOntology)

configure_file(nepomukshell-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/nepomukshell-config.h)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)


include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_CORE_INCLUDE_DIR}
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

  # Utils
  utils/resourcemodel.cpp
  utils/simpleresourcemodel.cpp
)

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
  ${NEPOMUK_CORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${SOPRANO_LIBRARIES}
  nepomukutils
)

install(TARGETS nepomukshell ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS nepomukshell.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES nepomukshellui.rc DESTINATION  ${DATA_INSTALL_DIR}/nepomukshell)

add_subdirectory(doc)
