#ifndef __MOD_MOD_HPP__
#define __MOD_MOD_HPP__

#include <Base/Meta.hpp>
#include "sky/skyGame.hpp"
#include "mod/moduleBarnExt.hpp"
#include "render/renderTest.hpp"
#include "world/heightMapChunkBarn.hpp"

class Mod;
META_DECLARE_CLASS(Mod)

class Mod: public Module {
public:
  Mod() { m_metaClassId = MetaClassId(Mod); }
  ~Mod() = default;

  void Initialize(Game *);
  void Terminate();
  void OnLevelLoadEarly();
  void OnLevelLoad(cstring levelName);
  void OnLevelLoadLate();
  void OnLevelUnload();
  void OnLevelUnloadLate();
  void HotLoad();
  void HotUnload();
  void Resize();
  void Update();
  void UpdateLate();
  void UpdateBackground();
  void BuildScene();
  void RenderFlush();
  void PostRender();
  void OnPause();
  void OnUnpause();
  void OnSuspend();
  void OnEvent();

private:
  Game *m_game = nullptr;
  ModuleBarnExt *m_moduleBarnExt = nullptr;
  RenderTest *m_renderTest = nullptr;
  HeightMapChunkBarn *m_chunks = nullptr;
};

#endif
