if (${XCB_DAMAGE_FOUND} AND ${XCB_SHM_FOUND} AND ${XCB_IMAGE_FOUND})
    add_subdirectory (xcb)
endif()

if (TARGET K::KPipeWire)
    add_subdirectory(pipewire)
endif()
