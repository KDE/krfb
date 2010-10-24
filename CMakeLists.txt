project(krfb)

if(NOT INSIDE_KDENETWORK)
    message("Not building inside KDENetwork, loading KDE CMake Macros.")

    set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

    find_package(KDE4 REQUIRED)

    include(KDE4Defaults)
    include(MacroLibrary)

    include(CheckSymbolExists)
    find_package(LibVNCServer REQUIRED)

    set(CMAKE_REQUIRED_DEFINITIONS ${_KDE_PLATFORM_DEFINITIONS})

    add_definitions(${QT_DEFINITIONS} ${QT_DBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
endif(NOT INSIDE_KDENETWORK)

# FIXME: Can we merge this check back into FindLibVNCServer.cmake?
check_symbol_exists(rfbInitServer "rfb/rfb.h" HAVE_LIBVNCSERVER)

macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE)
macro_bool_to_01(X11_XShm_FOUND HAVE_XSHM)

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb/ui"
)

if(Q_WS_X11)
  if(NOT X11_XTest_FOUND)
    macro_log_feature(X11_XTest_FOUND "libXtst" "X11 Testing Resource extension library" "http://xorg.freedesktop.org" FALSE "" "useful for automated testing of X clients.")
  endif(NOT X11_XTest_FOUND)
endif(Q_WS_X11)

if (HAVE_LIBVNCSERVER)
    #add_subdirectory(kinetd)
    #add_subdirectory(kcm_krfb)
    add_subdirectory(krfb)
    add_subdirectory (framebuffers)
endif (HAVE_LIBVNCSERVER)

