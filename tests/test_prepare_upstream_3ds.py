#!/usr/bin/env python3
import subprocess
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "prepare-upstream-3ds.py"


def fixture(declaration: str, length_name: str = "len") -> str:
    return f'''#include <sys/socket.h>
void connected(void *p)
{{
    int err = 0;
    {declaration}
    int rs;
    rs = getsockopt(1, SOL_SOCKET, SO_ERROR, (void *)&err, &{length_name});
}}
'''


class PrepareUpstream3DSTest(unittest.TestCase):
    def run_case(self, source: str, expected: str) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "connect.c"
            path.write_text(source, encoding="utf-8")
            subprocess.run(["python3", str(SCRIPT), str(root)], check=True)
            patched = path.read_text(encoding="utf-8")
            self.assertIn(expected, patched)
            subprocess.run(["python3", str(SCRIPT), str(root)], check=True)

    def test_initialized_int(self) -> None:
        self.run_case(fixture("int len = sizeof(int);"), "socklen_t len = sizeof(int);")

    def test_unsigned_custom_name(self) -> None:
        self.run_case(
            fixture("unsigned int optlen = sizeof(err);", "optlen"),
            "socklen_t optlen = sizeof(err);",
        )

    def test_combined_declaration(self) -> None:
        source = '''#include <sys/socket.h>
void connected(void *p)
{
    int err = 0, len = sizeof(int);
    int rs;
    rs = getsockopt(1, SOL_SOCKET, SO_ERROR, (void *)&err, &len);
}
'''
        self.run_case(source, "socklen_t len = sizeof(int);")

    def test_existing_socklen_t_is_idempotent(self) -> None:
        self.run_case(fixture("socklen_t len = sizeof(int);"), "socklen_t len = sizeof(int);")


if __name__ == "__main__":
    unittest.main()
