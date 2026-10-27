# Token unit test for SUITE_TOKENS.json (STYLE_GUIDE.md §2 and §9): WCAG 2.x contrast
# for every text token on every surface, both themes, and the generated Theme.qml
# block in step with the JSON.
#
#   pytest tests/unit/test_suite_tokens.py

import json
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
TOKENS = json.loads((REPO / "docs" / "04_UI_Architecture" / "SUITE_TOKENS.json").read_text(encoding="utf-8"))
THEMES = sorted(TOKENS["themes"])
SURFACES = ["bg0", "bg1", "bg2"]

BODY_TEXT = ["fg0", "fg1", "accentText"]
ALERT_TEXT = ["warnRedText"]
# always shown with an icon and a word, so body-text contrast is enough
STATUS_TEXT = ["cautionAmber", "advisoryCyan", "okGreen"]


def luminance(hex_colour: str) -> float:
    h = hex_colour.lstrip("#")
    channels = [int(h[i:i + 2], 16) / 255 for i in (0, 2, 4)]
    linear = [c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4 for c in channels]
    return 0.2126 * linear[0] + 0.7152 * linear[1] + 0.0722 * linear[2]


def contrast(a: str, b: str) -> float:
    hi, lo = sorted((luminance(a), luminance(b)), reverse=True)
    return (hi + 0.05) / (lo + 0.05)


def test_contrast_formula_known_values():
    assert contrast("#000000", "#FFFFFF") == pytest.approx(21.0)
    assert contrast("#777777", "#FFFFFF") == pytest.approx(4.48, abs=0.01)


def test_both_themes_define_the_same_tokens():
    keys = [set(TOKENS["themes"][t]) for t in THEMES]
    assert THEMES == ["dark", "light"]
    assert keys[0] == keys[1]


@pytest.mark.parametrize("theme", THEMES)
@pytest.mark.parametrize("surface", SURFACES)
@pytest.mark.parametrize("token", BODY_TEXT + STATUS_TEXT)
def test_text_is_at_least_4_5_to_1(theme, surface, token):
    t = TOKENS["themes"][theme]
    ratio = contrast(t[token], t[surface])
    assert ratio >= 4.5, f"{theme} {token} on {surface}: {ratio:.2f}:1"


@pytest.mark.parametrize("theme", THEMES)
@pytest.mark.parametrize("surface", SURFACES)
@pytest.mark.parametrize("token", ALERT_TEXT)
def test_alert_text_is_at_least_7_to_1(theme, surface, token):
    t = TOKENS["themes"][theme]
    ratio = contrast(t[token], t[surface])
    assert ratio >= 7.0, f"{theme} {token} on {surface}: {ratio:.2f}:1"


def test_theme_qml_matches_tokens():
    result = subprocess.run(
        [sys.executable, str(REPO / "tools" / "gen_theme_tokens.py"), "--check"],
        capture_output=True,
        text=True,
    )
    assert result.returncode == 0, result.stdout
