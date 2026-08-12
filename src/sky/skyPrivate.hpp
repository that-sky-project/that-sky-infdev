#ifndef __SKY_SKYPRIVATE_HPP__
#define __SKY_SKYPRIVATE_HPP__

#include <intrin.h>
#include <Utils/Types.h>

namespace Private {

inline constexpr cstring FileName(cstring path) {
  const char *start = path;
  for (const char *p = path; *p != '\0'; ++p) {
    if (*p == '/' || *p == '\\') {
      start = p + 1;
    }
  }
  return start;
}

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

#define Assert(expr) \
  do {\
    constexpr cstring FILENAME = Private::FileName(__FILE__);\
    ((void)(!(expr) && (Private::AssertImpl(#expr, FILENAME, __LINE__, 3), __debugbreak(), abort(), 1)));\
  } while (0)
#define AssertMsg(expr, msg, ...) \
  do {\
    constexpr cstring FILENAME = Private::FileName(__FILE__);\
    ((void)(!(expr) && (Private::AssertMsgImpl(#expr, FILENAME, __LINE__, 3, msg, ## __VA_ARGS__), __debugbreak(), abort(), 1)));\
  } while (0)

#endif
