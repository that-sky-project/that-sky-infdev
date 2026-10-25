#include "sky/skyTypePlaceholders.hpp"
#include "sky/skyVectorMath.hpp"

META_REGISTER_CLASS(ResourceManager)

class MetaClassStub: public MetaClass {
public:
  MetaClassStub(cstring name): MetaClass(name) { }

  virtual size_t  SizeOfType() const override               { return 0; }
  virtual size_t  AlignOfType() const override              { return 0; }

  virtual void *  CreateByType() const override             { return nullptr; }
  virtual void    DeleteByType(void *p) const override      { }

  virtual void *  ConstructByType(void *p) const override   { return p; }
  virtual void    DestructByType(void *p) const override    { }

  virtual bool    IsNumber() const override                 { return false; }
  virtual bool    IsString() const override                 { return false; }

  virtual LPMetaType Copy() const override                  { return new MetaClassStub{*this}; }

  virtual bool    IsAbstract() const override               { return true; }
  virtual bool    IsPolymorphic() const override            { return false; }

  virtual size_t  SizeOfObject() const override             { return 0; }
  virtual size_t  AlignOfObject() const override            { return 0; }

  virtual void *  NewObject() const override                { return nullptr; }
  virtual void    DeleteObject(void *) const override       { }

  virtual void *  ConstructObject(void *) const override    { return nullptr; }
  virtual void    DestructObject(void *) const override     { }

  virtual void *  Upcast(void *const &) const override      { return nullptr; }
  virtual void *  Downcast(Object *const &) const override  { return nullptr; }

  virtual void *ResolveMember(
    void *,
    LPCMetaClass,
    MetaMemberVariable::PMember
  ) const override {
    return nullptr;
  }
};

static MetaClassStub g_metaType_Matrix3{"Matrix3"};
META_REGISTER_TYPE(Matrix3)

static MetaClassStub g_metaType_Matrix4{"Matrix4"};
META_REGISTER_TYPE(Matrix4)

static MetaClassStub g_metaType_Vector2{"Vector2"};
META_REGISTER_TYPE(Vector2)

static MetaClassStub g_metaType_Vector3{"Vector3"};
META_REGISTER_TYPE(Vector3)

static MetaClassStub g_metaType_Vector4{"Vector4"};
META_REGISTER_TYPE(Vector4)
