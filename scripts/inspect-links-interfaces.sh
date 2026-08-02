#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
SOURCE_DIR="${ROOT_DIR}/upstream/links-2.30"
OUTPUT="${1:-${ROOT_DIR}/artifacts/links-2.30-interface-report.txt}"

if [ ! -f "${SOURCE_DIR}/links.h" ]; then
    printf '%s\n' "Links 2.30 source is missing. Run scripts/fetch-links.sh first." >&2
    exit 1
fi

mkdir -p "$(dirname -- "${OUTPUT}")"

extract_block() {
    file=$1
    pattern=$2
    lines=$3

    printf '\n===== %s: %s =====\n' "${file}" "${pattern}"
    grep -n -A "${lines}" -B 4 "${pattern}" "${SOURCE_DIR}/${file}" || true
}

{
    printf '%s\n' "Links 2.30 source interface audit"
    printf '%s\n' "Archive SHA-256: c4631c6b5a11527cdc3cb7872fc23b7f2b25c2b021d596be410dadb40315f166"
    printf '%s\n' "Generated from the verified upstream archive."

    extract_block links.h "struct graphics_driver" 140
    extract_block links.h "struct graphics_device" 100
    extract_block links.h "struct bitmap" 60
    extract_block drivers.c "graphics_drivers" 80
    extract_block sched.c "install_timer" 80
    extract_block sched.c "register_bottom_half" 60
    extract_block configure.in "GRDRV_FB" 80
    extract_block Makefile.in "drivers.o" 40
} > "${OUTPUT}"

printf '%s\n' "Wrote ${OUTPUT}"
