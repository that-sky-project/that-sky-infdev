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
  //std::vector<TerrainRenderChunk *> chunks = {};
};

#endif
