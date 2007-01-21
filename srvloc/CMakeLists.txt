set(libsrvloc_SRCS
   kinetinterface.cpp
   kinetinterfacewatcher.h
   uuid.cpp
   getifaddrs.cpp
   kinetinterface.h
   kserviceregistry.cpp
   uuid.h
   getifaddrs.h
   kinetinterfacewatcher.cpp
   kserviceregistry.h)

kde4_add_library(libsrvloc ${libsrvloc_SRCS})

target_link_libraries(libsrvloc ${KDE4_KDECORE_LIBS} slp)

install(TARGETS libsrvloc DESTINATION ${LIB_INSTALL_DIR})
