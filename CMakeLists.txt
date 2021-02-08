cmake_minimum_required(VERSION 3.0)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(krfb VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION 5.12.0)
set(KF5_MIN_VERSION 5.68.0)

find_package(ECM ${KF5_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(CheckIncludeFile)

check_include_file("linux/input.h" HAVE_LINUX_INPUT_H)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core DBus Widgets X11Extras)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    Config
    CoreAddons
    Crash
    DBusAddons
    DNSSD
    DocTools
    Notifications
    Wallet
    WidgetsAddons
    WindowSystem
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

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050600
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

find_package(LibVNCServer REQUIRED)

find_package(PipeWire)
set_package_properties(PipeWire PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for pipewire screencast plugin"
)

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KRFB
                  VERSION_HEADER "krfb_version.h")

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb/ui"
)

if(Q_WS_X11)
  if(NOT X11_XTest_FOUND)
    message(FATAL_ERROR "krfb requires the libXtst (https://xorg.freedesktop.org) to be built")
  endif(NOT X11_XTest_FOUND)
endif(Q_WS_X11)

add_subdirectory(events)
add_subdirectory(krfb)
add_subdirectory(framebuffers)
add_subdirectory(doc)
add_subdirectory(icons)

ecm_qt_install_logging_categories(
    EXPORT KRFB
    FILE krfb.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
