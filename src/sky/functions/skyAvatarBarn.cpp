#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "sky/skyPrivate.hpp"
#include "sky/skyVectorMath.hpp"
#include "sky/skyAvatarBarn.hpp"
#include "sky/skyMetaHelper.hpp"

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
