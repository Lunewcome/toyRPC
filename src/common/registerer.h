#ifndef COMMON_REGISTERER_REGISTERER_H_
#define COMMON_REGISTERER_REGISTERER_H_

#include <map>
#include <string>

#include "common/singleton.h"
#include "glog/logging.h"

namespace registerer {

// idea from boost any but make it more simple and don't use type_info.
class Any {
 public:
  Any() : content_(NULL) {}

  template<typename ValueType>
  Any(const ValueType &value)  // NOLINT
      : content_(new Holder<ValueType>(value)) {}

  Any(const Any &other)
      : content_(other.content_ ? other.content_->clone() : NULL) {
  }

  ~Any() {
    delete content_;
  }

  template<typename ValueType>
  ValueType *any_cast() {
    return content_ ?
      &static_cast<Holder<ValueType> *>(content_)->held_ : NULL;  // NOLINT
  }

 private:
  class PlaceHolder {
   public:
    virtual ~PlaceHolder() {}
    virtual PlaceHolder *clone() const = 0;
  };

  template<typename ValueType>
  class Holder : public PlaceHolder {
   public:
    explicit Holder(const ValueType &value) : held_(value) {}
    virtual PlaceHolder *clone() const {
      return new Holder(held_);
    }

    ValueType held_;
  };

  PlaceHolder *content_;
};

class ObjectFactory {
 public:
  ObjectFactory() {}
  virtual ~ObjectFactory() {}
  virtual Any NewInstance() {
    return Any();
  }
  virtual Any GetSingletonInstance() {
    return Any();
  }
 private:
  BAN_COPY_AND_ASSIGN(ObjectFactory);
};

typedef std::map<std::string, ObjectFactory*> FactoryMap;
typedef std::map<std::string, FactoryMap> BaseClassMap;
BaseClassMap& global_factory_map();

}  // namespace registerer

#define REGISTER_REGISTERER(base_class) \
  class base_class ## Registerer { \
    typedef ::registerer::Any Any;\
    typedef ::registerer::FactoryMap FactoryMap; \
    public: \
      static base_class *GetInstanceByName(const ::std::string &name) { \
        FactoryMap &map = ::registerer::global_factory_map()[#base_class]; \
        FactoryMap::iterator iter = map.find(name); \
        if (iter == map.end()) { \
          LOG(ERROR) << "Get instance " << name << " failed."; \
          return NULL; \
        } \
        Any object = iter->second->NewInstance(); \
        return *(object.any_cast<base_class*>()); \
      } \
      static base_class* GetSingletonInstanceByName( \
          const ::std::string& name) { \
        FactoryMap& map = ::registerer::global_factory_map()[#base_class]; \
        FactoryMap::iterator iter = map.find(name); \
        if (iter == map.end()) { \
          LOG(ERROR) << "Get singleton instance " << name << " failed."; \
          return NULL; \
        }\
        Any object = iter->second->GetSingletonInstance(); \
        return *(object.any_cast<base_class*>()); \
      } \
      static const ::std::string GetUniqInstanceName() { \
        FactoryMap &map = ::registerer::global_factory_map()[#base_class]; \
        CHECK_EQ(map.size(), 1uL); \
        return map.begin()->first; \
      } \
      static base_class *GetUniqInstance() { \
        FactoryMap &map = ::registerer::global_factory_map()[#base_class]; \
        CHECK_EQ(map.size(), 1uL); \
        Any object = map.begin()->second->NewInstance(); \
        return *(object.any_cast<base_class*>()); \
      } \
      static bool IsValid(const ::std::string &name) { \
        FactoryMap &map = ::registerer::global_factory_map()[#base_class]; \
        return map.find(name) != map.end(); \
      } \
  }; \

#define REGISTER_CLASS(clazz, name) \
  namespace registerer { \
  class ObjectFactory##name : public ::registerer::ObjectFactory { \
    public: \
      ::registerer::Any NewInstance() { \
        return ::registerer::Any(new name()); \
      } \
      ::registerer::Any GetSingletonInstance() { \
        return ::registerer::Any(Singleton<name>::GetInstance()); \
      } \
  }; \
  __attribute__((constructor)) void register_factory_##name(); \
  void register_factory_##name() { \
    ::registerer::FactoryMap &map = \
      ::registerer::global_factory_map()[#clazz]; \
    if (map.find(#name) == map.end()) \
      map[#name] = new ObjectFactory##name(); \
  } \
  }

#endif  // COMMON_REGISTERER_REGISTERER_H_
