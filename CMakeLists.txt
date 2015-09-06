cmake_minimum_required(VERSION 2.8.12)

project(krfb)

include(FeatureSummary)
find_package(Qt5 REQUIRED COMPONENTS Core DBus Widgets X11Extras)

find_package(ECM 1.7.0 NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(ECMAddAppIcon)

find_package(KF5 REQUIRED COMPONENTS
    I18n
    Crash
    CoreAddons
    DNSSD
    DocTools
    KDELibs4Support
    Wallet
    WidgetsAddons
)

find_package(X11 REQUIRED)

if(WIN32)
    set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES})
endif(WIN32)
add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

# set(IS_KTP_INTERNAL_MODULE TRUE)
set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
    ${CMAKE_MODULE_PATH}
)

find_package(LibVNCServer REQUIRED)

# macro_optional_find_package(TelepathyQt4)
# macro_log_feature(TelepathyQt4_FOUND "telepathy-qt" "Telepathy Qt Bindings" "http://telepathy.freedesktop.org" FALSE "0.9" "Needed to build Telepathy Tubes support.")

# macro_optional_find_package(KTp)
# macro_log_feature(KTP_FOUND "KTP" "KDE Telepathy" "https://projects.kde.org/projects/extragear/network/telepathy" FALSE "" "Needed to build KDE IM Contacts Display in KRFB.")

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
add_subdirectory(doc)
