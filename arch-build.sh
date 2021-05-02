#!/bin/bash

set -euo pipefail

which clang && CC=${CC:-clang}
which lld && CC_LD=${CC_LLD:-lld}

export CC CC_LD

PROJECT_ROOT=$(pwd)

ARCH_MESON="${PROJECT_ROOT}/arch.meson"
ARCH_BUILD="${ARCH_MESON}/arch.build"
BUILD_CONFIG='packaging/linux/distro/arch'
MESON_OPT=''
NINJA_OPT='--verbose'

export PROJECT_ROOT ARCH_MESON

rm -rf ${ARCH_MESON}
cp -av ${BUILD_CONFIG}/ ${ARCH_MESON}/

cd ${ARCH_MESON} && meson setup ${ARCH_BUILD} ${MESON_OPT}
cd ${ARCH_BUILD} && ninja ${NINJA_OPT}
