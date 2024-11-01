# CMakeLists.txt for libartnet project

cmake_minimum_required(VERSION 3.10)
project(libartnet C)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckTypeSize)
# include(CheckTypeExists)
include(CheckSymbolExists)
include(CheckCSourceRuns)
include(CheckStructHasMember)

# Check for standard libraries and functions
check_include_files("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_files("dlfcn.h" HAVE_DLFCN_H)
check_function_exists("_doprnt" HAVE_DOPRNT)
check_include_files("endian.h" HAVE_ENDIAN_H)
check_function_exists("getifaddrs" HAVE_GETIFADDRS)
check_function_exists("inet_aton" HAVE_INET_ATON)
check_function_exists("inet_ntoa" HAVE_INET_NTOA)
check_include_files("inttypes.h" HAVE_INTTYPES_H)
check_include_files("linux/if_packet.h" HAVE_LINUX_IF_PACKET_H)
# TODO GNU libc compatible malloc -> HAVE_MALLOC
check_include_files("memory.h" HAVE_MEMORY_H)
check_function_exists("memset" HAVE_MEMSET)
check_include_files("netinet/in.h" HAVE_NETINET_IN_H)
check_include_files("ppc/endian.h" HAVE_PPC_ENDIAN_H)
# TODO GNU libc compatible realloc
check_function_exists("select" HAVE_SELECT)
check_struct_has_member("struct sockaddr" "sa_len" "sys/socket.h" HAVE_SOCKADDR_SA_LEN)
check_function_exists("socket" HAVE_SOCKET)
# TODO C99 stdbool.h -> HAVE_STDBOOL_H
check_include_files("stdint.h" HAVE_STDINT_H)
check_include_files("stdlib.h" HAVE_STDLIB_H)
check_function_exists("strchr" HAVE_STRCHR)
check_function_exists("strdup" HAVE_STRDUP)
check_function_exists("strerror" HAVE_STRERROR)
check_include_files("strings.h" HAVE_STRINGS_H)
check_include_files("string.h" HAVE_STRING_H)
check_include_files("sys/ioctl.h" HAVE_SYS_IOCTL_H)
check_include_files("sys/select.h" HAVE_SYS_SELECT_H)
check_include_files("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("unistd.hi" HAVE_UNISTD_HI)
check_function_exists("vprintf" HAVE_VPRINTF)
# check_type_exists("_Bool" HAVE__BOOL)

# IPv6 check
check_c_source_runs("
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
int main() {
    int fd = socket(AF_INET6, SOCK_STREAM, 0);
    return fd >= 0 ? 0 : 1;
}
" IPV6)

if(IPV6)
    add_definitions(-DIPV6)
endif()

# Struct member check
check_type_size("size_t" SIZE_T)


if(HAVE_SOCKADDR_SA_LEN)
    add_definitions(-DHAVE_SOCKADDR_SA_LEN)
endif()

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    artnet/artnet.c
    artnet/misc.c
    artnet/network.c
    artnet/receive.c
    artnet/tod.c
    artnet/transmit.c
)

# Add header files
set(HEADERS
    artnet/artnet.h
    artnet/common.h
    artnet/misc.h
    artnet/packets.h
    artnet/private.h
    artnet/tod.h
)

# Create a static library
add_library(libartnet STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(libartnet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Compiler options (adjust as needed)
target_compile_options(libartnet PRIVATE -Wall -Wextra)

# Optionally link libraries here if needed
# target_link_libraries(libartnet <library_name>)
