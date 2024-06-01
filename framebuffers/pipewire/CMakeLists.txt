include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_pw_SRCS
     pw_framebuffer.cpp
     pw_framebufferplugin.cpp

     screencasting.cpp
)

ecm_qt_declare_logging_category(krfb_framebuffer_pw_SRCS
    HEADER krfb_fb_pipewire_debug.h
    IDENTIFIER KRFB_FB_PIPEWIRE
    CATEGORY_NAME krfb.framebuffer.pipewire
    DESCRIPTION "KRFB PipeWire framebuffer plugin"
    EXPORT KRFB
)

qt_add_dbus_interface(
   krfb_framebuffer_pw_SRCS
   ${CMAKE_SOURCE_DIR}/dbus/xdp_dbus_screencast_interface.xml
   xdp_dbus_screencast_interface
)

qt_add_dbus_interface(
   krfb_framebuffer_pw_SRCS
   ${CMAKE_SOURCE_DIR}/dbus/xdp_dbus_remotedesktop_interface.xml
   xdp_dbus_remotedesktop_interface
)

add_library(krfb_framebuffer_pw
    MODULE
    ${krfb_framebuffer_pw_SRCS}
)

if (Qt6_VERSION VERSION_LESS "6.7.1")
    ecm_add_qtwayland_client_protocol(krfb_framebuffer_pw
        PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/screencast.xml
        BASENAME zkde-screencast-unstable-v1
    )
else()
    qt6_generate_wayland_protocol_client_sources(krfb_framebuffer_pw
        FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/screencast.xml
    )
endif()

if (KPipeWire_VERSION VERSION_LESS "6.0.80")
    target_compile_definitions(krfb_framebuffer_pw PRIVATE -DKPIPEWIRE60=1)
else()
    target_compile_definitions(krfb_framebuffer_pw PRIVATE -DKPIPEWIRE60=0)
endif()
set_property(TARGET krfb_framebuffer_pw PROPERTY C_STANDARD 99)

target_link_libraries(krfb_framebuffer_pw
                      Qt::Core
                      Qt::Gui
                      Qt::DBus
                      KF6::CoreAddons
                      KF6::ConfigCore
                      PkgConfig::PipeWire
                      Plasma::KWaylandClient
                      Wayland::Client
                      krfbprivate
                      K::KPipeWire
                      K::KPipeWireDmaBuf
)

set_target_properties(krfb_framebuffer_pw PROPERTIES OUTPUT_NAME pw)
install (TARGETS krfb_framebuffer_pw
         DESTINATION ${KDE_INSTALL_PLUGINDIR}/krfb/framebuffer
)
