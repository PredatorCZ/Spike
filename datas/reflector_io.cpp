/*  a source for reflector_io

    Copyright 2020-2021 Lukas Cone

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

#include "reflector_io.hpp"
#include "base_128.hpp"
#include "except.hpp"
#include <sstream>

struct Fixups {
  struct Fixup {
    uint32 ptr, destination;
  };

  std::vector<Fixup> fixups;
  uint32 fixupIter = 0;
  BinWritterRef ref;

  Fixups(BinWritterRef _ref) : ref(_ref){};

  void AddPointer() { fixups.push_back({static_cast<uint32>(ref.Tell()), 0}); }
  void FixupDestination() {
    fixups[fixupIter++].destination = static_cast<uint32>(ref.Tell());
  }
  void AppendDestination(uint32 dest) {
    fixups[fixupIter++].destination = dest;
  }
  void AddDestination() {
    fixups.push_back({0, static_cast<uint32>(ref.Tell())});
  }
  void FixupPointer() {
    fixups[fixupIter++].ptr = static_cast<uint32>(ref.Tell());
  }
  void WriteFixups() {
    for (auto &f : fixups) {
      ref.Seek(f.ptr);
      ref.Write(f.destination);
    }
  }
};

struct ReflectorIOHeader {
  static constexpr uint32 ID = CompileFourCC("RFDB");
  static constexpr uint32 VERSION = 2000;

  uint32 id, version, numClasses, numEnums, enumsOffset, bufferSize;

  ReflectorIOHeader() : id(ID), version(VERSION) {}
};

struct reflectorStatic_io {
  uint32 classHash;
  uint32 nTypes;
  uintptr_t types;
  uintptr_t typeNames;
  uintptr_t className;
  uintptr_t typeAliases;
  uintptr_t typeAliasHashes;
  uintptr_t typeDescs;

  void Fixup(uintptr_t base) {
    auto fixup = [base](uintptr_t &item) {
      if (item) {
        item += base;
      }
    };

    auto fixupmore = [base, fixup](auto &...items) { (fixup(items), ...); };

    fixupmore(types, className, typeAliasHashes, typeDescs, typeNames,
              typeAliases);

    if (typeAliases) {
      auto castedBase = reinterpret_cast<uintptr_t *>(typeAliases);
      auto casted = &*castedBase;
      for (uint32 i = 0; i < nTypes; i++) {
        if (casted[i]) {
          casted[i] += base;
        }
      }
    }

    if (typeNames) {
      auto castedBase = reinterpret_cast<uintptr_t *>(typeNames);
      auto casted = &*castedBase;
      for (uint32 i = 0; i < nTypes; i++) {
        if (casted[i]) {
          casted[i] += base;
        }
      }
    }

    if (typeDescs) {
      struct Desc {
        uintptr_t data1;
        uintptr_t data2;
      };

      auto casted = reinterpret_cast<Desc *>(typeDescs);

      for (uint32 t = 0; t < nTypes; t++) {
        auto &item = casted[t];
        fixupmore(item.data1, item.data2);
      }
    }
  }
};

static_assert(sizeof(reflectorStatic_io) == sizeof(reflectorStatic));

struct ReflectedEnum_io {
  uint32 enumHash;
  uint32 numMembers;
  uintptr_t enumName;
  uintptr_t names;
  uintptr_t values;

  void Fixup(uintptr_t base) {
    auto fixup = [base](uintptr_t &item) {
      if (item) {
        item += base;
      }
    };

    auto fixupmore = [base, fixup](auto &...items) { (fixup(items), ...); };

    fixupmore(enumName, names, values);

    if (names) {
      auto castedBase = reinterpret_cast<uintptr_t *>(names);
      auto casted = &*castedBase;
      for (uint32 i = 0; i < numMembers; i++) {
        if (casted[i]) {
          casted[i] += base;
        }
      }
    }
  }
};

static_assert(sizeof(ReflectedEnum_io) == sizeof(ReflectedEnum));

int ReflectorIO::Load(BinReaderRef rd) {
  ReflectorIOHeader hdr;

  rd.Read(hdr);

  if (hdr.id != hdr.ID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  if (hdr.version != hdr.VERSION) {
    throw es::InvalidVersionError(hdr.version);
  }

  rd.ReadContainer(data, hdr.bufferSize);

  reflectorStatic_io *classesStart =
      reinterpret_cast<reflectorStatic_io *>(&data[0]);
  const uintptr_t bufferStart = reinterpret_cast<uintptr_t>(classesStart);

  for (uint32 c = 0; c < hdr.numClasses; c++) {
    classesStart[c].Fixup(bufferStart);

    classes.push_back(
        reinterpret_cast<const reflectorStatic *>(classesStart + c));
  }

  ReflectedEnum_io *enumsIter =
      reinterpret_cast<ReflectedEnum_io *>(&data[0] + hdr.enumsOffset);

  for (uint32 c = 0; c < hdr.numEnums; c++) {
    enumsIter[c].Fixup(bufferStart);
    enums.push_back(reinterpret_cast<const ReflectedEnum *>(enumsIter + c));
  }

  return 0;
}

int ReflectorIO::Save(BinWritterRef wr) {
  ReflectorIOHeader hdr;
  hdr.numClasses = static_cast<uint32>(classes.size());
  hdr.numEnums = static_cast<uint32>(enums.size());
  wr.Write(hdr);
  wr.SetRelativeOrigin(wr.Tell(), false);

  Fixups itemFixups(wr);
  Fixups itemStringsFixups(wr);
  Fixups classnamesFixups(wr);
  Fixups aliasesFixups(wr);
  Fixups descsFixups(wr);
  Fixups aliasHashesFixups(wr);

  for (auto i : classes) {
    wr.Write(i->classHash);
    wr.Write(i->nTypes);
    itemFixups.AddPointer();
    wr.Skip<uint64>();

    if (i->typeNames) {
      itemStringsFixups.AddPointer();
    }

    wr.Skip<uint64>();

    if (i->className) {
      classnamesFixups.AddPointer();
    }

    wr.Skip<uint64>();

    if (i->typeAliases) {
      aliasesFixups.AddPointer();
    }

    wr.Skip<uint64>();

    if (i->typeAliasHashes) {
      aliasHashesFixups.AddPointer();
    }

    wr.Skip<uint64>();

    if (i->typeDescs) {
      descsFixups.AddPointer();
    }

    wr.Skip<uint64>();
  }

  for (auto i : classes) {
    itemFixups.FixupDestination();

    for (uint32 r = 0; r < i->nTypes; r++) {
      wr.Write(i->types[r]);
    }
  }

  for (auto i : classes) {
    if (i->typeAliasHashes) {
      aliasHashesFixups.FixupDestination();

      for (uint32 r = 0; r < i->nTypes; r++) {
        wr.Write(i->typeAliasHashes[r]);
      }
    }
  }

  for (auto i : classes) {
    if (i->typeNames) {
      wr.ApplyPadding(8);
      itemStringsFixups.FixupDestination();

      for (uint32 r = 0; r < i->nTypes; r++) {
        if (i->typeNames[r]) {
          itemStringsFixups.AddPointer();
        }
        wr.Skip<uint64>();
      }
    }
  }

  for (auto i : classes) {
    if (i->typeAliases) {
      wr.ApplyPadding(8);
      aliasesFixups.FixupDestination();

      for (uint32 r = 0; r < i->nTypes; r++) {
        if (i->typeAliases[r]) {
          aliasesFixups.AddPointer();
        }
        wr.Skip<uint64>();
      }
    }
  }

  for (auto i : classes) {
    if (i->typeDescs) {
      descsFixups.FixupDestination();

      for (uint32 r = 0; r < i->nTypes; r++) {
        if (i->typeDescs[r].part1) {
          descsFixups.AddPointer();
        }

        wr.Skip<uint64>();

        if (i->typeDescs[r].part2) {
          descsFixups.AddPointer();
        }

        wr.Skip<uint64>();
      }
    }
  }

  for (auto i : classes) {
    if (i->typeNames) {
      for (uint32 r = 0; r < i->nTypes; r++) {
        if (i->typeNames[r]) {
          itemStringsFixups.FixupDestination();
          wr.WriteT(i->typeNames[r]);
        }
      }
    }

    if (i->typeAliases) {
      for (uint32 r = 0; r < i->nTypes; r++) {
        if (!i->typeAliases[r]) {
          continue;
        }

        aliasesFixups.FixupDestination();
        wr.WriteT(i->typeAliases[r]);
      }
    }

    if (i->typeDescs) {
      for (uint32 r = 0; r < i->nTypes; r++) {
        if (i->typeDescs[r].part1) {
          descsFixups.FixupDestination();
          wr.WriteT(i->typeDescs[r].part1);
        }

        if (i->typeDescs[r].part2) {
          descsFixups.FixupDestination();
          wr.WriteT(i->typeDescs[r].part2);
        }
      }
    }

    if (i->className) {
      classnamesFixups.FixupDestination();
      wr.WriteT(i->className);
    }
  }

  wr.ApplyPadding(8);
  hdr.enumsOffset = static_cast<uint32>(wr.Tell());

  for (auto e : enums) {
    wr.Write(e->enumHash);
    wr.Write(e->numMembers);
    classnamesFixups.AddPointer();
    wr.Skip<uint64>();
    itemStringsFixups.AddPointer();
    wr.Skip<uint64>();
    itemFixups.AddPointer();
    wr.Skip<uint64>();
  }

  for (auto e : enums) {
    itemFixups.FixupDestination();
    for (size_t v = 0; v < e->numMembers; v++) {
      wr.Write(e->values[v]);
    }

    itemStringsFixups.FixupDestination();
    for (size_t s = 0; s < e->numMembers; s++) {
      itemStringsFixups.AddPointer();
      wr.Skip<uint64>();
    }
  }

  for (auto e : enums) {
    classnamesFixups.FixupDestination();
    wr.WriteT(e->enumName);

    for (size_t v = 0; v < e->numMembers; v++) {
      itemStringsFixups.FixupDestination();
      wr.WriteT(e->names[v]);
    }
  }

  hdr.bufferSize = static_cast<uint32>(wr.Tell());
  wr.Push();
  wr.ResetRelativeOrigin();
  wr.Write(hdr);
  itemFixups.WriteFixups();
  itemStringsFixups.WriteFixups();
  classnamesFixups.WriteFixups();
  aliasesFixups.WriteFixups();
  descsFixups.WriteFixups();
  wr.Pop();

  return 0;
}

struct ReflectedInstanceFriend : ReflectedInstance {
  void *Instance() { return instance; }
  const void *Instance() const { return constInstance; }
  const reflectorStatic *Refl() const { return rfStatic; }
};

class ReflectorFriend : public Reflector {
public:
  using Reflector::GetReflectedInstance;
  using Reflector::GetReflectedType;
};

static int SaveClass(const Reflector &ri, ReflectedInstanceFriend inst,
                     BinWritterRef wr);

static int WriteDataItem(const Reflector &ri, BinWritterRef wr,
                         const char *objAddr, reflType type) {
  switch (type.type) {

  case REFType::Integer:
  case REFType::Enum:
    if (type.subSize > 1) {
      bint128 tvar;
      memcpy(&tvar, objAddr, type.subSize);
      const size_t shValue = 64 - (type.subSize * 8);
      tvar.value = (tvar.value << shValue) >> shValue;
      wr.Write(tvar);
      return 0;
    }

  case REFType::UnsignedInteger:
  case REFType::EnumFlags:
  case REFType::BitFieldClass:
    if (type.subSize > 1) {
      buint128 tvar;
      memcpy(&tvar, objAddr, type.subSize);
      wr.Write(tvar);
      return 0;
    }

  case REFType::Bool:
  case REFType::FloatingPoint:
    wr.WriteBuffer(objAddr, type.subSize);
    return 0;

  case REFType::Array:
  case REFType::ArrayClass:
  case REFType::Vector: {
    reflType subType = type;
    subType.type = type.subType;
    subType.subType = type.type;

    if (subType.type == REFType::Vector) {
      _DecomposedVectorHash dec = {subType.typeHash};
      subType.subType = dec.type;
      subType.numItems = dec.numItems;
      subType.subSize = dec.size;
    }

    for (uint32 i = 0; i < type.numItems; i++) {
      if (subType.type != REFType::Vector)
        subType.numItems = static_cast<uint16>(i);

      if (WriteDataItem(ri, wr, objAddr + (type.subSize * i), subType))
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
    auto sri = static_cast<ReflectedInstanceFriend &&>(
        ri.GetReflectedSubClass(type.ID, type.numItems));
    return SaveClass(ri, sri, wr);
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

    int rVal = WriteDataItem(ri, wrTmp, thisAddr + refData->types[i].offset,
                             refData->types[i]);

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

static int LoadClass(ReflectedInstanceFriend inst, BinReaderRef rd);

static int LoadDataItem(Reflector &ri, BinReaderRef rd, char *objAddr,
                        reflType type) {
  switch (type.type) {
  case REFType::Integer:
  case REFType::Enum:
    if (type.subSize > 1) {
      bint128 tvar;
      rd.Read(tvar);
      memcpy(objAddr, &tvar, type.subSize);
      return 0;
    }

  case REFType::UnsignedInteger:
  case REFType::EnumFlags:
  case REFType::BitFieldClass:
    if (type.subSize > 1) {
      buint128 tvar;
      rd.Read(tvar);
      memcpy(objAddr, &tvar, type.subSize);
      return 0;
    }

  case REFType::Bool:
  case REFType::FloatingPoint:
    rd.ReadBuffer(objAddr, type.subSize);
    return 0;

  case REFType::Array:
  case REFType::ArrayClass:
  case REFType::Vector: {
    reflType subType = type;
    subType.type = type.subType;
    subType.subType = type.type;

    if (subType.type == REFType::Vector) {
      _DecomposedVectorHash dec = {subType.typeHash};
      subType.subType = dec.type;
      subType.numItems = dec.numItems;
      subType.subSize = dec.size;
    }

    for (uint32 i = 0; i < type.numItems; i++) {
      if (subType.type != REFType::Vector)
        subType.numItems = static_cast<uint16>(i);

      if (LoadDataItem(ri, rd, objAddr + (type.subSize * i), subType))
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
    auto sri = static_cast<ReflectedInstanceFriend &&>(
        ri.GetReflectedSubClass(type.ID, type.numItems));
    return LoadClass(sri, rd);
  }
  default:
    return 1;
  }
}

static int LoadClass(ReflectedInstanceFriend inst, BinReaderRef rd) {
  auto refData = inst.Refl();
  const uint32 numItems = refData->nTypes;
  char *thisAddr = static_cast<char *>(inst.Instance());
  buint128 chunkSize;
  rd.Read(chunkSize);
  rd.Push();

  buint128 numIOItems;
  rd.Read(numIOItems);

  if (numIOItems != numItems) {
    rd.Pop();
    rd.Skip(chunkSize);
    return 1;
  }

  int errType = 0;

  for (uint32 i = 0; i < numItems; i++) {
    BinReaderRef rf(rd);
    JenHash valueNameHash;
    buint128 valueChunkSize;
    rd.Read(valueNameHash);
    rd.Read(valueChunkSize);
    rd.Push();

    ReflectorPureWrap ri(inst);
    auto &&rif = static_cast<ReflectorFriend &>(static_cast<Reflector &>(ri));
    const reflType *cType = rif.GetReflectedType(valueNameHash);

    if (!cType) {
      rd.Pop();
      rd.Skip(valueChunkSize);
      continue;
    }

    errType = LoadDataItem(ri, rf, thisAddr + cType->offset, *cType);

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
