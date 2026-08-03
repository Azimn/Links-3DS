#!/usr/bin/env python3
"""Compile every configured Links object and report the complete portability set."""

from __future__ import annotations

import argparse
import re
import shlex
import subprocess
from collections import defaultdict
from pathlib import Path

CATEGORIES = (
    ("Missing declaration or header", re.compile(r"implicit declaration|undeclared|unknown type name", re.I)),
    ("Socket ABI mismatch", re.compile(r"getsockopt|setsockopt|accept\(|recvfrom|socklen_t|incompatible pointer type", re.I)),
    ("Unsupported process API", re.compile(r"fork|vfork|exec|waitpid|popen", re.I)),
    ("Filesystem portability", re.compile(r"dirent|stat\b|chmod|readlink|symlink|truncate", re.I)),
    ("Time portability", re.compile(r"timeval|gettimeofday|clock_gettime|timezone", re.I)),
    ("Terminal or event API", re.compile(r"ioctl|termios|poll\b|epoll|kqueue", re.I)),
    ("Type or configuration mismatch", re.compile(r"conflicting types|redefinition|incompatible type|invalid conversion", re.I)),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--source-dir", required=True, type=Path)
    parser.add_argument("--object-dir", required=True, type=Path)
    parser.add_argument("--log-dir", required=True, type=Path)
    parser.add_argument("--objects-out", required=True, type=Path)
    parser.add_argument("--report-out", type=Path)
    parser.add_argument("--compiler", default="arm-none-eabi-gcc")
    parser.add_argument("cflags", nargs=argparse.REMAINDER)
    return parser.parse_args()


def classify(text: str) -> str:
    for name, pattern in CATEGORIES:
        if pattern.search(text):
            return name
    return "Other compile failure"


def main() -> int:
    args = parse_args()
    cflags = args.cflags[1:] if args.cflags and args.cflags[0] == "--" else args.cflags
    args.object_dir.mkdir(parents=True, exist_ok=True)
    args.log_dir.mkdir(parents=True, exist_ok=True)
    args.objects_out.parent.mkdir(parents=True, exist_ok=True)

    objects = [line.strip() for line in args.manifest.read_text().splitlines() if line.strip()]
    if not objects:
        raise SystemExit("configured object manifest is empty")

    successful: list[str] = []
    failures: list[tuple[str, Path, str]] = []

    for object_name in objects:
        base = object_name.removesuffix(".o")
        source = args.source_dir / f"{base}.c"
        output = args.object_dir / object_name
        log = args.log_dir / f"{base.replace('/', '__')}.log"

        if not source.is_file():
            text = f"Configured object has no direct C source: {source}\n"
            log.write_text(text, encoding="utf-8")
            failures.append((object_name, log, "Build manifest mismatch"))
            continue

        output.parent.mkdir(parents=True, exist_ok=True)
        command = [args.compiler, *cflags, "-c", str(source), "-o", str(output)]
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        text = "$ " + " ".join(shlex.quote(part) for part in command) + "\n" + result.stdout
        log.write_text(text, encoding="utf-8")
        if result.returncode == 0:
            successful.append(str(output))
        else:
            failures.append((object_name, log, classify(result.stdout)))

    args.objects_out.write_text("\n".join(successful) + ("\n" if successful else ""), encoding="utf-8")

    report_lines = [
        "Links 3DS upstream compile report",
        "=" * 34,
        f"Configured objects: {len(objects)}",
        f"Successful objects: {len(successful)}",
        f"Failed objects: {len(failures)}",
        "",
    ]
    grouped: dict[str, list[tuple[str, Path]]] = defaultdict(list)
    for object_name, log, category in failures:
        grouped[category].append((object_name, log))
    for category in sorted(grouped):
        report_lines.append(f"[{category}] {len(grouped[category])} object(s)")
        for object_name, log in grouped[category]:
            report_lines.append(f"  {object_name}: {log}")
        report_lines.append("")

    report = "\n".join(report_lines) + "\n"
    if args.report_out:
        args.report_out.parent.mkdir(parents=True, exist_ok=True)
        args.report_out.write_text(report, encoding="utf-8")
    print(report, end="")

    if failures:
        for object_name, log, category in failures:
            print(f"\n===== {category}: {object_name} =====")
            print(log.read_text(encoding="utf-8"), end="")
        return 1

    print(f"Compiled all {len(successful)} configured upstream objects")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
