#ifndef __SKY_SKYSCENE_HPP__
#define __SKY_SKYSCENE_HPP__

#include "sky/skyTypePlaceholders.hpp"
#include "sky/skyVertex.hpp"

class Scene {
public:
  RenderList *GetRenderListByName(cstring);
};

#endif
