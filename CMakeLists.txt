cmake_minimum_required(VERSION 2.8.12)

project(krfb)

include(FeatureSummary)
find_package(Qt5 REQUIRED COMPONENTS Core DBus Widgets X11Extras)

find_package(ECM 1.7.0 NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddAppIcon)

find_package(KF5 REQUIRED COMPONENTS
    I18n
    Completion
    Config
    CoreAddons
    Crash
    DBusAddons
    DNSSD
    DocTools
    Notifications
    Wallet
    WidgetsAddons
    XmlGui
)

find_package(KF5Wayland CONFIG)
set_package_properties(KF5Wayland PROPERTIES DESCRIPTION "KDE Frameworks Wayland integration package"
                       URL "https://github.com/KDE/kwayland"
                       TYPE OPTIONAL
                       PURPOSE "Required for wayland integration with KWin server"
                      )

find_package(epoxy)
set_package_properties(epoxy PROPERTIES DESCRIPTION "libepoxy"
                       URL "http://github.com/anholt/libepoxy"
                       TYPE OPTIONAL
                       PURPOSE "OpenGL dispatch library for GBM backend"
                      )

find_package(Libdrm)
set_package_properties(Libdrm PROPERTIES DESCRIPTION "Libdrm"
                       TYPE OPTIONAL
                       PURPOSE "Required for GBM access to compositor buffers on Wayland"
                      )

set(HAVE_DRM FALSE)
if(KF5Wayland_FOUND AND Libdrm_FOUND AND epoxy_FOUND)
    set(HAVE_DRM TRUE)
endif()

find_package(gbm)
set_package_properties(gbm PROPERTIES TYPE OPTIONAL PURPOSE "Required for egl ouput of drm backend.")
set(HAVE_GBM FALSE)
if(HAVE_DRM AND gbm_FOUND)
    set(HAVE_GBM TRUE)
endif()


find_package(X11 REQUIRED)

if(WIN32)
    set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES})
endif(WIN32)
add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
    ${CMAKE_MODULE_PATH}
)

find_package(LibVNCServer REQUIRED)

if (HAVE_XDAMAGE)
set(X11_Xdamage_FOUND 1)
else()
set(X11_Xdamage_FOUND 0)
endif()
if (HAVE_XSHM)
set(X11_XShm_FOUND 1)
else()
set(X11_XShm_FOUND 0)
endif()

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb/ui"
)

if(Q_WS_X11)
  if(NOT X11_XTest_FOUND)
    message(FATAL_ERROR "krfb requires the libXtst (http://xorg.freedesktop.org) to be built")
  endif(NOT X11_XTest_FOUND)
endif(Q_WS_X11)

add_subdirectory(krfb)
add_subdirectory(framebuffers)
add_subdirectory(events)
add_subdirectory(doc)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
