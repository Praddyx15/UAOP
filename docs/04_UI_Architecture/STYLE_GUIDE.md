# STYLE_GUIDE

**Version 0.1.0 · 2026-07-03 · Visual design tokens for the Qt/QML GCS. Tokens live as a QML theme singleton (`ui/styles/`); this document is their specification and rationale.**

## 1. Principles

Instrument, not application. Dark, low-glare, high-contrast, restrained. Color is a **semantic channel** reserved for state — decoration never competes with meaning. If two elements differ visually, the difference must mean something.

## 2. Color system

### Base (dark theme, primary)
| Token | Value | Use |
|---|---|---|
| `bg0` | `#0B0E11` | Window ground |
| `bg1` | `#12161B` | Panel surfaces |
| `bg2` | `#1A2027` | Cards, wells, input grounds |
| `stroke` | `#2A323C` | Panel borders, dividers, grid lines |
| `fg0` | `#E6EAEE` | Primary text/values |
| `fg1` | `#9AA5B1` | Labels, units, secondary text |
| `fg2` | `#5C6773` | Disabled, hints |
| `accent` | `#3B82C4` | Focus, selection, links — the only non-semantic color, used sparingly |

### Semantic (aviation-standard, non-negotiable meanings — UX_GUIDELINES.md §2)
| Token | Value | Meaning |
|---|---|---|
| `warn-red` | `#E5484D` | WARNING; failsafe; breach; loss |
| `caution-amber` | `#F5A524` | CAUTION; degradation |
| `advisory-cyan` | `#4CC2D9` | ADVISORY; informational state |
| `ok-green` | `#3DBB7C` | Nominal / pass — used for *state*, never decoration |
| `armed-strip` | `#E5484D` on `#3A0D0F` | Armed condition strip (persistent while armed) |
| `sim-violet` | `#8B7EC8` | Sim/replay watermark family — reserved exclusively; nothing else may be violet |

Rules: semantic colors never appear in charts as series colors (chart palette is a separate hue-distinct ramp: `#5B8DEF #E0A458 #7CC7A6 #C77CA6 #8FA35C #6BB8C4`); red/amber/green reserve their meanings absolutely (a red chart line implying nothing wrong is a defect); all foreground-on-background pairs ≥ 4.5:1 contrast (7:1 for alert text), verified by a token unit test, not eyeballs.

## 3. Typography

| Role | Face | Notes |
|---|---|---|
| UI text | Inter | Labels, controls, body |
| Numerics/telemetry | JetBrains Mono | **All live values, coordinates, ids** — tabular figures, fixed slots (UI_GUIDELINES.md §4) |

Scale (compact / control-room density): 11/12 body, 10/11 labels-units (uppercase, +2% tracking), 13/15 panel titles, 16/20 primary values, 22/28 HUD key values. Two densities from one token set; no free-form font sizes in QML — token references only, lint-enforced.

## 4. Spacing, geometry, iconography

4 px base grid (4/8/12/16/24); panel padding 12, card padding 8, control height 24 (compact) / 28; corner radius 3 px flat (instruments aren't bubbly); borders 1 px `stroke`, elevation shown by border+fill step, **no drop shadows**; icons: stroke-style 16/20 px set, single-weight, no filled decorative icons — alert icons pair with color per UX rules (shape redundancy for color-independence).

## 5. Component states (uniform across all controls)

`default → hover (bg +4% light) → active/pressed (accent border) → focused (1 px accent ring — keyboard-first demands visible focus everywhere) → disabled (fg2 + explanation affordance per UX §6)`. **Command-class controls** (UI_GUIDELINES.md §7): distinct treatment — heavier border, uppercase label, semantic ground when armed-relevant (ARM button on `warn-red` ground when ready-to-arm; E-STOP always-red, always-visible, never restyled).

## 6. Data visualization tokens

Chart grid `stroke` at 40% alpha; axis text `fg1` mono 10; gap regions: hatched `fg2` 25% (the mandated explicit-gap rendering); staleness: value desaturation curve + age badge (mono, `caution-amber` past threshold); thresholds/envelopes drawn as amber/red bands *behind* series (envelope visibility on tuning sliders, TUNING_ENGINE.md §4); oscilloscope traces: `accent` primary, chart ramp for multi-axis, setpoint dashed / actual solid / error filled-region 30% — one convention across Tuning, IMU-Live, and log review.

## 7. Motion tokens

`fast` 80 ms (state flips), `norm` 150 ms (cross-fades, max for anything non-instrument), instrument continuity (EFIS/tapes) = data-driven interpolation at render rate, not eased animation. Nothing else moves (UI_GUIDELINES.md §5).

## 8. Voice & microcopy

Operator language, factual, no exclamation marks, no praise ("Mission verified" not "Great! Mission uploaded successfully! 🎉"); labels are nouns, actions are verbs, alerts follow *condition → operational meaning → action* (UX §6); regulatory wording factual per COMPLIANCE.md §B.4 ("DAM zone: RED"); abbreviations from aviation convention only (ALT, HDG, GS, VS, EKF), expanded in tooltips.

## 9. Theme engineering

All tokens in one QML singleton + JSON source of truth (checked by the contrast/lint tests); high-brightness field variant (Phase 3) redefines token values only — components reference tokens exclusively, so themes are data. Per-customer whitelabeling (Phase 4 possibility) is a token-set swap plus logo slots; semantic colors are **excluded** from whitelabeling — no customer gets a green WARNING.
