/*  Class reflection definitions

    Copyright 2018-2024 Lukas Cone

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once
#include "reflector_type.hpp"
#include "spike/util/settings.hpp"
#include <map>

#ifdef REF_EXPORT
#define REF_EXPORT_ ES_EXPORT
#else
#define REF_EXPORT_
#endif

struct ReflDesc {
  const char *part1 = nullptr;
  const char *part2 = nullptr;
  ReflDesc() = default;
  ReflDesc(const char *part1_) : part1(part1_) {}
  ReflDesc(const char *part1_, const char *part2_)
      : part1(part1_), part2(part2_) {}
};

struct NoName {};

struct VectorMethods {
  enum Signal {
    // id0 = new size, id1 = unused
    Resize,
    // swap id0 with id1
    Swap,
    // id0 = insert after index, id1 = num items
    InsertNew,
    // id0 = duplicate index, id1 = num items
    DuplicateInsert,
    // id0 = duplicate index, id1 = num items
    DuplicateAppend,
    // id0 = start index, id1 = end index
    Erase,
  };
  using SignalWrap = void (*)(const void *, Signal, size_t id0, size_t id1);
  using AtWrap = void *(*)(const void *, size_t);
  using SizeWrap = size_t (*)(const void *);
  SignalWrap signal = nullptr;
  AtWrap at = nullptr;
  SizeWrap size = nullptr;
};

template <class C> consteval VectorMethods MakeVectorClass() {
  VectorMethods retval{
      .signal =
          [](const void *ptr_, VectorMethods::Signal sig, size_t id0,
             size_t id1) {
            C *recVec = static_cast<C *>(const_cast<void *>(ptr_));
            switch (sig) {
            case VectorMethods::Resize:
              recVec->resize(id0);
              break;
            case VectorMethods::Swap:
              std::swap(recVec->at(id0), recVec->at(id1));
              break;
            case VectorMethods::InsertNew:
              recVec->insert(std::next(recVec->begin(), id0 + 1), id1, {});
              break;
            case VectorMethods::DuplicateInsert:
              recVec->insert(std::next(recVec->begin(), id0), id1,
                             recVec->at(id0));
              break;
            case VectorMethods::DuplicateAppend:
              recVec->insert(recVec->end(), id1, recVec->at(id0));
              break;
            case VectorMethods::Erase:
              recVec->erase(std::next(recVec->begin(), id0),
                            std::next(recVec->begin(), id1));
              break;
            }
          },
      .at = [](const void *ptr_, size_t n) -> void * {
        auto &ref = static_cast<C *>(const_cast<void *>(ptr_))->at(n);
        return &ref;
      },
      .size =
          [](const void *ptr_) {
            return static_cast<const C *>(ptr_)->size();
          }};

  return retval;
}

struct MemberProxy {
  const char *typeName;
  const char *aliasName = nullptr;
  JenHash aliasHash;
  ReflType type;
  ReflDesc description;
  VectorMethods vectorMethods;

private:
  void setup(const char *aliasName_) {
    aliasName = aliasName_;
    aliasHash = JenHash(aliasName_);
  }

  void setup(const ReflDesc &desc) { description = desc; }

  void setup(NoName) { typeName = nullptr; }

public:
  template <class... Input, class type_>
  explicit MemberProxy(type_ getter, Input... inputs) {
    auto [offset, name, hash, rtfn, isbitmember] = getter();
    using MemberType = std::remove_pointer_t<decltype(rtfn)>;
    if constexpr (std::is_same_v<decltype(isbitmember), std::true_type>) {
      type =
          BuildBFReflType<std::remove_pointer_t<decltype(offset)>, MemberType>(
              hash);
    } else {
      constexpr REFContainer mainContainer = REFContainerType<MemberType>();
      type = BuildReflType<MemberType>(hash, 0, offset);
      if constexpr (mainContainer == REFContainer::ContainerVector ||
                    mainContainer == REFContainer::ContainerVectorMap) {
        vectorMethods = MakeVectorClass<MemberType>();
      }
    }

    typeName = name;

    (setup(inputs), ...);
  }
};

struct ClassMethods {
  using ConstructorWrap = void (*)(void *);
  using DestructorWrap = void (*)(void *);

  ConstructorWrap constructor = nullptr;
  DestructorWrap destructor;
};

template <class C> consteval ClassMethods MakeClassMethods() {
  ClassMethods retval{
      .destructor = [](void *ptr_) { static_cast<C *>(ptr_)->~C(); },
  };
  if constexpr (std::is_default_constructible_v<C>) {
    retval.constructor = [](void *ptr_) { ::new (ptr_) C(); };
  }
  return retval;
};

struct reflectorStatic {
  using RegistryType = std::map<JenHash, const reflectorStatic *>;
  const JenHash classHash;
  const uint32 nTypes;
  const ReflType *types;
  const char *const *typeNames = nullptr;
  const char *className;
  const char *const *typeAliases = nullptr;
  const JenHash *typeAliasHashes = nullptr;
  const ReflDesc *typeDescs = nullptr;
  const ClassMethods methods{};
  const uint32 classSize;
  const JenHash baseClass{};
  const VectorMethods *vectorMethods = nullptr;

  constexpr reflectorStatic(uint32 nTypes, const ReflType *types,
                            const char *const *typeNames, uint32 totalSize)
      : classHash(0), nTypes(nTypes), types(types), typeNames(typeNames),
        className("_unnamed_"), classSize(totalSize) {}

  template <class ClassType, class BaseType, class... C, size_t cs>
  reflectorStatic(JenHash baseClassHash, const BaseType *, const ClassType *,
                  const char (&className_)[cs], C... members)
      : classHash(JenHash(className_)), nTypes(sizeof...(C)),
        className(className_), methods(MakeClassMethods<ClassType>()),
        classSize(sizeof(ClassType)),
        baseClass(baseClassHash == JenHash("void") ? JenHash{}
                                                   : baseClassHash) {

    static_assert(std::is_same_v<BaseType, void> ||
                      std::is_base_of_v<BaseType, ClassType>,
                  "Provided base class is not actual base");
    static_assert(!std::is_same_v<ClassType, BaseType>,
                  "Base class cannot be same as class");
    static_assert(std::is_void_v<BaseType> ||
                      _getType<BaseType>::TYPE == REFType::Class,
                  "Base class must be class type and have ReflectorTag();");

    if constexpr (sizeof...(C) > 0) {
      size_t index = 0;
      struct reflType2 : ReflType {
        reflType2(const ReflType &r, size_t index_) : ReflType(r) {
          index = index_;
        }
      };
      static const ReflType types_[]{reflType2{members.type, index++}...};

      bool hasVector = false;
      for (auto &t : types_) {
        if (t.container == REFContainer::ContainerVector ||
            t.container == REFContainer::ContainerVectorMap) {
          hasVector = true;
          break;
        }
      }

      if (hasVector) {
        static const VectorMethods vecMethods[]{members.vectorMethods...};
        vectorMethods = vecMethods;
      }

      types = types_;
      union mutate {
        const char *h;
        uintptr_t i;
      };

      if ((mutate{members.typeName}.i | ...)) {
        static const char *typeNames_[]{members.typeName...};
        typeNames = typeNames_;
      }

      if ((mutate{members.aliasName}.i | ...)) {
        static const char *typeAliases_[]{members.aliasName...};
        typeAliases = typeAliases_;
        static JenHash typeAliasHashes_[]{members.aliasHash...};
        typeAliasHashes = typeAliasHashes_;
      }

      if ((mutate{members.description.part1}.i | ...) |
          (mutate{members.description.part2}.i | ...)) {
        static ReflDesc typeDescs_[]{members.description...};
        typeDescs = typeDescs_;
      }
    }

    if constexpr (constexpr REFType type = _getType<ClassType>::TYPE;
                  type == REFType::Class || type == REFType::BitFieldClass) {
      Registry()[classHash] = this;
    }
  }

  static RegistryType PC_EXTERN &Registry();
};

static_assert(sizeof(reflectorStatic) == 88);

struct ReflectedInstance {
private:
  friend class Reflector;
  friend class ReflectorMember;
  friend struct ReflectedInstanceFriend;

  const reflectorStatic *rfStatic = nullptr;
  union {
    const void *constInstance = nullptr;
    void *instance;
  };

public:
  ReflectedInstance() = default;
  ReflectedInstance(const reflectorStatic *rfStat, const void *inst)
      : rfStatic(rfStat), constInstance(inst) {}
  operator bool() const { return rfStatic != nullptr; }
};

template <class C> const reflectorStatic ES_IMPORT *GetReflectedClass();

namespace es::reflector::detail {
template <class C> class InvokeGuard;
} // namespace es::reflector::detail

#define MEMBERNAME(member, name, ...)                                          \
  MemberProxy {                                                                \
    [] {                                                                       \
      return std::make_tuple(                                                  \
          offsetof(class_type, member), name, JenHash(name),                   \
          std::add_pointer_t<decltype(class_type::member)>{nullptr},           \
          std::false_type{});                                                  \
    },                                                                         \
        __VA_ARGS__                                                            \
  }

#define MEMBER(member, ...) MEMBERNAME(member, #member, __VA_ARGS__)

#define BITMEMBERNAME(member, name, ...)                                       \
  MemberProxy {                                                                \
    [] {                                                                       \
      return std::make_tuple(                                                  \
          std::add_pointer_t<class_type>{nullptr}, name, JenHash(name),        \
          std::add_pointer_t<member>{nullptr}, std::true_type{});              \
    },                                                                         \
        __VA_ARGS__                                                            \
  }

#define BITMEMBER(member, ...) BITMEMBERNAME(member, #member, __VA_ARGS__)

#define CLASS(...) BASEDCLASS(void, __VA_ARGS__)

#define BASEDCLASS(base, ...)                                                  \
  template <> constexpr JenHash ClassHash<__VA_ARGS__>() {                     \
    return #__VA_ARGS__;                                                       \
  }                                                                            \
  template <>                                                                  \
  const reflectorStatic REF_EXPORT_ *GetReflectedClass<__VA_ARGS__>() {        \
    using class_type = __VA_ARGS__;                                            \
    static const reflectorStatic reflectedClass {                              \
      JenHash(#base), std::add_pointer_t<base>{nullptr},                       \
          std::add_pointer_t<class_type>{nullptr}, #__VA_ARGS__,

// internal, do not use
#define END_BASEDCLASS(base, ...) END_CLASS(__VA_ARGS__)
#define END_CLASS(...)                                                         \
  }                                                                            \
  ;                                                                            \
  return &reflectedClass;                                                      \
  }                                                                            \
  template <> class es::reflector::detail::InvokeGuard<__VA_ARGS__> {          \
    static inline const reflectorStatic *data =                                \
        GetReflectedClass<__VA_ARGS__>();                                      \
  };

#define ENUMERATION(...)                                                       \
  template <> constexpr JenHash EnumHash<__VA_ARGS__>() {                      \
    return #__VA_ARGS__;                                                       \
  }                                                                            \
  template <> inline const ReflectedEnum *GetReflectedEnum<__VA_ARGS__>() {    \
    using enum_type = __VA_ARGS__;                                             \
    static const ReflectedEnum reflectedEnum {                                 \
      std::add_pointer_t<enum_type>{nullptr}, #__VA_ARGS__,

#define ENUM_MEMBER(type)                                                      \
  EnumProxy { #type, static_cast<uint64>(enum_type::type) }

#define ENUM_MEMBERDESC(type, desc)                                            \
  EnumProxy { #type, static_cast<uint64>(enum_type::type), desc }

// internal, do not use
#define END_ENUMERATION(...)                                                   \
  }                                                                            \
  ;                                                                            \
  return &reflectedEnum;                                                       \
  }                                                                            \
  template <> class es::reflector::detail::InvokeGuard<__VA_ARGS__> {          \
    static inline const ReflectedEnum *data = GetReflectedEnum<__VA_ARGS__>(); \
  };

// Create reflection definition (use only in TU)
// what: CLASS(), ENUMERATION() or BASEDCLASS()
// args: MEMBER, MEMBERNAME or BITMEMBER, BITMEMBERNAME, invokes MemberProxy
// contructor
#define REFLECT(what, ...) what __VA_ARGS__ END_##what

// Create forward class reflection definition (use only in TU)
#define FWDREFLECTCLASS(...)                                                   \
  template <> constexpr JenHash ClassHash<__VA_ARGS__>() {                     \
    return #__VA_ARGS__;                                                       \
  }

// Create forward enum reflection definition (use only in TU)
#define FWDREFLECTENUMERATION(...)                                             \
  template <> constexpr JenHash EnumHash<__VA_ARGS__>() { return #__VA_ARGS__; }
