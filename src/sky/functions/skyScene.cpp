#include "utils/rva.hpp"
#include "sky/skyScene.hpp"

RenderList *Scene::GetRenderListByName(
  cstring name
) {
  using PFN = RenderList *(*)(Scene *, cstring);
  static const Rva s_Scene_GetRenderListByName = 0x00A42F70;
  return ((PFN)s_Scene_GetRenderListByName())(this, name);
}
