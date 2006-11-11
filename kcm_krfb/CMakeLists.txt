



########### next target ###############

set(kcm_krfb_PART_SRCS kcm_krfb.cpp )

kde4_automoc(${kcm_krfb_PART_SRCS})

kde4_add_ui_files(kcm_krfb_PART_SRCS configurationwidget.ui )

kde4_add_plugin(kcm_krfb ${kcm_krfb_PART_SRCS})


target_link_libraries(kcm_krfb  ${KDE4_KIO_LIBS} krfbconfig )

install(TARGETS kcm_krfb  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kcmkrfb.desktop  DESTINATION ${XDG_APPS_DIR})




