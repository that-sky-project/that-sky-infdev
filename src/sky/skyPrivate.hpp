#ifndef __SKY_SKYPRIVATE_HPP__
#define __SKY_SKYPRIVATE_HPP__

#include <intrin.h>
#include <Utils/Types.h>

namespace Private {

void AssertImpl(
  cstring expr,
  cstring file,
  u32 line,
  u32 hash);

void AssertMsgImpl(
  cstring expr,
  cstring file,
  u32 line,
  u32 hash,
  cstring format,
  ...);

}

#define Assert(expr) ((void)(!(expr) && (Private::AssertImpl(#expr, __FILE__, __LINE__, 3), __debugbreak(), abort(), 1)))
#define AssertMsg(expr, msg, ...) ((void)(!(expr) && (Private::AssertMsgImpl(#expr, __FILE__, __LINE__, 3, msg, ## __VA_ARGS__), __debugbreak(), abort(), 1)))

#endif
