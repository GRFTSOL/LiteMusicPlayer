cmake_minimum_required(VERSION 3.15)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UT "unit test" OFF)
if (UT)
    add_definitions(-DUNIT_TEST)
endif(UT)

if (APPLE)
    if (CMAKE_BUILD_TYPE MATCHES "Release")
        SET(CMAKE_XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING YES)
    endif(RELEASE)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g") # Clang generate debug symbols.
    add_definitions(-D_MAC_OS)
elseif (LINUX)
    add_definitions(-D_LINUX)
else(WIN32)
    add_definitions(-DUNICODE)
endif ()

add_definitions(-DASIO_STANDALONE)
add_definitions(-D_WEBSOCKETPP_CPP11_TYPE_TRAITS_)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

PROJECT(MusicPlayer)

#
# Compile third-parties first.
#

add_subdirectory(${PROJECT_SOURCE_DIR}/TinyJS TinyJS_STATIC)

set(BUILD_SHARED_LIBS OFF)
set(WITH_CUSTOM_PREFIX OFF)
set(WITH_GFLAGS OFF)
set(WITH_GTEST OFF)
set(WITH_PKGCONFIG OFF)
set(WITH_UNWIND OFF)
set(BUILD_TESTING OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/glog glog)

add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/libuv uv_a)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/Agg agg_STATIC)

set(ENABLE_PROGRAMS OFF)
set(ENABLE_TESTING OFF)
set(ENABLE_PROGRAMS OFF)
set(MBEDTLS_PYTHON_EXECUTABLE OFF)
set(USE_STATIC_MBEDTLS_LIBRARY ON)
set(USE_SHARED_MBEDTLS_LIBRARY OFF)
set(GEN_FILES OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/mbedtls mbedtls)

#
# Media Engine dependencies
#

set(BUILD_CXXLIBS OFF)
set(BUILD_PROGRAMS OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)
set(BUILD_DOCS OFF)
set(WITH_FORTIFY_SOURCE OFF)
set(WITH_STACK_PROTECTOR OFF)
set(INSTALL_MANPAGES OFF)
set(INSTALL_PKGCONFIG_MODULES OFF)
#set(ZLIB_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/zlib/Debug/zlib.lib)
set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third-parties/zlib)
set(ZLIB_BUILD_EXAMPLES OFF)
#set(INSTALL_CMAKE_CONFIG_MODULE OFF)

# flac options
set(INSTALL_CMAKE_CONFIG_MODULE OFF)
set(WITH_OGG ON)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/flac FLAC)

add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/faad2 libfaad2)

add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/zlib zlib)

set(SPNG_SHARED OFF)
set(SPNG_STATIC ON)
set(BUILD_EXAMPLES OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-parties/libspng libspng)

#
# Build MusicPlayer
#

set(SRC ${SRC}
    ${PROJECT_SOURCE_DIR}/third-parties/http-parser/http_parser.c

    ${PROJECT_SOURCE_DIR}/third-parties/faad2/frontend/mp4read.c
)

if (UT)
    set(SRC
        ${SRC}
        ${PROJECT_SOURCE_DIR}/third-parties/googletest/src/gtest-all.cc
    )
endif(UT)

set(SRC_PATHS
    ${PROJECT_SOURCE_DIR}/Utils
    ${PROJECT_SOURCE_DIR}/GfxRaw
    ${PROJECT_SOURCE_DIR}/HttpLib
    ${PROJECT_SOURCE_DIR}/ImageLib
    ${PROJECT_SOURCE_DIR}/ImageLib/jpeg
    ${PROJECT_SOURCE_DIR}/LocalServer
    ${PROJECT_SOURCE_DIR}/LocalServer/Http
    ${PROJECT_SOURCE_DIR}/LocalServer/WebSocket
    ${PROJECT_SOURCE_DIR}/LocalServer/WebSocketHandlers
    ${PROJECT_SOURCE_DIR}/LyricsLib
    ${PROJECT_SOURCE_DIR}/MediaTags
    ${PROJECT_SOURCE_DIR}/MediaTags/ID3
    ${PROJECT_SOURCE_DIR}/MLProtocol
    ${PROJECT_SOURCE_DIR}/MPlayer
    ${PROJECT_SOURCE_DIR}/MPlayerEngine
    ${PROJECT_SOURCE_DIR}/MPlayerUI
    ${PROJECT_SOURCE_DIR}/Skin
    ${PROJECT_SOURCE_DIR}/Skin/api-js
    ${PROJECT_SOURCE_DIR}/Window
    ${PROJECT_SOURCE_DIR}/third-parties/sqlite
    ${PROJECT_SOURCE_DIR}/third-parties/md5
)

if (APPLE)
    set(SRC_PATHS ${SRC_PATHS}
        ${PROJECT_SOURCE_DIR}/Utils/mac
        ${PROJECT_SOURCE_DIR}/Window/mac
        ${PROJECT_SOURCE_DIR}/Skin/mac
        ${PROJECT_SOURCE_DIR}/GfxRaw/mac
        ${PROJECT_SOURCE_DIR}/MPlayerUI/mac
        ${PROJECT_SOURCE_DIR}/MPlayerUI/mac/imported/SPMediaKeyTap
        ${PROJECT_SOURCE_DIR}/MPlayerEngine/mac
        ${PROJECT_SOURCE_DIR}/MusicPlayer
    )
elseif (LINUX)
    set(SRC_PATHS ${SRC_PATHS}
        ${PROJECT_SOURCE_DIR}/Utils/linux
        ${PROJECT_SOURCE_DIR}/Window/gtk2
        ${PROJECT_SOURCE_DIR}/Skin/gtk2
        ${PROJECT_SOURCE_DIR}/GfxRaw/gtk2
        ${PROJECT_SOURCE_DIR}/MPlayerUI/gtk2
        ${PROJECT_SOURCE_DIR}/MPlayerEngine/linux
    )
else ()
    set(SRC_PATHS ${SRC_PATHS}
        ${PROJECT_SOURCE_DIR}/Utils/win32
        ${PROJECT_SOURCE_DIR}/Window/win32
        ${PROJECT_SOURCE_DIR}/Skin/win32
        ${PROJECT_SOURCE_DIR}/GfxRaw/win32
        ${PROJECT_SOURCE_DIR}/MPlayerUI/win32
        ${PROJECT_SOURCE_DIR}/MPlayerEngine/win32
    )
endif (APPLE)

foreach(_PATH ${SRC_PATHS})
    aux_source_directory(${_PATH} SRC)
endforeach()

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(HEADER_FILES)
foreach(HEADER_PATH ${SRC_PATHS})
    file(GLOB _HEADERS_H "${HEADER_PATH}/*.h")
    file(GLOB _HEADERS_HPP "${HEADER_PATH}/*.hpp")
    set(HEADER_FILES ${HEADER_FILES} ${_HEADERS_H} ${_HEADERS_HPP})
endforeach()

if(APPLE)
    file(GLOB RESOURCES "${PROJECT_SOURCE_DIR}/MusicPlayer/tray-icons/*.png")
    set(RESOURCES ${RESOURCES}
        ${PROJECT_SOURCE_DIR}/MusicPlayer/Base.lproj/MainMenu.xib
        ${PROJECT_SOURCE_DIR}/Skins-Design/skins
        ${PROJECT_SOURCE_DIR}/Skins-Design/patterns
        ${PROJECT_SOURCE_DIR}/MusicPlayer/MusicPlayer.icns
    )

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC} ${RESOURCES} ${HEADER_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.crintsoft.${PROJECT_NAME}
        MACOSX_BUNDLE_ICON_FILE MusicPlayer.icns
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        RESOURCE "${RESOURCES}"
        # MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
        # XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
        # XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
    )
elseif (LINUX)
else(LINUX)
    # Win32
    add_executable(${PROJECT_NAME} ${SRC} ${HEADER_FILES})
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:${CMAKE_BUILD_TYPE}>:${CMAKE_BUILD_TYPE}>")
endif(LINUX)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/TinyJS
    ${PROJECT_SOURCE_DIR}/third-parties/googletest/include
    ${PROJECT_SOURCE_DIR}/third-parties/http-parser
    ${CMAKE_CURRENT_BINARY_DIR}/glog
    ${PROJECT_SOURCE_DIR}/third-parties/libuv/include
    ${PROJECT_SOURCE_DIR}/third-parties/rapidjson
    ${PROJECT_SOURCE_DIR}/third-parties/zlib
    ${PROJECT_SOURCE_DIR}/third-parties/glog/src
    ${PROJECT_SOURCE_DIR}/third-parties/googletest
    ${PROJECT_SOURCE_DIR}/third-parties/tiny-AES-c
    ${PROJECT_SOURCE_DIR}/third-parties/mbedtls/include
    ${PROJECT_SOURCE_DIR}/third-parties/asio/include
    ${PROJECT_SOURCE_DIR}/third-parties/websocketpp
)

if(WIN32)
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/MusicPlayer/win32/resource.rc)
endif(WIN32)

set(OS_LINK_OPTS)
if(WIN32)
    set(OS_LINK_OPTS odbc32.lib odbccp32.lib ws2_32.lib Winmm.lib comctl32.lib version.lib psapi.lib)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")
elseif (APPLE)
    set(OS_LINK_OPTS "pthread" "-framework CoreFoundation" "-framework Cocoa"
        "-framework AudioToolbox" "-framework CoreAudio" "-framework CoreData"
        "-framework AppKit" "-framework CoreText" "-framework CoreMedia"
        "-framework AVFoundation" "-framework CoreGraphics" "-framework Carbon"
        "-dead_strip")
else(APPLE)
    set(OS_LINK_OPTS "pthread --gc-sections")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${OS_LINK_OPTS} FLAC faad TinyJS_STATIC agg_STATIC glogbase uv_a mbedtls spng_static zlibstatic)
# -static-libstdc++
