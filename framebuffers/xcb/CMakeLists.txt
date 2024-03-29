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
                       Qt::Core
                       Qt::Gui
                       Qt::GuiPrivate
                       XCB::XCB
                       XCB::RENDER
                       XCB::SHAPE
                       XCB::XFIXES
                       XCB::DAMAGE
                       XCB::SHM
                       XCB::IMAGE
                       KF6::CoreAddons
                       krfbprivate
)

set_target_properties(krfb_framebuffer_xcb PROPERTIES OUTPUT_NAME xcb)
install (TARGETS krfb_framebuffer_xcb
         DESTINATION ${KDE_INSTALL_PLUGINDIR}/krfb/framebuffer
)
