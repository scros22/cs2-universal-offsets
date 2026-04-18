# archive/

Code that **is not part of the build**, kept for historical / research reference.
Nothing in here is `#include`d from `dllmain.cpp` or any file the linker sees.

If anything here becomes useful again, move it back into `features/` (or wherever it
belongs) and add the include to `dllmain.cpp` / `render/hooks.h` / `render/menu.h`.
If it stays unused for a release cycle, delete it.

## `unused-headers/`

Verified unreferenced as of the last cleanup pass. Reachability was traced from
`dllmain.cpp` through every transitive include in `core/`, `features/`, `render/`,
`sdk/`, `tools/`.

### Old skin/knife iterations (superseded by `features/skinchanger_test.h` + `features/knife_glove_manager.h`)
- `skinchanger.h` — original v1
- `skinchanger_v2.h` — Nuvora-sig rewrite
- `knife_changer_v2.h`, `knife_changer_v3.h`, `knife_changer_final.h`
- `equip_item_hook.h`, `setmodel_hook.h`, `setmodel_hook_v2.h`,
  `setmeshgroupmask_hook.h`, `update_subclass_hook.h`,
  `post_data_update_hook.h`, `inventory_spoof_hook.h`,
  `createmove_knife_hook.h`, `frame_stage_hook.h`

### Other orphans
- `grenade_prediction_new.h` — WIP rewrite, never wired in
- `menu_new.h` — alternate menu, never used
- `auto_signatures.h` — placeholder for dumper output, never generated
- `skin_changer.h`, `weapons.hpp` — only referenced by the deleted `unix.solutions/` Linux variant
