add_subdirectory(x11)

# Makes sense to use only when PW framebuffer is used
if (TARGET K::KPipeWire)
    add_subdirectory(xdp)
endif()
