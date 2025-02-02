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

function push() {
    echo "== push $1"
    cd $1
    git push origin master
    exit_if_err
    git push gh master
    exit_if_err
    cd ..
    echo ""
}

push LyricServer
push third-parties
push TinyJS
push .

echo "== Push all successfully =="
