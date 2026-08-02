#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${ROOT_DIR}/build/browser-3ds"
CIA_DIR="${BUILD_DIR}/cia"
TOOLS_DIR="${BUILD_DIR}/cia-tools"
ELF="${BUILD_DIR}/links-3ds-browser.elf"
CIA="${BUILD_DIR}/links-3ds-browser.cia"

mkdir -p "${CIA_DIR}" "${TOOLS_DIR}"
test -s "${ELF}"

install_tool() {
    local name=$1
    local url=$2
    local archive=$3
    local member=$4
    local expected_sha=${5:-}

    if command -v "${name}" >/dev/null 2>&1; then
        command -v "${name}"
        return
    fi

    curl --fail --location --retry 3 "${url}" -o "${TOOLS_DIR}/${archive}"
    if [[ -n "${expected_sha}" ]]; then
        echo "${expected_sha}  ${TOOLS_DIR}/${archive}" | sha256sum -c -
    fi
    python3 - "${TOOLS_DIR}/${archive}" "${member}" "${TOOLS_DIR}/${name}" <<'PY'
import sys
import zipfile
from pathlib import Path

archive, member, output = map(Path, sys.argv[1:])
with zipfile.ZipFile(archive) as source:
    output.write_bytes(source.read(str(member)))
output.chmod(0o755)
PY
    printf '%s\n' "${TOOLS_DIR}/${name}"
}

MAKEROM=$(install_tool \
    makerom \
    "https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.19.0/makerom-v0.19.0-ubuntu_x86_64.zip" \
    makerom.zip \
    makerom \
    287b809dec064e0ad597e3d272c49ecb7eed41693d5ee6fef9d8a8aa24c2497e)

BANNERTOOL=$(install_tool \
    bannertool \
    "https://github.com/diasurgical/bannertool/releases/download/1.2.0/bannertool.zip" \
    bannertool.zip \
    linux-x86_64/bannertool)

python3 "${ROOT_DIR}/scripts/generate-cia-assets.py" "${CIA_DIR}"

"${BANNERTOOL}" makesmdh \
    -s "Links 3DS" \
    -l "Links 2.30 Browser for Nintendo 3DS" \
    -p "Links Project / Azimn" \
    -i "${CIA_DIR}/icon.png" \
    -o "${CIA_DIR}/icon.icn"

"${BANNERTOOL}" makebanner \
    -i "${CIA_DIR}/banner.png" \
    -a "${CIA_DIR}/banner.wav" \
    -o "${CIA_DIR}/banner.bnr"

"${MAKEROM}" \
    -f cia \
    -o "${CIA}" \
    -target t \
    -elf "${ELF}" \
    -rsf "${ROOT_DIR}/cia/links-3ds.rsf" \
    -icon "${CIA_DIR}/icon.icn" \
    -banner "${CIA_DIR}/banner.bnr" \
    -exefslogo

test -s "${CIA}"
sha256sum "${CIA}" >"${BUILD_DIR}/CIA-SHA256SUMS.txt"
printf 'Built %s\n' "${CIA}"
