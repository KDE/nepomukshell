include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${NEPOMUK_INCLUDE_DIR}
)

kde4_add_executable(resourceinputtest
  resourceinputwidgettest.cpp
  ../resourceinputwidget.cpp
  ../resourcecompletion.cpp
  ../completion.cpp
  ../completionitem.cpp
  ../completionmodel.cpp)

target_link_libraries(resourceinputtest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${NEPOMUK_LIBRARIES}
  )


set(semdesktest_SRCS
   main.cpp
   semdesktest.cpp
 )

kde4_add_executable(semdesktest ${semdesktest_SRCS})
target_link_libraries(semdesktest pimo ${KDE4_KDEUI_LIBS})
install(TARGETS semdesktest DESTINATION ${BIN_INSTALL_DIR} )
install( FILES semdesktest.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES semdesktest_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/semdesktest )
