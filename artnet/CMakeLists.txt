# Set source files
set(ARTNET_SOURCES
    artnet.c
    misc.c
    network.c
    receive.c
    transmit.c
    tod.c
)

# Set header files to install
set(ARTNET_PUBLIC_HEADERS
    artnet.h
    packets.h
    common.h
)

# Create the library
add_library(artnet ${ARTNET_SOURCES})

# Add alias for namespaced target
add_library(libartnet::artnet ALIAS artnet)

# Set library properties
set_target_properties(artnet PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    OUTPUT_NAME artnet
    PUBLIC_HEADER "${ARTNET_PUBLIC_HEADERS}"
)

# Include directories
target_include_directories(artnet
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Compiler flags
if(MSVC)
    target_compile_options(artnet PRIVATE /W4)
else()
    target_compile_options(artnet PRIVATE -Wall -Werror -Wformat -W)
endif()

# Platform-specific linking
if(WIN32)
    target_link_libraries(artnet PRIVATE
        iphlpapi
        netapi32
        ws2_32
    )
    # Check if dpnet library exists (optional)
    find_library(DPNET_LIB dpnet)
    if(DPNET_LIB)
        target_link_libraries(artnet PRIVATE ${DPNET_LIB})
    endif()
    
    # For shared library on Windows, export symbols
    if(BUILD_SHARED_LIBS)
        set_target_properties(artnet PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
endif()

# Install library
install(TARGETS artnet
    EXPORT libartnetTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/artnet
)
