include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_qt_SRCS
     qtframebuffer.cpp
     qtframebufferplugin.cpp
)

add_library(krfb_framebuffer_qt
 MODULE 
                 ${krfb_framebuffer_qt_SRCS}
)

target_link_libraries (krfb_framebuffer_qt
                       Qt5::Core
                       Qt5::Gui
                       KF5::KDELibs4Support
                       krfbprivate
)

install (TARGETS krfb_framebuffer_qt
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES krfb_framebuffer_qt.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

