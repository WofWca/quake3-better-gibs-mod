#!/bin/bash

set -e

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
-DBUILD_GAME_QVMS=1 \
-DBUILD_SERVER=0 \
-DBUILD_CLIENT=0 \
-DBUILD_RENDERER_GL1=0 \
-DBUILD_RENDERER_GL2=0 \
-DBUILD_GAME_LIBRARIES=0 \
-DBUILD_STANDALONE=0
cmake --build build

function copy_cfgs () {
    git checkout main -- ../../../better-gibs-cfgs
    7z u -- "$1" '../../../better-gibs-cfgs'
    git reset HEAD -- '../../../better-gibs-cfgs/*'
    rm -r ../../../better-gibs-cfgs
}

function maybe_start_q3 () {
    if [ "${VANILLAQ3_OR_IOQUAKE3}" != "vanillaq3" ]; then
        return
    fi

    if [ "$IOQ3_QUAKE_EXE_PATH" == "" ]; then
        echo "\$IOQ3_QUAKE_EXE_PATH env var not set, skipping Quake launch"
        echo ""
        echo "If you want to set it, also make sure"
        echo "to add a symlink to $(realpath "$2")"
        echo "to your ~/.q3a/ioq3-gibs-dev and ~/.q3a/ioq3-gibs-mp"
        return
    fi;

    echo "Starting Quake"
    $IOQ3_QUAKE_EXE_PATH +set fs_game "$1" +devmap q3dm1
}

ZIP_OPTS="-tzip -mx=9 -mpass=8 -mfb=255 -mtc=off -mtm=off -mta=off"
PK3="pak999-${VANILLAQ3_OR_IOQUAKE3}-baseq3-better-gibs-mod.pk3"
PK3_1=$PK3
cd build/Release/baseq3
7z u $ZIP_OPTS -- "${PK3}" vm/qagame.qvm vm/cgame.qvm
7z u -- "${PK3}" '../../../COPYING.txt'
git checkout main -- ../../../README.md
7z u -- "${PK3}" '../../../README.md'
git checkout HEAD -- ../../../README.md
copy_cfgs "${PK3}"
maybe_start_q3 ioq3-gibs-dev "$PK3"


PK3="pak999-${VANILLAQ3_OR_IOQUAKE3}-missionpack-better-gibs-mod.pk3"
PK3_2=$PK3
cd ../missionpack
7z u $ZIP_OPTS -- "${PK3}" vm/qagame.qvm vm/cgame.qvm
7z u -- "${PK3}" '../../../COPYING.txt'
git checkout main -- ../../../README.md
7z u -- "${PK3}" '../../../README.md'
git checkout HEAD -- ../../../README.md
copy_cfgs "${PK3}"
maybe_start_q3 ioq3-gibs-mp "$PK3"

echo Output files: "$(realpath "$PK3_1")"
realpath "$PK3_2"
echo ioq3 build successful!
