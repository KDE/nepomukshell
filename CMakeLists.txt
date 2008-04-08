project(pimoshell)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${NEPOMUK_INCLUDES}
)

set( pimoshell_BIN_SRCS
  main.cpp
  mainwindow.cpp
  pimo.cpp
  pimoitemmodel.cpp
  pimomodel.cpp
  newclassdialog.cpp
  nrlmodel.cpp
  resourcepropertymodel.cpp
  resourcecompletion.cpp
  resourceinputwidget.cpp
  resourcepropertydelegate.cpp
  completion.cpp
  completionitem.cpp
  completionmodel.cpp
  knx.cpp
)

kde4_add_ui_files(pimoshell_BIN_SRCS mainwindow.ui)
kde4_add_ui_files(pimoshell_BIN_SRCS newclassdialog.ui)

kde4_add_executable(pimoshell ${pimoshell_BIN_SRCS})

target_link_libraries(pimoshell
  nepomukvisualization
  ${NEPOMUK_LIBRARIES}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
)

configure_file(pimo.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/pimo.desktop)

install(TARGETS pimoshell DESTINATION ${BIN_INSTALL_DIR})
install(FILES pimoshell.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  pimo.rdfs
  ${CMAKE_CURRENT_BINARY_DIR}/pimo.desktop
  DESTINATION
  ${DATA_INSTALL_DIR}/nepomuk/ontologies
)

add_subdirectory(test)
