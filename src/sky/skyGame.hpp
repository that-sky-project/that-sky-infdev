#ifndef __SKY_SKYGAME_HPP__
#define __SKY_SKYGAME_HPP__

#include <Utils/Types.h>
#include <Utils/Assert.hpp>
#include <Base/Meta.hpp>
#include "sky/skyPrivate.hpp"

// The size of the Game object is not fixed; it needs to be handled based on the
// actually allocated memory. This type is only used as a placeholder.
class Game: public Object {
public:
  Game() = default;
  virtual ~Game() = default;

  template<typename T>
  T &resolveMember(
    cstring name
  ) {
    T *ptr = static_cast<T *>(resolveMember(name));
    AssertMsg(ptr, "Try to resolve null pointer of Game::%s", name);
    return *ptr;
  }

private:
  void *resolveMember(
    cstring name);
};

META_DECLARE_CLASS(Game)

#endif
