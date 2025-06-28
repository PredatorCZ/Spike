/*  a XML I/O source for Reflector class

    Copyright 2019-2024 Lukas Cone

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

#include "spike/reflect/reflector_xml.hpp"
#include "spike/master_printer.hpp"
#include "spike/util/pugiex.hpp"

struct ReflectedInstanceFriend : ReflectedInstance {
  using ReflectedInstance::rfStatic;
  void *Instance() { return instance; }
  const void *Instance() const { return constInstance; }
  const reflectorStatic *Refl() const { return rfStatic; }
};

class ReflectorFriend : public Reflector {
public:
  using Reflector::GetReflectedInstance;
};

auto GetMakeAttribute(pugi::xml_node node, const std::string &name) {
  if (auto child = node.attribute(name.c_str()); child) {
    return child;
  } else {
    return node.append_attribute(name.c_str());
  }
}

auto GetMakeChild(pugi::xml_node node, const std::string &name) {
  if (auto child = node.child(name.c_str()); child) {
    return child;
  } else {
    return node.append_child(name.c_str());
  }
}

class ReflectorMemberFriend : public ReflectorMember {
public:
  using ReflectorMember::data;
  using ReflectorMember::id;
  ReflectedInstanceFriend Ref() const { return ReflectedInstanceFriend{data}; }
  operator const ReflType &() const { return Ref().Refl()->types[id]; }

  std::string Name() const {
    auto stat = Ref().Refl();
    std::string varName;

    if (stat->typeNames && stat->typeNames[id]) {
      varName = stat->typeNames[id];
    } else {
      varName.resize(15);
      const auto cHash = stat->types[id].valueNameHash.raw();
      snprintf(&varName[0], 15, "h:%X", cHash);
    }

    return varName;
  }
};

static bool SaveV2(ReflectorMemberFriend member, pugi::xml_node thisNode,
                   ReflectorXMLUtil::flag_type flags) {
  const ReflType &cType = member;
  std::string varName = member.Name();

  switch (cType.type) {
  case REFType::String:
  case REFType::CString: {
    if (!flags[ReflectorXMLUtil::Flags_StringAsAttribute]) {
      return false;
    }
  }
    [[fallthrough]];
  case REFType::Bool:
  case REFType::Enum:
  case REFType::FloatingPoint:
  case REFType::Integer:
  case REFType::UnsignedInteger:
  case REFType::BitFieldMember: {
    std::string str = member.ReflectedValue();
    auto cNode = GetMakeAttribute(thisNode, varName);
    cNode.set_value(str.data());
    return true;
  }
  case REFType::Vector: {
    static const char axes[4][2]{"x", "y", "z", "w"};
    pugi::xml_node cNode = GetMakeChild(thisNode, varName);

    for (size_t a = 0; a < cType.asVector.numItems; a++) {
      std::string str = member.ReflectedValue(a);
      GetMakeAttribute(cNode, axes[a]).set_value(str.data());
    }

    return true;
  }

  case REFType::EnumFlags: {
    if (!ReflectedEnum::Registry().count(JenHash(cType.asClass.typeHash))) {
      return false;
    }

    auto cEnum = ReflectedEnum::Registry().at(JenHash(cType.asClass.typeHash));
    pugi::xml_node cNode = GetMakeChild(thisNode, varName);

    for (size_t e = 0; e < cEnum->numMembers; e++) {
      auto name = cEnum->names[e];
      const uint64 value = cEnum->values[e];
      auto valueName = member.ReflectedValue(value);
      GetMakeAttribute(cNode, name).set_value(valueName.data());
    }

    return true;
  }
  default:
    return false;
  }
}

static pugi::xml_node MakeNode(const Reflector &, const reflectorStatic *stat,
                               pugi::xml_node node) {
  std::string className;

  if (stat->className)
    className = stat->className;
  else {
    className.resize(15);
    const auto cHash = stat->classHash.raw();
    snprintf(&className[0], 15, "h:%X", cHash);
  }

  return GetMakeChild(node, className.c_str());
}

static std::string GetName(const Reflector &, const reflectorStatic *stat,
                           const ReflType &cType, size_t t) {
  std::string varName;

  if (stat->typeNames && stat->typeNames[t]) {
    varName = stat->typeNames[t];
  } else {
    varName.resize(15);
    const auto cHash = cType.valueNameHash.raw();
    snprintf(&varName[0], 15, "h:%X", cHash);
  }

  return varName;
}

pugi::xml_node ReflectorXMLUtil::Save(const Reflector &ri, pugi::xml_node node,
                                      flag_type opts) {
  const ReflectorFriend &rif = static_cast<const ReflectorFriend &>(ri);
  ReflectedInstanceFriend instance{rif.GetReflectedInstance()};
  ReflectorPureWrap wrap(instance);
  const reflectorStatic *stat = instance.Refl();
  pugi::xml_node thisNode =
      opts[Flags_ClassNode] ? MakeNode(ri, stat, node) : node;

  while (true) {
    for (ReflectorMember mem_ : wrap) {
      ReflectorMemberFriend mem{mem_};
      std::string varName = mem.Name();
      const ReflType &cType = mem;

      if (!mem.IsArray() && ::SaveV2(mem, thisNode, opts)) {
        continue;
      }

      if (mem.IsReflectedSubClass()) {
        if (mem.IsArray()) {
          const size_t numItems = mem.Size();

          for (size_t s = 0; s < numItems; s++) {
            auto subRef = mem.ReflectedSubClass(s);
            auto nodeName = varName + '-' + std::to_string(s);
            pugi::xml_node cNode = GetMakeChild(thisNode, nodeName);
            auto subOpts = opts;
            subOpts -= Flags_ClassNode;
            Save(subRef, cNode, subOpts);
          }
        } else {
          auto subRef = mem.ReflectedSubClass();
          pugi::xml_node cNode = GetMakeChild(thisNode, varName);
          auto subOpts = opts;
          subOpts -= Flags_ClassNode;
          Save(subRef, cNode, subOpts);
        }
      } else if (mem.IsArray()) {
        const size_t numItems = mem.Size();
        ReflType arr = cType.asArray;
        REFType type = cType.container == REFContainer::InlineArray
                           ? arr.type
                           : cType.type;
        switch (type) {
        case REFType::String: {
          if (!opts[ReflectorXMLUtil::Flags_StringAsAttribute]) {
            for (size_t s = 0; s < numItems; s++) {
              std::string str = mem.ReflectedValue(s);
              auto nodeName = varName + '-' + std::to_string(s);
              auto cNode = GetMakeChild(thisNode, nodeName.data());
              cNode.append_buffer(str.data(), str.size());
            }
            break;
          }
        }
          [[fallthrough]];
        case REFType::Bool:
        case REFType::Enum:
        case REFType::FloatingPoint:
        case REFType::Integer:
        case REFType::UnsignedInteger:
        case REFType::BitFieldMember: {
          for (size_t s = 0; s < numItems; s++) {
            std::string str = mem.ReflectedValue(s);
            auto nodeName = varName + '-' + std::to_string(s);
            auto cNode = GetMakeAttribute(thisNode, nodeName.data());
            cNode.set_value(str.data());
          }
          break;
        }
        case REFType::Vector: {
          for (size_t s = 0; s < numItems; s++) {
            static const char axes[4][2]{"x", "y", "z", "w"};
            auto nodeName = varName + '-' + std::to_string(s);
            pugi::xml_node cNode = GetMakeChild(thisNode, nodeName.c_str());
            ReflectorPureWrap rfWrap(mem.ReflectedSubClass(s));

            for (size_t a = 0; a < arr.asVector.numItems; a++) {
              std::string str = rfWrap[a].ReflectedValue();
              GetMakeAttribute(cNode, axes[a]).set_value(str.data());
            }
          }
          break;
        }
        case REFType::EnumFlags: {
          if (!ReflectedEnum::Registry().count(JenHash(arr.asClass.typeHash))) {
            break;
          }

          auto &&cEnum =
              ReflectedEnum::Registry().at(JenHash(arr.asClass.typeHash));

          for (size_t s = 0; s < numItems; s++) {
            auto nodeName = varName + '-' + std::to_string(s);
            pugi::xml_node cNode = GetMakeChild(thisNode, nodeName.c_str());
            ReflectorPureWrap rfWrap(mem.ReflectedSubClass(s));

            for (auto m_ : rfWrap) {
              ReflectorMemberFriend m(m_);
              std::string mValue(m);
              GetMakeAttribute(cNode, m.Name()).set_value(mValue.c_str());
            }
          }
          break;
        }
        default: {
          pugi::xml_node cNode = GetMakeChild(thisNode, varName.c_str());
          std::string str = mem.ReflectedValue();
          cNode.append_buffer(str.c_str(), str.size());
          break;
        }
        }
      } else {
        pugi::xml_node cNode = GetMakeChild(thisNode, varName);
        std::string str = mem.ReflectedValue();
        cNode.append_buffer(str.c_str(), str.size());
      }
    }

    if (stat->baseClass.raw() == 0) {
      break;
    }

    stat = reflectorStatic::Registry().at(stat->baseClass);
    instance.rfStatic = stat;
    wrap = instance;
  };

  return thisNode;
}

pugi::xml_node ReflectorXMLUtil::Load(Reflector &ri, pugi::xml_node node,
                                      bool lookupClassNode) {
  auto &&rif = static_cast<ReflectorFriend &>(ri);
  const reflectorStatic *stat =
      static_cast<ReflectedInstanceFriend &&>(rif.GetReflectedInstance())
          .Refl();
  pugi::xml_node thisNode;
  static constexpr size_t nan_ = -1;
  struct retval {
    JenHash hash;
    size_t index = nan_;
  };

  auto MakeHash = [](std::string_view name) -> JenHash {
    if (name[0] == 'h' && name[1] == ':') {
      name.remove_prefix(2);
      return JenHash(strtoul(name.data(), nullptr, 16));
    } else {
      return name;
    }
  };

  auto MakeNode = [MakeHash](auto a) {
    std::string_view name(a.name());
    retval retVal;
    const size_t found = name.find_last_of('-');

    if (found != name.npos) {
      char *endChar = nullptr;
      const char *startChar = name.data() + found + 1;
      auto index = strtoll(startChar, &endChar, 10);

      if (startChar != endChar) {
        name = name.substr(0, found);
        retVal.index = index;
      }
    }

    retVal.hash = MakeHash(name);

    return retVal;
  };

  if (lookupClassNode) {
    thisNode = node.find_child([&](pugi::xml_node nde) {
      return !nde.empty() && MakeHash(nde.name()) == stat->classHash;
    });
  } else {
    thisNode = node;
  }

  for (auto a : thisNode.attributes()) {
    auto node = MakeNode(a);
    ReflectorMember mem(ri[node.hash]);

    if (node.index == nan_) {
      mem.ReflectValue(a.value());
    } else {
      mem.ReflectValue(a.value(), node.index);
    }
  }

  for (auto a : thisNode.children()) {
    auto node = MakeNode(a);
    ReflectorMember mem(ri[node.hash]);

    if (mem.IsReflectedSubClass()) {
      if (node.index == nan_) {
        node.index = 0;
      }

      auto rfInst = mem.ReflectedSubClass(node.index);
      ReflectorXMLUtil::Load(rfInst, a);
      continue;
    }

    if (a.attributes_begin() == a.attributes_end()) {
      mem.ReflectValue(a.text().as_string());
      continue;
    }

    ReflectorMemberFriend memFriend{mem};
    const ReflType &refType = memFriend;

    auto DoFlags = [&a](ReflectorPureWrap wrp) {
      for (auto t : a.attributes()) {
        wrp[std::string_view(t.name())] = t.as_string();
      }
    };

    switch (refType.type) {
    case REFType::EnumFlags:
    case REFType::Vector: {
      DoFlags(mem.ReflectedSubClass(node.index == nan_ ? 0 : node.index));
      break;
    }

    default:
      break;
    }
  }

  return thisNode;
}
