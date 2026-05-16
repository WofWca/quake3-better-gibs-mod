#!/bin/bash

set -e

sed --in-place 's/COMMAND ${Q3ASM} -o/COMMAND ${Q3ASM} -vq3 -o/' cmake/utils/qvm_tools.cmake
VANILLAQ3_OR_IOQUAKE3=vanillaq3 ./build-ioq3.sh
git checkout HEAD -- cmake/utils/qvm_tools.cmake
