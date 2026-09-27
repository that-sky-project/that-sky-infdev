#ifndef __SKY_SKYAUTOLISTER_HPP__
#define __SKY_SKYAUTOLISTER_HPP__

#include <Utils/TSPOS.hpp>
#include "sky/skyPrivate.hpp"

template<typename T>
class AutoLister {
private:
  static AutoLister<T> *ms_vars;

public:
  static void Append(T *p) {
    /*ms_vars->m_lock.BeginLock();

        if (!ms_vars)
        {
            Private::AssertImpl("ms_vars",
                                "../../../Code/Base/AutoLister.h",
                                __LINE__,
                                3);
            pthread_kill(pthread_self(), 20);
            abort();
        }

        p->m_pNext = ms_vars->m_pHead;

        if (ms_vars->m_pHead)
            ms_vars->m_pHead->m_pPrev = p;

        ms_vars->m_pHead = p;
        p->m_pPrev = nullptr;

        Lock::EndLock(&ms_vars->m_mutex);*/
    }

private:
  Lock m_lock = {};
};

template<class T>
AutoLister<T> *AutoLister<T>::ms_vars = nullptr;

#endif
