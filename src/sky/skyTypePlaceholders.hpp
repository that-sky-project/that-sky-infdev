#ifndef __SKY_SKYTYPEPLACEHOLDERS_HPP__
#define __SKY_SKYTYPEPLACEHOLDERS_HPP__

#include <Base/Meta.hpp>

using ShaderProgram = void *;

class ResourceManager { };
META_DECLARE_CLASS(ResourceManager)

class AvatarBarn { };
META_DECLARE_CLASS(AvatarBarn)

class Avatar { };
META_DECLARE_CLASS(Avatar)

class Module: public Object { };
META_DECLARE_CLASS(Module);

#endif