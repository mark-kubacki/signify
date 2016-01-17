cmake_minimum_required(VERSION 2.8.8)

project(Signify)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEFAULT_SOURCE -DHAVE_SYS_SYSCALL_H -Wnonnull -pipe -ffunction-sections -fdata-sections -finline-functions")
set(CMAKE_EXE_LINKER_FLAGS "-s -static -Wl,--gc-sections -fwhole-program")

include_directories(include)
file(GLOB SOURCES "src/*.c")

add_executable(signify ${SOURCES})
