cmake_minimum_required(VERSION 3.10)
project(libartnet VERSION 1.1.2 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include GNUInstallDirs for standard installation directories
include(GNUInstallDirs)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_IPV6 "Enable IPv6 support" ON)

# Platform-specific settings
if(WIN32)
    set(USING_WIN32 TRUE)
    add_definitions(-D_WIN32_WINNT=0x0600)
else()
    set(USING_WIN32 FALSE)
endif()

# Check for headers
include(CheckIncludeFile)
check_include_file("endian.h" HAVE_ENDIAN_H)
check_include_file("ppc/endian.h" HAVE_PPC_ENDIAN_H)
check_include_file("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_file("netinet/in.h" HAVE_NETINET_IN_H)
check_include_file("stddef.h" HAVE_STDDEF_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("stdlib.h" HAVE_STDLIB_H)
check_include_file("string.h" HAVE_STRING_H)
check_include_file("sys/ioctl.h" HAVE_SYS_IOCTL_H)
check_include_file("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("linux/if_packet.h" HAVE_LINUX_IF_PACKET_H)
check_include_file("stdbool.h" HAVE_STDBOOL_H)

# Check for functions
include(CheckFunctionExists)
check_function_exists(inet_ntoa HAVE_INET_NTOA)
check_function_exists(inet_aton HAVE_INET_ATON)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(select HAVE_SELECT)
check_function_exists(socket HAVE_SOCKET)
check_function_exists(strchr HAVE_STRCHR)
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(strerror HAVE_STRERROR)
check_function_exists(getifaddrs HAVE_GETIFADDRS)

# Check for struct members
include(CheckStructHasMember)
check_struct_has_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_SOCKADDR_SA_LEN)

# Check for IPv6 support
if(ENABLE_IPV6)
    include(CheckCSourceRuns)
    if(NOT CMAKE_CROSSCOMPILING)
        check_c_source_runs("
            #include <sys/types.h>
            #include <sys/socket.h>
            #include <netinet/in.h>
            int main() {
                int fd;
                struct sockaddr_in6 foo;
                fd = socket(AF_INET6, SOCK_STREAM, 0);
                return fd >= 0 ? 0 : 1;
            }
        " IPV6)
    else()
        # Assume IPv6 is available when cross-compiling
        set(IPV6 1)
    endif()
endif()

# Generate config.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# Add artnet subdirectory
add_subdirectory(artnet)

# Generate pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/libartnet.pc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libartnet.pc
    @ONLY
)

# Install pkg-config file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libartnet.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# CMake package config
include(CMakePackageConfigHelpers)

# Generate the config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/libartnetConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libartnetConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libartnet
)

# Generate the version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libartnetConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install CMake config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libartnetConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libartnetConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libartnet
)

# Export targets
install(EXPORT libartnetTargets
    FILE libartnetTargets.cmake
    NAMESPACE libartnet::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libartnet
)
