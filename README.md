# Flag Finder

Buy the $125 radar in Modded Shop; equip it in the native hotbar.

## Install

Install [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) first.

Download [flag-finder-0.4.0.msmod](content/flag-finder-0.4.0.msmod), then drag and drop the `.msmod` file onto the [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) window. You can also use **Add mods…** (older loader builds: **Install mod package**).

Select the mod and its required frameworks, click **Prepare files**, then **Launch**.

Required mods: [shop-framework](https://github.com/swooping-mods/shop-framework), [equipment-framework](https://github.com/swooping-mods/equipment-framework), [hints-framework](https://github.com/swooping-mods/hints-framework).

## Controls

- Buy the $125 handheld radar in Modded Shop (Ctrl+Shift+F2); assign it in Inventory and select its hotbar slot.
- Authored handheld scanner displays live bearing and range on its rebuilt screen; T cycles flags, sites and all.
- Native build and display checks pass; rendered gameplay and co-op remain unqualified.

## Compatibility

Version 0.4.0. Targets the pinned Steam Windows x64 IL2CPP build of Moon Sweepers (Steam app 5335510). On Linux, the game runs through Proton. Experimental: back up saves before using save-affecting mods.

## Build

Run `python3 build.py --check` to verify source hashes, then `python3 build.py` to write a build under `.local/build/`. Requires installed Clang, lld-link and llvm-dlltool.

## Credits

See [CREDITS.txt](CREDITS.txt) and [third-party notices](vendored/THIRD_PARTY_NOTICES.md).
