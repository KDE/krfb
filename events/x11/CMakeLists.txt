include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_events_x11_SRCS
     x11events.cpp
     x11eventsplugin.cpp
)

add_library (krfb_events_x11 MODULE ${krfb_events_x11_SRCS})

target_link_libraries (krfb_events_x11
                       ${X11_XTest_LIB}
                       KF6::CoreAddons
                       krfbprivate
)
set_target_properties(krfb_events_x11 PROPERTIES OUTPUT_NAME x11)

install (TARGETS krfb_events_x11 DESTINATION ${KDE_INSTALL_PLUGINDIR}/krfb/events)
