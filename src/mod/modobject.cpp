#include <includes/htmodloader.h>
#include <Base/Meta.hpp>
#include "sky/skyAvatarBarn.hpp"
#include "mod/modobject.hpp"

META_REGISTER_CLASS(Mod, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

META_REGISTER_FUNCTION_MEMBER(Mod, Initialize)
void Mod::Initialize(
  Game *game
) {
  m_game = game;

  m_moduleBarnExt = new ModuleBarnExt();
  m_moduleBarnExt->Initialize(game);

  m_renderTest = new RenderTest();
  m_renderTest->Initialize(game);

  m_chunks = new HeightMapChunkBarn();
  m_chunks->Initialize(
    m_game->resolveMember<CollisionGeoBarn *>("collisionGeoBarn"));
}

META_REGISTER_FUNCTION_MEMBER(Mod, Terminate)
void Mod::Terminate() {
  m_chunks->Terminate();
  delete m_chunks;

  m_renderTest->Terminate();
  delete m_renderTest;

  m_moduleBarnExt->Terminate();
  delete m_moduleBarnExt;
}

META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelLoadEarly)
void Mod::OnLevelLoadEarly() { }

META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelLoad)
META_DATA_MEMBER_FUNCTION(Mod, OnLevelLoad, ArgName, "levelName")
void Mod::OnLevelLoad(
  cstring levelName
) {
  HTTellText("[ThatSkyInfDev] Mod::OnLevelLoad(%s)", levelName);
  m_chunks->OnLevelLoad(
    m_game,
    m_game->resolveMember<ResourceManager *>("resources"),
    m_game->resolveMember<Scene *>("scene"),
    m_game->resolveMember<MaterialDefBarn *>("materialDefBarn"),
    m_game->resolveMember<cstring>("levelName")
  );
}

META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelLoadLate)
void Mod::OnLevelLoadLate() { }

META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelUnload)
void Mod::OnLevelUnload() {
  m_chunks->OnLevelUnload(
    m_game->resolveMember<cstring>("levelName")
  );
}

META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelUnloadLate)
void Mod::OnLevelUnloadLate() { }

META_REGISTER_FUNCTION_MEMBER(Mod, HotLoad)
void Mod::HotLoad() { }

META_REGISTER_FUNCTION_MEMBER(Mod, HotUnload)
void Mod::HotUnload() { }

META_REGISTER_FUNCTION_MEMBER(Mod, Resize)
void Mod::Resize() { }

META_REGISTER_FUNCTION_MEMBER(Mod, Update)
void Mod::Update() {
  //HTTellText("Mod::Update(%p)", m_game);
  m_renderTest->Update();
  m_chunks->Update(
    m_game,
    m_game->resolveMember<AvatarBarn *>("avatarBarn"),
    m_game->resolveMember<cstring>("levelName")
  );
}

META_REGISTER_FUNCTION_MEMBER(Mod, UpdateLate)
void Mod::UpdateLate() { }

META_REGISTER_FUNCTION_MEMBER(Mod, UpdateBackground)
void Mod::UpdateBackground() { }

META_REGISTER_FUNCTION_MEMBER(Mod, BuildScene)
void Mod::BuildScene() {
  m_chunks->BuildScene(
    m_game->resolveMember<cstring>("levelName")
  );
}

META_REGISTER_FUNCTION_MEMBER(Mod, RenderFlush)
void Mod::RenderFlush() { }

META_REGISTER_FUNCTION_MEMBER(Mod, PostRender)
void Mod::PostRender() { }

META_REGISTER_FUNCTION_MEMBER(Mod, OnPause)
void Mod::OnPause() { }

META_REGISTER_FUNCTION_MEMBER(Mod, OnUnpause)
void Mod::OnUnpause() { }

META_REGISTER_FUNCTION_MEMBER(Mod, OnSuspend)
void Mod::OnSuspend() { }

META_REGISTER_FUNCTION_MEMBER(Mod, OnEvent)
void Mod::OnEvent() { }
