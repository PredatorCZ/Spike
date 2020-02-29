#pragma once
#include "../datas/reflector_io.hpp"
#include "../datas/unit_testing.hpp"

struct refBasic {
  DECLARE_REFLECTOR; 

  uint32 item00;
  int32 item01;
  uint64 item02;
  int64 item03;
};

REFLECTOR_CREATE(refBasic, 0, item00, item01, item02, item03);

int test_reflector_decl00(
    const reflectorStatic *mainRefl = refBasic::__rfPtrStatic) {
  TEST_EQUAL(mainRefl->className, nullptr);
  TEST_EQUAL(mainRefl->nTypes, 4);
  TEST_EQUAL(mainRefl->typeAliases, nullptr);
  TEST_EQUAL(mainRefl->typeDescs, nullptr);
  TEST_EQUAL(mainRefl->typeNames, nullptr);
  TEST_EQUAL(mainRefl->typeAliasHashes, nullptr);

  return 0;
}

struct refTypeNames {
  DECLARE_REFLECTOR;

  float pitch;
  float volume;
  uint64 seed;
};

REFLECTOR_CREATE(refTypeNames, 1, VARNAMES, pitch, volume, seed);

int test_reflector_decl01(
    const reflectorStatic *mainRefl = refTypeNames::__rfPtrStatic) {
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
  DECLARE_REFLECTOR;

  float pitch;
  float volume;
  uint64 seed;
};

REFLECTOR_CREATE(refTypeNames01, 1, EXTENDED, (, pitch), (, volume), (, seed));

int test_reflector_decl02(
    const reflectorStatic *mainRefl = refTypeNames01::__rfPtrStatic) {
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
  DECLARE_REFLECTOR;

  float roomSize;
  float roomDensity;
  float reverb;
  int32 reverbType;
};

REFLECTOR_CREATE(roomInfo, 1, EXTENDED, (A, roomSize, "room_size"),
                 (A, roomDensity, "room_density"), (, reverb),
                 (A, reverbType, "reverb_type"));

int test_reflector_decl03(
    const reflectorStatic *mainRefl = roomInfo::__rfPtrStatic) {
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

  TEST_EQUAL(JenkinsHashC("room_size"), mainRefl->typeAliasHashes[0]);
  TEST_EQUAL(JenkinsHashC("room_density"), mainRefl->typeAliasHashes[1]);
  TEST_EQUAL(mainRefl->typeAliasHashes[2], 0);
  TEST_EQUAL(JenkinsHashC("reverb_type"), mainRefl->typeAliasHashes[3]);

  for (uint32 i = 0; i < mainRefl->nTypes; i++) {
    TEST_CHECK(mainRefl->typeDescs[i].part1.empty());
    TEST_CHECK(mainRefl->typeDescs[i].part2.empty());
  }

  return 0;
}

struct roomInfo01 {
  DECLARE_REFLECTOR;

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
    const reflectorStatic *mainRefl = roomInfo01::__rfPtrStatic) {
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
  DECLARE_REFLECTOR;

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
    const reflectorStatic *mainRefl = roomInfo02::__rfPtrStatic) {
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

  TEST_EQUAL(JenkinsHashC("room_size"), mainRefl->typeAliasHashes[0]);
  TEST_EQUAL(JenkinsHashC("room_density"), mainRefl->typeAliasHashes[1]);
  TEST_EQUAL(0, mainRefl->typeAliasHashes[2]);
  TEST_EQUAL(JenkinsHashC("reverb_delay"), mainRefl->typeAliasHashes[3]);

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