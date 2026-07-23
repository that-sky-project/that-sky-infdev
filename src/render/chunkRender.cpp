#include "render/chunkRender.hpp"

void ChunkRender::Initialize(
  Game *game,
  Heap *heap,
  Scene *scene,
  ResourceManager *resources,
  MaterialDefBarn *materialDefBarn
) {
  m_game = game;
  m_heap = heap;
  m_scene = scene;
  m_resources = resources;
  m_materialDefBarn = materialDefBarn;
}
