#!/bin/bash

set -e

ZIP_NAME="quake3-better-gibs-mod.zip"
rm ${ZIP_NAME} || true

ZIP_OPTS="-tzip -mx=9 -mpass=8 -mfb=255 -mtc=off -mtm=off -mta=off"

git switch baseq3a
CLEAN=1 ./build-baseq3a.sh
cd ./build/linux-qvm
mkdir -p baseq3a/baseq3
cp pak8a.pk3 baseq3a/baseq3
7z u ${ZIP_OPTS} -- ../../${ZIP_NAME} baseq3a
cd -

git checkout HEAD -- \
build/linux-qvm/tools/q3asm \
build/linux-qvm/tools/q3cpp \
build/linux-qvm/tools/q3lcc \
build/linux-qvm/tools/q3rcc

git switch ioq3
rm ./build -r || true
VANILLAQ3_OR_IOQUAKE3=ioquake3 ./build-ioq3.sh
cd build/Release
mkdir ioquake3-only
cp -r baseq3 missionpack ioquake3-only/
7z u ${ZIP_OPTS} -- ../../${ZIP_NAME} ioquake3-only/**/*.pk3
cd -

rm ./build -r || true
./build-ioq3-vq3.sh
cd build/Release
mkdir ioquake3-or-vanillaq3
cp -r baseq3 missionpack ioquake3-or-vanillaq3/
7z u ${ZIP_OPTS} -- ../../${ZIP_NAME} ioquake3-or-vanillaq3/**/*.pk3
cd -

# cp ${ZIP_NAME} ~/Downloads/

echo Release output file: "$(realpath "${ZIP_NAME}")"
echo Release build successful!
