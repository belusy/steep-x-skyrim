# Skyrim x Steep: Snowboard (working title)

Ride a snowboard down Skyrim's mountains, with Steep-style tricks and marked runs.
Skyrim Special Edition is the required game. Steep is the inspiration only: no Steep files are used.
Solo play.

## Status: source only, NOT yet running in Skyrim

| Part | State |
|---|---|
| Design sheets (`sheets/`) | written, every row `verified: false` (tuning values are first guesses) |
| Rider physics, surfaces, tricks, controller (`include/`) | written, unit-tested on Linux |
| In-game plugin (`plugin/plugin.cpp`) | skeleton only, never compiled; inactive until hooks are verified |
| Marked run coordinates (`runs.json`) | unfilled, needs the game |
| Release package / Melty recipe | not started |

## How the work is organised

- `sheets/*.json` are the source of truth. Change a sheet first, then the code.
- `python3 tools/preflight.py [--scope logic|all]` lists unfilled cells and references that do not resolve.
- `python3 tools/gen.py` turns the logic sheets into `include/generated_sheets.h`.
- `make test` runs preflight, generates the header and runs the unit tests.

## Delivery plan

Melty installs Ultimate ASI Loader for Skyrim SE by itself, so the plugin is an ASI plugin.
It avoids depending on SKSE64, which Melty installs only for some game versions.
The board model and animations must be original (Steep's assets cannot be shipped).

## Open items

- Key bindings in `inputs.json` are not yet checked against Skyrim's defaults.
- Skyrim's character controller may resist being used for snowboarding; this can only be judged in the running game.
- See HANDOFF.md for the remaining steps.
