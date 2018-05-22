add_subdirectory (qt)

if (${XCB_DAMAGE_FOUND} AND ${XCB_SHM_FOUND} AND ${XCB_IMAGE_FOUND})
    add_subdirectory (xcb)
endif()

if (${PipeWire_FOUND} AND ${SPA_FOUND})
    add_subdirectory(pipewire)
endif()
