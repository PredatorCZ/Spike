/*  a source for reflector_io

    Copyright 2020-2023 Lukas Cone

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

#include "spike/reflect/reflector_io.hpp"
#include "spike/except.hpp"
#include "spike/type/base_128.hpp"
#include <sstream>

struct ReflectedInstanceFriend : ReflectedInstance {
  void *Instance() { return instance; }
  const void *Instance() const { return constInstance; }
  const reflectorStatic *Refl() const { return rfStatic; }
};

class ReflectorFriend : public Reflector {
public:
  using Reflector::GetReflectedInstance;
};

static int SaveClass(const Reflector &ri, ReflectedInstanceFriend inst,
                     BinWritterRef wr);

static int WriteDataItem(const Reflector &ri, BinWritterRef wr,
                         const char *objAddr, ReflType type, size_t index,
                         const reflectorStatic *refl) {
  switch (type.container) {
  case REFContainer::ContainerVector: {
    const VectorMethods methods = refl->vectorMethods[type.index];
    const uint32 numItems = methods.size(objAddr);
    bint128 tvar(numItems);
    wr.Write(numItems);

    for (uint32 i = 0; i < numItems; i++) {
      if (WriteDataItem(ri, wr,
                        static_cast<const char *>(methods.at(objAddr, i)), type,
                        type.index, refl))
        return 2;
    }
    return 0;
  }
  case REFContainer::InlineArray: {
    for (uint32 i = 0; i < type.asArray.numItems; i++) {
      if (WriteDataItem(ri, wr, objAddr + (type.asArray.stride * i),
                        type.asArray, type.index, refl))
        return 2;
    }
    return 0;
  }
  }
  switch (type.type) {
  case REFType::Integer:
  case REFType::Enum:
    if (type.size > 1) {
      bint128 tvar;
      memcpy(&tvar, objAddr, type.size);
      const size_t shValue = 64 - (type.size * 8);
      tvar.value = (tvar.value << shValue) >> shValue;
      wr.Write(tvar);
      return 0;
    }

    [[fallthrough]];
  case REFType::UnsignedInteger:
  case REFType::EnumFlags:
  case REFType::BitFieldClass:
    if (type.size > 1) {
      buint128 tvar;
      memcpy(&tvar, objAddr, type.size);
      wr.Write(tvar);
      return 0;
    }

    [[fallthrough]];
  case REFType::Bool:
  case REFType::FloatingPoint:
    wr.WriteBuffer(objAddr, type.size);
    return 0;

  case REFType::Vector: {
    for (uint32 i = 0; i < type.asArray.numItems; i++) {
      if (WriteDataItem(ri, wr, objAddr + (type.asArray.stride * i),
                        type.asArray, type.index, refl))
        return 2;
    }
    return 0;
  }

  case REFType::String: {
    const std::string *rStr = reinterpret_cast<const std::string *>(objAddr);
    wr.WriteContainerWCount(*rStr);
    return 0;
  }

  case REFType::Class: {
    auto found =
        reflectorStatic::Registry().find(JenHash(type.asClass.typeHash));

    if (found == reflectorStatic::Registry().end()) {
      return 2;
    }

    ReflectedInstance subInst(found->second, objAddr);
    return SaveClass(ri, ReflectedInstanceFriend{subInst}, wr);
  }
  default:
    return 1;
  }
}

static int SaveClass(const Reflector &ri, ReflectedInstanceFriend inst,
                     BinWritterRef wr) {
  auto refData = inst.Refl();
  const buint128 numItems = refData->nTypes;
  const char *thisAddr = static_cast<const char *>(inst.Instance());
  std::stringstream tmpClassBuffer;

  BinWritterRef wrTmpClass(tmpClassBuffer);
  wrTmpClass.Write(numItems);

  for (size_t i = 0; i < numItems; i++) {
    wrTmpClass.Write(refData->types[i].valueNameHash);

    std::stringstream tmpValueBuffer;
    BinWritterRef wrTmp(tmpValueBuffer);

    int rVal =
        WriteDataItem(ri, wrTmp, thisAddr + refData->types[i].offset,
                      refData->types[i], refData->types[i].index, refData);

    if (rVal)
      return rVal;

    const auto sVarBuffer = tmpValueBuffer.str();
    wrTmpClass.WriteContainerWCount<buint128>(sVarBuffer);
  }

  const auto sClassBuffer = tmpClassBuffer.str();
  wr.WriteContainerWCount<buint128>(sClassBuffer);

  return 0;
}

int ReflectorBinUtil::Save(const Reflector &ri, BinWritterRef wr) {
  auto &&rif = static_cast<const ReflectorFriend &>(ri);
  auto inst =
      static_cast<ReflectedInstanceFriend &&>(rif.GetReflectedInstance());
  auto refData = inst.Refl();

  wr.Write(refData->classHash);

  return SaveClass(ri, inst, wr);
}

static int LoadClass(ReflectorPureWrap inst, BinReaderRef rd);

class ReflectorMemberFriend : public ReflectorMember {
public:
  using ReflectorMember::data;
  using ReflectorMember::id;
  ReflectedInstanceFriend Ref() const { return ReflectedInstanceFriend{data}; }
  operator const ReflType &() const { return Ref().Refl()->types[id]; }
};

static int LoadDataItem(BinReaderRef rd, ReflType type, char *objAddr,
                        const reflectorStatic *refl) {
  switch (type.container) {
  case REFContainer::ContainerVector: {
    const VectorMethods methods = refl->vectorMethods[type.index];
    bint128 tvar;
    rd.Read(tvar);
    uint32 numItems = tvar;

    for (uint32 i = 0; i < numItems; i++) {
      if (LoadDataItem(rd, type, static_cast<char *>(methods.at(objAddr, i)),
                       refl))
        return 2;
    }
    return 0;
  }
  case REFContainer::InlineArray: {
    for (uint32 i = 0; i < type.asArray.numItems; i++) {
      if (LoadDataItem(rd, type.asArray, objAddr + (type.asArray.stride * i),
                       refl))
        return 2;
    }
    return 0;
  }
  }

  switch (type.type) {
  case REFType::Integer:
  case REFType::Enum:
    if (type.size > 1) {
      bint128 tvar;
      rd.Read(tvar);
      memcpy(objAddr, &tvar, type.size);
      return 0;
    }

    [[fallthrough]];
  case REFType::UnsignedInteger:
  case REFType::EnumFlags:
  case REFType::BitFieldClass:
    if (type.size > 1) {
      buint128 tvar;
      rd.Read(tvar);
      memcpy(objAddr, &tvar, type.size);
      return 0;
    }

    [[fallthrough]];
  case REFType::Bool:
  case REFType::FloatingPoint:
    rd.ReadBuffer(objAddr, type.size);
    return 0;

  case REFType::Vector: {
    for (uint32 i = 0; i < type.asArray.numItems; i++) {
      if (LoadDataItem(rd, type.asArray, objAddr + (type.asArray.stride * i),
                       refl))
        return 2;
    }
    return 0;
  }

  case REFType::String: {
    std::string *rStr = reinterpret_cast<std::string *>(objAddr);
    rd.ReadContainer(*rStr);
    return 0;
  }

  case REFType::Class: {
    auto found =
        reflectorStatic::Registry().find(JenHash(type.asClass.typeHash));

    if (found == reflectorStatic::Registry().end()) {
      return 2;
    }

    ReflectedInstance subInst(found->second, objAddr);
    return LoadClass(subInst, rd);
  }
  default:
    return 1;
  }
}

static int LoadClass(ReflectorPureWrap rfWrap, BinReaderRef rd) {
  buint128 chunkSize;
  rd.Read(chunkSize);
  rd.Push();
  buint128 numIOItems;
  rd.Read(numIOItems);

  int errType = 0;

  for (uint32 i = 0; i < numIOItems; i++) {
    BinReaderRef rf(rd);
    JenHash valueNameHash;
    buint128 valueChunkSize;
    rd.Read(valueNameHash);
    rd.Read(valueChunkSize);
    rd.Push();

    ReflectorMemberFriend mem{rfWrap[valueNameHash]};

    if (!mem) {
      rd.Pop();
      rd.Skip(valueChunkSize);
      continue;
    }

    ReflType refl = mem;

    char *objAddr = static_cast<char *>(mem.Ref().Instance()) + refl.offset;

    errType = LoadDataItem(rf, refl, objAddr, mem.Ref().Refl());

    if (errType) {
      rd.Pop();
      rd.Skip(valueChunkSize);
    }
  }

  return errType;
}

int ReflectorBinUtil::Load(Reflector &ri, BinReaderRef rd) {
  auto &&rif = static_cast<ReflectorFriend &>(ri);
  auto inst =
      static_cast<ReflectedInstanceFriend &&>(rif.GetReflectedInstance());
  auto refData = inst.Refl();
  JenHash clsHash;

  rd.Push();
  rd.Read(clsHash);

  if (clsHash != refData->classHash) {
    rd.Pop();
    return 1;
  }

  return LoadClass(inst, rd);
}
