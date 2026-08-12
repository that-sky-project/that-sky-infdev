#include <Windows.h>
#include <htskymodbase.h>
#include "mod/modbasecall.hpp"

extern HMODULE hModuleDll;

void Modbase::RegisterAssets() {
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/SimpleColorTest-12f.win.vs.spv", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/SimpleColorTest-12f.win.fs.spv", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/SimpleColorTest-12f.win.ref", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/SimpleColorTest-13f.win.vs.spv", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/SimpleColorTest-13f.win.fs.spv", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/SimpleColorTest-13f.win.ref", nullptr, false);

  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/EndPortal-12f.win.vs.spv", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/EndPortal-12f.win.fs.spv", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/EndPortal-12f.win.ref", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/EndPortal-13f.win.vs.spv", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/EndPortal-13f.win.fs.spv", nullptr, false);
  SkyEx_Resources_RegisterSingleEx(hModuleDll, "Data/Shaders/Bin/EndPortal-13f.win.ref", nullptr, false);
}
