# That Sky Infdev
Infdev, short for **<u>Inf</u>inite <u>Dev</u>elopment**, is a pioneering mod content of Sky.

The mod's name comes from a [development phase of Minecraft](https://minecraft.wiki/w/Java_Edition_Infdev), during which infinite world generation was added; this mod similarly adds infinite world generation in its testing.

The mod makes deep modifications and hard-links to the game, aiming to pre-test many API features that may later become stable in [Flame API](https://github.com/that-sky-project/flame.cc). Therefore, this mod is only available for game version **v0.32.2** and is incompatible with almost all other mods and APIs.

## Build
Building this mod requires the following tools to be installed:
1. [MSVC 2022](https://visualstudio.microsoft.com/zh-hans/vs/features/cplusplus/), to compile cpp codes.
2. [Node.js](https://nodejs.org), to run all build scripts.
3. [Vulkan SDK](https://vulkan.lunarg.com/sdk/home), to compile shaders.

### Process
- Recursively clone the repository, and run `npm i` to install dependencies.
- Open `libraries/that-sky-modbase/that-sky-modbase.sln` with MSVC, and build `that-sky-modbase` SDK.
- Open `projects/vs/that-sky-infdev.sln` with MSVC, and build `that-sky-infdev.dll`.
- Run `mingw32-make` in the repository root directory to build the resource files.
- Copy `manifest.json` to `dist`.
- Finish! The mod can be copied to the game directly.
