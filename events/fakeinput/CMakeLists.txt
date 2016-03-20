include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_events_gbm_SRCS
     fakeinputevents.cpp
     fakeinputeventsplugin.cpp
)

add_library(krfb_events_fakeinput
 MODULE
                 ${krfb_events_gbm_SRCS}
)

target_link_libraries (krfb_events_fakeinput
                       ${GBM_XTest_LIB}
                       KF5::CoreAddons
                       KF5::WaylandClient
                       KF5::I18n
                       krfbprivate
)

install (TARGETS krfb_events_fakeinput
         DESTINATION ${PLUGIN_INSTALL_DIR}/krfb
)
