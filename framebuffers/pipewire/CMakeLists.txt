include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_pw_SRCS
     pw_framebuffer.cpp
     pw_framebufferplugin.cpp
)


ecm_qt_declare_logging_category(krfb_framebuffer_pw_SRCS
    HEADER krfb_fb_pipewire_debug.h
    IDENTIFIER KRFB_FB_PIPEWIRE
    CATEGORY_NAME krfb.framebuffer.pipewire
    DESCRIPTION "KRFB PipeWire framebuffer plugin"
    EXPORT KRFB
)

qt5_add_dbus_interface(
   krfb_framebuffer_pw_SRCS
   ${CMAKE_SOURCE_DIR}/dbus/xdp_dbus_screencast_interface.xml
   xdp_dbus_screencast_interface
)

qt5_add_dbus_interface(
   krfb_framebuffer_pw_SRCS
   ${CMAKE_SOURCE_DIR}/dbus/xdp_dbus_remotedesktop_interface.xml
   xdp_dbus_remotedesktop_interface
)

add_library(krfb_framebuffer_pw
    MODULE
    ${krfb_framebuffer_pw_SRCS}
)

target_link_libraries (krfb_framebuffer_pw
                       Qt5::Core
                       Qt5::Gui
                       Qt5::DBus
                       KF5::CoreAddons
                       krfbprivate
                       PipeWire::PipeWire
)

install (TARGETS krfb_framebuffer_pw
         DESTINATION ${PLUGIN_INSTALL_DIR}/krfb
)
