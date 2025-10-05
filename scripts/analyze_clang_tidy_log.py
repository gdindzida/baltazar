#!/usr/bin/env python3
"""
Parse a clang-tidy log and count warnings per check type.

Usage:
    python analyze_tidy_warnings.py path/to/tidy.log
"""

import re
import sys
from collections import Counter
from typing import Tuple


def parse_tidy_log(filename: str) -> Tuple[int, Counter[str]]:
    """
    Parse a clang-tidy log file and return:
      - total number of warning lines
      - a Counter mapping each check name to its count

    Args:
        filename: Path to the clang-tidy log file.

    Returns:
        (total_warnings, check_counts)
    """
    check_pattern: re.Pattern[str] = re.compile(r'\[([a-zA-Z0-9_\-.,]+)\]')
    total_warnings: int = 0
    check_counts: Counter[str] = Counter()

    with open(filename, encoding='utf-8', errors='ignore') as f:
        for line in f:
            if "warning:" not in line:
                continue
            total_warnings += 1

            match: re.Match[str] | None = check_pattern.search(line)
            if match:
                raw: str = match.group(1)
                checks: list[str] = [c.strip() for c in raw.split(",") if c.strip()]
                for check in checks:
                    check_counts[check] += 1
            else:
                check_counts["<unknown>"] += 1

    return total_warnings, check_counts


def main() -> None:
    """
    Entry point for the script. Expects a single argument (log file path).
    Prints a summary of warnings and per-check counts.
    """
    if len(sys.argv) != 2:
        print("Usage: python analyze_tidy_warnings.py path/to/tidy.log")
        sys.exit(1)

    filename: str = sys.argv[1]
    total: int
    counts: Counter[str]
    total, counts = parse_tidy_log(filename)

    print(f"\n📊 Clang-Tidy Warning Summary for {filename}")
    print("-------------------------------------------------")
    print(f"Total warnings (lines with 'warning:'): {total}\n")

    print("Per-check counts:")
    print("-----------------")
    for check, count in sorted(counts.items(), key=lambda x: (-x[1], x[0])):
        print(f"{check:45s} {count}")

    print("-------------------------------------------------")
    print("Tip: disable specific checks in your .clang-tidy via:")
    print("      Checks: '-check-name'")
    print()


if __name__ == "__main__":
    main()
