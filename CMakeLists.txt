project(krfb)

if(NOT INSIDE_KDENETWORK)
    message("Not building inside KDENetwork, loading KDE CMake Macros.")

    find_package(KDE4 REQUIRED)

    include(KDE4Defaults)
    include(MacroLibrary)

    include(CheckIncludeFile)
    include(CheckIncludeFiles)
    include(CheckSymbolExists)
    include(CheckFunctionExists)
    include(CheckLibraryExists)
    include(CheckPrototypeExists)
    include(CheckTypeSize)

    set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
    if(WIN32)
       set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
       set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES})
    endif(WIN32)
    add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
    add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
endif(NOT INSIDE_KDENETWORK)

macro_optional_find_package(TelepathyQt4)
macro_log_feature(TelepathyQt4_FOUND "telepathy-qt" "Telepathy Qt Bindings" "http://telepathy.freedesktop.org" FALSE "0.9" "Needed to build Telepathy Tubes support.")

macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE)
macro_bool_to_01(X11_XShm_FOUND HAVE_XSHM)

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb"
                     "${CMAKE_CURRENT_SOURCE_DIR}/krfb/ui"
                     "${CMAKE_CURRENT_SOURCE_DIR}/libvncserver/"
                     "${CMAKE_CURRENT_BINARY_DIR}/libvncserver/"
)

if(Q_WS_X11)
  if(NOT X11_XTest_FOUND)
    message(FATAL_ERROR "krfb requires the libXtst (http://xorg.freedesktop.org) to be built")
  endif(NOT X11_XTest_FOUND)
endif(Q_WS_X11)

add_subdirectory(libvncserver)
add_subdirectory(krfb)
add_subdirectory (framebuffers)
add_subdirectory(doc)

if (NOT INSIDE_KDENETWORK)
    macro_display_feature_log()
endif ()
