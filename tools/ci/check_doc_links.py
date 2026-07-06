#!/usr/bin/env python3
"""Documentation link checker (DOCUMENTATION_PROCESS.md section 6).

Verifies every relative .md link in docs/, README.md, CLAUDE.md, AGENTS.md, and
in-tree READMEs resolves to an existing file. ADR references (ADR-NNNN) in docs/
must exist in DECISIONS.md.
"""

import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
LINK_RE = re.compile(r"\]\(([^)#:\s]+\.md)(#[^)]*)?\)")
ADR_RE = re.compile(r"ADR-(\d{4})")
SKIP_DIRS = {".git", "build", "node_modules", "external-resources", ".claude"}


def md_files() -> list[Path]:
    files = []
    for p in REPO.rglob("*.md"):
        if not any(part in SKIP_DIRS for part in p.parts):
            files.append(p)
    return files


def main() -> int:
    errors: list[str] = []

    for f in md_files():
        text = f.read_text(encoding="utf-8", errors="replace")
        for m in LINK_RE.finditer(text):
            target = (f.parent / m.group(1)).resolve()
            if not target.exists():
                errors.append(f"{f.relative_to(REPO)} -> broken link {m.group(1)}")

    decisions = REPO / "docs" / "00_Project_Foundation" / "DECISIONS.md"
    defined = set(re.findall(r"^## ADR-(\d{4})", decisions.read_text(encoding="utf-8"), re.M))
    for f in md_files():
        for m in ADR_RE.finditer(f.read_text(encoding="utf-8", errors="replace")):
            if m.group(1) not in defined:
                errors.append(f"{f.relative_to(REPO)} -> undefined ADR-{m.group(1)}")

    if errors:
        print("DOC LINK CHECK FAILED:")
        for e in sorted(set(errors)):
            print(f"  {e}")
        return 1
    print(f"doc-link check: OK ({len(md_files())} files, {len(defined)} ADRs)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
