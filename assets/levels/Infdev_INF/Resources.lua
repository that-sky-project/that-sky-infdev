-- Notes:  All resources of the level is dynamic loaded and generated,
--         so we leave an basic resource script.
--
-- Author: HTMonkeyG
-- Data C: 2026-09-20

resource "LevelObjects" "Infdev_INF" { level = "Infdev_INF", source = "Levels/Infdev_INF/Objects.level", sourcesCount = 1, sources = { "Infdev_INF.level.json" }, stripDebug = true }
resource "LevelData" "Infdev_INF" { source = "Levels/Infdev_INF/BstBaked.meshes", lodToLoad = 0 }
