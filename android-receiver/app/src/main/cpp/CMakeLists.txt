cmake_minimum_required(VERSION 3.22)
project(vstream_decoder LANGUAGES C CXX)

include(FetchContent)

set(OPUS_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(OPUS_INSTALL_PKG_CONFIG_MODULE OFF CACHE BOOL "" FORCE)
set(OPUS_INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    Opus
    URL https://downloads.xiph.org/releases/opus/opus-1.6.1.tar.gz
    URL_HASH SHA256=6ffcb593207be92584df15b32466ed64bbec99109f007c82205f0194572411a1
)
FetchContent_MakeAvailable(Opus)

add_library(vstream_decoder SHARED NativeDecoder.cpp)
target_include_directories(vstream_decoder PRIVATE ${opus_SOURCE_DIR}/include)
target_link_libraries(vstream_decoder PRIVATE Opus::opus log)
