cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "80")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(krfb VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION 6.5.0)
set(KF6_MIN_VERSION 6.2.0)

find_package(ECM ${KF6_MIN_VERSION} NO_MODULE REQUIRED)
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

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core DBus Widgets)
if (Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
    find_package(Qt6GuiPrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
endif()
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    Config
    CoreAddons
    DBusAddons
    DNSSD
    DocTools
    Notifications
    Wallet
    WidgetsAddons
    WindowSystem
    XmlGui
    StatusNotifierItem
    Crash
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
    QT 6.7.0
    KF 6.5.0
)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

find_package(LibVNCServer REQUIRED)


option(DISABLE_PIPEWIRE "Disable PipeWire support." OFF)
if(NOT DISABLE_PIPEWIRE)
    find_package(KPipeWire CONFIG REQUIRED)
    pkg_check_modules(PipeWire IMPORTED_TARGET libpipewire-0.3)
    add_feature_info(PipeWire PipeWire_FOUND "Required for pipewire screencast plugin")
endif()

find_package(PlasmaWaylandProtocols 1.5.0)

if(KPipeWire_FOUND AND PipeWire_FOUND AND PlasmaWaylandProtocols_FOUND)
    find_package(KWayland REQUIRED)
    find_package(QtWaylandScanner REQUIRED)
    find_package(Qt6WaylandClient)
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
