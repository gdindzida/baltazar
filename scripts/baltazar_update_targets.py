 #!/usr/bin/env python3
"""
Recursively collect all CMake targets starting from a folder
and save them to a text file.
"""

import re
from pathlib import Path
from typing import List, Set

# Regex patterns
TARGET_PATTERNS = [
    re.compile(r"^\s*add_executable\s*\(\s*([A-Za-z0-9_\-]+)"),
    re.compile(r"^\s*add_library\s*\(\s*([A-Za-z0-9_\-]+)"),
]
SUBDIR_PATTERN = re.compile(r"^\s*add_subdirectory\s*\(\s*([^\s\)]+)")

def parse_cmake_file(cmake_path: Path, visited: Set[Path]) -> List[str]:
    """Parse a CMakeLists.txt file for targets and recurse into subdirectories."""
    targets: List[str] = []
    if cmake_path in visited:
        return targets
    visited.add(cmake_path)

    try:
        content = cmake_path.read_text(encoding="utf-8")
    except Exception as e:
        print(f"Failed to read {cmake_path}: {e}")
        return targets

    lines = content.splitlines()

    for line in lines:
        # Find targets
        for pat in TARGET_PATTERNS:
            m = pat.search(line)
            if m:
                targets.append(m.group(1))

    # Find subdirectories
    for line in lines:
        m = SUBDIR_PATTERN.search(line)
        if m:
            subdir = m.group(1).strip()
            subpath = (cmake_path.parent / subdir).resolve()
            cmake_sub = subpath / "CMakeLists.txt"
            if cmake_sub.exists():
                targets.extend(parse_cmake_file(cmake_sub, visited))

    return targets


def collect_targets(start_dir: str) -> List[str]:
    """Entry point: start from a folder and return all targets."""
    start_path = Path(start_dir).resolve()
    cmake_file = start_path / "CMakeLists.txt"
    if not cmake_file.exists():
        raise FileNotFoundError(f"No CMakeLists.txt found in {start_path}")
    return parse_cmake_file(cmake_file, set())


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Collect CMake targets recursively.")
    parser.add_argument("directory", help="Path to the root folder containing CMakeLists.txt")
    parser.add_argument(
        "-o", "--output",
        default="cmake_targets.txt",
        help="Path to the output file (default: cmake_targets.txt)"
    )
    args = parser.parse_args()

    all_targets = collect_targets(args.directory)

    out_path = Path(args.output).resolve()
    out_path.write_text("\n".join(all_targets), encoding="utf-8")
    print(f"Saved {len(all_targets)} targets to {out_path}")
