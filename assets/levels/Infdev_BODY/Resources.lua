-- Author: HTMonkeyG
-- Data C: 2026-09-20

resource "LevelObjects" "Infdev_BODY" { level = "Infdev_BODY", source = "Levels/Infdev_BODY/Objects.level", sourcesCount = 1, sources = { "Infdev_BODY.level.json" }, stripDebug = true }
resource "LevelData" "Infdev_BODY" { source = "Levels/Infdev_BODY/BstBaked.meshes", lodToLoad = 0 }
