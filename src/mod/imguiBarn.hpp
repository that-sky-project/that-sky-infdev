#ifndef __MOD_IMGUIBARN_HPP__
#define __MOD_IMGUIBARN_HPP__

#include "mod/moduleBarnExt.hpp"
#include "sky/skyScene.hpp"

class ImGuiBarn: public Module {
public:
  ImGuiBarn() = default;
  ~ImGuiBarn() = default;

  void Initialize(Scene *scene);
  void Terminate();
  void Update();
  void RenderFlush();

private:

};

#endif
