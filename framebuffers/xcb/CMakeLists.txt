include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_xcb_SRCS
      xcb_framebufferplugin.cpp
      xcb_framebuffer.cpp
)

ecm_qt_declare_logging_category(krfb_framebuffer_xcb_SRCS
    HEADER krfb_fb_xcb_debug.h
    IDENTIFIER KRFB_FB_XCB
    CATEGORY_NAME krfb.framebuffer.xcb
    DESCRIPTION "KRFB XCB framebuffer plugin"
    EXPORT KRFB
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
         DESTINATION ${KDE_INSTALL_PLUGINDIR}/krfb
)
