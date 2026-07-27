#ifndef __SKY_SKYTGCOS_HPP__
#define __SKY_SKYTGCOS_HPP__

class Lock {
public:
  inline void BeginLock() { }
  inline void EndLock() { }
};

class RwLock { void BeginWriteReadLock() { } };

#endif
