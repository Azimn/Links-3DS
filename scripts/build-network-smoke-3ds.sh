#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${ROOT_DIR}/build/network-smoke-3ds"
SOURCE="${ROOT_DIR}/source/network_smoke_3ds.c"
ELF="${BUILD_DIR}/links-3ds-network-smoke.elf"
THREEDSX="${BUILD_DIR}/links-3ds-network-smoke.3dsx"

: "${DEVKITPRO:?DEVKITPRO must be set}"
: "${DEVKITARM:?DEVKITARM must be set}"

export PATH="${DEVKITARM}/bin:${DEVKITPRO}/tools/bin:${PATH}"
mkdir -p "${BUILD_DIR}"

arm-none-eabi-gcc \
    -std=gnu11 -Os -Wall -Wextra -Werror \
    -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft \
    -mword-relocations -ffunction-sections -fdata-sections \
    -I"${DEVKITPRO}/libctru/include" \
    -c "${SOURCE}" -o "${BUILD_DIR}/network_smoke_3ds.o"

arm-none-eabi-gcc \
    -specs=3dsx.specs \
    -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft \
    -Wl,--gc-sections \
    "${BUILD_DIR}/network_smoke_3ds.o" \
    -L"${DEVKITPRO}/libctru/lib" -lctru -lm \
    -o "${ELF}"

3dsxtool "${ELF}" "${THREEDSX}"
arm-none-eabi-size "${ELF}" >"${BUILD_DIR}/size-report.txt"
sha256sum "${ELF}" "${THREEDSX}" >"${BUILD_DIR}/SHA256SUMS.txt"
printf 'Built %s\n' "${THREEDSX}"
