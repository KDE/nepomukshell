include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${NEPOMUK_INCLUDES}
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
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${NEPOMUK_LIBRARIES}
  )
