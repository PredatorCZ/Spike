
inline ReflectedInstance Reflector::GetReflectedInstance() {
  return const_cast<const Reflector *>(this)->GetReflectedInstance();
}

inline size_t Reflector::NumReflectedValues() const {
  return GetReflectedInstance().rfStatic->nTypes;
}

inline std::string_view Reflector::ClassName() const {
  return GetReflectedInstance().rfStatic->className;
}

inline ReflectorMember Reflector::operator[](size_t ID) const {
  ReflectedInstance inst = GetReflectedInstance();
  if (!inst) {
    return {inst, 0};
  }

  if (ID >= inst.rfStatic->nTypes) {
    throw std::out_of_range("Reflected member index is out of range");
  }

  return {inst, ID};
}

inline bool ReflectorMember::IsReflectedSubClass() const {
  if (!data) {
    return false;
  }
  const ReflType fl = data.rfStatic->types[id];

  return fl.type == REFType::Class || fl.type == REFType::BitFieldClass ||
         (IsArray() && (fl.asArray.type == REFType::Class ||
                        fl.asArray.type == REFType::BitFieldClass));
}

inline bool ReflectorMember::IsArray() const {
  if (!data) {
    return false;
  }
  const ReflType fl = data.rfStatic->types[id];

  return !(fl.container == REFContainer::None ||
           fl.container == REFContainer::Pointer);
}

inline size_t ReflectorMember::Size() const {
  if (!data) {
    return 0;
  }

  const ReflType fl = data.rfStatic->types[id];

  if (fl.container == REFContainer::InlineArray) {
    return fl.asArray.numItems;
  }

  if (fl.container == REFContainer::ContainerVector ||
      fl.container == REFContainer::ContainerVectorMap) {
    VectorMethods methods(data.rfStatic->vectorMethods[id]);
    return methods.size(
        static_cast<const char *>(data.constInstance) + fl.offset);
  }

  return 0;
}

inline ReflectorMember::KVPair
ReflectorMember::ReflectedPair(KVPairFormat settings) const {
  KVPair retval;
  if (!data) {
    return retval;
  }
  const auto refInt = data.rfStatic;

  if (settings.aliasName && refInt->typeAliases && refInt->typeAliases[id]) {
    retval.name = refInt->typeAliases[id];
  } else {
    retval.name = refInt->typeNames[id];
  }

  if (settings.formatValue && refInt->typeDescs[id].part1) {
    retval.value = refInt->typeDescs[id].part1 + (' ' + ReflectedValue());

    if (refInt->typeDescs[id].part2) {
      retval.value += ' ';
      retval.value += refInt->typeDescs[id].part2;
    }
  } else {
    retval.value = ReflectedValue();
  }

  return retval;
}
