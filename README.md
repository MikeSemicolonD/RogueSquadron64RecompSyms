# RogueSquadron64RecompSyms

Symbols and the mod SDK for [Rogue Squadron 64 Recompiled](https://github.com/MikeSemicolonD/RogueSquadron64Recomp) (*Star Wars: Rogue Squadron*, N64, USA v1.0). Code mods (`.nrm`) are built against them, so you can write a mod without cloning or building the game.

| Path | Contents |
|---|---|
| `rogue_squadron.syms.toml` | Every game function, by section (main code and the three overlays): name, address, size. |
| `rogue_squadron.datasyms.toml` | Data symbols at their original addresses; mods link against this. While the multiplayer mod is enabled, its per-player fields (`gObjectiveCounts` and others) live in host RAM: read them through `rs64_relocated(&field)`. |
| `include/` | The SDK headers: `rs64_api.h` (the mod macros and this game's imports) and the standard recomp modding API, `recomputils.h` (`recomp_alloc`, `recomp_free`, `recomp_printf`), `recompconfig.h` (config options, save and mod paths) and `recompdata.h` (hashmaps, hashsets, slotmaps), as in other recomps' mod templates. |

The symbol files hold only names, addresses and sizes, no game code.

## Making a code mod

The quickest start is the [mod template](https://github.com/MikeSemicolonD/RogueSquadron64RecompModTemplate): it has this repository as its `syms` submodule, a working example mod and a one-command build.

A code mod is MIPS C that RecompModTool packs into a `.nrm`. The game loads it at startup and recompiles it with LiveRecomp. You need:

- **MIPS GCC** (`mips64-elf-gcc` and `mips64-elf-ld`). Windows builds are at [n64-tools/gcc-toolchain-mips64](https://github.com/n64-tools/gcc-toolchain-mips64/releases).
- **RecompModTool**, built from [N64ModernRuntime](https://github.com/MikeSemicolonD/N64ModernRuntime) (`N64Recomp/`, target `RecompModTool`).

To set one up without the template, add this repository to your mod as a submodule:

```
git submodule add https://github.com/MikeSemicolonD/RogueSquadron64RecompSyms.git syms
```

### Layout

```
my-mod/
  mod.toml       RecompModTool manifest
  mod.ld         optional linker script; without one, use syms/include/mod.ld
  src/my_mod.c
  syms/          this repository (symbols and include/)
```

`mod.toml`:

```toml
[manifest]
id = "my_mod"                 # a C identifier
version = "1.0.0"
display_name = "My Mod"
description = "What it does."
short_description = "What it does."
authors = ["you"]
game_id = "rs64"
minimum_recomp_version = "0.0.0"
enabled_by_default = true

[inputs]
elf_path = "build/mod.elf"
mod_filename = "my_mod"
func_reference_syms_file = "syms/rogue_squadron.syms.toml"
data_reference_syms_files = [ "syms/rogue_squadron.datasyms.toml" ]
```

### Writing the code

Refer to game functions and data by their names in the symbol files. A function still called `func_XXXXXXXX` is one nobody has named yet; use that name.

```c
#include "rs64_api.h"
#include "recomputils.h"

// Replace a game function.
RECOMP_PATCH unsigned int getSecondaryWeaponCount(unsigned int craft, unsigned int weapon) {
    return 0xFF;
}

// Run before a game function (RECOMP_HOOK_RETURN runs after it returns).
RECOMP_HOOK("runInMissionFrame") void on_mission_frame(void) {
    recomp_printf("mission running\n");
}

// Subscribe to one of the game's hook points; EVENTS.md lists every one.
RECOMP_CALLBACK("*", rs64_on_mission_frame_pads) void on_pads(void) {
}
```

[EVENTS.md](EVENTS.md) lists every event and what a callback can read, change or return there. `rs64_api.h` also imports the base game's helpers: `rs64_draw_dist`, `rs64_terrain_dist`, `rs64_tgrid_base` (the relocated terrain-grid tables) and `rs64_relocated` (where a co-op per-player field moved). Use `rs64_event_return(v)` from a callback at a function-entry hook point to make the game function return `v`, and `rs64_event_set_gpr` / `rs64_event_set_fpr` at any hook point to change a register (for example `rs64_event_set_fpr(20, dt)` at `rs64_on_mission_frame_dt` sets the frame time). `recomp_printf` prints to the game's console.

### Building

```
mips64-elf-gcc -mips2 -mabi=32 -O2 -G0 -mno-abicalls -fomit-frame-pointer -fno-builtin -fno-optimize-sibling-calls -nostdinc -I syms/include -c src/my_mod.c -o build/my_mod.o
mips64-elf-ld -nostdlib -T syms/include/mod.ld --unresolved-symbols=ignore-all --emit-relocs --noinhibit-exec -e 0 build/my_mod.o -o build/mod.elf
RecompModTool mod.toml build
```

ld reports every call into the game as out of range; that's expected, because RecompModTool resolves those calls by name. Keep `-fno-optimize-sibling-calls`: GCC otherwise turns a final call into a tail jump that LiveRecomp can't recompile. Copy `build/my_mod.nrm` into the game's `mods/` folder and enable it in the Mods panel (F1) or `mods.json`.

The game repository's `mods/infinite-secondary`, `mods/any-craft` and `mods/larger-object-pool` folders are more examples.

### Functions the base game patches

The recomp fixes about 90 game functions with hooks and instruction patches of its own (crash guards, pacing, draw distance).

- **`RECOMP_HOOK` on one works.** The game rebuilds the function with your hook and re-applies its own patches. Your hook runs before the base game's entry hook, and if a base hook returns from the function early, your `RECOMP_HOOK_RETURN` doesn't run for that call.
- **`RECOMP_PATCH` on one is refused** with "Attempted to replace a function that's been patched by the base recomp", because your copy would drop the fix.
- **`RECOMP_FORCE_PATCH`** replaces it anyway. Your copy must then carry the base patches itself.

### Instruction patches

To change only a few instructions (a constant, a limit), don't copy the function: add entries to `mod.toml`.

```toml
[[patches.instruction]]
func = "loadEffectModelInstancePool"   # a function name from rogue_squadron.syms.toml
vram = 0x800AE13C                      # the instruction's address in it
expect_asm = "addiu $a0, $zero, 0x3A20" # the game's original instruction
asm = "ori $a0, $zero, 0x9B00"          # what replaces it
```

RecompModTool assembles `asm` and `expect_asm` with your MIPS GCC (`--as <gcc>`; the mod template passes it) and packs the result into the `.nrm` as `instruction_patches.json`. Instead of text you can give words: `value = 0x...` for the new one (needed for branches and jumps, which can't be assembled on their own), and `expect = 0x...` for the original. Or pass `--rom <rom.z64>` (`-DGAME_ROM=` in the template) to read the original from the game, which also checks every `expect` at build time. Either way the game checks each original word when it loads the mod, so a stale patch is refused with the address instead of corrupting code. It rebuilds each patched function with your patches, its own patches and any hooks on it. Two mods writing different words to one instruction conflict. A mod can be only patches: no sources and no `elf_path`. The game's `mods/long-tow-cable` is a complete example: 21 patches and no code.

## Updating

The symbols follow the game's function names, which change as the decomp names more functions, and the headers follow the game's exports. Before releasing a mod, update the submodule (`git submodule update --remote syms`) and rebuild. A renamed function shows up as a RecompModTool error, not as a broken mod.

Maintainers regenerate the symbol files from the game repository with `cmake --build build --config Debug --target dump_syms` and edit `include/` with the game's exports, commit here, then bump the game's `syms` submodule.
