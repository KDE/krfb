include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_gbm_SRCS
     logging.cpp
     gbmframebuffer.cpp
     gbmframebufferplugin.cpp
)

add_library(krfb_framebuffer_gbm
 MODULE
                 ${krfb_framebuffer_gbm_SRCS}
)

target_link_libraries (krfb_framebuffer_gbm
                       Qt5::Core
                       Qt5::Gui
                       KF5::CoreAddons
                       KF5::WaylandClient
                       ${epoxy_LIBRARY}
                       Libdrm::Libdrm
                       gbm::gbm
                       krfbprivate
)

install (TARGETS krfb_framebuffer_gbm
         DESTINATION ${PLUGIN_INSTALL_DIR}/krfb
)
