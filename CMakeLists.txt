cmake_minimum_required(VERSION 3.0)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "18")
set (KDE_APPLICATIONS_VERSION_MINOR "03")
set (KDE_APPLICATIONS_VERSION_MICRO "70")
set (KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

project(krfb VERSION ${KDE_APPLICATIONS_VERSION})

set(QT_MIN_VERSION 5.6.0)
set(KF5_MIN_VERSION 5.31.0)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core DBus Widgets X11Extras)

find_package(ECM ${KF5_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(FeatureSummary)

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KRFB
                  VERSION_HEADER "krfb_version.h")

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

find_package(X11 REQUIRED)

find_package(XCB REQUIRED COMPONENTS
    XCB
    RENDER
    SHAPE
    XFIXES
    DAMAGE
    SHM
    IMAGE
)

if(WIN32)
    set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES})
endif(WIN32)

add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
add_definitions(-DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
    ${CMAKE_MODULE_PATH}
)

find_package(LibVNCServer REQUIRED)


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
add_subdirectory(doc)
add_subdirectory(icons)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
