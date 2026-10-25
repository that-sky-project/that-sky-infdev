#ifndef __BODYCAPTURE_BODYCAPTURE_HPP__
#define __BODYCAPTURE_BODYCAPTURE_HPP__

#include <unordered_set>
#include <shared_mutex>
#include <cstring>
#include <vector>
#include <limits>
#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "sky/skyVectorMath.hpp"
#include "mod/moduleBarnExt.hpp"

// ----------------------------------------------------------------------------
// [SECTION] BodyCaptureArea
// ----------------------------------------------------------------------------

class BodyCaptureArea;
META_DECLARE_CLASS(BodyCaptureArea)

class BodyCaptureArea: public Object {
public:
  BodyCaptureArea() { m_metaClassId = MetaClassId(BodyCaptureArea); }
  ~BodyCaptureArea() = default;

  Matrix4 transform = {};
};

// ----------------------------------------------------------------------------
// [SECTION] HeightMapChunkBarn
// ----------------------------------------------------------------------------

class BodyCaptureController;
META_DECLARE_CLASS(BodyCaptureController)

class BodyCaptureController: public Module {
public:
  BodyCaptureController() { m_metaClassId = MetaClassId(BodyCaptureController); }
  ~BodyCaptureController() = default;

  void Initialize();
  void Terminate();
  void OnLevelLoad();
  void OnLevelUnload();
  void Update();

  BodyCaptureArea *CreateCaptureArea(MetaClass *mc);

private:
  // Configurations.
  bool m_hasParams = false;
  BodyCaptureArea m_params = {};
};

#endif
