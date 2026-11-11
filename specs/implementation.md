# Flag Finder: behavior and implementation

Describes the exported implementation **0.4.0**. The linked source and metadata define this version; gameplay qualification is a separate requirement.

## Behavior

Buy the $125 handheld radar through Shop Framework, assign it in Equipment inventory, then select that quickslot. The authored scanner screen shows bearing/range to native flags and sites. T cycles flags, sites and all. Landmark searches are bounded to 1,024 entries at 5 Hz within 80 m. The visual and controls require the actual selected tool and hide during map/UI/focus/view changes.

## State and lifecycle

Purchased ownership uses the existing per-world/player checked sidecar; selection remains a normal native saved quickslot. The filter, current target and held screen resources are session presentation. Shop transactions and authority-loss handling belong to the shared provider adapter; world changes drop current target and visual roots.

## Integration and configuration

Runtime dependencies: [shop-framework](https://github.com/swooping-mods/shop-framework), [equipment-framework](https://github.com/swooping-mods/equipment-framework), [hints-framework](https://github.com/swooping-mods/hints-framework).

The [module metadata](../source/mods/flag-finder/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

| Service | ABI | Table size |
|---|---:|---|
| Requires `shop-framework.shop` | 1 | at least 56 bytes |
| Requires `equipment-framework.equipment` | 1 | at least 32 bytes |
| Requires `hints-framework.hints` | 1 | at least 24 bytes |

## Implementation sources

- [mod.c](../source/mods/flag-finder/mod.c)
- [radar_screen.h](../source/mods/flag-finder/radar_screen.h)
- [framework.h](../source/mods/flag-finder/framework.h)
- [runtime.h](../source/mods/flag-finder/runtime.h)
- [store.h](../source/mods/flag-finder/store.h)

## Verification boundary

No radar audio, authored hands or third-person hand attachment. This reports existing native landmarks rather than discovering arbitrary mod objects. Rendered bearing/range, ownership reload and live co-op need separate checks.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
