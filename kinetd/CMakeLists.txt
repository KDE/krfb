
include_directories( ${CMAKE_SOURCE_DIR}/krfb/srvloc  )


########### next target ###############

set(kded_kinetd_PART_SRCS kinetd.cpp )

kde4_automoc(${kded_kinetd_PART_SRCS})

kde4_add_dcop_skels(kded_kinetd_PART_SRCS kinetd.h )

kde4_add_plugin(kded_kinetd ${kded_kinetd_PART_SRCS})


target_link_libraries(kded_kinetd  ${KDE4_KIO_LIBS} srvloc ${KDE4_KDNSSD_LIBS} )

install(TARGETS kded_kinetd  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kinetdmodule.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES kinetd.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded)




