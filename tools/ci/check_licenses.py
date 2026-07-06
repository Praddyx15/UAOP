#!/usr/bin/env python3
"""License-allowlist gate (ADR-0019, R5/F35).

Two checks over source code (never over docs, which legitimately discuss GPL):
1. GPL-text denylist — no source file may carry GPL/AGPL/LGPL license markers.
2. Provenance rule — any file declaring third-party origin must have a matching
   entry in THIRD_PARTY_NOTICES.md.
"""

import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
CODE_DIRS = ["api", "backend", "middleware", "frontend", "tools", "plugins",
             "integrations", "simulation", "compliance", "infrastructure", "tests"]
CODE_EXT = {".c", ".cc", ".cpp", ".h", ".hpp", ".py", ".qml", ".js", ".ts",
            ".cmake", ".txt", ".yaml", ".yml", ".sh", ".ps1"}
DENYLIST = [
    re.compile(r"GNU (Affero )?(Lesser )?General Public License", re.I),
    re.compile(r"\b[AL]?GPL-?[23]\.0(-only|-or-later)?\b"),
    re.compile(r"copyleft", re.I),
]
PROVENANCE_RE = re.compile(r"(?:Source|Ported from|Origin):\s*(?:https?://)?github\.com/([\w.-]+/[\w.-]+)", re.I)
SKIP_NAMES = {"check_licenses.py"}  # this file names the patterns it hunts


def main() -> int:
    errors: list[str] = []
    notices_path = REPO / "THIRD_PARTY_NOTICES.md"
    notices = notices_path.read_text(encoding="utf-8") if notices_path.exists() else ""

    for d in CODE_DIRS:
        base = REPO / d
        if not base.is_dir():
            continue
        for p in base.rglob("*"):
            if not p.is_file() or p.suffix.lower() not in CODE_EXT or p.name in SKIP_NAMES:
                continue
            if "build" in p.parts or "__pycache__" in p.parts:
                continue
            try:
                text = p.read_text(encoding="utf-8", errors="replace")
            except OSError:
                continue
            head = text[:4000]
            for pattern in DENYLIST:
                if pattern.search(head):
                    errors.append(
                        f"{p.relative_to(REPO)}: GPL-family license marker "
                        f"('{pattern.pattern}') — Tier 3 code never enters the tree (ADR-0019)")
                    break
            for m in PROVENANCE_RE.finditer(head):
                repo_name = m.group(1)
                if repo_name.lower() not in notices.lower():
                    errors.append(
                        f"{p.relative_to(REPO)}: provenance '{repo_name}' has no entry "
                        f"in THIRD_PARTY_NOTICES.md (ADR-0019 rule 1)")

    if errors:
        print("LICENSE GATE FAILED (ADR-0019):")
        for e in sorted(set(errors)):
            print(f"  {e}")
        return 1
    print("license gate: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
