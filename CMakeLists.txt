cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "70")
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
    pkg_check_modules(PipeWire IMPORTED_TARGET libpipewire-0.3)
endif()
add_feature_info(PipeWire PipeWire_FOUND "Required for pipewire screencast plugin")
find_package(PlasmaWaylandProtocols 1.5.0)

if(PipeWire_FOUND AND PlasmaWaylandProtocols_FOUND)
    find_package(KF5Wayland ${KF5_MIN_VERSION})
    find_package(QtWaylandScanner REQUIRED)
    find_package(Qt5WaylandClient)
    find_package(Qt5XkbCommonSupport)
    find_package(Wayland REQUIRED COMPONENTS Client)
endif()

find_package(gbm)
set_package_properties(gbm PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for dma-buf support in pipewire screencast plugin."
)

find_package(EGL)
set_package_properties(EGL PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for dma-buf support in pipewire screencast plugin."
)

find_package(epoxy)
set_package_properties(epoxy PROPERTIES DESCRIPTION "libepoxy"
    URL "https://github.com/anholt/libepoxy"
    TYPE OPTIONAL
    PURPOSE "Required for dma-buf support in pipewire screencast plugin."
)

if(EGL_FOUND AND gbm_FOUND AND epoxy_FOUND)
  set (HAVE_DMA_BUF TRUE)
else()
  set (HAVE_DMA_BUF FALSE)
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
