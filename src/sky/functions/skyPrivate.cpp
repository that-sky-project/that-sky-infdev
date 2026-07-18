#include "utils/rva.hpp"
#include "sky/skyPrivate.hpp"

using PFN_AssertHandler = void (*)(cstring, cstring, cstring, u32, u32);
static const Rva s_pfn_AssertHandler = 0x01E27BA0;

void Private::AssertImpl(
  cstring expr,
  cstring file,
  u32 line,
  u32 hash
) {
  ((PFN_AssertHandler)*s_pfn_AssertHandler)(expr, "", file, line, hash);
}

void Private::AssertMsgImpl(
  cstring expr,
  cstring file,
  u32 line,
  u32 hash,
  cstring format,
  ...
) {
  char buffer[2056];

  va_list va;
  va_start(va, format);
  vsnprintf(buffer, sizeof(buffer), format, va);
  va_end(va);

  ((PFN_AssertHandler)*s_pfn_AssertHandler)(expr, buffer, file, line, hash);
}
