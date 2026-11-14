-- Notes:  All resources of the level is dynamic loaded and generated,
--         so we leave an basic resource script.
--
-- Author: HTMonkeyG
-- Data C: 2026-09-20

resource "LevelObjects" "Infdev_VOXEL" { level = "Infdev_VOXEL", source = "Levels/Infdev_VOXEL/Objects.level", sourcesCount = 1, sources = { "Infdev_VOXEL.level.json" }, stripDebug = true }
resource "LevelData" "Infdev_VOXEL" { source = "Levels/Infdev_VOXEL/BstBaked.meshes", lodToLoad = 0 }
