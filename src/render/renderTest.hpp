#ifndef __RENDER_RENDERTEST_HPP__
#define __RENDER_RENDERTEST_HPP__

#include "sky/skyTypePlaceholders.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyVertex.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "render/terrainRender.hpp"

class RenderTest {
public:
  RenderTest() = default;
  ~RenderTest() = default;

  void Initialize(
    Game *game);
  void Terminate();
  void Update();
  void OnLevelLoad();
  void OnLevelUnload();

private:
  void m_InitializeTerrain();
  void m_InitializeEndPortal();

  void m_TerminateTerrain();
  void m_TerminateEndPortal();

  void m_UpdateTerrain();
  void m_UpdateEndPortal();

  Game *game = nullptr;
  Scene *scene = nullptr;
  ResourceManager *resourceManager = nullptr;
  Heap *heap = nullptr;
  MaterialDefBarn *materialDefBarn = nullptr;

  VertexRender depthR;
  VertexRender matR;
  VertexData depthD;
  VertexData matD;

  VertexRender endportalR;
  VertexData endportalD;

  void *depthVtxBuffer;
  void *matVtxBuffer;

  //std::vector<TerrainRenderChunk *> chunks = {};
};

#endif
