#!/usr/bin/env python3
from pathlib import Path
import re
import sys


def diagnostic_context(source: str, needle: str = "getsockopt", radius: int = 8) -> str:
    lines = source.splitlines()
    hits = [index for index, line in enumerate(lines) if needle in line]
    output = [f"===== {needle} locations ====="]
    if not hits:
        output.append(f"No {needle} call found")
        return "\n".join(output)

    for index in hits:
        output.append(f"line {index + 1}: {lines[index]}")

    output.append(f"===== context around first {needle} =====")
    start = max(0, hits[0] - radius)
    end = min(len(lines), hits[0] + radius + 1)
    for index in range(start, end):
        output.append(f"{index + 1:5}: {lines[index]}")
    return "\n".join(output)


def extract_balanced_call(source: str, function_name: str, start: int) -> tuple[str, int, int]:
    open_paren = source.find("(", start + len(function_name))
    if open_paren < 0:
        raise ValueError(f"{function_name} call has no opening parenthesis")

    depth = 0
    in_string: str | None = None
    escaped = False
    for index in range(open_paren, len(source)):
        char = source[index]
        if in_string is not None:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == in_string:
                in_string = None
            continue
        if char in {'"', "'"}:
            in_string = char
        elif char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
            if depth == 0:
                return source[start:index + 1], start, index + 1
    raise ValueError(f"unterminated {function_name} call")


def find_so_error_getsockopt(source: str) -> tuple[str, int, int]:
    for match in re.finditer(r"\bgetsockopt\b", source):
        call, start, end = extract_balanced_call(source, "getsockopt", match.start())
        if re.search(r"\bSO_ERROR\b", call):
            return call, start, end
    raise ValueError("no getsockopt SO_ERROR call found")


def split_declarators(body: str) -> list[str]:
    parts: list[str] = []
    start = 0
    depth = 0
    for index, char in enumerate(body):
        if char in "([{":
            depth += 1
        elif char in ")]}" and depth:
            depth -= 1
        elif char == "," and depth == 0:
            parts.append(body[start:index].strip())
            start = index + 1
    parts.append(body[start:].strip())
    return [part for part in parts if part]


def patch_length_declaration(source: str, variable: str, call_start: int) -> tuple[str, bool]:
    declaration = re.compile(
        rf"(?m)^(?P<indent>[ \t]*)(?P<type>socklen_t|(?:unsigned\s+)?int)"
        rf"(?P<spacing>[ \t]+)(?P<body>[^;\n]*\b{re.escape(variable)}\b[^;\n]*);"
    )
    candidates = [match for match in declaration.finditer(source, 0, call_start)]
    if not candidates:
        raise ValueError(f"no declaration for getsockopt length variable '{variable}' was found")

    match = candidates[-1]
    if match.group("type") == "socklen_t":
        return source, False

    declarators = split_declarators(match.group("body"))
    selected = [item for item in declarators if re.search(rf"\b{re.escape(variable)}\b", item)]
    remaining = [item for item in declarators if item not in selected]
    if len(selected) != 1:
        raise ValueError(
            f"unable to isolate declaration for getsockopt length variable '{variable}'"
        )

    indent = match.group("indent")
    replacement_lines: list[str] = []
    if remaining:
        replacement_lines.append(
            f"{indent}{match.group('type')}{match.group('spacing')}{', '.join(remaining)};"
        )
    replacement_lines.append(f"{indent}socklen_t {selected[0]};")
    replacement = "\n".join(replacement_lines)
    return source[:match.start()] + replacement + source[match.end():], True


def patch_connect(path: Path) -> None:
    source = path.read_text(encoding="utf-8")
    try:
        call, call_start, _ = find_so_error_getsockopt(source)
        arguments = re.findall(r"&\s*([A-Za-z_]\w*)", call)
        if not arguments:
            raise ValueError("getsockopt SO_ERROR call has no address-valued length argument")
        length_variable = arguments[-1]
        source, changed = patch_length_declaration(source, length_variable, call_start)

        patched_call, _, _ = find_so_error_getsockopt(source)
        if not re.search(rf"&\s*{re.escape(length_variable)}\b", patched_call):
            raise ValueError("patched call no longer uses the detected length variable")
        if not re.search(
            rf"(?m)^\s*socklen_t\s+[^;]*\b{re.escape(length_variable)}\b[^;]*;",
            source[: source.find(patched_call)],
        ):
            raise ValueError("socklen_t declaration was not present after patching")
    except ValueError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        print(diagnostic_context(source), file=sys.stderr)
        raise SystemExit(1) from error

    if changed:
        path.write_text(source, encoding="utf-8")
        print(
            f"Patched {path}: getsockopt length variable '{length_variable}' now uses socklen_t"
        )
    else:
        print(
            f"Verified {path}: getsockopt length variable '{length_variable}' already uses socklen_t"
        )


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("usage: prepare-upstream-3ds.py <links-source-dir>")

    root = Path(sys.argv[1])
    connect = root / "connect.c"
    if not connect.is_file():
        raise SystemExit(f"connect.c not found: {connect}")
    patch_connect(connect)


if __name__ == "__main__":
    main()
