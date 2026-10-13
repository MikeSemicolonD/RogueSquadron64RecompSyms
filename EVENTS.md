# Events and exports for code mods

The game raises an event at each of its hook points. A mod subscribes with `RECOMP_CALLBACK("*", <event>) void f(void) { ... }` from `include/rs64_api.h`.

- **Reading:** a callback gets `a0`-`a3` as its arguments. Any other register, as it was at the hook point, comes from `rs64_event_get_gpr(n)` or `rs64_event_get_fpr(n)`.
- **Changing registers:** the event restores registers after every callback, so a change goes through `rs64_event_set_gpr(n, value)` (any but `r0` and `sp`) or `rs64_event_set_fpr(n, value)` (single-precision float). The game applies them after the last callback, up to 8 per event.
- **Returning:** `rs64_event_return(v)` makes the game function return `v` at once. It works only at the events marked **entry** below and is ignored elsewhere, because returning mid-function would skip the function's epilogue.
- **Changing memory** always works: it's the game's RAM.

Register numbers: `v0` 2, `a0`-`a3` 4-7, `s0`-`s7` 16-23. Float registers are `f0`-`f31`.

## Front end

| Event | When | Mods can |
|---|---|---|
| `rs64_on_menu_input` | `menuControllerInput`, after the pad poll | Edit the pressed-buttons word at 0x8013A960 + 4 * port |
| `rs64_on_menu_pilot_chosen` | A pilot was confirmed on the account screen | |
| `rs64_on_menu_frame` (**entry**) | Once per front-end frame | Return to skip the frame |
| `rs64_on_mission_select_init` | The mission select screen was built | |
| `rs64_on_mission_confirmed` | A level was confirmed | Read or replace `s0` (16), the level |
| `rs64_on_mission_select_fonts` | Mission select allocates its fonts | Raise `a2` (6), the slot count |
| `rs64_on_mission_select_tick` | Once per mission-select frame | |
| `rs64_on_craft_select_init` | The hangar (craft select) is set up | Raise `a2` (6), the font slot count |
| `rs64_on_craft_select_tick` | Once per hangar frame, after its pad poll | |

## Mission

| Event | When | Mods can |
|---|---|---|
| `rs64_on_mission_init` | Before the level loads | |
| `rs64_on_craft_assets` | After the player craft's assets load | Load extra meshes |
| `rs64_on_mission_frame_dt` | Each mission frame, with its frame time | Read or replace `f20`, the frame time in seconds |
| `rs64_on_mission_frame_pads` | Each mission frame, after the pad read | |
| `rs64_on_mission_frame_npcs` | Each mission frame, before the NPC tick pass | |
| `rs64_on_hud_draw` | Each mission frame, after the 2D overlay list is built | |
| `rs64_on_freeze_check_a`, `rs64_on_freeze_check_b` | The world's pause/freeze test | Set `v0` (2) to 0 to keep the world running |
| `rs64_on_cutscene_freeze_check` | A cutscene actor's freeze test | Set `v0` (2) to 0 to keep it running |
| `rs64_on_mission_end` (**entry**) | The mission is being torn down | |
| `rs64_on_transition_request` (**entry**) | A mission transition is requested, `a0` = the mode | Return 0 to refuse it |
| `rs64_on_objective_count` (**entry**) | An objective counter is about to update | Return to drop the update |
| `rs64_on_result_fail` (**entry**) | Mission failure is requested | Return to drop it |
| `rs64_on_result_success` (**entry**) | Mission success is requested | Return to drop it |
| `rs64_on_trigger_effect` (**entry**) | A DAT trigger volume's effect is about to run: `a0` = the event, `a1` = 1 enter / 0 exit | Return to drop the effect |
| `rs64_on_wingman_tick` | A wingman NPC's update, after its state handler | Read `s3` (19) = its context, `s2` (18) = the action (3 = tick) |
| `rs64_on_npc_activation` | An NPC's activation range test | Read `s0` (16), the NPC |
| `rs64_on_grid_stream` | The player's terrain cell lookup | |
| `rs64_on_powerup_touch` | Before a power-up's touch test | Read `s1` (17), the power-up; set `f4` to 0 to collect it |
| `rs64_on_powerup_collect` | A power-up is being collected | Read `s1` (17), the power-up |
| `rs64_on_walker_tripped` | An AT-AT accepts a tow-cable trip | Read `s1` (17), its extension (`+0x34` its DAT item) |
| `rs64_on_radar` | After the radar's dots are placed | Read `s0` (16) = the radar, `s2` (18) = its centre |

## Audio and speech

| Event | When | Mods can |
|---|---|---|
| `rs64_on_song_active` | The song fade timer checks whether a song plays | Replace `v0` (2) |
| `rs64_on_speech_response_1`, `rs64_on_speech_response_2` | A speech response checks whether speech is idle | Replace `v0` (2): 1 idle, 0 busy |
| `rs64_on_hud_fonts` | The subtitle system allocates its fonts | Raise `a2` (6), the slot count |

## Base exports

| Function | Returns |
|---|---|
| `float rs64_draw_dist(void)` | The draw-distance multiplier in effect (1.0 = the game's own) |
| `float rs64_terrain_dist(void)` | The terrain reach and fog multiplier (follows the draw distance, capped at 2.5) |
| `unsigned rs64_tgrid_base(unsigned original)` | Where a terrain-grid table now lives (other addresses pass through) |
| `unsigned rs64_relocated(unsigned address)` | Where a co-op per-player field moved (other addresses pass through) |

The standard recomp modding API (`recomp_alloc`, `recomp_printf`, `recomp_get_config_*`, the `recomputil_*` collections) is in `include/recomputils.h`, `recompconfig.h` and `recompdata.h`.
