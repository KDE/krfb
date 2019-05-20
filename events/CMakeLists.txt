add_subdirectory(x11)

# Makes sense to use only when PW framebuffer is used
if (${PipeWire_FOUND})
    add_subdirectory(xdp)
endif()
