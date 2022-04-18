include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_pw_SRCS
     pw_framebuffer.cpp
     pw_framebufferplugin.cpp

     screencasting.cpp
)

ecm_add_qtwayland_client_protocol(krfb_framebuffer_pw_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/screencast.xml
    BASENAME zkde-screencast-unstable-v1
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
set_property(TARGET krfb_framebuffer_pw PROPERTY C_STANDARD 99)

target_link_libraries(krfb_framebuffer_pw
                      Qt5::Core
                      Qt5::Gui
                      Qt5::DBus
                      KF5::CoreAddons
                      KF5::WaylandClient
                      Wayland::Client
                      krfbprivate
                      PkgConfig::PipeWire
)

if (HAVE_DMA_BUF)
    target_link_libraries(krfb_framebuffer_pw
                          ${epoxy_LIBRARIES}
                          gbm::gbm
    )
endif()

set_target_properties(krfb_framebuffer_pw PROPERTIES OUTPUT_NAME pw)
install (TARGETS krfb_framebuffer_pw
         DESTINATION ${KDE_INSTALL_PLUGINDIR}/krfb/framebuffer
)
