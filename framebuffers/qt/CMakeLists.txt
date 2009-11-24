include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_qt_SRCS
     qtframebuffer.cpp
     qtframebufferplugin.cpp
)

kde4_add_plugin (krfb_framebuffer_qt
                 ${krfb_framebuffer_qt_SRCS}
)

target_link_libraries (krfb_framebuffer_qt
                       ${QT_QTCORE_LIBRARY}
                       ${QT_QTGUI_LIBRARY}
                       ${KDE4_KDEUI_LIBS}
                       krfbprivate
)

install (TARGETS krfb_framebuffer_qt
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES krfb_framebuffer_qt.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

