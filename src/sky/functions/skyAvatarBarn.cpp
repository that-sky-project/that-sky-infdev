#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "utils/rva.hpp"
#include "sky/skyPrivate.hpp"
#include "sky/skyVectorMath.hpp"
#include "sky/skyAvatarBarn.hpp"
#include "sky/skyMetaHelper.hpp"

// ----------------------------------------------------------------------------
// [SECTION] Avatar
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(Avatar)

Matrix4 Avatar::GetTransform() {
  using Call = decltype(&Avatar::GetTransform);
  static Call p = nullptr;
  if (p) return (this->*p)();

  auto &functions = GetMetaClassByType<Avatar *>()->m_metaDataContainer->m_functions;
  auto mmf = functions.find("GetTransform");
  Assert(mmf != functions.end());

  p = reinterpret_cast<Matrix4 (Avatar::*)()>(mmf->second->Function());

  return (this->*p)();
}

void Avatar::Teleport(
  const Vector3 &pos,
  bool isCenter
) {
  using PFN = void (*)(Avatar *, const Vector3 &, bool);
  static const Rva s_Avatar_Teleport = 0x014C0860;
  return ((PFN)s_Avatar_Teleport())(
    this, pos, isCenter);
}

// ----------------------------------------------------------------------------
// [SECTION] AvatarBarn
// ----------------------------------------------------------------------------

META_REGISTER_CLASS(AvatarBarn)

Avatar *AvatarBarn::TryGetLocalAvatar(
  bool a1
) {
  static Avatar *(AvatarBarn::*p)(bool) = nullptr;
  if (p)
    return (this->*p)(a1);

  auto &functions = GetMetaClassByType<AvatarBarn *>()->m_metaDataContainer->m_functions;
  auto mmf = functions.find("TryGetLocalAvatar");
  Assert(mmf != functions.end());

  p = reinterpret_cast<Avatar *(AvatarBarn::*)(bool)>(mmf->second->Function());

  return (this->*p)(a1);
}
