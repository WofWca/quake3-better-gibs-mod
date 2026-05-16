#!/bin/bash

set -e

cd build/linux-qvm

chmod 744 tools/*
function restore_perms() {
    # git update-index --assume-unchanged ./tools/*
    chmod 644 tools/*
    echo "restored tools/* mod"
}
trap restore_perms EXIT

if [[ -v CLEAN ]]; then
    echo "Clean baseq3a build"
    make clean
fi
make
restore_perms

7z u -- pak8a.pk3 '../../QIIIA Game Source License.txt'

git checkout main -- ../../README.md
7z u -- pak8a.pk3 '../../README.md'
git checkout HEAD -- ../../README.md

git checkout main -- ../../better-gibs-cfgs
7z u -- pak8a.pk3 '../../better-gibs-cfgs'
git reset HEAD -- '../../better-gibs-cfgs/*'
rm -r ../../better-gibs-cfgs

# Use link instead

if [ "$BASEQ3A_QUAKE_EXE_PATH" != "" ]; then
    echo "Starting Quake"
    $BASEQ3A_QUAKE_EXE_PATH +set fs_game baseq3a +devmap q3dm1
else
    echo "\$BASEQ3A_QUAKE_EXE_PATH env var not set, skipping Quake launch."
    echo ""
    echo "If you want to set it, also make sure"
    echo "to add a symlink to $(realpath pak8a.pk3)"
    echo "to your ~/.q3a/baseq3a"
fi

echo Output file: "$(realpath pak8a.pk3)"
echo baseq3a build successful!
