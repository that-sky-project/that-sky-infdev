@echo off
cd %~dp0

mkdir .\dist
mkdir .\dist\Data

:: Level
mkdir .\dist\Data\Levels

mkdir .\dist\Data\Levels\Infdev_INF
node .\scripts\level.js .\assets\levels\Infdev_INF\Infdev_INF.level.json .\assets\levels\Infdev_INF\Infdev_INF.decl.json .\dist\Data\Levels\Infdev_INF\Objects.level.bin
node .\node_modules\@that-sky-project\that-sky-level\dist\cli.js -C -i .\assets\levels\Infdev_INF\Infdev_INF.obj -o .\dist\Data\Levels\Infdev_INF\BstBaked.meshes
copy .\assets\levels\Infdev_INF\Resources.lua .\dist\Data\Levels\Infdev_INF\Resources.lua

:: Shader
mkdir .\dist\Data\Shaders
mkdir .\dist\Data\Shaders\Bin

:: - EndPortal
:: - - 12f
glslc -g --target-env=vulkan1.2 .\assets\shaders\EndPortal.vert -o .\dist\Data\Shaders\Bin\EndPortal-12f.win.vs.spv
glslc -g --target-env=vulkan1.2 .\assets\shaders\EndPortal.frag -o .\dist\Data\Shaders\Bin\EndPortal-12f.win.fs.spv
spirv-cross .\dist\Data\Shaders\Bin\EndPortal-12f.win.vs.spv --reflect > .\dist\Data\Shaders\Bin\EndPortal-12f.win.vs.refl.json
spirv-cross .\dist\Data\Shaders\Bin\EndPortal-12f.win.fs.spv --reflect > .\dist\Data\Shaders\Bin\EndPortal-12f.win.fs.refl.json
node .\scripts\shader-refl.js .\dist\Data\Shaders\Bin\EndPortal-12f.win.fs.refl.json .\dist\Data\Shaders\Bin\EndPortal-12f.win.vs.refl.json -o .\dist\Data\Shaders\Bin\EndPortal-12f.win.ref

:: - - 13f
glslc -g --target-env=vulkan1.3 .\assets\shaders\EndPortal.vert -o .\dist\Data\Shaders\Bin\EndPortal-13f.win.vs.spv
glslc -g --target-env=vulkan1.3 .\assets\shaders\EndPortal.frag -o .\dist\Data\Shaders\Bin\EndPortal-13f.win.fs.spv
spirv-cross .\dist\Data\Shaders\Bin\EndPortal-13f.win.vs.spv --reflect > .\dist\Data\Shaders\Bin\EndPortal-13f.win.vs.refl.json
spirv-cross .\dist\Data\Shaders\Bin\EndPortal-13f.win.fs.spv --reflect > .\dist\Data\Shaders\Bin\EndPortal-13f.win.fs.refl.json
node .\scripts\shader-refl.js .\dist\Data\Shaders\Bin\EndPortal-13f.win.fs.refl.json .\dist\Data\Shaders\Bin\EndPortal-13f.win.vs.refl.json -o .\dist\Data\Shaders\Bin\EndPortal-13f.win.ref

:: - - 13h
glslc -g --target-env=vulkan1.3 .\assets\shaders\EndPortal.vert -o .\dist\Data\Shaders\Bin\EndPortal-13h.win.vs.spv
glslc -g --target-env=vulkan1.3 .\assets\shaders\EndPortal.frag -o .\dist\Data\Shaders\Bin\EndPortal-13h.win.fs.spv
spirv-cross .\dist\Data\Shaders\Bin\EndPortal-13h.win.vs.spv --reflect > .\dist\Data\Shaders\Bin\EndPortal-13h.win.vs.refl.json
spirv-cross .\dist\Data\Shaders\Bin\EndPortal-13h.win.fs.spv --reflect > .\dist\Data\Shaders\Bin\EndPortal-13h.win.fs.refl.json
node .\scripts\shader-refl.js .\dist\Data\Shaders\Bin\EndPortal-13h.win.fs.refl.json .\dist\Data\Shaders\Bin\EndPortal-13h.win.vs.refl.json -o .\dist\Data\Shaders\Bin\EndPortal-13h.win.ref

:: - SimpleColorTest
:: - - 12f
glslc -g --target-env=vulkan1.2 .\assets\shaders\SimpleColorTest.vert -o .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.vs.spv
glslc -g --target-env=vulkan1.2 .\assets\shaders\SimpleColorTest.frag -o .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.fs.spv
spirv-cross .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.vs.spv --reflect > .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.vs.refl.json
spirv-cross .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.fs.spv --reflect > .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.fs.refl.json
node .\scripts\shader-refl.js .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.fs.refl.json .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.vs.refl.json -o .\dist\Data\Shaders\Bin\SimpleColorTest-12f.win.ref

:: - - 13f
glslc -g --target-env=vulkan1.3 .\assets\shaders\SimpleColorTest.vert -o .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.vs.spv
glslc -g --target-env=vulkan1.3 .\assets\shaders\SimpleColorTest.frag -o .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.fs.spv
spirv-cross .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.vs.spv --reflect > .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.vs.refl.json
spirv-cross .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.fs.spv --reflect > .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.fs.refl.json
node .\scripts\shader-refl.js .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.fs.refl.json .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.vs.refl.json -o .\dist\Data\Shaders\Bin\SimpleColorTest-13f.win.ref

:: - - 13h
glslc -g --target-env=vulkan1.3 .\assets\shaders\SimpleColorTest.vert -o .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.vs.spv
glslc -g --target-env=vulkan1.3 .\assets\shaders\SimpleColorTest.frag -o .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.fs.spv
spirv-cross .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.vs.spv --reflect > .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.vs.refl.json
spirv-cross .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.fs.spv --reflect > .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.fs.refl.json
node .\scripts\shader-refl.js .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.fs.refl.json .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.vs.refl.json -o .\dist\Data\Shaders\Bin\SimpleColorTest-13h.win.ref

del .\dist\Data\Shaders\Bin\*.refl.json
