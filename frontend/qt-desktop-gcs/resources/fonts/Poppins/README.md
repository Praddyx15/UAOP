# Poppins

**Source:** [github.com/itfoundry/Poppins](https://github.com/itfoundry/Poppins) via [github.com/google/fonts](https://github.com/google/fonts/tree/main/ofl/poppins), **SIL Open Font License 1.1** (`OFL.txt` in this directory). See [THIRD_PARTY_NOTICES.md](../../../../../THIRD_PARTY_NOTICES.md).

## Why Poppins

Platform brand-font decision (founder preference, 2026-07-07), replacing the earlier Inter/system-fallback placeholder in [STYLE_GUIDE.md](../../../../../docs/04_UI_Architecture/STYLE_GUIDE.md) §3.

## Faces bundled

8 static TTFs: Light, LightItalic, Regular, Italic, Medium, MediumItalic, Bold, BoldItalic. All registered under the `"Poppins"` family via `QFontDatabase::addApplicationFont` (`src/main.cpp`) — Qt's font matcher selects the correct static face for a requested weight/italic combination. Embedded as Qt resources (`qt_add_resources` in `CMakeLists.txt`), never loaded from a loose filesystem path, so rendering is identical regardless of what fonts the host OS has installed (air-gap consistency, UAOP-NFR-001).

## Scope note

Numeric telemetry values intentionally stay on a monospace font (currently a system fallback, `Consolas`) — Poppins is proportional and would break the tabular alignment STYLE_GUIDE.md §3 requires for live values. Bundling JetBrains Mono the same way is tracked as separate, not-yet-done work.
