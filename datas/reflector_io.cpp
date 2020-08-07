/*  a source for reflector_io

    Copyright 2020 Lukas Cone

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

static constexpr uint32 PAD_CHECK[6] = {0x78563412, 0xEFCDAB90, 0XFECAADDE,
                                        0XADDEBEBA, 0XBEBAFECA, 0xADDEEFBE};
struct reflectorStatic_io {
  JenHash hash;
  uint32 numItems, _x64Padding[6], typesPtr, memberNamesPtr, classNamePtr,
      aliassesPtr, aliassesHashesPtr, descsPtr;

  bool IsValid() const {
    return _x64Padding[0] == PAD_CHECK[0] && _x64Padding[1] == PAD_CHECK[1] &&
           _x64Padding[2] == PAD_CHECK[2] && _x64Padding[3] == PAD_CHECK[3] &&
           _x64Padding[4] == PAD_CHECK[4] && _x64Padding[5] == PAD_CHECK[5];
  }
};

ES_STATIC_ASSERT(__sizeof_reflectorStatic == sizeof(reflectorStatic_io));

class ReflectedEnum_io : public ReflectedEnum {
public:
  using parent::resize;
};

struct ReflectorIOHeader {
  static constexpr uint32 ID = CompileFourCC("RFDB");
  static constexpr uint32 VERSION = 1000;

  uint32 id, version, numClasses, numEnums, numEnumStrings, enumsOffset,
      bufferSize, reserved;

  ReflectorIOHeader() : id(ID), version(VERSION), reserved() {}
};

int ReflectorIO::Load(BinReaderRef rd) {
  ReflectorIOHeader hdr;

  rd.Read(hdr);

  if (hdr.id != hdr.ID)
    return 1;

  if (hdr.version > hdr.VERSION)
    return 2;

  rd.ReadContainer(data, hdr.bufferSize);

  const reflectorStatic *classesStart =
      reinterpret_cast<const reflectorStatic *>(data.data());

  for (uint64 c = 0; c < hdr.numClasses; c++)
    classes.push_back(classesStart + c);

  const esIntPtr bufferStart = reinterpret_cast<esIntPtr>(classesStart);

  for (auto i : classes) {
    reflectorStatic_io *itemIO = reinterpret_cast<reflectorStatic_io *>(
        const_cast<reflectorStatic *>(i));
    esIntPtr *itemPtrsBegin = reinterpret_cast<esIntPtr *>(itemIO->_x64Padding);

    itemPtrsBegin[0] = bufferStart + itemIO->typesPtr;

    if (itemIO->memberNamesPtr) {
      itemPtrsBegin[1] = bufferStart + itemIO->memberNamesPtr;

      esIntPtr *itemNamePtrs = reinterpret_cast<esIntPtr *>(itemPtrsBegin[1]);
      uint32 *itemNameOffsets =
          reinterpret_cast<uint32 *>(itemNamePtrs) + itemIO->numItems;

      for (uint32 t = 0; t < itemIO->numItems; t++) {
        itemNamePtrs[t] = bufferStart + itemNameOffsets[t];
      }
    } else {
      itemPtrsBegin[1] = 0;
    }

    if (itemIO->classNamePtr) {
      itemPtrsBegin[2] = bufferStart + itemIO->classNamePtr;
    } else {
      itemPtrsBegin[2] = 0;
    }

    if (itemIO->aliassesPtr) {
      itemPtrsBegin[3] = bufferStart + itemIO->aliassesPtr;

      esIntPtr *aliasesPtrs = reinterpret_cast<esIntPtr *>(itemPtrsBegin[3]);
      uint32 *aliasesOffsets =
          reinterpret_cast<uint32 *>(aliasesPtrs) + itemIO->numItems;

      for (uint32 t = 0; t < itemIO->numItems; t++) {
        if (aliasesOffsets[t])
          aliasesPtrs[t] = bufferStart + aliasesOffsets[t];
        else
          aliasesPtrs[t] = 0;
      }
    } else {
      itemPtrsBegin[3] = 0;
    }

    if (itemIO->aliassesHashesPtr) {
      itemPtrsBegin[3] = bufferStart + itemIO->aliassesHashesPtr;
    } else {
      itemPtrsBegin[4] = 0;
    }

    if (itemIO->descsPtr) {
      itemPtrsBegin[5] = bufferStart + itemIO->descsPtr;

      esIntPtr *descsPtrs = reinterpret_cast<esIntPtr *>(itemPtrsBegin[5]);
      uint32 *descsOffsets =
          reinterpret_cast<uint32 *>(descsPtrs) + itemIO->numItems * 4;

      for (uint32 t = 0; t < itemIO->numItems * 2; t++) {
        if (*descsOffsets) {
          *descsPtrs++ = bufferStart + *descsOffsets++;
        } else {
          *descsPtrs++ = 0;
          *descsOffsets++ = 0;
        }

        *descsPtrs++ = *descsOffsets++;
      }
    } else {
      itemPtrsBegin[5] = 0;
    }
  }

  uint64 *enumsIter = reinterpret_cast<uint64 *>(&data[0] + hdr.enumsOffset);

  for (uint64 c = 0; c < hdr.numEnums; c++) {
    enums.emplace_back();
    ReflectedEnum_io &lEn =
        static_cast<ReflectedEnum_io &>(*std::prev(enums.end()));

    lEn.hash = *reinterpret_cast<const JenHash *>(enumsIter);
    lEn.resize(*(reinterpret_cast<const uint32 *>(enumsIter++) + 1));
    lEn.values = enumsIter;
    enumsIter += lEn.size();
  }

  esIntPtr *enumNamesPtrs = reinterpret_cast<esIntPtr *>(enumsIter);
  const uint32 *enumNamesOffsets =
      reinterpret_cast<const uint32 *>(enumsIter) + hdr.numEnumStrings;
  const char **enumNamesIter = reinterpret_cast<const char **>(enumsIter);

  for (size_t s = 0; s < hdr.numEnumStrings; s++) {
    enumNamesPtrs[s] = bufferStart + enumNamesOffsets[s];
  }

  for (auto &e : enums) {
    for (auto &n : e) {
      n = {*enumNamesIter};
      enumNamesIter++;
    }

    e.name = *enumNamesIter++;
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
    wr.Write(PAD_CHECK);
    itemFixups.AddPointer();
    wr.Skip<uint32>();

    if (i->typeNames)
      itemStringsFixups.AddPointer();

    wr.Skip<uint32>();

    if (i->className)
      classnamesFixups.AddPointer();

    wr.Skip<uint32>();

    if (i->typeAliases)
      aliasesFixups.AddPointer();

    wr.Skip<uint32>();

    if (i->typeAliasHashes)
      aliasHashesFixups.AddPointer();

    wr.Skip<uint32>();

    if (i->typeDescs)
      descsFixups.AddPointer();

    wr.Skip<uint32>();
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
      itemStringsFixups.FixupDestination();
      wr.Skip(sizeof(uint32) * i->nTypes);

      for (uint32 r = 0; r < i->nTypes; r++) {
        itemStringsFixups.AddPointer();
        wr.Skip<uint32>();
      }
    }
  }

  for (auto i : classes) {
    if (i->typeAliases) {
      aliasesFixups.FixupDestination();
      wr.Skip(sizeof(uint32) * i->nTypes);

      for (uint32 r = 0; r < i->nTypes; r++) {
        if (i->typeAliases[r])
          aliasesFixups.AddPointer();
        wr.Skip<uint32>();
      }
    }
  }

  for (auto i : classes) {
    if (i->typeDescs) {
      descsFixups.FixupDestination();
      wr.Skip(sizeof(uint64) * i->nTypes * 2);

      for (uint32 r = 0; r < i->nTypes; r++) {
        if (!i->typeDescs[r].part1.empty())
          descsFixups.AddPointer();

        wr.Skip<uint32>();
        wr.Write(static_cast<uint32>(i->typeDescs[r].part1.size()));

        if (!i->typeDescs[r].part2.empty())
          descsFixups.AddPointer();

        wr.Skip<uint32>();
        wr.Write(static_cast<uint32>(i->typeDescs[r].part2.size()));
      }
    }
  }

  for (auto i : classes) {
    if (i->typeNames) {
      for (uint32 r = 0; r < i->nTypes; r++) {
        itemStringsFixups.FixupDestination();
        wr.WriteT(i->typeNames[r]);
      }
    }

    if (i->typeAliases) {
      for (uint32 r = 0; r < i->nTypes; r++) {
        if (!i->typeAliases[r])
          continue;

        aliasesFixups.FixupDestination();
        wr.WriteT(i->typeAliases[r]);
      }
    }

    if (i->typeDescs) {
      for (uint32 r = 0; r < i->nTypes; r++) {
        if (!i->typeDescs[r].part1.empty()) {
          descsFixups.FixupDestination();
          wr.WriteContainer(i->typeDescs[r].part1);
          wr.Skip(1);
        }

        if (!i->typeDescs[r].part2.empty()) {
          descsFixups.FixupDestination();
          wr.WriteContainer(i->typeDescs[r].part2);
          wr.Skip(1);
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

  size_t totalStrings = 0;

  for (auto &e : enums) {
    wr.Write(e.hash);
    wr.Write(static_cast<uint32>(e.size()));
    totalStrings += e.size() + 1;

    for (size_t v = 0; v < e.size(); v++) {
      wr.Write(e.values[v]);
    }
  }

  hdr.numEnumStrings = static_cast<uint32>(totalStrings);
  wr.Skip(sizeof(uint32) * totalStrings);

  for (size_t s = 0; s < totalStrings; s++) {
    itemStringsFixups.AddPointer();
    wr.Skip<uint32>();
  }

  for (auto &e : enums) {
    for (auto &n : e) {
      itemStringsFixups.FixupDestination();
      wr.WriteContainer(n);
      wr.Skip(1);
    }

    itemStringsFixups.FixupDestination();
    wr.WriteContainer(e.name);
    wr.Skip(1);
  }

  hdr.bufferSize = static_cast<uint32>(wr.Tell());
  wr.ResetRelativeOrigin();
  wr.Write(hdr);
  itemFixups.WriteFixups();
  itemStringsFixups.WriteFixups();
  classnamesFixups.WriteFixups();
  aliasesFixups.WriteFixups();
  descsFixups.WriteFixups();
  wr.Seek(0, std::ios_base::end);

  return 0;
}

static int SaveClass(const Reflector &ri, const reflectorInstanceConst &inst,
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
    ReflectorSubClass sri(ri.GetReflectedSubClass(type.ID, type.numItems));
    return SaveClass(sri, sri.data.instc, wr);
  }
  default:
    return 1;
  }
}

static int SaveClass(const Reflector &ri, const reflectorInstanceConst &inst,
                     BinWritterRef wr) {
  auto refData = inst.rfStatic;
  const buint128 numItems = refData->nTypes;
  const char *thisAddr = static_cast<const char *>(inst.rfInstance);
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
  auto inst = ri.GetReflectedInstance();
  auto refData = inst.rfStatic;

  wr.Write(refData->classHash);

  return SaveClass(ri, inst, wr);
}

static int LoadClass(Reflector &ri, reflectorInstance &inst, BinReaderRef rd);

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
    ReflectorSubClass sri(ri.GetReflectedSubClass(type.ID, type.numItems));
    return LoadClass(sri, sri.data.inst, rd);
  }
  default:
    return 1;
  }
}

class ReflectorBinUtilFriend : ReflectorBinUtil {
public:
  using ReflectorBinUtil::Find;
};

const reflType *ReflectorBinUtil::Find(Reflector &ri, JenHash hash) {
  return ri.GetReflectedType(JenHashStrong(hash));
}

static int LoadClass(Reflector &ri, reflectorInstance &inst, BinReaderRef rd) {
  auto refData = inst.rfStatic;
  const uint32 numItems = refData->nTypes;
  char *thisAddr = static_cast<char *>(inst.rfInstance);
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

    const reflType *cType = ReflectorBinUtilFriend::Find(ri, valueNameHash);

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
  auto inst = ri.GetReflectedInstance();
  auto refData = inst.rfStatic;
  JenHash clsHash;

  rd.Push();
  rd.Read(clsHash);

  if (clsHash != refData->classHash) {
    rd.Pop();
    return 1;
  }

  return LoadClass(ri, inst, rd);
}