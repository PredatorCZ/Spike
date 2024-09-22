#pragma once
#include "spike/reflect/reflector_io.hpp"
#include "spike/type/bitfield.hpp"
#include "spike/util/unit_testing.hpp"

using namespace std::string_view_literals;
using namespace es::jenhash_literals;

struct refTypeNames {
  float pitch;
  float volume;
  uint64 seed;
};

REFLECT(CLASS(refTypeNames), MEMBER(pitch), MEMBER(volume), MEMBER(seed));

int test_reflector_decl01(
    const reflectorStatic *mainRefl = GetReflectedClass<refTypeNames>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL("refTypeNames"sv, mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 3);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL("pitch"sv, mainRefl->typeNames[0]);
  TEST_EQUAL("volume"sv, mainRefl->typeNames[1]);
  TEST_EQUAL("seed"sv, mainRefl->typeNames[2]);

  return 0;
}

struct refTypeNames01 {
  float pitch;
  float volume;
  uint64 seed;
};

REFLECT(CLASS(refTypeNames01), MEMBER(pitch), MEMBER(volume, NoName{}),
        MEMBER(seed));

int test_reflector_decl02(
    const reflectorStatic *mainRefl = GetReflectedClass<refTypeNames01>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL("refTypeNames01"sv, mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 3);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL("pitch"sv, mainRefl->typeNames[0]);
  TEST_NOT_CHECK(mainRefl->typeNames[1]);
  TEST_EQUAL("seed"sv, mainRefl->typeNames[2]);

  return 0;
}

struct roomInfo {
  float roomSize;
  float roomDensity;
  float reverb;
  int32 reverbType;
};

REFLECT(CLASS(roomInfo), MEMBER(roomSize, "room_size"),
        MEMBER(roomDensity, "room_density"), MEMBER(reverb),
        MEMBER(reverbType, "reverb_type"));

int test_reflector_decl03(
    const reflectorStatic *mainRefl = GetReflectedClass<roomInfo>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL("roomInfo"sv, mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 4);
  TEST_NOT_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL("roomSize"sv, mainRefl->typeNames[0]);
  TEST_EQUAL("roomDensity"sv, mainRefl->typeNames[1]);
  TEST_EQUAL("reverb"sv, mainRefl->typeNames[2]);
  TEST_EQUAL("reverbType"sv, mainRefl->typeNames[3]);

  TEST_EQUAL("room_size"sv, mainRefl->typeAliases[0]);
  TEST_EQUAL("room_density"sv, mainRefl->typeAliases[1]);
  TEST_EQUAL(mainRefl->typeAliases[2], nullptr);
  TEST_EQUAL("reverb_type"sv, mainRefl->typeAliases[3]);

  TEST_EQUAL("room_size"_jh, mainRefl->typeAliasHashes[0]);
  TEST_EQUAL("room_density"_jh, mainRefl->typeAliasHashes[1]);
  TEST_EQUAL(mainRefl->typeAliasHashes[2], 0);
  TEST_EQUAL("reverb_type"_jh, mainRefl->typeAliasHashes[3]);

  return 0;
}

struct roomInfo01 {
  float roomSize;
  float roomDensity;
  float reverb;
  int32 reverbDelay;
};

REFLECT(CLASS(roomInfo01), MEMBER(roomSize, ReflDesc{"Size of room is", "m3"}),
        MEMBER(roomDensity, ReflDesc{"Density of a room is", "%"}),
        MEMBER(reverb, ReflDesc{"Reverb intensity is", "dB"}),
        MEMBER(reverbDelay, ReflDesc{"Reverb delay is", "seconds"}));

int test_reflector_decl04(
    const reflectorStatic *mainRefl = GetReflectedClass<roomInfo01>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL("roomInfo01"sv, mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 4);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL("roomSize"sv, mainRefl->typeNames[0]);
  TEST_EQUAL("roomDensity"sv, mainRefl->typeNames[1]);
  TEST_EQUAL("reverb"sv, mainRefl->typeNames[2]);
  TEST_EQUAL("reverbDelay"sv, mainRefl->typeNames[3]);

  TEST_EQUAL(mainRefl->typeDescs[0].part1, "Size of room is"sv);
  TEST_EQUAL(mainRefl->typeDescs[0].part2, "m3"sv);
  TEST_EQUAL(mainRefl->typeDescs[1].part1, "Density of a room is"sv);
  TEST_EQUAL(mainRefl->typeDescs[1].part2, "%"sv);
  TEST_EQUAL(mainRefl->typeDescs[2].part1, "Reverb intensity is"sv);
  TEST_EQUAL(mainRefl->typeDescs[2].part2, "dB"sv);
  TEST_EQUAL(mainRefl->typeDescs[3].part1, "Reverb delay is"sv);
  TEST_EQUAL(mainRefl->typeDescs[3].part2, "seconds"sv);

  return 0;
}

struct roomInfo02 {
  float roomSize;
  float roomDensity;
  float reverb;
  int32 reverbDelay;
};

REFLECT(CLASS(roomInfo02),
        MEMBER(roomSize, "room_size", ReflDesc{"Size of room is", "m3"}),
        MEMBER(roomDensity, "room_density",
               ReflDesc{"Density of a room is", "%"}),
        MEMBER(reverb, ReflDesc{"Reverb intensity is", "dB"}),
        MEMBER(reverbDelay, "reverb_delay",
               ReflDesc{"Reverb delay is", "seconds"}));

int test_reflector_decl05(
    const reflectorStatic *mainRefl = GetReflectedClass<roomInfo02>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL("roomInfo02"sv, mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 4);
  TEST_NOT_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL("roomSize"sv, mainRefl->typeNames[0]);
  TEST_EQUAL("roomDensity"sv, mainRefl->typeNames[1]);
  TEST_EQUAL("reverb"sv, mainRefl->typeNames[2]);
  TEST_EQUAL("reverbDelay"sv, mainRefl->typeNames[3]);

  TEST_EQUAL("room_size"_jh, mainRefl->typeAliasHashes[0]);
  TEST_EQUAL("room_density"_jh, mainRefl->typeAliasHashes[1]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[2]);
  TEST_EQUAL("reverb_delay"_jh, mainRefl->typeAliasHashes[3]);

  TEST_EQUAL(mainRefl->typeDescs[0].part1, "Size of room is"sv);
  TEST_EQUAL(mainRefl->typeDescs[0].part2, "m3"sv);
  TEST_EQUAL(mainRefl->typeDescs[1].part1, "Density of a room is"sv);
  TEST_EQUAL(mainRefl->typeDescs[1].part2, "%"sv);
  TEST_EQUAL(mainRefl->typeDescs[2].part1, "Reverb intensity is"sv);
  TEST_EQUAL(mainRefl->typeDescs[2].part2, "dB"sv);
  TEST_EQUAL(mainRefl->typeDescs[3].part1, "Reverb delay is"sv);
  TEST_EQUAL(mainRefl->typeDescs[3].part2, "seconds"sv);

  TEST_EQUAL("room_size"sv, mainRefl->typeAliases[0]);
  TEST_EQUAL("room_density"sv, mainRefl->typeAliases[1]);
  TEST_EQUAL(mainRefl->typeAliases[2], nullptr);
  TEST_EQUAL("reverb_delay"sv, mainRefl->typeAliases[3]);

  return 0;
}

template <class C1, class C2> struct templatedClass {
  C1 item0;
  C2 item1;
};

REFLECT(CLASS(templatedClass<int, float>), MEMBER(item0), MEMBER(item1));

int test_reflector_decl06(const reflectorStatic *mainRefl =
                              GetReflectedClass<templatedClass<int, float>>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL("templatedClass<int, float>"sv, mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 2);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL("item0"sv, mainRefl->typeNames[0]);
  TEST_EQUAL("item1"sv, mainRefl->typeNames[1]);

  return 0;
}

using member0 = BitMemberDecl<0, 2>;
using member1 = BitMemberDecl<1, 5>;
using member2 = BitMemberDecl<2, 3>;
using member3 = BitMemberDecl<3, 1>;
using member4 = BitMemberDecl<4, 2>;
using member5 = BitMemberDecl<5, 3>;
using BitType0 =
    BitFieldType<uint16, member0, member1, member2, member3, member4, member5>;

REFLECT(CLASS(BitType0), BITMEMBER(member0), BITMEMBER(member1),
        BITMEMBER(member2), BITMEMBER(member3), BITMEMBER(member4),
        BITMEMBER(member5));

int test_reflector_decl07(
    const reflectorStatic *mainRefl = GetReflectedClass<BitType0>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL("BitType0"sv, mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 6);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL("member0"sv, mainRefl->typeNames[0]);
  TEST_EQUAL("member1"sv, mainRefl->typeNames[1]);
  TEST_EQUAL("member2"sv, mainRefl->typeNames[2]);
  TEST_EQUAL("member3"sv, mainRefl->typeNames[3]);
  TEST_EQUAL("member4"sv, mainRefl->typeNames[4]);
  TEST_EQUAL("member5"sv, mainRefl->typeNames[5]);

  return 0;
}

using member41 = BitMemberDecl<4, 3>;
using member51 = BitMemberDecl<5, 2>;

using BitType2 = BitFieldType<uint16, member0, member1, member2, member3,
                              member41, member51>;

REFLECT(CLASS(BitType2), BITMEMBER(member0), BITMEMBER(member1, "memAlias1"),
        BITMEMBER(member2, ReflDesc{"memDescr2"}),
        BITMEMBER(member3, "memAlias3", ReflDesc{"memDescr3"}),
        BITMEMBER(member41), BITMEMBER(member51));

int test_reflector_decl09(
    const reflectorStatic *mainRefl = GetReflectedClass<BitType2>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL("BitType2"sv, mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 6);
  TEST_NOT_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL("member0"sv, mainRefl->typeNames[0]);
  TEST_EQUAL("member1"sv, mainRefl->typeNames[1]);
  TEST_EQUAL("member2"sv, mainRefl->typeNames[2]);
  TEST_EQUAL("member3"sv, mainRefl->typeNames[3]);
  TEST_EQUAL("member41"sv, mainRefl->typeNames[4]);
  TEST_EQUAL("member51"sv, mainRefl->typeNames[5]);

  TEST_EQUAL(0, mainRefl->typeAliasHashes[0]);
  TEST_EQUAL("memAlias1"_jh, mainRefl->typeAliasHashes[1]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[2]);
  TEST_EQUAL("memAlias3"_jh, mainRefl->typeAliasHashes[3]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[4]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[5]);

  TEST_EQUAL(mainRefl->typeDescs[0].part1, nullptr);
  TEST_EQUAL(mainRefl->typeDescs[1].part1, nullptr);
  TEST_EQUAL(mainRefl->typeDescs[2].part1, "memDescr2"sv);
  TEST_EQUAL(mainRefl->typeDescs[3].part1, "memDescr3"sv);
  TEST_EQUAL(mainRefl->typeDescs[4].part1, nullptr);
  TEST_EQUAL(mainRefl->typeDescs[5].part1, nullptr);

  TEST_EQUAL(mainRefl->typeAliases[0], nullptr);
  TEST_EQUAL("memAlias1"sv, mainRefl->typeAliases[1]);
  TEST_EQUAL(mainRefl->typeAliases[2], nullptr);
  TEST_EQUAL("memAlias3"sv, mainRefl->typeAliases[3]);
  TEST_EQUAL(mainRefl->typeAliases[4], nullptr);
  TEST_EQUAL(mainRefl->typeAliases[5], nullptr);

  for (uint32 i = 0; i < mainRefl->nTypes; i++) {
    TEST_NOT_CHECK(mainRefl->typeDescs[i].part2);
  }

  return 0;
}

struct ReflVectors {
  Vector vec;
  Vector vecArr[10];
  void ReflectorTag();
};

REFLECT(CLASS(ReflVectors), MEMBER(vec), MEMBER(vecArr));

int test_reflector_decl10(
    const reflectorStatic *mainRefl = GetReflectedClass<ReflVectors>()) {
  ReflType vecType = mainRefl->types[0];

  TEST_EQUAL(vecType.size, sizeof(ReflVectors::vec));

  ReflTypeVector asVec = vecType.asVector;

  TEST_EQUAL(asVec.numItems, 3);
  TEST_EQUAL(asVec.stride, 4);
  TEST_EQUAL(asVec.type, REFType::FloatingPoint);

  ReflType vecArrType = mainRefl->types[1];

  TEST_EQUAL(vecArrType.size, sizeof(ReflVectors::vecArr));
  TEST_EQUAL(vecArrType.type, REFType::Vector);

  ReflTypeArray asVecArr = vecArrType.asArray;

  TEST_EQUAL(asVecArr.numItems, 10);
  TEST_EQUAL(asVecArr.stride, 12);
  TEST_EQUAL(asVecArr.type, REFType::Vector);

  ReflTypeVector asVecArrVec = asVecArr.asVector;

  TEST_EQUAL(asVecArrVec.numItems, 3);
  TEST_EQUAL(asVecArrVec.stride, 4);
  TEST_EQUAL(asVecArrVec.type, REFType::FloatingPoint);

  return 0;
}

struct ReflVector {
  std::vector<float> flVector;
  std::vector<float> flVectors[2];
  std::vector<ReflVectors> vectots;
};

REFLECT(CLASS(ReflVector), MEMBER(flVector), MEMBER(flVectors[0]),
        MEMBER(flVectors[1]), MEMBER(vectots));

int test_reflector_decl11(
    const reflectorStatic *mainRefl = GetReflectedClass<ReflVector>()) {
  ReflType vecType = mainRefl->types[0];

  TEST_EQUAL(vecType.size, 4);
  TEST_EQUAL(vecType.type, REFType::FloatingPoint);
  TEST_EQUAL(vecType.container, REFContainer::ContainerVector);

  vecType = mainRefl->types[3];

  TEST_EQUAL(vecType.size, sizeof(ReflVectors));
  TEST_EQUAL(vecType.type, REFType::Class);
  TEST_EQUAL(vecType.container, REFContainer::ContainerVector);
  TEST_EQUAL(vecType.asClass.typeHash,
             GetReflectedClass<ReflVectors>()->classHash);

  return 0;
}
