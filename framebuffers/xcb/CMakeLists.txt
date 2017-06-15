include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_xcb_SRCS
      xcb_framebufferplugin.cpp
      xcb_framebuffer.cpp
)

add_library(krfb_framebuffer_xcb MODULE ${krfb_framebuffer_xcb_SRCS})

target_link_libraries (krfb_framebuffer_xcb
                       Qt5::Core
                       Qt5::Gui
                       XCB::XCB
                       XCB::RENDER
                       XCB::SHAPE
                       XCB::XFIXES
                       XCB::DAMAGE
                       XCB::SHM
                       XCB::IMAGE
                       KF5::CoreAddons
                       krfbprivate
)

install (TARGETS krfb_framebuffer_xcb
         DESTINATION ${PLUGIN_INSTALL_DIR}/krfb
)
