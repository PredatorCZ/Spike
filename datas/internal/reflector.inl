inline const reflType *Reflector::GetReflectedType(es::string_view name) const {
  const JenHashStrong hash(JenkinsHash(name));
  return GetReflectedType(hash);
}

inline const reflType *Reflector::GetReflectedType(size_t ID) const {
  const reflectorStatic *inst = _rfRetreive().rfStatic;

  if (ID >= inst->nTypes)
    return nullptr;

  const reflType *cItem = inst->types + ID;

  if (cItem->offset == 0xffff)
    throw std::runtime_error(
        "Register your class: " +
        (inst->className ? inst->className : std::to_string(inst->classHash)));

  return inst->types + ID;
}

inline Reflector::ErrorType
Reflector::SetReflectedValue(es::string_view name, es::string_view value) {
  const JenHashStrong hash(JenkinsHash(name));
  return SetReflectedValue(hash, value);
}

inline Reflector::ErrorType
Reflector::SetReflectedValue(JenHashStrong hash, es::string_view value) {
  const reflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValue(*reflValue, value);
}

inline Reflector::ErrorType
Reflector::SetReflectedValue(size_t id, es::string_view value) {
  const reflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValue(*reflValue, value);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueInt(es::string_view name, int64 value,
                                size_t subID) {
  const JenHashStrong hash(JenkinsHash(name));
  return SetReflectedValueInt(hash, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueInt(JenHashStrong hash, int64 value, size_t subID) {
  const reflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueInt(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueInt(size_t id, int64 value, size_t subID) {
  const reflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueInt(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueUInt(es::string_view name, uint64 value,
                                 size_t subID) {
  const JenHashStrong hash(JenkinsHash(name));
  return SetReflectedValueUInt(hash, value, subID);
}

inline Reflector::ErrorType Reflector::SetReflectedValueUInt(JenHashStrong hash,
                                                             uint64 value,
                                                             size_t subID) {
  const reflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueUInt(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueUInt(size_t id, uint64 value, size_t subID) {
  const reflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueUInt(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueFloat(es::string_view name, double value,
                                  size_t subID) {
  const JenHashStrong hash(JenkinsHash(name));
  return SetReflectedValueFloat(hash, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueFloat(JenHashStrong hash, double value,
                                  size_t subID) {
  const reflType *reflValue = GetReflectedType(hash);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueFloat(*reflValue, value, subID);
}

inline Reflector::ErrorType
Reflector::SetReflectedValueFloat(size_t id, double value, size_t subID) {
  const reflType *reflValue = GetReflectedType(id);

  if (!reflValue)
    return Reflector::ErrorType::InvalidDestination;

  return SetReflectedValueFloat(*reflValue, value, subID);
}

inline std::string Reflector::GetReflectedValue(es::string_view name) const {
  const JenHashStrong hash(JenkinsHash(name));
  return GetReflectedValue(hash);
}

inline size_t Reflector::GetNumReflectedValues() const {
  return _rfRetreive().rfStatic->nTypes;
}

inline bool Reflector::UseNames() const {
  return _rfRetreive().rfStatic->typeNames != nullptr;
}

inline std::string Reflector::GetReflectedValue(JenHashStrong hash) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return "";

  return GetReflectedValue(found->ID);
}

inline const Reflector::SubClass
Reflector::GetReflectedSubClass(es::string_view name, size_t subID) const {
  const JenHashStrong hash(JenkinsHash(name));
  return GetReflectedSubClass(hash, subID);
}

inline const Reflector::SubClass
Reflector::GetReflectedSubClass(JenHashStrong hash, size_t subID) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return {};

  return GetReflectedSubClass(found->ID, subID);
}

inline const Reflector::SubClass
Reflector::GetReflectedSubClass(es::string_view name, size_t subID) {
  const JenHashStrong hash(JenkinsHash(name));
  return GetReflectedSubClass(hash, subID);
}

inline const Reflector::SubClass
Reflector::GetReflectedSubClass(JenHashStrong hash, size_t subID) {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return {};

  return GetReflectedSubClass(found->ID, subID);
}

inline Reflector::KVPair
Reflector::GetReflectedPair(es::string_view name,
                            const KVPairFormat &settings) const {
  const JenHashStrong hash(JenkinsHash(name));
  return GetReflectedPair(hash, settings);
}

inline Reflector::KVPair
Reflector::GetReflectedPair(size_t id, const KVPairFormat &settings) const {
  if (id >= GetNumReflectedValues())
    return {};

  KVPair retval;
  const auto refInt = _rfRetreive().rfStatic;

  if (UseNames()) {
    if (settings.aliasName && refInt->typeAliases[id]) {
      retval.name = refInt->typeAliases[id];
    } else {
      retval.name = refInt->typeNames[id];
    }
  }

  if (settings.formatValue && !refInt->typeDescs[id].part1.empty()) {
    retval.value =
        refInt->typeDescs[id].part1.to_string() + ' ' + GetReflectedValue(id);

    if (!refInt->typeDescs[id].part2.empty()) {
      retval.value += ' ' + refInt->typeDescs[id].part2.to_string();
    }
  } else {
    retval.value = GetReflectedValue(id);
  }

  return retval;
}

inline Reflector::KVPair
Reflector::GetReflectedPair(JenHashStrong hash,
                            const KVPairFormat &settings) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return {};

  return GetReflectedPair(found->ID, settings);
}

inline es::string_view Reflector::GetClassName() const {
  return _rfRetreive().rfStatic->className;
}

inline bool Reflector::IsReflectedSubClass(es::string_view name) const {
  const JenHashStrong hash(JenkinsHash(name));
  return IsReflectedSubClass(hash);
}

inline bool Reflector::IsReflectedSubClass(JenHashStrong hash) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return false;

  return IsReflectedSubClass(found->ID);
}

inline bool Reflector::IsReflectedSubClass(size_t id) const {
  if (id >= GetNumReflectedValues())
    return false;

  const reflType fl = _rfRetreive().rfStatic->types[id];

  return fl.type == REFType::Class ||
         (IsArray(fl.ID) && fl.subType == REFType::Class);
}

inline bool Reflector::IsArray(es::string_view name) const {
  const JenHashStrong hash(JenkinsHash(name));
  return IsArray(hash);
}

inline bool Reflector::IsArray(JenHashStrong hash) const {
  const reflType *found = GetReflectedType(hash);

  if (!found)
    return false;

  return IsArray(found->ID);
}

inline bool Reflector::IsArray(size_t id) const {
  if (id >= GetNumReflectedValues())
    return false;

  const reflType fl = _rfRetreive().rfStatic->types[id];

  return fl.type == REFType::Array || fl.type == REFType::ArrayClass;
}