#ifndef __MOD_OBJECTSOURCE_HPP__
#define __MOD_OBJECTSOURCE_HPP__

#include "sky/skyObjectFactory.hpp"

// The extended verion of ObjectFactory.
class ObjectSource: public ObjectFactory {
public:
  ObjectSource() = default;
  ~ObjectSource() = default;

  bool TryAddFactory(const Variable &creator);
};

#endif
