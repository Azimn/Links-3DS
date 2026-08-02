#!/usr/bin/env python3
from pathlib import Path
import re
import sys


def patch_connect(path: Path) -> None:
    source = path.read_text(encoding="utf-8")
    if "getsockopt" not in source or "SO_ERROR" not in source:
        raise SystemExit("connect.c does not contain the expected getsockopt SO_ERROR path")

    replacements = 0

    patterns = [
        (
            re.compile(r"(?P<indent>^[ \t]*)int[ \t]+err[ \t]*,[ \t]*len[ \t]*;(?P<tail>[ \t]*\n)", re.MULTILINE),
            lambda m: f"{m.group('indent')}int err;{m.group('tail')}{m.group('indent')}socklen_t len;{m.group('tail')}",
        ),
        (
            re.compile(r"(?P<indent>^[ \t]*)int[ \t]+len[ \t]*;(?P<gap>[ \t]*\n[ \t]*)len[ \t]*=[ \t]*sizeof[ \t]+err[ \t]*;", re.MULTILINE),
            lambda m: f"{m.group('indent')}socklen_t len;{m.group('gap')}len = sizeof err;",
        ),
    ]

    for pattern, replacement in patterns:
        source, count = pattern.subn(replacement, source, count=1)
        replacements += count
        if count:
            break

    if replacements != 1:
        raise SystemExit("unable to identify the getsockopt length declaration in connect.c")

    if not re.search(r"getsockopt\s*\([^;]+&len\s*\)", source, re.DOTALL):
        raise SystemExit("patched connect.c no longer contains the expected getsockopt length argument")

    path.write_text(source, encoding="utf-8")


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("usage: prepare-upstream-3ds.py <links-source-dir>")

    root = Path(sys.argv[1])
    patch_connect(root / "connect.c")
    print(f"Prepared {root / 'connect.c'} for libctru socklen_t ABI")


if __name__ == "__main__":
    main()
