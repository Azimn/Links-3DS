#!/usr/bin/env sh
set -eu

VERSION="2.30"
ARCHIVE="links-${VERSION}.tar.bz2"
URL="https://links.twibright.com/download/${ARCHIVE}"
ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
UPSTREAM_DIR="${ROOT_DIR}/upstream"
DESTINATION="${UPSTREAM_DIR}/links-${VERSION}"
CACHE_DIR="${ROOT_DIR}/.cache"
ARCHIVE_PATH="${CACHE_DIR}/${ARCHIVE}"

if [ -d "${DESTINATION}" ]; then
    printf '%s\n' "${DESTINATION} already exists"
    exit 0
fi

mkdir -p "${UPSTREAM_DIR}" "${CACHE_DIR}"

if [ ! -f "${ARCHIVE_PATH}" ]; then
    if command -v curl >/dev/null 2>&1; then
        curl --fail --location --output "${ARCHIVE_PATH}" "${URL}"
    elif command -v wget >/dev/null 2>&1; then
        wget --output-document="${ARCHIVE_PATH}" "${URL}"
    else
        printf '%s\n' "curl or wget is required" >&2
        exit 1
    fi
fi

tar -xjf "${ARCHIVE_PATH}" -C "${UPSTREAM_DIR}"
printf '%s\n' "Extracted Links ${VERSION} to ${DESTINATION}"
