#!/bin/bash

CUR_DIR=`dirname ${BASH_SOURCE-$0}`
cd ${CUR_DIR}
CUR_DIR="$(pwd)"

function exit_if_err() {
    rc=$?
    if [ $rc -ne 0 ]; then
        echo $*
        exit $rc
    fi
}

function create_music_player_ini() {
    mkdir -p ${CUR_DIR}/build/Debug
    file_ini="${CUR_DIR}/build/Debug/MusicPlayer.ini"
    if ! test -f ${file_ini} ; then
        # lyrics-server.ini
        echo "Creating ${file_ini} ..."

        echo "[MusicPlayer]
SkinRootDir=${CUR_DIR}/Skins-Design/skins" > ${file_ini}
        exit_if_err "Failed to create ${file_ini}."
        echo "OK"
    fi
}

python3 TinyJS/build-script/build.py

mkdir -p build
cd build

# cmake -D CMAKE_C_COMPILER="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc"  -D CMAKE_CXX_COMPILER="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++" -G Xcode ..
cmake -G Xcode ..
exit_if_err

create_music_player_ini

# cmake -DBUILD_CXXLIBS=OFF -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF \
#     -DWITH_FORTIFY_SOURCE=OFF -DWITH_STACK_PROTECTOR=OFF -DINSTALL_MANPAGES=OFF -DINSTALL_PKGCONFIG_MODULES=OFF \
#     -DINSTALL_CMAKE_CONFIG_MODULE=OFF -DBUILD_SHARED_LIBS=OFF ../flac

make -j $(sysctl -n hw.ncpu)
# make
exit_if_err

echo "== build successfully =="

# 编译失败需要安装 mbedtls:
# python3 -m pip install jsonschema