include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_events_xdp_SRCS
     xdpevents.cpp
     xdpeventsplugin.cpp
)

qt5_add_dbus_interface(
   krfb_events_xdp_SRCS
   ${CMAKE_SOURCE_DIR}/dbus/xdp_dbus_remotedesktop_interface.xml
   xdp_dbus_remotedesktop_interface
)

add_library (krfb_events_xdp MODULE ${krfb_events_xdp_SRCS})

target_link_libraries (krfb_events_xdp
                       KF5::CoreAddons
                       KF5::I18n
                       Qt5::DBus
                       krfbprivate
)
set_target_properties(krfb_events_xdp PROPERTIES OUTPUT_NAME xdp)

install (TARGETS krfb_events_xdp
         DESTINATION ${KDE_INSTALL_PLUGINDIR}/krfb/events
)
