cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "0")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(krfb VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION 5.15.2)
set(KF5_MIN_VERSION 5.91.0)

find_package(ECM ${KF5_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
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

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.91
)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

find_package(LibVNCServer REQUIRED)


option(DISABLE_PIPEWIRE "Disable PipeWire support." OFF)
if(NOT DISABLE_PIPEWIRE)
    find_package(KPipeWire REQUIRED)
endif()
add_feature_info(PipeWire KPipeWire_FOUND "Required for pipewire screencast plugin")
find_package(PlasmaWaylandProtocols 1.5.0)

if(KPipeWire_FOUND AND PlasmaWaylandProtocols_FOUND)
    find_package(KF5Wayland ${KF5_MIN_VERSION})
    find_package(QtWaylandScanner REQUIRED)
    find_package(Qt5WaylandClient)
    find_package(Qt5XkbCommonSupport)
    find_package(Wayland REQUIRED COMPONENTS Client)
endif()

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KRFB
                  VERSION_HEADER "krfb_version.h")

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb/ui"
)

add_subdirectory(events)
add_subdirectory(krfb)
add_subdirectory(framebuffers)
add_subdirectory(doc)
add_subdirectory(icons)

ki18n_install(po)
kdoctools_install(po)

ecm_qt_install_logging_categories(
    EXPORT KRFB
    FILE krfb.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
