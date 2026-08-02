#!/usr/bin/env sh
set -eu

VERSION="2.30"
ARCHIVE="links-${VERSION}.tar.bz2"
EXPECTED_SHA256="c4631c6b5a11527cdc3cb7872fc23b7f2b25c2b021d596be410dadb40315f166"
ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
UPSTREAM_DIR="${ROOT_DIR}/upstream"
DESTINATION="${UPSTREAM_DIR}/links-${VERSION}"
CACHE_DIR="${ROOT_DIR}/.cache"
ARCHIVE_PATH="${CACHE_DIR}/${ARCHIVE}"

URLS="
https://links.twibright.com/download/${ARCHIVE}
https://mirrors.omnios.org/links/${ARCHIVE}
https://fossies.org/linux/www/${ARCHIVE}
"

verify_archive() {
    actual=$(sha256sum "${ARCHIVE_PATH}" | awk '{print $1}')
    if [ "${actual}" != "${EXPECTED_SHA256}" ]; then
        printf '%s\n' "Checksum mismatch for ${ARCHIVE_PATH}" >&2
        printf '%s\n' "expected: ${EXPECTED_SHA256}" >&2
        printf '%s\n' "actual:   ${actual}" >&2
        return 1
    fi
}

download_archive() {
    for url in ${URLS}; do
        printf '%s\n' "Downloading ${url}"
        rm -f "${ARCHIVE_PATH}.part"

        if command -v curl >/dev/null 2>&1; then
            if curl --fail --location --retry 3 --connect-timeout 20 \
                --output "${ARCHIVE_PATH}.part" "${url}"; then
                mv "${ARCHIVE_PATH}.part" "${ARCHIVE_PATH}"
                return 0
            fi
        elif command -v wget >/dev/null 2>&1; then
            if wget --tries=3 --timeout=20 \
                --output-document="${ARCHIVE_PATH}.part" "${url}"; then
                mv "${ARCHIVE_PATH}.part" "${ARCHIVE_PATH}"
                return 0
            fi
        else
            printf '%s\n' "curl or wget is required" >&2
            return 1
        fi
    done

    printf '%s\n' "Unable to download ${ARCHIVE} from any configured mirror" >&2
    return 1
}

if [ -d "${DESTINATION}" ]; then
    printf '%s\n' "${DESTINATION} already exists"
    exit 0
fi

mkdir -p "${UPSTREAM_DIR}" "${CACHE_DIR}"

if [ -f "${ARCHIVE_PATH}" ] && ! verify_archive; then
    printf '%s\n' "Discarding invalid cached archive" >&2
    rm -f "${ARCHIVE_PATH}"
fi

if [ ! -f "${ARCHIVE_PATH}" ]; then
    download_archive
fi

verify_archive

tar -xjf "${ARCHIVE_PATH}" -C "${UPSTREAM_DIR}"
test -f "${DESTINATION}/links.h"
test -f "${DESTINATION}/drivers.c"
test -f "${DESTINATION}/sched.c"

printf '%s\n' "Extracted verified Links ${VERSION} to ${DESTINATION}"
