#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${ROOT_DIR}/build/runtime-smoke-3ds"

: "${DEVKITPRO:?DEVKITPRO must be set}"
: "${DEVKITARM:?DEVKITARM must be set}"
export PATH="${DEVKITARM}/bin:${DEVKITPRO}/tools/bin:${PATH}"

mkdir -p "${BUILD_DIR}"

arm-none-eabi-gcc \
    -std=gnu11 -Os -Wall -Wextra -Werror \
    -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft \
    -mword-relocations -ffunction-sections -fdata-sections \
    -I"${DEVKITPRO}/libctru/include" \
    -c "${ROOT_DIR}/source/runtime_smoke_3ds.c" \
    -o "${BUILD_DIR}/runtime_smoke_3ds.o"

arm-none-eabi-gcc \
    -specs=3dsx.specs \
    -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft \
    -Wl,--gc-sections \
    "${BUILD_DIR}/runtime_smoke_3ds.o" \
    -L"${DEVKITPRO}/libctru/lib" -lctru -lm \
    -o "${BUILD_DIR}/links-3ds-runtime-smoke.elf"

3dsxtool "${BUILD_DIR}/links-3ds-runtime-smoke.elf" \
    "${BUILD_DIR}/links-3ds-runtime-smoke.3dsx"

arm-none-eabi-size "${BUILD_DIR}/links-3ds-runtime-smoke.elf" \
    >"${BUILD_DIR}/size-report.txt"
sha256sum "${BUILD_DIR}/links-3ds-runtime-smoke.elf" \
          "${BUILD_DIR}/links-3ds-runtime-smoke.3dsx" \
    >"${BUILD_DIR}/SHA256SUMS.txt"
