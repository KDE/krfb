include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

set (krfb_framebuffer_qt_SRCS
     qtframebuffer.cpp
     qtframebufferplugin.cpp
)

ecm_qt_declare_logging_category(krfb_framebuffer_qt_SRCS
    HEADER krfb_fb_qt_debug.h
    IDENTIFIER KRFB_FB_QT
    CATEGORY_NAME krfb.framebuffer.qt
    DESCRIPTION "KRFB Qt framebuffer plugin"
    EXPORT KRFB
)

add_library(krfb_framebuffer_qt
 MODULE 
                 ${krfb_framebuffer_qt_SRCS}
)

target_link_libraries (krfb_framebuffer_qt
                       Qt5::Core
                       Qt5::Gui
                       KF5::CoreAddons
                       krfbprivate
)

install (TARGETS krfb_framebuffer_qt
         DESTINATION ${KDE_INSTALL_PLUGINDIR}/krfb/framebuffer
)
