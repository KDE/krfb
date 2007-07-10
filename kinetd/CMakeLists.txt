
include_directories( ${CMAKE_SOURCE_DIR}/krfb/srvloc  )


########### next target ###############

set(kded_kinetd_PART_SRCS kinetd.cpp ${srvloc_SRCS})

message(STATUS "Port to krfb DBUS")
# kde4_add_dcop_skels(kded_kinetd_PART_SRCS kinetd.h )

kde4_add_plugin(kded_kinetd ${kded_kinetd_PART_SRCS})


target_link_libraries(kded_kinetd  ${KDE4_KIO_LIBS} ${KDE4_KDNSSD_LIBS} ${SLP_LIBRARIES})

install(TARGETS kded_kinetd  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kinetd.notifyrc DESTINATION ${DATA_INSTALL_DIR}/krfb)
install(FILES kinetdmodule.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES kinetd.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded)




