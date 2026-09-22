#ifndef __SKY_SKYCOLLISIONGEOBARN_HPP__
#define __SKY_SKYCOLLISIONGEOBARN_HPP__

#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "sky/skyGfx.hpp"
#include "sky/skyVectorMath.hpp"

struct CollisionGeoInstanceData {
  inline bool IsValid() const {
    return (!lightData || (lightType != kGfxType_UNKNOWN && lightStride))
      && (!colorData || colorType != kGfxType_UNKNOWN)
      && (!mtrlData || mtrlType != kGfxType_UNKNOWN);
  }

  const void *lightData = nullptr;
  // WARN: Only accepts kGfxType_UBYTE4.
  GfxType lightType = kGfxType_UNKNOWN;
  // NOTE: The stride is not directly related to the element size; in other
  // words, the stride is the size of the vertex.
  // 
  // If the element stride is greater than the element size, the game will
  // still fetch the corresponding element and then advance by the specified
  // number of bytes. This is for better vertex "decompression", users only
  // need to add an offset to the buffer.
  // 
  // For an example, the stride of vertices in BstBaked.meshes is 0x24.
  u32 lightStride = 0;

  const void *colorData = nullptr;
  // WARN: Only accepts kGfxType_UBYTE4 or kGfxType_FLOAT4.
  GfxType colorType = kGfxType_UNKNOWN;
  u32 colorStride = 0;

  // Points to the major material of terrain vertex.
  const void *mtrlData = nullptr;
  // WARN: Only accepts kGfxType_UBYTE or kGfxType_BYTE.
  GfxType mtrlType = kGfxType_UNKNOWN;
  u32 mtrlStride = 0;

  // WARN: Cannot be zero.
  u32 mask = 0;
  f32 unk_1 = 1.0f;
};

struct CollisionGeoMeshData {
  inline bool IsValid() const { return idxCount && vtxCount && idxBuffer && vtxBuffer && vtxStride; }

  cstring tag = nullptr;

  u32 idxCount = 0;
  const void *idxBuffer = nullptr;
  u32 idxStride = 0;

  u32 vtxCount = 0;
  const void *vtxBuffer = nullptr;
  u32 vtxStride = 0;

  Vector4 min = {};
  Vector4 max = {};
};

class CollisionGeoInstance { };

class CollisionInstTree { };

class CollisionGeoBarn {
//private:
public:
  ~CollisionGeoBarn() = default;
  CollisionGeoBarn() = default;
  CollisionGeoBarn(const CollisionGeoBarn &) = delete;
  CollisionGeoBarn(CollisionGeoBarn &&) = delete;
  CollisionGeoBarn &operator=(const CollisionGeoBarn &) = delete;

public:
  u32 AddGeo(const CollisionGeoMeshData &meshData);
  void RemoveGeo(u32 geoIdx);

  CollisionGeoInstance *AddInstance(
    u32 geoIdx,
    const Matrix4 &transform,
    const CollisionGeoInstanceData &data,
    void *userdata);
  void RemoveInstance(CollisionGeoInstance *inst);

private:

};
META_DECLARE_CLASS(CollisionGeoBarn)

#endif
