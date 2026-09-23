#include <Windows.h>
#include <htskymodbase.h>
#include <Utils/Types.h>
#include "sky/skyPrivate.hpp"
#include "mod/modbasecall.hpp"

extern HMODULE hModuleDll;

static void s_ModbaseRegisterShader(
  cstring shader
) {
  static const cstring s_template[] = {
    "Data/Shaders/Bin/%s-12f.win.vs.spv",
    "Data/Shaders/Bin/%s-12f.win.fs.spv",
    "Data/Shaders/Bin/%s-12f.win.ref",
    "Data/Shaders/Bin/%s-13f.win.vs.spv",
    "Data/Shaders/Bin/%s-13f.win.fs.spv",
    "Data/Shaders/Bin/%s-13f.win.ref",
    "Data/Shaders/Bin/%s-13h.win.vs.spv",
    "Data/Shaders/Bin/%s-13h.win.fs.spv",
    "Data/Shaders/Bin/%s-13h.win.ref",
  };

  char buffer[256] = {0};
  for (cstring name: s_template) {
    snprintf(buffer, sizeof(buffer), name, shader);
    HTStatus s = SkyEx_Resources_RegisterSingleEx(hModuleDll, buffer, nullptr, false);
    AssertMsg(s == HT_SUCCESS, "Register shader %s failed", shader);
  }
}

static void s_ModbaseRegisterLevel(
  cstring levelName
) {
  static const cstring s_template[] = {
    "Data/Levels/%s/Resources.lua",
    "Data/Levels/%s/Objects.level.bin",
    "Data/Levels/%s/BstBaked.meshes"
  };

  char buffer[256] = {0};
  for (cstring name: s_template) {
    snprintf(buffer, sizeof(buffer), name, levelName);
    HTStatus s = SkyEx_Resources_RegisterSingleEx(hModuleDll, buffer, nullptr, false);
    AssertMsg(s == HT_SUCCESS, "Register level %s failed", levelName);
  }

  HTStatus s = SkyEx_Definitions_RegisterLevel(levelName);
  AssertMsg(s == HT_SUCCESS, "Register level %s failed", levelName);
}

void Modbase::RegisterAssets() {
  s_ModbaseRegisterShader("SimpleColorTest");
  s_ModbaseRegisterShader("EndPortal");

  s_ModbaseRegisterLevel("Infdev_INF");
}
