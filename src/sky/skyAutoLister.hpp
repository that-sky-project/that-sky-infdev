#ifndef __SKY_SKYAUTOLISTER_HPP__
#define __SKY_SKYAUTOLISTER_HPP__

template<typename T>
class AutoLister {
private:
  struct Vars {

  };

  static Vars ms_vars = {};

public:

private:
  AutoLister<T> *m_next = nullptr;
  AutoLister<T> *m_prev = nullptr;
};

#endif
