# Handoff: finishing this on your PC (Claude Code)

This project is source-only. Run your Melty "Publish" prompt in Claude Code on the PC that has
Skyrim Special Edition and Steep installed, point it at this folder, and ask it to continue from here.

## What already works (tested on Linux)
- `sheets/` are the design source of truth (all rows `verified: false` = untuned first guesses).
- `include/rider.h`: slope physics, steering, braking, jumping, surfaces, tricks, combos, wipeouts.
- `include/controller.h`: wiring between the rider logic and the game via `IGame` (fake-game tested).
- `make test` runs preflight, generates `include/generated_sheets.h` and runs all tests.

## What is NOT done (in order)
1. **Fill `sheets/hooks.json`** for the player's exact Skyrim SE version (Melty's game_info lists loaders
   and version notes): movement override, ground reading, input, equip detection, snow surface lookup.
   `python tools/preflight.py --scope all` must come back with 0 errors before building the plugin.
2. **Implement `SkyrimGame` in `plugin/plugin.cpp`** (every method has a TODO naming its hook row).
   Test each hook in the running game, set its row to `verified: true`, then flip `kHooksVerified`.
3. **Fill `sheets/runs.json`** (worldspace, start/finish coordinates) from the real game.
4. **Original assets:** board model and animations. Steep's assets cannot be shipped. AI art generation
   would need a paid fal.ai key, so ask first.
5. **Check key bindings** in `sheets/inputs.json` against Skyrim's defaults and Melty's game_info.
6. **Tune** the numbers in `sheets/board.json`, `surfaces.json`, `tricks.json`, `scoring.json` by riding.
7. **Package and publish with Melty's tools:** the plugin builds to `SkyrimSnowboard.asi`; depend on
   Ultimate ASI Loader through the recipe (Melty installs it itself), not SKSE64. Run inspect_package,
   validate_recipe and one_click_check, take a real screenshot of the game running, then test in Melty.

## Listing decisions still open (ask the owner, do not guess)
- Required game: Skyrim Special Edition only. Is Steep tagged as a second game, or credited as inspiration only?
- Title, tagline, description, credits, license, and whether others may remix it.
- Solo only (Skyrim SE has no built-in multiplayer).

## Never put in this folder
Your Melty token or any other secret.
