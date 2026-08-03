#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
from collections import defaultdict
from pathlib import Path

RULES = {
    "Process API": ["fork", "vfork", "execve", "execvp", "waitpid", "popen"],
    "Socket ABI": ["getsockopt", "setsockopt", "accept", "recvfrom", "getpeername", "getsockname"],
    "Event API": ["poll", "epoll_create", "epoll_wait", "kqueue"],
    "Terminal or ioctl": ["ioctl", "tcgetattr", "tcsetattr", "isatty"],
    "Signals": ["signal", "sigaction", "sigprocmask", "alarm"],
    "Threads": ["pthread_create", "pthread_mutex", "pthread_cond"],
    "Time": ["gettimeofday", "clock_gettime", "nanosleep", "usleep"],
    "Filesystem": ["readlink", "symlink", "chmod", "fchmod", "truncate", "ftruncate"],
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("source_dir", type=Path)
    parser.add_argument("output", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    findings: dict[str, list[tuple[str, int, str]]] = defaultdict(list)
    patterns = {
        category: re.compile(r"\b(" + "|".join(map(re.escape, names)) + r")\s*\(")
        for category, names in RULES.items()
    }

    for path in sorted(args.source_dir.glob("*.c")):
        for line_number, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
            stripped = line.strip()
            if stripped.startswith("//") or stripped.startswith("/*") or stripped.startswith("*"):
                continue
            for category, pattern in patterns.items():
                for match in pattern.finditer(line):
                    findings[category].append((path.name, line_number, match.group(1)))

    lines = ["Links 2.30 3DS portability scan", "=" * 35, ""]
    total = 0
    for category in RULES:
        entries = findings.get(category, [])
        total += len(entries)
        lines.append(f"[{category}] {len(entries)} occurrence(s)")
        for filename, line_number, api in entries:
            lines.append(f"  {filename}:{line_number}: {api}")
        lines.append("")
    lines.append(f"Total flagged occurrences: {total}")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {args.output} with {total} flagged occurrence(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
