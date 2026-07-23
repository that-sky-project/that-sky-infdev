#include "utils/rva.hpp"
#include "sky/skyRendererUtils.hpp"

Renderer *GetRenderer() {
  using PFN = Renderer *(*)();
  static const Rva s_GetRenderer = 0x000A65B0;
  return ((PFN)s_GetRenderer())();
}
