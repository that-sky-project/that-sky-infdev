#include "sky/skyPrivate.hpp"
#include "bodyCapture/bodyCapture.hpp"

// ----------------------------------------------------------------------------
// [SECTION] BodyCaptureArea
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(BodyCaptureArea, MetaClassImpl<Object>::Must_call_META_REGISTER_CLASS)
META_DATA_CLASS(BodyCaptureArea, Tool_Export, "true")
META_DATA_CLASS(BodyCaptureArea, Infdev_LevelExport, "true")

META_REGISTER_SIMPLE_MEMBER(BodyCaptureArea, transform)

// ----------------------------------------------------------------------------
// [SECTION] BodyCaptureController
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(BodyCaptureController, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

// Initialize chunk renderer contexts.
META_REGISTER_FUNCTION_MEMBER(BodyCaptureController, OnLevelLoad)
void BodyCaptureController::OnLevelLoad() {
  if (!m_hasParams)
    return;
}

// Deinitialize chunk renderer contexts.
META_REGISTER_FUNCTION_MEMBER(BodyCaptureController, OnLevelUnload)
void BodyCaptureController::OnLevelUnload() {
  if (!m_hasParams)
    return;

  m_hasParams = false;
}

META_REGISTER_FUNCTION_MEMBER(BodyCaptureController, CreateCaptureArea)
META_DATA_MEMBER_FUNCTION(BodyCaptureController, CreateCaptureArea, ObjectFactory_CreatesType, "BodyCaptureArea")
BodyCaptureArea *BodyCaptureController::CreateCaptureArea(
  MetaClass *mc
) {
  Assert(!m_hasParams);
  m_hasParams = true;
  return &m_params;
}
