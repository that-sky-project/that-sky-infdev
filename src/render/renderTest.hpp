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
  static void installHook();

  RenderTest() = default;
  ~RenderTest() = default;

  void initialize(
    Game *game);
  void update();
  void onLevelLoad();
  void onLevelUnload();

private:
  Game *game = nullptr;
  Scene *scene = nullptr;
  ResourceManager *resourceManager = nullptr;
  Heap *heap = nullptr;
  MaterialDefBarn *materialDefBarn = nullptr;

  VertexRender depthR;
  VertexRender matR;
  VertexData depthD;
  VertexData matD;

  //std::vector<TerrainRenderChunk *> chunks = {};
};

#endif
