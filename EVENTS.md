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
| `rs64_on_mission_ready` | The first mission frame, once the level's objects are loaded | Look objects up with `rs64_object_by_name` |
| `rs64_on_unit_destroyed` | A unit's death effect starts: `a0` = its DAT record, `a1` = its name | |
| `rs64_on_area_crossed` | A trigger area's effect runs: `a0` = its DAT record, `a1` = 1 entered / 0 left, `a2` = its name | |
| `rs64_on_path_point` | An NPC reached a path event: `a0` = its DAT record, `a1` = its name | |
| `rs64_on_objective_state` | A mod level's own objective changed: `a0` = the objective (1-4), `a1` = 1 done / 0 failed | |
| `rs64_on_player_damage` (**entry**) | Damage reaches a player's craft: `a0` = its record (player 1 is 0x80137DBC), `a1` = the damage as float bits | Replace `a1` (5) to scale it; return to take none |
| `rs64_on_player_fire` (**entry**) | A player craft fires: `a0` = the weapon context, `a1` = the hardpoint | Return to skip the shot |
| `rs64_on_camera` | A camera's projection is set up: `a0` = the camera (`+0x20` FOV in degrees, `+0x24` near, `+0x28` far) | Change those fields in memory (they also drive the game's view tests) |

## Any state

| Event | When | Mods can |
|---|---|---|
| `rs64_on_frame` | Every frame the game presents, in menus, cutscenes and missions: `a0` = the framebuffer | |
| `rs64_on_state_changed` | The game state changed: `a0` = the new state number, `a1` = the old one | Name it with `rs64_state_id` |
| `rs64_on_cutscene` | A cutscene started (`a0` = 1) or ended (`a0` = 0) | |

`rs64_on_mission_ready`, `rs64_on_unit_destroyed`, `rs64_on_area_crossed`, `rs64_on_path_point`, `rs64_on_objective_state` and the Any state events run with a copy of the game's registers: read `a0`-`a2`, but a register change has no effect there.

## Audio and speech

| Event | When | Mods can |
|---|---|---|
| `rs64_on_song_active` | The song fade timer checks whether a song plays | Replace `v0` (2) |
| `rs64_on_speech_response_1`, `rs64_on_speech_response_2` | A speech response checks whether speech is idle | Replace `v0` (2): 1 idle, 0 busy |
| `rs64_on_hud_fonts` | The subtitle system allocates its fonts | Raise `a2` (6), the slot count |
| `rs64_on_song_start` (**entry**) | A song starts: `a0` = its id | Replace `a0` (4) with another loaded song; return -1 to skip it |
| `rs64_on_sound_3d` (**entry**) | A positioned sound effect starts: `a1` = its position; its id is the u16 at `sp+0x1A`, its volume the byte at `sp+0x1F` | Rewrite the id or volume in memory; return -1 to mute it |

## Base exports

| Function | Returns |
|---|---|
| `float rs64_draw_dist(void)` | The draw-distance multiplier in effect (1.0 = the game's own) |
| `float rs64_terrain_dist(void)` | The terrain reach and fog multiplier (follows the draw distance, capped at 2.5) |
| `unsigned rs64_tgrid_base(unsigned original)` | Where a terrain-grid table now lives (other addresses pass through) |
| `unsigned rs64_relocated(unsigned address)` | Where a co-op per-player field moved (other addresses pass through) |
| `void rs64_state_id(char* buf, unsigned size)` / `int rs64_in_cutscene(void)` | The game state's id (`menu.main`, `mission`, ...) / non-zero during a cutscene |
| `float rs64_frame_seconds(void)` / `float rs64_mission_seconds(void)` | One mission frame's game time / seconds since the mission started |
| `int rs64_get_flag(const char*)` / `void rs64_set_flag(const char*, int)` | The flags native mods share |
| `int rs64_fire_rule_event(const char* name, const char* object)` | Raises a level rules `when custom` |
| `int rs64_run_rule_line(const char* line)` | Runs a level rules action line (`"message 4 Hold the bridge"`); 0 ran |
| `int rs64_rule_var_get(const char*)` / `void rs64_rule_var_set(const char*, int)` | Level rules variables |
| `unsigned rs64_object_by_name(const char*)` / `float rs64_object_health(const char*)` / `int rs64_destroy_object(const char*)` | A level object's DAT record / its health 0-100 / destroy it (missions only) |
| `int rs64_terrain_get_cell(...)` / `int rs64_terrain_set_cell(...)` | A level map cell's 25 heights and texture |
| `int rs64_key_down(const char* key)` / `int rs64_save_slot(void)` | A keyboard key by SDL name is down / the chosen pilot, -1 before one |
| `int rs64_light_set(const char* name, const char* preset_json)` | Sets a ray-traced light preset (one `roguesq_lights.json` preset object) from the next frame; `NULL` removes it |
| `int rs64_spawn_unit(const char* template_name, const char* name, const float pos[3])` | Spawns a copy of a level unit (missions only); the NPC slot or < 0 |
| `void rs64_camera_view(const float view[5])` | Turns (yaw, pitch in degrees) and moves (x right, y down, z forward) the drawn view relative to the game's camera; `NULL` restores it |

The standard recomp modding API (`recomp_alloc`, `recomp_printf`, `recomp_get_config_*`, the `recomputil_*` collections) is in `include/recomputils.h`, `recompconfig.h` and `recompdata.h`.
