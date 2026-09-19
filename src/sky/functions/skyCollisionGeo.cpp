#include "utils/rva.hpp"
#include "sky/skyCollisionGeo.hpp"

META_REGISTER_CLASS(CollisionGeoBarn)

u32 CollisionGeoBarn::AddGeo(
  const CollisionGeoMeshData &meshData
) {
  using PFN = u32 (*)(CollisionGeoBarn *, const CollisionGeoMeshData &);
  static const Rva s_CollisionGeoBarn_AddGeo = 0x0004BE30;
  return ((PFN)s_CollisionGeoBarn_AddGeo())(
    this, meshData);
}

void CollisionGeoBarn::RemoveGeo(
  u32 geoIdx
) {
  using PFN = void (*)(CollisionGeoBarn *, u32);
  static const Rva s_CollisionGeoBarn_RemoveGeo = 0x0004C3E0;
  return ((PFN)s_CollisionGeoBarn_RemoveGeo())(
    this, geoIdx);
}

CollisionGeoInstance *CollisionGeoBarn::AddInstance(
  u32 geoIdx,
  const Matrix4 &transform,
  const CollisionGeoInstanceData &data,
  void *userdata
) {
  using PFN = CollisionGeoInstance *(*)(CollisionGeoBarn *, u32, const Matrix4 &, const CollisionGeoInstanceData &, void *);
  static const Rva s_CollisionGeoBarn_AddInstance = 0x0004C790;
  return ((PFN)s_CollisionGeoBarn_AddInstance())(
    this, geoIdx, transform, data, userdata);
}

void CollisionGeoBarn::RemoveInstance(
  CollisionGeoInstance *inst
) {
  using PFN = void (*)(CollisionGeoBarn *, CollisionGeoInstance *);
  static const Rva s_CollisionGeoBarn_RemoveInstance = 0x0004D0E0;
  return ((PFN)s_CollisionGeoBarn_RemoveInstance())(
    this, inst);
}
