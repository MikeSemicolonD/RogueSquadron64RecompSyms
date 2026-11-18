// Shared header for .nrm code mods (MIPS C built with tools/mods/code_mod.cmake). Section names are the ones RecompModTool reads.
#ifndef RS64_API_H
#define RS64_API_H

#define RECOMP_PATCH __attribute__((section(".recomp_patch")))
#define RECOMP_FORCE_PATCH __attribute__((section(".recomp_force_patch")))
#define RECOMP_EXPORT __attribute__((section(".recomp_export")))
#define RECOMP_HOOK(func) __attribute__((section(".recomp_hook." func)))
#define RECOMP_HOOK_RETURN(func) __attribute__((section(".recomp_hook_return." func)))

// Runs on an event: RECOMP_CALLBACK("*", rs64_on_mission_frame_pads) for the game's hook sites ("*" is the base game), or another mod's id for its RECOMP_DECLARE_EVENT events.
#define RECOMP_CALLBACK(mod, event) __attribute__((section(".recomp_callback." mod ":" #event)))

// Declares a function from another mod ("*" = the base game); the empty weak body only gives RecompModTool a symbol to resolve.
#define RECOMP_IMPORT(mod, func) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("GCC diagnostic ignored \"-Wreturn-type\"") \
    __attribute__((noinline, weak, used, section(".recomp_import." mod))) func {} \
    _Pragma("GCC diagnostic pop")

#define RECOMP_DECLARE_EVENT(func) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    __attribute__((noinline, weak, used, section(".recomp_event"))) void func {} \
    _Pragma("GCC diagnostic pop")

// From an rs64_on_* callback at a function-entry site (menu_frame, mission_end, transition_request, objective_count, result_fail, result_success, trigger_effect): the game function returns at once with v0 = r2. Ignored elsewhere.
RECOMP_IMPORT("*", void rs64_event_return(unsigned int r2))

// From an rs64_on_* callback at any hook point: set a register of the game function once every callback has run (the event restores registers after each one).
// set_gpr takes 1-31 except 29 (sp); set_fpr writes a single-precision float, e.g. f20 at rs64_on_mission_frame_dt is the frame time. Up to 8 writes per event.
RECOMP_IMPORT("*", void rs64_event_set_gpr(unsigned int reg, unsigned int value))
RECOMP_IMPORT("*", void rs64_event_set_fpr(unsigned int reg, float value))

// From an rs64_on_* callback: a register as it was at the hook point (a callback only gets a0-a3 as arguments), e.g. rs64_event_get_gpr(16) for s0. 0 outside a callback.
RECOMP_IMPORT("*", unsigned int rs64_event_get_gpr(unsigned int reg))
RECOMP_IMPORT("*", float rs64_event_get_fpr(unsigned int reg))

// The player's draw-distance multiplier (1.0 = the game's own) and the terrain reach / fog multiplier that follows it (capped at 2.5).
RECOMP_IMPORT("*", float rs64_draw_dist(void))
RECOMP_IMPORT("*", float rs64_terrain_dist(void))

// The terrain grid tables moved into host RAM with doubled strides: pass a table's original address, get where it lives now (others pass through).
RECOMP_IMPORT("*", unsigned int rs64_tgrid_base(unsigned int original))

// Where a per-player field moved for co-op (the R1-R4 ranges); addresses outside them pass through.
RECOMP_IMPORT("*", unsigned int rs64_relocated(unsigned int address))

// The classified game state's id ("menu.main", "mission", ...) copied into buf (NUL-terminated, cut to size).
RECOMP_IMPORT("*", void rs64_state_id(char* buf, unsigned int size))
// Non-zero while a cutscene plays.
RECOMP_IMPORT("*", int rs64_in_cutscene(void))
// The game time one mission frame covers (1/30 s, or 1/60 s with 60 logic frames a second) / seconds since the mission started (0 outside one).
RECOMP_IMPORT("*", float rs64_frame_seconds(void))
RECOMP_IMPORT("*", float rs64_mission_seconds(void))

// Process-wide int flags shared with native mods (unset reads 0).
RECOMP_IMPORT("*", int rs64_get_flag(const char* name))
RECOMP_IMPORT("*", void rs64_set_flag(const char* name, int value))

// Level rules: raise a "when custom <name> [for <object>]" (object may be ""); run one action line as rules.txt writes it
// ("message 4 Hold the bridge"; 0 ran, -1 does not read, -2 no mission, -3 no such action); read and write rule variables.
RECOMP_IMPORT("*", int rs64_fire_rule_event(const char* name, const char* object))
RECOMP_IMPORT("*", int rs64_run_rule_line(const char* line))
RECOMP_IMPORT("*", int rs64_rule_var_get(const char* name))
RECOMP_IMPORT("*", void rs64_rule_var_set(const char* name, int value))

// Level objects by name, missions only: the DAT record (0 if none), health 0-100 (-1 if none), destroy as a lethal hit (0 done, -1 none, -2 already gone).
RECOMP_IMPORT("*", unsigned int rs64_object_by_name(const char* name))
RECOMP_IMPORT("*", float rs64_object_health(const char* name))
RECOMP_IMPORT("*", int rs64_destroy_object(const char* name))

// Non-zero while the keyboard key with that SDL name is down ("F7", "K", "Left Shift"); let players pick it with a string config option.
RECOMP_IMPORT("*", int rs64_key_down(const char* key))
// The pilot (save slot, 0-based) chosen on the account screen, -1 before one is.
RECOMP_IMPORT("*", int rs64_save_slot(void))
// Sets a ray-traced light preset from the next frame: preset_json is one roguesq_lights.json preset object (up to 16 KB), NULL removes it. 0, -1 no name, -2 bad JSON.
RECOMP_IMPORT("*", int rs64_light_set(const char* name, const char* preset_json))
// Missions only: spawns a copy of the level unit named template_name at pos (map space, Y down), named name (NULL keeps the template's). The NPC slot, or < 0 (see spawn_unit in the host API docs).
RECOMP_IMPORT("*", int rs64_spawn_unit(const char* template_name, const char* name, const float pos[3]))
// Turns and moves the drawn view relative to the game's camera: view = yaw left, pitch up (degrees), then an offset x right, y down, z forward; NULL or all zero restores it. Drawing only.
RECOMP_IMPORT("*", void rs64_camera_view(const float view[5]))

// The level map's cell: 25 heights (the HMP's units) and its texture id; set_cell's tex < 0 keeps the texture. 0 ok, -1 no map, -2 out of range, -3 too many tiles.
RECOMP_IMPORT("*", int rs64_terrain_get_cell(unsigned int col, unsigned int row, signed char heights[25], unsigned short* tex))
RECOMP_IMPORT("*", int rs64_terrain_set_cell(unsigned int col, unsigned int row, const signed char heights[25], int tex))

#endif
