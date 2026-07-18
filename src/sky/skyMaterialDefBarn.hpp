#ifndef __SKY_SKYMATERIALDEFBARN_HPP__
#define __SKY_SKYMATERIALDEFBARN_HPP__

#include <Utils/Types.h>
#include "sky/skyTypePlaceholders.hpp"

enum Material {
  kMaterial_None = 0,
  kMaterial_Transparent = 2,
  kMaterial_Void = 3,
  kMaterial_Particle = 4,
  kMaterial_WoodSlippery = 5,
  kMaterial_VoidMinor = 6,
  kMaterial_WoodPlank = 7,
  kMaterial_Cliff = 16,
  kMaterial_Soil = 17,
  kMaterial_CliffLight = 18,
  kMaterial_WallDamaged = 19,
  kMaterial_Wall = 20,
  kMaterial_Gold = 21,
  kMaterial_Glacier = 22,
  kMaterial_TileCeiling = 23,
  kMaterial_TileFloor = 24,
  kMaterial_TileWall = 25,
  kMaterial_WallBrick = 26,
  kMaterial_SoilWet = 27,
  kMaterial_CliffWet = 28,
  kMaterial_Bone = 29,
  kMaterial_Wood = 30,
  kMaterial_Ceramics = 31,
  kMaterial_Sand = 32,
  kMaterial_SandWet = 33,
  kMaterial_SandLight = 34,
  kMaterial_Snow = 35,
  kMaterial_SandDeep = 36,
  kMaterial_Mud = 37,
  kMaterial_Grass = 48,
  kMaterial_GrassWet = 49,
  kMaterial_GrassLight = 50,
  kMaterial_GrassMoss = 51,
  kMaterial_Cloth = 52,
  kMaterial_Cloud = 80,
};

class MaterialDef { };

class MaterialDefBarn {
public:
  static void SetMaterialShaderUniforms(
    PipelineInstance *pipelineInstance,
    const MaterialDef &materialDef,
    ResourceManager *resources);

  const MaterialDef &GetDef(
    Material mat);
};

#endif
