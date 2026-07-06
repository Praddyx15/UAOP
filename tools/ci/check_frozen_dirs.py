#!/usr/bin/env python3
"""Frozen-directory guard (PROJECT_STRUCTURE.md section 2, R3/F24).

Fails if backend/services/dji-service or backend/services/fpv-service contain
anything besides FROZEN.md and .gitkeep, until an ADR resolves OQ-1.
"""

import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
FROZEN_DIRS = [
    REPO / "backend" / "services" / "dji-service",
    REPO / "backend" / "services" / "fpv-service",
]
ALLOWED = {"FROZEN.md", ".gitkeep"}


def main() -> int:
    violations: list[str] = []
    for d in FROZEN_DIRS:
        if not d.is_dir():
            violations.append(f"{d.relative_to(REPO)}: frozen directory missing entirely")
            continue
        for p in d.rglob("*"):
            if p.is_file() and p.name not in ALLOWED:
                violations.append(str(p.relative_to(REPO)))
    if violations:
        print("FROZEN-DIRECTORY GUARD FAILED (OQ-1 unresolved — see DECISIONS.md):")
        for v in violations:
            print(f"  {v}")
        return 1
    print("frozen-dir guard: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
