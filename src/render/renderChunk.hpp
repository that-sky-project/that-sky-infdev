#ifndef __RENDER_RENDERCHUNK_HPP__
#define __RENDER_RENDERCHUNK_HPP__

#include "mc/posTypes.hpp"
#include "sky/skyTypePlaceholders.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyVertex.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyMaterialDefBarn.hpp"

class ClientChunk {
public:
  ClientChunk() = default;
  ClientChunk(ClientChunk &&) = default;
  ClientChunk(const ClientChunk &) = delete;
  ClientChunk &operator=(const ClientChunk &) = delete;

  virtual ~ClientChunk() = default;
  virtual void Initialize(
    Game *game,
    Heap *heap,
    Scene *scene,
    ResourceManager *resources,
    MaterialDefBarn *materialDefBarn) = 0;
  virtual void Terminate() = 0;
  virtual void BuildScene() = 0;
  virtual void Update() = 0;
};

#endif
