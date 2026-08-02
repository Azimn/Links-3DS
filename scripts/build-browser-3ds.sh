#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
UPSTREAM_DIR="${ROOT_DIR}/upstream/links-2.30"
BUILD_DIR="${ROOT_DIR}/build/browser-3ds"
OBJ_DIR="${BUILD_DIR}/obj"
LOG_DIR="${BUILD_DIR}/logs"
MANIFEST="${BUILD_DIR}/links-objects.txt"

: "${DEVKITPRO:?DEVKITPRO must be set}"
: "${DEVKITARM:?DEVKITARM must be set}"

export PATH="${DEVKITARM}/bin:${DEVKITPRO}/tools/bin:${PATH}"

mkdir -p "${OBJ_DIR}" "${LOG_DIR}"

"${ROOT_DIR}/scripts/fetch-links.sh"
python3 "${ROOT_DIR}/scripts/prepare-graphics-3ds.py"

grep -q '\.param = NULL' "${ROOT_DIR}/src-3ds/graphics_3ds.c"
grep -q 'links_3ds_event_bridge_attach(dev)' "${ROOT_DIR}/src-3ds/graphics_3ds.c"
grep -q 'links_3ds_platform_stop_input_timer()' "${ROOT_DIR}/src-3ds/graphics_3ds.c"

cd "${UPSTREAM_DIR}"

# Configure natively so feature tests can execute. Keep the first full-port
# milestone deliberately small by disabling optional rendering and crypto
# libraries when the corresponding configure switches are available.
CONFIGURE_ARGS=(
    --build=x86_64-pc-linux-gnu
    --host=x86_64-pc-linux-gnu
)

CONFIGURE_HELP=$(./configure --help 2>/dev/null || true)
for option in \
    --without-ssl \
    --without-png \
    --without-jpeg \
    --without-tiff \
    --without-webp \
    --without-svg \
    --without-freetype \
    --without-javascript \
    --without-libevent; do
    if grep -Fq -- "${option}" <<<"${CONFIGURE_HELP}"; then
        CONFIGURE_ARGS+=("${option}")
    fi
done

env CC=cc CFLAGS=-O2 ./configure "${CONFIGURE_ARGS[@]}" \
    >"${LOG_DIR}/configure-host.log" 2>&1

test -s cfg.h
test -s Makefile

# Register the staged driver only in the fetched working copy. The repository
# remains a clean patch layer over the pinned upstream release.
python3 - "${UPSTREAM_DIR}/drivers.c" <<'PY'
from pathlib import Path
import re
import sys

path = Path(sys.argv[1])
source = path.read_text(encoding="utf-8")

if "extern struct graphics_driver links_3ds_driver;" not in source:
    marker = re.search(r"struct\s+graphics_driver\s*\*\s*graphics_drivers\s*\[\s*\]\s*=\s*\{", source)
    if marker is None:
        raise SystemExit("unable to locate graphics_drivers array in drivers.c")
    source = source[:marker.start()] + "extern struct graphics_driver links_3ds_driver;\n\n" + source[marker.start():]

array = re.search(r"(struct\s+graphics_driver\s*\*\s*graphics_drivers\s*\[\s*\]\s*=\s*\{)", source)
if array is None:
    raise SystemExit("unable to locate graphics_drivers initializer")

if "&links_3ds_driver" not in source[array.end():]:
    source = source[:array.end()] + "\n\t&links_3ds_driver," + source[array.end():]

path.write_text(source, encoding="utf-8")
PY

# Ask the generated Makefile for its configured object manifest. Links 2.30
# keeps sources at the archive root, so object names map directly to .c files.
cat > "${BUILD_DIR}/print-objs.mk" <<EOF
include ${UPSTREAM_DIR}/Makefile
.PHONY: print-objs
print-objs:
	@printf '%s\n' "\$(OBJS)"
EOF

make -s -f "${BUILD_DIR}/print-objs.mk" print-objs \
    | tr ' ' '\n' \
    | sed '/^$/d' \
    | sort -u >"${MANIFEST}"

if ! grep -q '\.o$' "${MANIFEST}"; then
    echo "Generated Makefile did not expose an OBJS manifest" >&2
    grep -nE '(^|[[:space:]])(OBJS|OBJECTS)[[:space:]]*=' Makefile >&2 || true
    exit 1
fi

COMMON_CFLAGS=(
    -std=gnu11
    -Os
    -Wall
    -Wextra
    -Wno-unused-parameter
    -march=armv6k
    -mtune=mpcore
    -mfloat-abi=hard
    -mtp=soft
    -mword-relocations
    -ffunction-sections
    -fdata-sections
    -DG
    -DGRDRV_3DS
    -DHAVE_DIRENT_H=1
    -DHAVE_SYS_TIME_H=1
    -DHAVE_GETTIMEOFDAY=1
    -DHAVE_STRUCT_TIMEZONE=1
    -I"${DEVKITPRO}/libctru/include"
    -I"${UPSTREAM_DIR}"
    -I"${ROOT_DIR}/source"
    -I"${ROOT_DIR}/platform/links_3ds"
    -I"${ROOT_DIR}/src-3ds"
)

OBJECTS=()
while IFS= read -r object; do
    base=${object%.o}
    source="${UPSTREAM_DIR}/${base}.c"
    output="${OBJ_DIR}/upstream/${object}"

    if [[ ! -f "${source}" ]]; then
        echo "Configured object has no direct C source: ${object}" >&2
        exit 1
    fi

    mkdir -p "$(dirname -- "${output}")"
    arm-none-eabi-gcc "${COMMON_CFLAGS[@]}" -c "${source}" -o "${output}"
    OBJECTS+=("${output}")
done <"${MANIFEST}"

PORT_SOURCES=(
    "${ROOT_DIR}/source/gfx_3ds.c"
    "${ROOT_DIR}/src-3ds/graphics_3ds.c"
    "${ROOT_DIR}/platform/links_3ds/platform_3ds.c"
    "${ROOT_DIR}/platform/links_3ds/input_3ds.c"
    "${ROOT_DIR}/platform/links_3ds/timer_3ds.c"
    "${ROOT_DIR}/platform/links_3ds/event_bridge_3ds.c"
)

for source in "${PORT_SOURCES[@]}"; do
    relative=${source#"${ROOT_DIR}/"}
    output="${OBJ_DIR}/port/${relative%.c}.o"
    mkdir -p "$(dirname -- "${output}")"
    arm-none-eabi-gcc "${COMMON_CFLAGS[@]}" -Werror -c "${source}" -o "${output}"
    OBJECTS+=("${output}")
done

arm-none-eabi-gcc \
    -specs=3dsx.specs \
    -march=armv6k \
    -mtune=mpcore \
    -mfloat-abi=hard \
    -mtp=soft \
    -Wl,--gc-sections \
    -Wl,-Map,"${BUILD_DIR}/links-3ds-browser.map" \
    "${OBJECTS[@]}" \
    -L"${DEVKITPRO}/libctru/lib" \
    -lctru -lm \
    -o "${BUILD_DIR}/links-3ds-browser.elf"

3dsxtool "${BUILD_DIR}/links-3ds-browser.elf" \
    "${BUILD_DIR}/links-3ds-browser.3dsx"

arm-none-eabi-size "${BUILD_DIR}/links-3ds-browser.elf" \
    >"${BUILD_DIR}/size-report.txt"
sha256sum "${BUILD_DIR}/links-3ds-browser.elf" \
          "${BUILD_DIR}/links-3ds-browser.3dsx" \
    >"${BUILD_DIR}/SHA256SUMS.txt"

printf 'Built %s\n' "${BUILD_DIR}/links-3ds-browser.3dsx"
