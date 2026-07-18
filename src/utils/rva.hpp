#ifndef __SKY_RVA_HPP__
#define __SKY_RVA_HPP__

#include <Windows.h>
#include <Utils/Types.h>
#include <stddef.h>

class Rva {
public:
  Rva(
    uintptr_t rva,
    cstring module = nullptr
  ) {
    m_address = reinterpret_cast<uintptr_t>(GetModuleHandleA(module)) + rva;
  }

  inline uintptr_t operator()() const { return m_address; }
  inline uintptr_t operator*() const { return *(uintptr_t *)m_address; }

private:
  uintptr_t m_address = 0;
};

#endif
