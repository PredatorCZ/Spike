
inline ReflectedInstance Reflector::GetReflectedInstance() {
  return const_cast<const Reflector *>(this)->GetReflectedInstance();
}

inline const ReflType *Reflector::GetReflectedType(size_t ID) const {
  const reflectorStatic *inst = GetReflectedInstance().rfStatic;

  if (ID >= inst->nTypes)
    return nullptr;

  return inst->types + ID;
}

inline Reflector::ErrorType
Reflector::SetReflectedValue(JenHash hash, es::string_view value) {
  const ReflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValue(*reflValue, value);
}

inline Reflector::ErrorType
Reflector::SetReflectedValue(size_t id, es::string_view value) {
  const ReflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValue(*reflValue, value);
}

inline Reflector::ErrorType Reflector::SetReflectedValue(JenHash hash,
                                                         es::string_view value,
                                                         size_t subID) {
  const ReflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  switch (reflValue->type) {
  case REFType::Array:
  case REFType::Vector:
  case REFType::ArrayClass:
  case REFType::EnumFlags:
    return SetReflectedValue(*reflValue, value, subID);
  default:
    return ErrorType::InvalidDestination;
  }
}

inline Reflector::ErrorType
Reflector::SetReflectedValue(size_t id, es::string_view value, size_t subID) {
  const ReflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  switch (reflValue->type) {
  case REFType::Array:
  case REFType::Vector:
  case REFType::ArrayClass:
  case REFType::EnumFlags:
    return SetReflectedValue(*reflValue, value, subID);
  default:
    return ErrorType::InvalidDestination;
  }
}

inline Reflector::ErrorType Reflector::SetReflectedValue(JenHash hash,
                                                         es::string_view value,
                                                         size_t subID,
                                                         size_t element) {
  const ReflType *reflValue = GetReflectedType(hash);

  if (!reflValue || !IsArray(hash))
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValue(*reflValue, value, subID, element);
}

inline Reflector::ErrorType Reflector::SetReflectedValue(size_t id,
                                                         es::string_view value,
                                                         size_t subID,
                                                         size_t element) {
  const ReflType *reflValue = GetReflectedType(id);

  if (!reflValue || !IsArray(id))
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValue(*reflValue, value, subID, element);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueInt(JenHash hash, int64 value, size_t subID) {
  const ReflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueInt(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueInt(size_t id, int64 value, size_t subID) {
  const ReflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueInt(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueUInt(JenHash hash, uint64 value, size_t subID) {
  const ReflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueUInt(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueUInt(size_t id, uint64 value, size_t subID) {
  const ReflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueUInt(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueFloat(JenHash hash, double value, size_t subID) {
  const ReflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueFloat(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueFloat(size_t id, double value, size_t subID) {
  const ReflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueFloat(*reflValue, value, subID);
}

inline size_t Reflector::GetNumReflectedValues() const {
  return GetReflectedInstance().rfStatic->nTypes;
}

inline std::string Reflector::GetReflectedValue(JenHash hash) const {
  const ReflType *found = GetReflectedType(hash);

  if (!found)
    return "";

  return GetReflectedValue(found->index);
}

inline std::string Reflector::GetReflectedValue(JenHash hash,
                                                size_t subID) const {
  const ReflType *found = GetReflectedType(hash);

  if (!found)
    return "";

  return GetReflectedValue(found->index, subID);
}

inline std::string Reflector::GetReflectedValue(JenHash hash, size_t subID,
                                                size_t element) const {
  const ReflType *found = GetReflectedType(hash);

  if (!found)
    return "";

  return GetReflectedValue(found->index, subID, element);
}

inline ReflectedInstance Reflector::GetReflectedSubClass(JenHash hash,
                                                         size_t subID) const {
  const ReflType *found = GetReflectedType(hash);

  if (!found)
    return {};

  return GetReflectedSubClass(found->index, subID);
}

inline ReflectedInstance Reflector::GetReflectedSubClass(JenHash hash,
                                                         size_t subID) {
  return const_cast<const Reflector *>(this)->GetReflectedSubClass(hash, subID);
}

inline Reflector::KVPair
Reflector::GetReflectedPair(size_t id, const KVPairFormat &settings) const {
  if (id >= GetNumReflectedValues())
    return {};

  KVPair retval;
  const auto refInt = GetReflectedInstance().rfStatic;

  if (settings.aliasName && refInt->typeAliases && refInt->typeAliases[id]) {
    retval.name = refInt->typeAliases[id];
  } else {
    retval.name = refInt->typeNames[id];
  }

  if (settings.formatValue && refInt->typeDescs[id].part1) {
    retval.value = refInt->typeDescs[id].part1 + (' ' + GetReflectedValue(id));

    if (refInt->typeDescs[id].part2) {
      retval.value += ' ';
      retval.value += refInt->typeDescs[id].part2;
    }
  } else {
    retval.value = GetReflectedValue(id);
  }

  return retval;
}

inline Reflector::KVPair
Reflector::GetReflectedPair(JenHash hash, const KVPairFormat &settings) const {
  const ReflType *found = GetReflectedType(hash);

  if (!found)
    return {};

  return GetReflectedPair(found->index, settings);
}

inline es::string_view Reflector::GetClassName() const {
  return GetReflectedInstance().rfStatic->className;
}

inline bool Reflector::IsReflectedSubClass(JenHash hash) const {
  const ReflType *found = GetReflectedType(hash);

  if (!found)
    return false;

  return IsReflectedSubClass(found->index);
}

inline bool Reflector::IsReflectedSubClass(size_t id) const {
  if (id >= GetNumReflectedValues())
    return false;

  const ReflType fl = GetReflectedInstance().rfStatic->types[id];

  return fl.type == REFType::Class || fl.type == REFType::BitFieldClass ||
         (IsArray(fl.index) && (fl.asArray.type == REFType::Class ||
                             fl.asArray.type == REFType::BitFieldClass));
}

inline bool Reflector::IsArray(JenHash hash) const {
  const ReflType *found = GetReflectedType(hash);

  if (!found)
    return false;

  return IsArray(found->index);
}

inline bool Reflector::IsArray(size_t id) const {
  if (id >= GetNumReflectedValues())
    return false;

  const ReflType fl = GetReflectedInstance().rfStatic->types[id];

  return fl.type == REFType::Array || fl.type == REFType::ArrayClass;
}
