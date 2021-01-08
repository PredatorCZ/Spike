#pragma once
#include "../datas/bitfield.hpp"
#include "../datas/reflector_io.hpp"
#include "../datas/unit_testing.hpp"

struct refBasic {
  uint32 item00;
  int32 item01;
  uint64 item02;
  int64 item03;
};

REFLECTOR_CREATE(refBasic, 0, item00, item01, item02, item03);

int test_reflector_decl00(
    const reflectorStatic *mainRefl = GetReflectedClass<refBasic>()) {
  TEST_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(mainRefl->nTypes, 4);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  return 0;
}

struct refTypeNames {
  float pitch;
  float volume;
  uint64 seed;
};

REFLECTOR_CREATE(refTypeNames, 1, VARNAMES, pitch, volume, seed);

int test_reflector_decl01(
    const reflectorStatic *mainRefl = GetReflectedClass<refTypeNames>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(es::string_view("refTypeNames"), mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 3);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL(es::string_view("pitch"), mainRefl->typeNames[0]);
  TEST_EQUAL(es::string_view("volume"), mainRefl->typeNames[1]);
  TEST_EQUAL(es::string_view("seed"), mainRefl->typeNames[2]);

  return 0;
}

struct refTypeNames01 {
  float pitch;
  float volume;
  uint64 seed;
};

REFLECTOR_CREATE(refTypeNames01, 1, EXTENDED, (, pitch), (, volume), (, seed));

int test_reflector_decl02(
    const reflectorStatic *mainRefl = GetReflectedClass<refTypeNames01>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(es::string_view("refTypeNames01"), mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 3);
  TEST_NOT_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL(es::string_view("pitch"), mainRefl->typeNames[0]);
  TEST_EQUAL(es::string_view("volume"), mainRefl->typeNames[1]);
  TEST_EQUAL(es::string_view("seed"), mainRefl->typeNames[2]);

  for (uint32 i = 0; i < mainRefl->nTypes; i++) {
    TEST_EQUAL(mainRefl->typeAliases[i], nullptr);
    TEST_EQUAL(mainRefl->typeAliasHashes[i], 0);
    TEST_CHECK(mainRefl->typeDescs[i].part1.empty());
    TEST_CHECK(mainRefl->typeDescs[i].part2.empty());
  }

  return 0;
}

struct roomInfo {
  float roomSize;
  float roomDensity;
  float reverb;
  int32 reverbType;
};

REFLECTOR_CREATE(roomInfo, 1, EXTENDED, (A, roomSize, "room_size"),
                 (A, roomDensity, "room_density"), (, reverb),
                 (A, reverbType, "reverb_type"));

int test_reflector_decl03(
    const reflectorStatic *mainRefl = GetReflectedClass<roomInfo>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(es::string_view("roomInfo"), mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 4);
  TEST_NOT_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL(es::string_view("roomSize"), mainRefl->typeNames[0]);
  TEST_EQUAL(es::string_view("roomDensity"), mainRefl->typeNames[1]);
  TEST_EQUAL(es::string_view("reverb"), mainRefl->typeNames[2]);
  TEST_EQUAL(es::string_view("reverbType"), mainRefl->typeNames[3]);

  TEST_EQUAL(es::string_view("room_size"), mainRefl->typeAliases[0]);
  TEST_EQUAL(es::string_view("room_density"), mainRefl->typeAliases[1]);
  TEST_EQUAL(mainRefl->typeAliases[2], nullptr);
  TEST_EQUAL(es::string_view("reverb_type"), mainRefl->typeAliases[3]);

  TEST_EQUAL(JenHash("room_size"), mainRefl->typeAliasHashes[0]);
  TEST_EQUAL(JenHash("room_density"), mainRefl->typeAliasHashes[1]);
  TEST_EQUAL(mainRefl->typeAliasHashes[2], 0);
  TEST_EQUAL(JenHash("reverb_type"), mainRefl->typeAliasHashes[3]);

  for (uint32 i = 0; i < mainRefl->nTypes; i++) {
    TEST_CHECK(mainRefl->typeDescs[i].part1.empty());
    TEST_CHECK(mainRefl->typeDescs[i].part2.empty());
  }

  return 0;
}

struct roomInfo01 {
  float roomSize;
  float roomDensity;
  float reverb;
  int32 reverbDelay;
};

REFLECTOR_CREATE(roomInfo01, 1, EXTENDED, (D, roomSize, "Size of room is%m3"),
                 (D, roomDensity, "Density of a room is%%"),
                 (D, reverb, "Reverb intensity is%dB"),
                 (D, reverbDelay, "Reverb delay is%seconds"));

int test_reflector_decl04(
    const reflectorStatic *mainRefl = GetReflectedClass<roomInfo01>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(es::string_view("roomInfo01"), mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 4);
  TEST_NOT_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL(es::string_view("roomSize"), mainRefl->typeNames[0]);
  TEST_EQUAL(es::string_view("roomDensity"), mainRefl->typeNames[1]);
  TEST_EQUAL(es::string_view("reverb"), mainRefl->typeNames[2]);
  TEST_EQUAL(es::string_view("reverbDelay"), mainRefl->typeNames[3]);

  TEST_EQUAL(mainRefl->typeDescs[0].part1, "Size of room is");
  TEST_EQUAL(mainRefl->typeDescs[0].part2, "m3");
  TEST_EQUAL(mainRefl->typeDescs[1].part1, "Density of a room is");
  TEST_EQUAL(mainRefl->typeDescs[1].part2, "%");
  TEST_EQUAL(mainRefl->typeDescs[2].part1, "Reverb intensity is");
  TEST_EQUAL(mainRefl->typeDescs[2].part2, "dB");
  TEST_EQUAL(mainRefl->typeDescs[3].part1, "Reverb delay is");
  TEST_EQUAL(mainRefl->typeDescs[3].part2, "seconds");

  for (uint32 i = 0; i < mainRefl->nTypes; i++) {
    TEST_EQUAL(mainRefl->typeAliases[i], nullptr);
    TEST_EQUAL(mainRefl->typeAliasHashes[i], 0);
  }

  return 0;
}

struct roomInfo02 {
  float roomSize;
  float roomDensity;
  float reverb;
  int32 reverbDelay;
};

REFLECTOR_CREATE(roomInfo02, 1, EXTENDED,
                 (AD, roomSize, "room_size", "Size of room is%m3"),
                 (AD, roomDensity, "room_density", "Density of a room is%%"),
                 (D, reverb, "Reverb intensity is%dB"),
                 (AD, reverbDelay, "reverb_delay", "Reverb delay is%seconds"));

int test_reflector_decl05(
    const reflectorStatic *mainRefl = GetReflectedClass<roomInfo02>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(es::string_view("roomInfo02"), mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 4);
  TEST_NOT_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL(es::string_view("roomSize"), mainRefl->typeNames[0]);
  TEST_EQUAL(es::string_view("roomDensity"), mainRefl->typeNames[1]);
  TEST_EQUAL(es::string_view("reverb"), mainRefl->typeNames[2]);
  TEST_EQUAL(es::string_view("reverbDelay"), mainRefl->typeNames[3]);

  TEST_EQUAL(JenHash("room_size"), mainRefl->typeAliasHashes[0]);
  TEST_EQUAL(JenHash("room_density"), mainRefl->typeAliasHashes[1]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[2]);
  TEST_EQUAL(JenHash("reverb_delay"), mainRefl->typeAliasHashes[3]);

  TEST_EQUAL(mainRefl->typeDescs[0].part1, "Size of room is");
  TEST_EQUAL(mainRefl->typeDescs[0].part2, "m3");
  TEST_EQUAL(mainRefl->typeDescs[1].part1, "Density of a room is");
  TEST_EQUAL(mainRefl->typeDescs[1].part2, "%");
  TEST_EQUAL(mainRefl->typeDescs[2].part1, "Reverb intensity is");
  TEST_EQUAL(mainRefl->typeDescs[2].part2, "dB");
  TEST_EQUAL(mainRefl->typeDescs[3].part1, "Reverb delay is");
  TEST_EQUAL(mainRefl->typeDescs[3].part2, "seconds");

  TEST_EQUAL(es::string_view("room_size"), mainRefl->typeAliases[0]);
  TEST_EQUAL(es::string_view("room_density"), mainRefl->typeAliases[1]);
  TEST_EQUAL(mainRefl->typeAliases[2], nullptr);
  TEST_EQUAL(es::string_view("reverb_delay"), mainRefl->typeAliases[3]);

  return 0;
}

template <class C1, class C2> struct templatedClass {
  C1 item0;
  C2 item1;
};

REFLECTOR_CREATE((templatedClass<int, float>), 2, TEMPLATE, VARNAMES, item0,
                 item1);

int test_reflector_decl06(const reflectorStatic *mainRefl =
                              GetReflectedClass<templatedClass<int, float>>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(es::string_view("templatedClass<int, float>"),
             mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 2);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL(es::string_view("item0"), mainRefl->typeNames[0]);
  TEST_EQUAL(es::string_view("item1"), mainRefl->typeNames[1]);

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

REFLECTOR_CREATE(BitType0, BITFIELD, 0, member0, member1, member2, member3,
                 member4, member5);

int test_reflector_decl07(
    const reflectorStatic *mainRefl = GetReflectedClass<BitType0>()) {
  TEST_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(mainRefl->nTypes, 6);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  return 0;
}

using BitType1 =
    BitFieldType<uint16, member0, member1, member2, member3, member4>;

REFLECTOR_CREATE(BitType1, BITFIELD, 1, VARNAMES, member0, member1, member2,
                 member3, member4);

int test_reflector_decl08(
    const reflectorStatic *mainRefl = GetReflectedClass<BitType1>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(es::string_view("BitType1"), mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 5);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL(es::string_view("member0"), mainRefl->typeNames[0]);
  TEST_EQUAL(es::string_view("member1"), mainRefl->typeNames[1]);
  TEST_EQUAL(es::string_view("member2"), mainRefl->typeNames[2]);
  TEST_EQUAL(es::string_view("member3"), mainRefl->typeNames[3]);
  TEST_EQUAL(es::string_view("member4"), mainRefl->typeNames[4]);

  return 0;
}

using member41 = BitMemberDecl<4, 3>;
using member51 = BitMemberDecl<5, 2>;

using BitType2 = BitFieldType<uint16, member0, member1, member2, member3,
                              member41, member51>;

REFLECTOR_CREATE(BitType2, BITFIELD, 1, EXTENDED, (, member0),
                 (A, member1, "memAlias1"), (D, member2, "memDescr2"),
                 (AD, member3, "memAlias3", "memDescr3"), (, member41),
                 (, member51));

int test_reflector_decl09(
    const reflectorStatic *mainRefl = GetReflectedClass<BitType2>()) {
  TEST_NOT_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(es::string_view("BitType2"), mainRefl->className);
  TEST_EQUAL(mainRefl->nTypes, 6);
  TEST_NOT_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeNames, nullptr);
  TEST_NOT_EQUAL(mainRefl->typeAliasHashes, nullptr);

  TEST_EQUAL(es::string_view("member0"), mainRefl->typeNames[0]);
  TEST_EQUAL(es::string_view("member1"), mainRefl->typeNames[1]);
  TEST_EQUAL(es::string_view("member2"), mainRefl->typeNames[2]);
  TEST_EQUAL(es::string_view("member3"), mainRefl->typeNames[3]);
  TEST_EQUAL(es::string_view("member41"), mainRefl->typeNames[4]);
  TEST_EQUAL(es::string_view("member51"), mainRefl->typeNames[5]);

  TEST_EQUAL(0, mainRefl->typeAliasHashes[0]);
  TEST_EQUAL(JenHash("memAlias1"), mainRefl->typeAliasHashes[1]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[2]);
  TEST_EQUAL(JenHash("memAlias3"), mainRefl->typeAliasHashes[3]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[4]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[5]);

  TEST_CHECK(mainRefl->typeDescs[0].part1.empty());
  TEST_CHECK(mainRefl->typeDescs[1].part1.empty());
  TEST_EQUAL(mainRefl->typeDescs[2].part1, "memDescr2");
  TEST_EQUAL(mainRefl->typeDescs[3].part1, "memDescr3");
  TEST_CHECK(mainRefl->typeDescs[4].part1.empty());
  TEST_CHECK(mainRefl->typeDescs[5].part1.empty());

  TEST_EQUAL(mainRefl->typeAliases[0], nullptr);
  TEST_EQUAL(es::string_view("memAlias1"), mainRefl->typeAliases[1]);
  TEST_EQUAL(mainRefl->typeAliases[2], nullptr);
  TEST_EQUAL(es::string_view("memAlias3"), mainRefl->typeAliases[3]);
  TEST_EQUAL(mainRefl->typeAliases[4], nullptr);
  TEST_EQUAL(mainRefl->typeAliases[5], nullptr);

  for (uint32 i = 0; i < mainRefl->nTypes; i++) {
    TEST_CHECK(mainRefl->typeDescs[i].part2.empty());
  }

  return 0;
}
