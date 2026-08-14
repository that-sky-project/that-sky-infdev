#include <includes/htmodloader.h>
#include <Base/Meta.hpp>
#include "mod/modobject.hpp"

META_REGISTER_CLASS(Mod, MetaClassImpl<Module>::Must_call_META_REGISTER_CLASS)
META_REGISTER_FUNCTION_MEMBER(Mod, Initialize)

void Mod::Initialize(
  Game *game
) {
  m_game = game;
}

void Mod::Terminate() { }
void Mod::OnLevelLoadEarly() { }
void Mod::OnLevelLoad() { }
void Mod::OnLevelLoadLate() { }
void Mod::OnLevelUnload() { }
void Mod::OnLevelUnloadLate() { }
void Mod::HotLoad() { }
void Mod::HotUnload() { }
void Mod::Resize() { }

void Mod::Update() {
  HTTellText("Mod::Update(%p)", m_game);
}

void Mod::UpdateLate() { }
void Mod::UpdateBackground() { }
void Mod::BuildScene() { }
void Mod::RenderFlush() { }
void Mod::PostRender() { }
void Mod::OnPause() { }
void Mod::OnUnpause() { }
void Mod::OnSuspend() { }
void Mod::OnEvent() { }
