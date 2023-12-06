cmake_minimum_required(VERSION 3.15)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_FLAGS "-std=c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UT "unit test" OFF)
if (UT)
    add_definitions(-DUNIT_TEST)
endif(UT)

if (APPLE)
    add_definitions(-D_MAC_OS)
elseif (LINUX)
    add_definitions(-D_LINUX)
endif (APPLE)

add_definitions(-DASIO_STANDALONE)
add_definitions(-D_WEBSOCKETPP_CPP11_TYPE_TRAITS_)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -DASIO_STANDALONE")

PROJECT(Mp3Player)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/third-parties/googletest/include
    ${PROJECT_SOURCE_DIR}/third-parties/http-parser
    # ${PROJECT_SOURCE_DIR}/third-parties/libuv/include
    ${PROJECT_SOURCE_DIR}/third-parties/rapidjson
    ${PROJECT_SOURCE_DIR}/third-parties/zlib
    ${PROJECT_SOURCE_DIR}/third-parties/glog
    ${PROJECT_SOURCE_DIR}/third-parties/googletest
    ${PROJECT_SOURCE_DIR}/third-parties/tiny-AES-c
    ${PROJECT_SOURCE_DIR}/third-parties/mbedtls/include
    ${PROJECT_SOURCE_DIR}/third-parties/asio/include
    ${PROJECT_SOURCE_DIR}/third-parties/websocketpp
    ${PROJECT_SOURCE_DIR}/TinyJS
)

set(SRC ${SRC}
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/adler32.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/compress.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/crc32.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/deflate.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/gzio.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/infback.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/inffast.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/inflate.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/inftrees.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/minigzip.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/trees.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/uncompr.c
    ${PROJECT_SOURCE_DIR}/third-parties/zlib/zutil.c
)

if (UT)
    set(SRC
        ${SRC}
        ${PROJECT_SOURCE_DIR}/third-parties/googletest/src/gtest-all.cc
    )
endif(UT)

aux_source_directory(${PROJECT_SOURCE_DIR}/Utils SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/GfxRaw SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/HttpLib SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/ImageLib SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/ImageLib/jpeg SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/ImageLib/png SRC)
# aux_source_directory(${PROJECT_SOURCE_DIR}/ImageLib/tiff SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/LocalServer SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/LocalServer/Http SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/LocalServer/WebSocket SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/LocalServer/WebSocketHandlers SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/LyricsLib SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/MediaTags SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/MediaTags/ID3 SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/MLProtocol SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/MPlayer SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/MPlayerUI SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/Skin SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/Window SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/third-parties/md5 SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/third-parties/sqlite SRC)
# message(SRC="${SRC}")

set(LINK_OPT_FRAME_WORK)

if (APPLE)
    aux_source_directory(${PROJECT_SOURCE_DIR}/Utils/mac SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/Window/mac SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/GfxRaw/mac SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/MPlayerUI/mac SRC)
    set(LINK_OPT_FRAME_WORK "-framework CoreFoundation" "-framework Cocoa")
elseif (LINUX)
    aux_source_directory(${PROJECT_SOURCE_DIR}/Utils/linux SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/Window/gtk2 SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/GfxRaw/gtk2 SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/MPlayerUI/gtk2 SRC)
else ()
    aux_source_directory(${PROJECT_SOURCE_DIR}/Utils/win32 SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/Window/win32 SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/GfxRaw/win32 SRC)
    aux_source_directory(${PROJECT_SOURCE_DIR}/MPlayerUI/win32 SRC)
endif (APPLE)

add_subdirectory(${PROJECT_SOURCE_DIR}/TinyJS TinyJS)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/glog glog)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/libuv uv_a)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread glog uv_a -static-libstdc++ ${LINK_OPT_FRAME_WORK})
