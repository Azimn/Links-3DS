#!/usr/bin/env python3
"""Generate deterministic PNG and WAV assets for the Links 3DS CIA."""

from __future__ import annotations

import binascii
import struct
import sys
import wave
import zlib
from pathlib import Path


def png_chunk(kind: bytes, data: bytes) -> bytes:
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", binascii.crc32(kind + data) & 0xFFFFFFFF)


def write_png(path: Path, width: int, height: int, pixel) -> None:
    rows = bytearray()
    for y in range(height):
        rows.append(0)
        for x in range(width):
            rows.extend(pixel(x, y))
    payload = b"\x89PNG\r\n\x1a\n"
    payload += png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
    payload += png_chunk(b"IDAT", zlib.compress(bytes(rows), 9))
    payload += png_chunk(b"IEND", b"")
    path.write_bytes(payload)


def icon_pixel(x: int, y: int) -> tuple[int, int, int, int]:
    cx, cy = 23.5, 23.5
    dx, dy = x - cx, y - cy
    r2 = dx * dx + dy * dy
    if r2 > 22 * 22:
        return (18, 25, 39, 255)
    if r2 > 19 * 19:
        return (220, 232, 244, 255)
    if abs(dx) < 2 or abs(dy) < 2 or abs(r2 - 12 * 12) < 28:
        return (220, 232, 244, 255)
    if x < 24:
        return (36, 107, 196, 255)
    return (62, 154, 89, 255)


def banner_pixel(x: int, y: int) -> tuple[int, int, int, int]:
    if y < 12 or y >= 116:
        return (18, 25, 39, 255)
    stripe = (x // 32) % 2
    base = (31, 86, 161) if stripe == 0 else (40, 132, 84)
    if 28 <= x < 228 and 34 <= y < 94:
        return (230, 237, 246, 255)
    if 34 <= x < 222 and 40 <= y < 88:
        return (18, 25, 39, 255)
    return (*base, 255)


def write_silence(path: Path) -> None:
    with wave.open(str(path), "wb") as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(22050)
        output.writeframes(b"\x00\x00" * 22050)


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} OUTPUT_DIR", file=sys.stderr)
        return 2
    output = Path(sys.argv[1])
    output.mkdir(parents=True, exist_ok=True)
    write_png(output / "icon.png", 48, 48, icon_pixel)
    write_png(output / "banner.png", 256, 128, banner_pixel)
    write_silence(output / "banner.wav")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
