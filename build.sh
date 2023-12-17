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

python3 TinyJS/build-script/build.py

mkdir -p build
cd build

cmake -G Xcode ..
exit_if_err

# cmake -DBUILD_CXXLIBS=OFF -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF \
#     -DWITH_FORTIFY_SOURCE=OFF -DWITH_STACK_PROTECTOR=OFF -DINSTALL_MANPAGES=OFF -DINSTALL_PKGCONFIG_MODULES=OFF \
#     -DINSTALL_CMAKE_CONFIG_MODULE=OFF -DBUILD_SHARED_LIBS=OFF ../flac

make -j $(sysctl -n hw.ncpu)
# make
exit_if_err

echo "== build successfully =="
