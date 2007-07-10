include_directories(${CMAKE_BINARY_DIR}/krfb/krfb/ ${CMAKE_SOURCE_DIR}/krfb/srvloc/)

set(kcm_krfb_PART_SRCS kcm_krfb.cpp ${srvloc_SRCS} ${krfbconfig_SRCS})

kde4_add_ui3_files(kcm_krfb_PART_SRCS configurationwidget.ui)
kde4_add_ui3_files(kcm_krfb_PART_SRCS
	${CMAKE_SOURCE_DIR}/krfb/krfb/manageinvitations.ui
	${CMAKE_SOURCE_DIR}/krfb/krfb/personalinvitewidget.ui
	${CMAKE_SOURCE_DIR}/krfb/krfb/invitewidget.ui)

kde4_add_plugin(kcm_krfb ${kcm_krfb_PART_SRCS})

target_link_libraries(kcm_krfb ${KDE4_KDECORE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${SLP_LIBRARIES})

install(TARGETS kcm_krfb DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kcmkrfb.desktop DESTINATION ${SERVICES_INSTALL_DIR})
