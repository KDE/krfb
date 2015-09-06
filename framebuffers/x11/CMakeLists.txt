include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_x11_SRCS
     x11framebuffer.cpp
     x11framebufferplugin.cpp
)

add_library(krfb_framebuffer_x11
 MODULE 
                 ${krfb_framebuffer_x11_SRCS}
)

target_link_libraries (krfb_framebuffer_x11
                       
                       Qt5::Core
                       Qt5::Gui
                       ${X11_X11_LIB}
                       ${X11_Xdamage_LIB}
                       ${X11_Xext_LIB}
                       KF5::KDELibs4Support
                       krfbprivate
)

install (TARGETS krfb_framebuffer_x11
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES krfb_framebuffer_x11.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

