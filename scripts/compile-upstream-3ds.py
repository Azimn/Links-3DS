#!/usr/bin/env python3
"""Compile the configured Links object manifest for 3DS and collect all failures.

This deliberately compiles every configured object before returning failure, so one
CI run exposes the complete current portability boundary instead of stopping at the
first source file.
"""

from __future__ import annotations

import argparse
import shlex
import subprocess
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--source-dir", required=True, type=Path)
    parser.add_argument("--object-dir", required=True, type=Path)
    parser.add_argument("--log-dir", required=True, type=Path)
    parser.add_argument("--objects-out", required=True, type=Path)
    parser.add_argument("--compiler", default="arm-none-eabi-gcc")
    parser.add_argument("cflags", nargs=argparse.REMAINDER)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    cflags = args.cflags
    if cflags and cflags[0] == "--":
        cflags = cflags[1:]

    args.object_dir.mkdir(parents=True, exist_ok=True)
    args.log_dir.mkdir(parents=True, exist_ok=True)
    args.objects_out.parent.mkdir(parents=True, exist_ok=True)

    objects = [line.strip() for line in args.manifest.read_text().splitlines() if line.strip()]
    if not objects:
        raise SystemExit("configured object manifest is empty")

    successful: list[str] = []
    failures: list[tuple[str, Path]] = []

    for object_name in objects:
        base = object_name.removesuffix(".o")
        source = args.source_dir / f"{base}.c"
        output = args.object_dir / object_name
        log = args.log_dir / f"{base.replace('/', '__')}.log"

        if not source.is_file():
            log.write_text(f"Configured object has no direct C source: {source}\n")
            failures.append((object_name, log))
            continue

        output.parent.mkdir(parents=True, exist_ok=True)
        command = [args.compiler, *cflags, "-c", str(source), "-o", str(output)]
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        log.write_text(
            "$ " + " ".join(shlex.quote(part) for part in command) + "\n" + result.stdout,
            encoding="utf-8",
        )

        if result.returncode == 0:
            successful.append(str(output))
        else:
            failures.append((object_name, log))

    args.objects_out.write_text("\n".join(successful) + ("\n" if successful else ""), encoding="utf-8")

    if failures:
        print(f"Upstream compilation failed for {len(failures)} of {len(objects)} objects:")
        for object_name, log in failures:
            print(f"\n===== {object_name} =====")
            print(log.read_text(encoding="utf-8"), end="")
        return 1

    print(f"Compiled all {len(successful)} configured upstream objects")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
