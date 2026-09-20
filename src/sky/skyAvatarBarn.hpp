#ifndef __SKY_SKYAVATARBARN_HPP__
#define __SKY_SKYAVATARBARN_HPP__

#include <intrin.h>
#include <Utils/Types.h>
#include <Base/Meta.hpp>
#include "sky/skyVectorMath.hpp"
#include "mod/moduleBarnExt.hpp"

class Avatar {
//private:
public:
  ~Avatar() = default;
  Avatar() = default;
  Avatar(const Avatar &) = delete;
  Avatar(Avatar &&) = delete;
  Avatar &operator=(const Avatar &) = delete;

public:
  Matrix4 GetTransform();
  void Teleport(const Vector3 &pos, bool isCenter);

private:
  __m128 _align = {};
  u08 _gap[0x00010A40 - 0x10] = {0};
};
META_DECLARE_CLASS(Avatar)

class AvatarBarn: public Module {
//private:
public:
  ~AvatarBarn() = default;
  AvatarBarn() = default;
  AvatarBarn(const AvatarBarn &) = delete;
  AvatarBarn(AvatarBarn &&) = delete;
  AvatarBarn &operator=(const AvatarBarn &) = delete;

public:
  Avatar *TryGetLocalAvatar(bool a1);

private:
  __m128 _align = {};
  u08 _gap[0x004044E0 - 0x10] = {0};
};
META_DECLARE_CLASS(AvatarBarn)

#endif
