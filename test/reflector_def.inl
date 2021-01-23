#pragma once
#include "datas/bitfield.hpp"
#include "datas/endian.hpp"
#include "datas/flags.hpp"
#include "datas/reflector.hpp"
#include "datas/vectors_simd.hpp"
#include "datas/unit_testing.hpp"

REFLECTOR_CREATE(EnumWrap00, ENUM, 1, CLASS, E1, E2,
                 E3 = 0x7); // as enum class EnumWrap00 {};
REFLECTOR_CREATE(EnumWrap01, ENUM, 0, EnumWrap01_E1, EnumWrap01_E2,
                 EnumWrap01_E3); // as enum EnumWrap01 {};
REFLECTOR_CREATE(EnumWrap02, ENUM, 2, CLASS, 8, E4, E5,
                 E6); // as enum class EnumWrap02 : uchar {};
REFLECTOR_CREATE(EnumWrap03, ENUM, 2, CLASS, 16, E7 = 7, E8 = 16586,
                 E9 = 0x8bcd); // as enum class EnumWrap03 : ushort {};
REFLECTOR_CREATE(EnumWrap04, ENUM, 2, CLASS, 32, E10, E11,
                 E12); // as enum class EnumWrap04 : uint {};

struct subrefl {
  int data0;
  float data1;

  void SwapEndian() {
    FByteswapper(data0);
    FByteswapper(data1);
  }
  void ReflectorTag();
};

REFLECTOR_CREATE(subrefl, 1, VARNAMES, data0, data1);

using member0 = BitMemberDecl<0, 2>;
using member1 = BitMemberDecl<1, 5>;
using member2 = BitMemberDecl<2, 3>;
using member3 = BitMemberDecl<3, 1>;
using member42 = BitMemberDecl<4, 5>;
using BitTypeRefl =
    BitFieldType<uint16, member0, member1, member2, member3, member42>;

REFLECTOR_CREATE(BitTypeRefl, BITFIELD, 1, VARNAMES, member0, member1, member2,
                 member3, member42);

struct _ReflClassData {
  bool test1;
  int8 test2;
  uint8 test3;
  int16 test4;
  uint16 test5;
  int32 test6;
  uint32 test7;
  int64 test8;
  uint64 test9;
  float test10;
  double test11;

  es::Flags<EnumWrap00, uint8> test12;
  es::Flags<EnumWrap01, uint16> test13;

  EnumWrap00 test14;
  EnumWrap02 test15;
  EnumWrap03 test16;
  EnumWrap04 test17;

  Vector test18;
  Vector2 test19;
  Vector4 test20;
  Vector4A16 test21;

  subrefl test22;
  BitTypeRefl test23;

  bool test40[4];
  int8 test41[2];
  uint8 test42[4];
  int16 test43[2];
  uint16 test44[4];
  int32 test45[2];
  uint32 test46[4];
  int64 test47[2];
  uint64 test48[4];
  float test49[2];
  double test50[4];

  es::Flags<EnumWrap00, uint8> test51[2];
  es::Flags<EnumWrap01, uint16> test52[4];

  EnumWrap00 test53[2];
  EnumWrap02 test54[4];
  EnumWrap03 test55[2];
  EnumWrap04 test56[4];

  Vector test57[2];
  Vector2 test58[4];
  Vector4 test59[2];
  Vector4A16 test60[4];

  subrefl test61[2];
  BitTypeRefl test62[2];

  // Need to enclose padding because of RPO,
  // otherwise cast write will corrupt test80
  Vector4A16 padClose;

  _ReflClassData() = default;
};

struct reflClass : ReflectorBase<reflClass>, _ReflClassData {
  std::string test80;

  reflClass() = default;
};

REFLECTOR_CREATE(reflClass, 1, VARNAMES, test1, test2, test3, test4, test5,
                 test6, test7, test8, test9, test10, test11, test12, test13,
                 test14, test15, test16, test17, test18, test19, test20, test21,
                 test22, test40, test41, test42, test43, test44, test45, test46,
                 test47, test48, test49, test50, test51, test52, test53, test54,
                 test55, test56, test57, test58, test59, test60, test61, test80,
                 test23, test62)

int compare_classes(const reflClass &rClass, const reflClass &rClass2) {
  TEST_EQUAL(rClass.test1, rClass2.test1);
  TEST_EQUAL(rClass.test2, rClass2.test2);
  TEST_EQUAL(rClass.test3, rClass2.test3);
  TEST_EQUAL(rClass.test4, rClass2.test4);
  TEST_EQUAL(rClass.test5, rClass2.test5);
  TEST_EQUAL(rClass.test6, rClass2.test6);
  TEST_EQUAL(rClass.test7, rClass2.test7);
  TEST_EQUAL(rClass.test8, rClass2.test8);
  TEST_EQUAL(rClass.test9, rClass2.test9);
  TEST_EQUAL(rClass.test10, rClass2.test10);
  TEST_EQUAL(rClass.test11, rClass2.test11);
  TEST_EQUAL(rClass.test12, rClass2.test12);
  TEST_EQUAL(rClass.test13, rClass2.test13);
  TEST_EQUAL(rClass.test14, rClass2.test14);
  TEST_EQUAL(rClass.test15, rClass2.test15);
  TEST_EQUAL(rClass.test16, rClass2.test16);
  TEST_EQUAL(rClass.test17, rClass2.test17);
  TEST_EQUAL(rClass.test18, rClass2.test18);
  TEST_EQUAL(rClass.test19, rClass2.test19);
  TEST_EQUAL(rClass.test20, rClass2.test20);
  TEST_EQUAL(rClass.test21, rClass2.test21);
  TEST_EQUAL(rClass.test22.data0, rClass2.test22.data0);
  TEST_EQUAL(rClass.test22.data1, rClass2.test22.data1);
  TEST_EQUAL(rClass.test40[0], rClass2.test40[0]);
  TEST_EQUAL(rClass.test40[1], rClass2.test40[1]);
  TEST_EQUAL(rClass.test40[2], rClass2.test40[2]);
  TEST_EQUAL(rClass.test40[3], rClass2.test40[3]);
  TEST_EQUAL(rClass.test41[0], rClass2.test41[0]);
  TEST_EQUAL(rClass.test41[1], rClass2.test41[1]);
  TEST_EQUAL(rClass.test42[0], rClass2.test42[0]);
  TEST_EQUAL(rClass.test42[1], rClass2.test42[1]);
  TEST_EQUAL(rClass.test42[2], rClass2.test42[2]);
  TEST_EQUAL(rClass.test42[3], rClass2.test42[3]);
  TEST_EQUAL(rClass.test43[0], rClass2.test43[0]);
  TEST_EQUAL(rClass.test43[1], rClass2.test43[1]);
  TEST_EQUAL(rClass.test44[0], rClass2.test44[0]);
  TEST_EQUAL(rClass.test44[1], rClass2.test44[1]);
  TEST_EQUAL(rClass.test44[2], rClass2.test44[2]);
  TEST_EQUAL(rClass.test44[3], rClass2.test44[3]);
  TEST_EQUAL(rClass.test45[0], rClass2.test45[0]);
  TEST_EQUAL(rClass.test45[1], rClass2.test45[1]);
  TEST_EQUAL(rClass.test46[0], rClass2.test46[0]);
  TEST_EQUAL(rClass.test46[1], rClass2.test46[1]);
  TEST_EQUAL(rClass.test46[2], rClass2.test46[2]);
  TEST_EQUAL(rClass.test46[3], rClass2.test46[3]);
  TEST_EQUAL(rClass.test47[0], rClass2.test47[0]);
  TEST_EQUAL(rClass.test47[1], rClass2.test47[1]);
  TEST_EQUAL(rClass.test48[0], rClass2.test48[0]);
  TEST_EQUAL(rClass.test48[1], rClass2.test48[1]);
  TEST_EQUAL(rClass.test48[2], rClass2.test48[2]);
  TEST_EQUAL(rClass.test48[3], rClass2.test48[3]);
  TEST_EQUAL(rClass.test49[0], rClass2.test49[0]);
  TEST_EQUAL(rClass.test49[1], rClass2.test49[1]);
  TEST_EQUAL(rClass.test50[0], rClass2.test50[0]);
  TEST_EQUAL(rClass.test50[1], rClass2.test50[1]);
  TEST_EQUAL(rClass.test50[2], rClass2.test50[2]);
  TEST_EQUAL(rClass.test50[3], rClass2.test50[3]);
  TEST_EQUAL(rClass.test51[0], rClass2.test51[0]);
  TEST_EQUAL(rClass.test51[1], rClass2.test51[1]);
  TEST_EQUAL(rClass.test52[0], rClass2.test52[0]);
  TEST_EQUAL(rClass.test52[1], rClass2.test52[1]);
  TEST_EQUAL(rClass.test52[2], rClass2.test52[2]);
  TEST_EQUAL(rClass.test52[3], rClass2.test52[3]);
  TEST_EQUAL(rClass.test53[0], rClass2.test53[0]);
  TEST_EQUAL(rClass.test53[1], rClass2.test53[1]);
  TEST_EQUAL(rClass.test54[0], rClass2.test54[0]);
  TEST_EQUAL(rClass.test54[1], rClass2.test54[1]);
  TEST_EQUAL(rClass.test54[2], rClass2.test54[2]);
  TEST_EQUAL(rClass.test54[3], rClass2.test54[3]);
  TEST_EQUAL(rClass.test55[0], rClass2.test55[0]);
  TEST_EQUAL(rClass.test55[1], rClass2.test55[1]);
  TEST_EQUAL(rClass.test56[0], rClass2.test56[0]);
  TEST_EQUAL(rClass.test56[1], rClass2.test56[1]);
  TEST_EQUAL(rClass.test56[2], rClass2.test56[2]);
  TEST_EQUAL(rClass.test56[3], rClass2.test56[3]);
  TEST_EQUAL(rClass.test57[0], rClass2.test57[0]);
  TEST_EQUAL(rClass.test57[1], rClass2.test57[1]);
  TEST_EQUAL(rClass.test58[0], rClass2.test58[0]);
  TEST_EQUAL(rClass.test58[1], rClass2.test58[1]);
  TEST_EQUAL(rClass.test58[2], rClass2.test58[2]);
  TEST_EQUAL(rClass.test58[3], rClass2.test58[3]);
  TEST_EQUAL(rClass.test59[0], rClass2.test59[0]);
  TEST_EQUAL(rClass.test59[1], rClass2.test59[1]);
  TEST_EQUAL(rClass.test60[0], rClass2.test60[0]);
  TEST_EQUAL(rClass.test60[1], rClass2.test60[1]);
  TEST_EQUAL(rClass.test60[2], rClass2.test60[2]);
  TEST_EQUAL(rClass.test60[3], rClass2.test60[3]);
  TEST_EQUAL(rClass.test61[0].data0, rClass2.test61[0].data0);
  TEST_EQUAL(rClass.test61[0].data1, rClass2.test61[0].data1);
  TEST_EQUAL(rClass.test61[1].data0, rClass2.test61[1].data0);
  TEST_EQUAL(rClass.test61[1].data1, rClass2.test61[1].data1);
  TEST_EQUAL(rClass.test80, rClass2.test80);
  TEST_EQUAL(rClass.test23.value, rClass2.test23.value);
  TEST_EQUAL(rClass.test62[0].value, rClass2.test62[0].value);
  TEST_EQUAL(rClass.test62[1].value, rClass2.test62[1].value);

  return 0;
}
