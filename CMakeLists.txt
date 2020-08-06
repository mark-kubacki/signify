cmake_minimum_required(VERSION 2.8.8)

project(Signify C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEFAULT_SOURCE -DHAVE_SYS_SYSCALL_H -Wnonnull -pipe -ffunction-sections -fdata-sections -finline-functions")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,--strip-all -fwhole-program")

include_directories(include)
file(GLOB SOURCES "src/*.c")

add_executable(signify ${SOURCES})
TARGET_LINK_LIBRARIES(signify seccomp)
