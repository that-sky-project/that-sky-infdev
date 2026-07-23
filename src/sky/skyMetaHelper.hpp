#ifndef __SKY_SKYMETAHELPER_HPP__
#define __SKY_SKYMETAHELPER_HPP__

#include <Base/Meta.hpp>
#include "sky/skyPrivate.hpp"

template<typename Ty>
class MetaResolve {
public:
  ~MetaResolve() = default;
  MetaResolve() = delete;
  MetaResolve(MetaResolve &&) = default;
  MetaResolve(const MetaResolve &) = delete;
  MetaResolve &operator=(const MetaResolve &) = delete;

  template<typename Tm>
  explicit MetaResolve(
    Tm *obj
  )
    : m_reg(GetMetaClassByType<Tm *>)
    , m_obj(reinterpret_cast<void *>(obj))
  { }

  Ty &operator[](const TgcString &k) {
    Assert(m_reg && m_obj);

    LPCMetaClass mc = m_reg()->AsClass();
    AssertMsg(mc, "%s is not a class type.", m_reg()->GetName());

    const auto &itmv = mc->m_metaDataContainer->m_variables.find(k.c_str());
    AssertMsg(
      itmv != mc->m_metaDataContainer->m_variables.end(),
      "Try to resolve non-existent member %s::%s",
      mc->GetName(),
      k.c_str());

    const MetaMemberVariable *mmv = itmv->second;
    return m_obj->*reinterpret_cast<Ty VoidClass::*>(mmv->Address());
  }

private:
  PFN_GetClass m_reg = nullptr;
  VoidClass *m_obj = nullptr;
};

template<typename Fn>
class MetaCall;

template<typename Class, typename Return, typename ...Args>
class MetaCall<Return (Class::*)(Args...)> {
private:
  struct Call {
    explicit Call(
      Return (VoidClass::*fn)(Args...)
    ) {

    }

    Return operator()(Args ...args) {

    }

    Return (VoidClass::*fn)(Args...) = nullptr;
  };

public:
  ~MetaCall() = default;
  MetaCall() = delete;
  MetaCall(MetaCall &&) = default;
  MetaCall(const MetaCall &) = delete;
  MetaCall &operator=(const MetaCall &) = delete;

  template<typename Tm>
  explicit MetaCall(
    Tm *obj
  )
    : m_reg(GetMetaClassByType<Tm *>)
    , m_obj(reinterpret_cast<void *>(obj))
  { }

  MetaCall::Call operator[](const TgcString &k) {
    Assert(m_reg && m_obj);

    LPCMetaClass mc = m_reg()->AsClass();
    AssertMsg(mc, "%s is not a class type.", m_reg()->GetName());

    const auto &itmv = mc->m_metaDataContainer->m_functions.find(k.c_str());
    AssertMsg(
      itmv != mc->m_metaDataContainer->m_functions.end(),
      "Try to resolve non-existent member %s::%s",
      mc->GetName(),
      k.c_str());

    const MetaMemberFunction *mmv = itmv->second;
    return MetaCall::Call(reinterpret_cast<Return (VoidClass::*)(Args...)>(mmv->m_function));
  }

private:
  PFN_GetClass m_reg = nullptr;
  VoidClass *m_obj = nullptr;
};

#endif
