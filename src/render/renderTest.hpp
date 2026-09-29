#ifndef __RENDER_RENDERTEST_HPP__
#define __RENDER_RENDERTEST_HPP__

#include "sky/skyTypePlaceholders.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyVertex.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "sky/skyCollisionGeo.hpp"
#include "sky/skyTexture.hpp"

namespace RenderTest {

// Interface for tests.
class Test {
public:
  Test() = default;
  Test(const Test &) = delete;
  Test(Test &&) = delete;
  Test &operator=(const Test &) = delete;

  virtual ~Test() = default;
  virtual void Initialize(Game *game) = 0;
  virtual void Terminate() = 0;
  virtual void BuildScene() = 0;
};

// Render a terrain chunk with Sky's shader.
class TerrainTest: public Test {
public:
  virtual ~TerrainTest() override = default;

  virtual void Initialize(Game *game) override { }
  virtual void Terminate() override { }
  virtual void BuildScene() override { }

private:
  VertexRender m_depthR = {};
  VertexRender m_matR = {};
  VertexData m_depthD = {};
  VertexData m_matD = {};
};

// Render a Minecraft-like EndPortal with customized shader and collision.
class ShaderTest: public Test {
public:
  virtual ~ShaderTest() override = default;

  virtual void Initialize(Game *game) override;
  virtual void Terminate() override;
  virtual void BuildScene() override;

private:
  void m_Initialize(
    Scene *scene,
    ResourceManager *resources,
    MaterialDefBarn *materialDefBarn,
    CollisionGeoBarn *collisionGeoBarn);

  CollisionGeoBarn *m_collisionGeoBarn = nullptr;

  VertexRender m_endportalR = {};
  VertexData m_endportalD = {};

  u32 m_geoIndex = 0;
  CollisionGeoInstance *m_geoInst = nullptr;
};

// Render a triangle in the world with UV.
class TextureTest: public Test {
public:
  virtual ~TextureTest() override = default;

  virtual void Initialize(Game *game) override;
  virtual void Terminate() override;
  virtual void BuildScene() override;

private:
  VertexRender m_render = {};
  VertexData m_data = {};

  Texture m_texture = {};
};

class Render {
public:
  ~Render() = default;
  Render() = default;
  Render(const Render &) = delete;
  Render(Render &&) = delete;
  Render &operator=(const Render &) = delete;

  void Initialize(Game *game);
  void Terminate();

  void BuildScene();

private:
  Test **tests = nullptr;
};

}

#endif
