# RogueSquadron64RecompSyms

Symbol files for [Rogue Squadron 64 Recompiled](https://github.com/MikeSemicolonD/RogueSquadron64Recomp) (Star Wars: Rogue Squadron, N64, USA v1.0). Code mods (`.nrm`) and the game's patches ELF build against them, so a mod can be built without the game repository.

| File | Contents |
|---|---|
| `rogue_squadron.syms.toml` | Every function by section: name, address, size. Reference these names in `RECOMP_PATCH` / `RECOMP_HOOK` and calls. |
| `rogue_squadron.datasyms.toml` | Data symbols at their original addresses. |
| `rogue_squadron.datasyms.mods.toml` | The data symbols mods link against: the same file with the co-op per-player fields moved to their host RAM addresses. |

In a mod's `mod.toml`:

```toml
func_reference_syms_file = "syms/rogue_squadron.syms.toml"
data_reference_syms_files = [ "syms/rogue_squadron.datasyms.mods.toml" ]
```

The files contain only names, addresses and sizes. They are regenerated from the decomp ELF in the game repository with `cmake --build build --config Debug --target dump_syms` (N64Recomp `--dump-context`, then `tools/mods/mod_datasyms.cmake`); commit here, then bump the `syms` submodule there.
