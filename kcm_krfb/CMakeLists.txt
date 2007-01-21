include_directories(${CMAKE_BINARY_DIR}/krfb/krfb/)

set(kcm_krfb_PART_SRCS kcm_krfb.cpp)

kde4_automoc(${kcm_krfb_PART_SRCS})

kde4_add_ui3_files(kcm_krfb_PART_SRCS configurationwidget.ui)

kde4_add_plugin(kcm_krfb ${kcm_krfb_PART_SRCS})

target_link_libraries(kcm_krfb ${KDE4_KDECORE_LIBS} libkrfbconfig libsrvloc ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kcm_krfb DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kcmkrfb.desktop DESTINATION ${XDG_APPS_DIR})
