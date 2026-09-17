#include <includes/htmodloader.h>
#include <Base/Meta.hpp>
#include "mod/modobject.hpp"

META_REGISTER_CLASS(Mod, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)

META_REGISTER_FUNCTION_MEMBER(Mod, Initialize)
META_REGISTER_FUNCTION_MEMBER(Mod, Terminate)
META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelLoadEarly)
META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelLoad)
META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelLoadLate)
META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelUnload)
META_REGISTER_FUNCTION_MEMBER(Mod, OnLevelUnloadLate)
META_REGISTER_FUNCTION_MEMBER(Mod, HotLoad)
META_REGISTER_FUNCTION_MEMBER(Mod, HotUnload)
META_REGISTER_FUNCTION_MEMBER(Mod, Resize)
META_REGISTER_FUNCTION_MEMBER(Mod, Update)
META_REGISTER_FUNCTION_MEMBER(Mod, UpdateLate)
META_REGISTER_FUNCTION_MEMBER(Mod, UpdateBackground)
META_REGISTER_FUNCTION_MEMBER(Mod, BuildScene)
META_REGISTER_FUNCTION_MEMBER(Mod, RenderFlush)
META_REGISTER_FUNCTION_MEMBER(Mod, PostRender)
META_REGISTER_FUNCTION_MEMBER(Mod, OnPause)
META_REGISTER_FUNCTION_MEMBER(Mod, OnUnpause)
META_REGISTER_FUNCTION_MEMBER(Mod, OnSuspend)
META_REGISTER_FUNCTION_MEMBER(Mod, OnEvent)

META_DATA_MEMBER_FUNCTION(Mod, OnLevelLoad, ArgName, "levelName")

void Mod::Initialize(
  Game *game
) {
  m_game = game;

  m_moduleBarnExt = new ModuleBarnExt();
  m_moduleBarnExt->Initialize(game);

  m_renderTest = new RenderTest();
  m_renderTest->Initialize(game);

  m_chunks = new HeightMapChunkBarn();
  m_chunks->Initialize();
}

void Mod::Terminate() {
  m_chunks->Terminate();
  delete m_chunks;

  m_renderTest->Terminate();
  delete m_renderTest;

  m_moduleBarnExt->Terminate();
  delete m_moduleBarnExt;
}

void Mod::OnLevelLoadEarly() { }

void Mod::OnLevelLoad(
  cstring levelName
) {
  HTTellText("Mod::OnLevelLoad(%s)", levelName);
  m_chunks->OnLevelLoad(
    m_game->resolveMember<ResourceManager *>("resources"),
    m_game->resolveMember<Scene *>("scene"),
    m_game->resolveMember<MaterialDefBarn *>("materialDefBarn"),
    m_game->resolveMember<cstring>("levelName")
  );
}

void Mod::OnLevelLoadLate() { }

void Mod::OnLevelUnload() {
  m_chunks->OnLevelUnload(
    m_game->resolveMember<cstring>("levelName")
  );
}
void Mod::OnLevelUnloadLate() { }
void Mod::HotLoad() { }
void Mod::HotUnload() { }
void Mod::Resize() { }

void Mod::Update() {
  //HTTellText("Mod::Update(%p)", m_game);
  m_renderTest->Update();
  m_chunks->Update(
    m_game,
    m_game->resolveMember<AvatarBarn *>("avatarBarn"),
    m_game->resolveMember<cstring>("levelName")
  );
}

void Mod::UpdateLate() { }
void Mod::UpdateBackground() { }

void Mod::BuildScene() {
  m_chunks->BuildScene(
    m_game->resolveMember<cstring>("levelName")
  );
}

void Mod::RenderFlush() { }
void Mod::PostRender() { }
void Mod::OnPause() { }
void Mod::OnUnpause() { }
void Mod::OnSuspend() { }
void Mod::OnEvent() { }
