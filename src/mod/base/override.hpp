#ifndef __MOD_BASE_OVERRIDE_HPP__
#define __MOD_BASE_OVERRIDE_HPP__

#include <thread>
#include <Base/Meta.hpp>
#include "mod/meta.hpp"
#include "sky/skyGame.hpp"

class Override {
public:
  Override() = default;
  ~Override() = default;

  void initialize();

  void setMetaSystem(
    FakeMetaSystem *self);

  void overrideMetaSystem(
    Game *target);
  void overrideGame();

  LPCMetaClass getMetaClassById(
    u32 id
  ) const;
  LPCMetaClass getMetaClassByName(
    cstring name,
    bool isConstString
  ) const;

private:
  std::thread::id m_mainThread = {};
};

Override &getOverride();

#endif
