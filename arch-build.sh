#!/bin/bash

set -euo pipefail

export CC=clang
export CC_LD=lld

PROJECT_ROOT=$(pwd)

ARCH_MESON="${PROJECT_ROOT}/arch.meson"
ARCH_BUILD="${ARCH_MESON}/arch.build"
MESON_BUILD='packaging/linux/distro/arch/meson.build'

rm -rf ${ARCH_MESON}
mkdir -p ${ARCH_MESON}
cp -v ${MESON_BUILD} ${ARCH_MESON}
ln -sv ${PROJECT_ROOT}/src ${ARCH_MESON}/src
ln -sv ${PROJECT_ROOT}/h ${ARCH_MESON}/h

cd ${ARCH_MESON} && meson setup ${ARCH_BUILD}

cd ${ARCH_BUILD} && ninja
