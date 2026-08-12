#include <Windows.h>
#include <includes/htmod.h>
#include "mod/meta.hpp"
#include "mod/modbasecall.hpp"
#include "render/renderTest.hpp"

HMODULE hModuleDll = NULL;

__declspec(dllexport) HTStatus HTMLAPI HTModOnInit(
  void *reserved
) {
  MetaSystemOverride::initialize();
  RenderTest::installHook();
  Modbase::RegisterAssets();
  return HT_SUCCESS;
}

__declspec(dllexport) HTStatus HTMLAPI HTModOnEnable(
  void *reserved
) {
  return HT_SUCCESS;
}

__declspec(dllexport) void HTMLAPI HTModRenderGui(
  float timeElapesed,
  void *reserved
) {
  ;
}

BOOL APIENTRY DllMain(
  HMODULE hModule,
  DWORD dwReason,
  LPVOID lpReserved
) {
  if (dwReason == DLL_PROCESS_ATTACH)
    hModuleDll = hModule;
  return TRUE;
}
