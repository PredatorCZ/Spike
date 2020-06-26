#pragma once
#include "../datas/endian.hpp"
#include "../datas/reflector_io.hpp"
#include "../datas/unit_testing.hpp"

#include "../datas/vectors_simd.hpp"
#include "../datas/flags.hpp"

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

ES_STATIC_ASSERT(sizeof(EnumWrap02) == 1);
ES_STATIC_ASSERT(sizeof(EnumWrap03) == 2);
ES_STATIC_ASSERT(sizeof(EnumWrap04) == 4);

int test_reflector_enum00() {
  ReflectedEnum rEnum = GetReflectedEnum<EnumWrap00>();

  TEST_EQUAL(rEnum.hash, JenkinsHashC("EnumWrap00"));
  TEST_EQUAL(rEnum.size(), 3);
  TEST_EQUAL(rEnum.name, std::string("EnumWrap00"));

  static const char *names[] = {"E1", "E2", "E3"};
  static const uint64 ids[] = {0, 1, 7};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum[t], names[t]);
    TEST_EQUAL(rEnum.values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum01() {
  ReflectedEnum rEnum = GetReflectedEnum<EnumWrap01>();

  TEST_EQUAL(rEnum.hash, JenkinsHashC("EnumWrap01"));
  TEST_EQUAL(rEnum.size(), 3);
  TEST_EQUAL(rEnum.name, std::string("EnumWrap01"));

  static const char *names[] = {"EnumWrap01_E1", "EnumWrap01_E2",
                                "EnumWrap01_E3"};
  static const uint64 ids[] = {0, 1, 2};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum[t], names[t]);
    TEST_EQUAL(rEnum.values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum02() {
  ReflectedEnum rEnum = GetReflectedEnum<EnumWrap02>();

  TEST_EQUAL(rEnum.hash, JenkinsHashC("EnumWrap02"));
  TEST_EQUAL(rEnum.size(), 3);
  TEST_EQUAL(rEnum.name, std::string("EnumWrap02"));

  static const char *names[] = {"E4", "E5", "E6"};
  static const uint64 ids[] = {0, 1, 2};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum[t], names[t]);
    TEST_EQUAL(rEnum.values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum03() {
  ReflectedEnum rEnum = GetReflectedEnum<EnumWrap03>();

  TEST_EQUAL(rEnum.hash, JenkinsHashC("EnumWrap03"));
  TEST_EQUAL(rEnum.size(), 3);
  TEST_EQUAL(rEnum.name, std::string("EnumWrap03"));

  static const char *names[] = {"E7", "E8", "E9"};
  static const uint64 ids[] = {7, 16586, 0x8bcd};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum[t], names[t]);
    TEST_EQUAL(rEnum.values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum04() {
  ReflectedEnum rEnum = GetReflectedEnum<EnumWrap04>();

  TEST_EQUAL(rEnum.hash, JenkinsHashC("EnumWrap04"));
  TEST_EQUAL(rEnum.size(), 3);
  TEST_EQUAL(rEnum.name, std::string("EnumWrap04"));

  static const char *names[] = {"E10", "E11", "E12"};
  static const uint64 ids[] = {0, 1, 2};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum[t], names[t]);
    TEST_EQUAL(rEnum.values[t], ids[t]);
  }

  return 0;
}

struct subrefl : ReflectorInterface<subrefl> {
  int data0;
  float data1;

  void SwapEndian() {
    FByteswapper(data0);
    FByteswapper(data1);
  }
};

REFLECTOR_CREATE(subrefl, 1, VARNAMES, data0, data1);

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

  esFlags<uint8, EnumWrap00> test12;
  esFlags<uint16, EnumWrap01> test13;

  EnumWrap00 test14;
  EnumWrap02 test15;
  EnumWrap03 test16;
  EnumWrap04 test17;

  Vector test18;
  Vector2 test19;
  Vector4 test20;
  Vector4A16 test21;

  subrefl test22;

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

  esFlags<int8, EnumWrap00> test51[2];
  esFlags<uint16, EnumWrap01> test52[4];

  EnumWrap00 test53[2];
  EnumWrap02 test54[4];
  EnumWrap03 test55[2];
  EnumWrap04 test56[4];

  Vector test57[2];
  Vector2 test58[4];
  Vector4 test59[2];
  Vector4A16 test60[4];

  subrefl test61[2];

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
                 test55, test56, test57, test58, test59, test60, test61, test80)

int test_reflector_bool(reflClass &input) {
  TEST_EQUAL(input.test1, false);
  TEST_EQUAL(input.SetReflectedValue("test1", "true"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test1, true);
  Reflector::KVPair cPair = input.GetReflectedPair(0);
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "true");

  TEST_EQUAL(input.SetReflectedValue("test1", "False"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test1, false);
  cPair = input.GetReflectedPair(0);
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "false");

  TEST_EQUAL(input.SetReflectedValue("test1", "TRUE"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test1, true);
  cPair = input.GetReflectedPair(0);
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "true");

  TEST_EQUAL(input.SetReflectedValue("test1", "tralse"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test1, false);
  cPair = input.GetReflectedPair(0);
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "false");

  TEST_EQUAL(input.SetReflectedValue("test1", "   TrUE   "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test1, true);
  cPair = input.GetReflectedPair(0);
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "true");

  return 0;
}

int test_reflector_int8(reflClass &input) {
  TEST_EQUAL(input.test2, 0);
  TEST_EQUAL(input.SetReflectedValue("test2", "-107"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test2, -107);
  Reflector::KVPair cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "-107");

  TEST_EQUAL(input.SetReflectedValue("test2", "-1070"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test2, -0x80);
  cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "-128");

  TEST_EQUAL(input.SetReflectedValue("test2", "1070"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test2, 0x7f);
  cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "127");

  TEST_EQUAL(input.SetReflectedValue("test2", "\t   107 \t  "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test2, 107);
  cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "107");

  TEST_EQUAL(input.SetReflectedValue("test2", "    107dis    "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test2, 107);
  cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "107");

  TEST_EQUAL(input.SetReflectedValue("test2", "    dis121    "),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test2, 107);
  cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "107");

  TEST_EQUAL(input.SetReflectedValue("test2", "0x7f"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test2, 0x7f);
  cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "127");

  TEST_EQUAL(input.SetReflectedValue("test2", "0x80"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test2, 0x7f);
  cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "127");

  TEST_EQUAL(input.SetReflectedValue("test2", "-0x80"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test2, -0x80);
  cPair = input.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "-128");

  TEST_EQUAL(input.SetReflectedValueInt("test2", 56),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test2, 56);

  TEST_EQUAL(input.SetReflectedValueInt("test2", -107),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test2, -107);

  TEST_EQUAL(input.SetReflectedValueUInt("test2", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test2, -107);

  TEST_EQUAL(input.SetReflectedValueFloat("test2", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test2, -107);

  return 0;
}

int test_reflector_uint8(reflClass &input) {
  TEST_EQUAL(input.test3, 0);
  TEST_EQUAL(input.SetReflectedValue("test3", "157"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test3, 157);
  Reflector::KVPair cPair = input.GetReflectedPair(2);
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "157");

  TEST_EQUAL(input.SetReflectedValue("test3", "1258"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test3, 0xff);
  cPair = input.GetReflectedPair(2);
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "255");

  TEST_EQUAL(input.SetReflectedValue("test3", "-10"),
             Reflector::ErrorType::SignMismatch);
  TEST_EQUAL(input.test3, 246);
  cPair = input.GetReflectedPair(2);
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "246");

  TEST_EQUAL(input.SetReflectedValue("test3", "0x80"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test3, 0x80);
  cPair = input.GetReflectedPair(2);
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "128");

  TEST_EQUAL(input.SetReflectedValue("test3", "-0x80"),
             Reflector::ErrorType::SignMismatch);
  TEST_EQUAL(input.test3, 0x80);
  cPair = input.GetReflectedPair(2);
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "128");

  TEST_EQUAL(input.SetReflectedValueUInt("test3", -107),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test3, uint8(-107));

  TEST_EQUAL(input.SetReflectedValueUInt("test3", 157),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test3, 157);

  TEST_EQUAL(input.SetReflectedValueInt("test3", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test3, 157);

  TEST_EQUAL(input.SetReflectedValueFloat("test3", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test3, 157);

  return 0;
}

int test_reflector_int16(reflClass &input) {
  TEST_EQUAL(input.test4, 0);
  TEST_EQUAL(input.SetReflectedValue("test4", "-1070"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test4, -1070);
  Reflector::KVPair cPair = input.GetReflectedPair(3);
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "-1070");

  TEST_EQUAL(input.SetReflectedValue("test4", "-54987"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test4, -0x8000);
  cPair = input.GetReflectedPair(3);
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "-32768");

  TEST_EQUAL(input.SetReflectedValue("test4", "39641"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test4, 0x7fff);
  cPair = input.GetReflectedPair(3);
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "32767");

  TEST_EQUAL(input.SetReflectedValue("test4", "0x7fc"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test4, 0x7fc);
  cPair = input.GetReflectedPair(3);
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "2044");

  TEST_EQUAL(input.SetReflectedValue("test4", "0x80f2"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test4, 0x7fff);
  cPair = input.GetReflectedPair(3);
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "32767");

  TEST_EQUAL(input.SetReflectedValue("test4", "-0x4048"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test4, -0x4048);
  cPair = input.GetReflectedPair(3);
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "-16456");

  TEST_EQUAL(input.SetReflectedValueInt("test4", 56),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test4, 56);

  TEST_EQUAL(input.SetReflectedValueInt("test4", -1070),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test4, -1070);

  TEST_EQUAL(input.SetReflectedValueUInt("test4", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test4, -1070);

  TEST_EQUAL(input.SetReflectedValueFloat("test4", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test4, -1070);

  return 0;
}

int test_reflector_uint16(reflClass &input) {
  TEST_EQUAL(input.test5, 0);
  TEST_EQUAL(input.SetReflectedValue("test5", "1570"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test5, 1570);
  Reflector::KVPair cPair = input.GetReflectedPair(4);
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "1570");

  TEST_EQUAL(input.SetReflectedValue("test5", "105896"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test5, 0xffff);
  cPair = input.GetReflectedPair(4);
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "65535");

  TEST_EQUAL(input.SetReflectedValue("test5", "-10"),
             Reflector::ErrorType::SignMismatch);
  TEST_EQUAL(input.test5, 65526);
  cPair = input.GetReflectedPair(4);
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "65526");

  TEST_EQUAL(input.SetReflectedValue("test5", "0x8bca"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test5, 35786);
  cPair = input.GetReflectedPair(4);
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "35786");

  TEST_EQUAL(input.SetReflectedValue("test5", "-0x80"),
             Reflector::ErrorType::SignMismatch);
  TEST_EQUAL(input.test5, 65408);
  cPair = input.GetReflectedPair(4);
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "65408");

  TEST_EQUAL(input.SetReflectedValueUInt("test5", -107),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test5, uint16(-107));

  TEST_EQUAL(input.SetReflectedValueUInt("test5", 1570),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test5, 1570);

  TEST_EQUAL(input.SetReflectedValueInt("test5", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test5, 1570);

  TEST_EQUAL(input.SetReflectedValueFloat("test5", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test5, 1570);

  return 0;
}

int test_reflector_int32(reflClass &input) {
  TEST_EQUAL(input.test6, 0);
  TEST_EQUAL(input.SetReflectedValue("test6", "-1586954"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test6, -1586954);
  Reflector::KVPair cPair = input.GetReflectedPair(5);
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "-1586954");

  TEST_EQUAL(input.SetReflectedValue("test6", "-5896321478"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test6, std::numeric_limits<int32>::min());
  cPair = input.GetReflectedPair(5);
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "-2147483648");

  TEST_EQUAL(input.SetReflectedValue("test6", "5269874106"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test6, 0x7fffffff);
  cPair = input.GetReflectedPair(5);
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "2147483647");

  TEST_EQUAL(input.SetReflectedValue("test6", "0x5b6214a9"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test6, 0x5b6214a9);
  cPair = input.GetReflectedPair(5);
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "1533154473");

  TEST_EQUAL(input.SetReflectedValue("test6", "0xab6214a9"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test6, 0x7fffffff);
  cPair = input.GetReflectedPair(5);
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "2147483647");

  TEST_EQUAL(input.SetReflectedValue("test6", "-0x5b6214a9"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test6, -0x5b6214a9);
  cPair = input.GetReflectedPair(5);
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "-1533154473");

  TEST_EQUAL(input.SetReflectedValueInt("test6", 56),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test6, 56);

  TEST_EQUAL(input.SetReflectedValueInt("test6", -1586954),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test6, -1586954);

  TEST_EQUAL(input.SetReflectedValueUInt("test6", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test6, -1586954);

  TEST_EQUAL(input.SetReflectedValueFloat("test6", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test6, -1586954);

  return 0;
}

int test_reflector_uint32(reflClass &input) {
  TEST_EQUAL(input.test7, 0);
  TEST_EQUAL(input.SetReflectedValue("test7", "3896542158"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test7, 3896542158);
  Reflector::KVPair cPair = input.GetReflectedPair(6);
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "3896542158");

  TEST_EQUAL(input.SetReflectedValue("test7", "126987541236"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test7, 0xffffffff);
  cPair = input.GetReflectedPair(6);
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "4294967295");

  TEST_EQUAL(input.SetReflectedValue("test7", "-10"),
             Reflector::ErrorType::SignMismatch);
  TEST_EQUAL(input.test7, 4294967286);
  cPair = input.GetReflectedPair(6);
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "4294967286");

  TEST_EQUAL(input.SetReflectedValue("test7", "0x8bca7854"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test7, 0x8bca7854);
  cPair = input.GetReflectedPair(6);
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "2345302100");

  TEST_EQUAL(input.SetReflectedValue("test7", "-0x80"),
             Reflector::ErrorType::SignMismatch);
  TEST_EQUAL(input.test7, 4294967168);
  cPair = input.GetReflectedPair(6);
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "4294967168");

  TEST_EQUAL(input.SetReflectedValueUInt("test7", -107),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test7, uint32(-107));

  TEST_EQUAL(input.SetReflectedValueUInt("test7", 3896542158),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test7, 3896542158);

  TEST_EQUAL(input.SetReflectedValueInt("test7", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test7, 3896542158);

  TEST_EQUAL(input.SetReflectedValueFloat("test7", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test7, 3896542158);

  return 0;
}

int test_reflector_int64(reflClass &input) {
  TEST_EQUAL(input.test8, 0);
  TEST_EQUAL(input.SetReflectedValue("test8", "-125896354410"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test8, -125896354410);
  Reflector::KVPair cPair = input.GetReflectedPair(7);
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "-125896354410");

  TEST_EQUAL(input.SetReflectedValue("test8", "-58963214786424582492542623145"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test8, std::numeric_limits<int64>::min());
  cPair = input.GetReflectedPair(7);
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "-9223372036854775808");

  TEST_EQUAL(input.SetReflectedValue("test8", "984165254656562566174615456"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test8, 0x7fffffffffffffff);
  cPair = input.GetReflectedPair(7);
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "9223372036854775807");

  TEST_EQUAL(input.SetReflectedValue("test8", "0x59f5772c64b1a785"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test8, 0x59f5772c64b1a785);
  cPair = input.GetReflectedPair(7);
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "6482218271221327749");

  TEST_EQUAL(input.SetReflectedValue("test8", "0xd9f5772c64b1a785"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test8, 0x7fffffffffffffff);
  cPair = input.GetReflectedPair(7);
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "9223372036854775807");

  TEST_EQUAL(input.SetReflectedValue("test8", "-0x59f5772c64b1a785"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test8, -0x59f5772c64b1a785);
  cPair = input.GetReflectedPair(7);
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "-6482218271221327749");

  TEST_EQUAL(input.SetReflectedValueInt("test8", 56),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test8, 56);

  TEST_EQUAL(input.SetReflectedValueInt("test8", -125896354410),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test8, -125896354410);

  TEST_EQUAL(input.SetReflectedValueUInt("test8", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test8, -125896354410);

  TEST_EQUAL(input.SetReflectedValueFloat("test8", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test8, -125896354410);

  return 0;
}

int test_reflector_uint64(reflClass &input) {
  TEST_EQUAL(input.test9, 0);
  TEST_EQUAL(input.SetReflectedValue("test9", "86125863479851"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test9, 86125863479851);
  Reflector::KVPair cPair = input.GetReflectedPair(8);
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "86125863479851");

  TEST_EQUAL(
      input.SetReflectedValue("test9", "81529745125936574564614465245525426"),
      Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test9, 0xffffffffffffffff);
  cPair = input.GetReflectedPair(8);
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "18446744073709551615");

  TEST_EQUAL(input.SetReflectedValue("test9", "-10"),
             Reflector::ErrorType::SignMismatch);
  TEST_EQUAL(input.test9, 18446744073709551606ULL);
  cPair = input.GetReflectedPair(8);
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "18446744073709551606");

  TEST_EQUAL(input.SetReflectedValue("test9", "0x8bca78548514c6a9"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test9, 0x8bca78548514c6a9);
  cPair = input.GetReflectedPair(8);
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "10072995820972852905");

  TEST_EQUAL(input.SetReflectedValue("test9", "-0x80"),
             Reflector::ErrorType::SignMismatch);
  TEST_EQUAL(input.test9, 18446744073709551488ULL);
  cPair = input.GetReflectedPair(8);
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "18446744073709551488");

  TEST_EQUAL(input.SetReflectedValueUInt("test9", -107),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test9, uint64(-107));

  TEST_EQUAL(input.SetReflectedValueUInt("test9", 86125863479851),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test9, 86125863479851);

  TEST_EQUAL(input.SetReflectedValueInt("test9", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test9, 86125863479851);

  TEST_EQUAL(input.SetReflectedValueFloat("test9", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test9, 86125863479851);

  return 0;
}

int test_reflector_float(reflClass &input) {
  TEST_EQUAL(input.test10, 0);
  TEST_EQUAL(input.SetReflectedValue("test10", "1.56"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test10, 1.56f);
  Reflector::KVPair cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "1.56");

  TEST_EQUAL(input.SetReflectedValue("test10", "3.41282347e+38"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test10, FLT_MAX);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "3.40282e+38");

  TEST_EQUAL(input.SetReflectedValue("test10", "-3.41282347e+38"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test10, -FLT_MAX);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "-3.40282e+38");

  TEST_EQUAL(input.SetReflectedValue("test10", "1.10549435e-38F"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test10, FLT_MIN);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "1.17549e-38");

  TEST_EQUAL(input.SetReflectedValue("test10", "-1.10549435e-38F"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test10, -FLT_MIN);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "-1.17549e-38");

  TEST_EQUAL(input.SetReflectedValue("test10", "\t   1.48 \t  "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test10, 1.48f);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "1.48");

  TEST_EQUAL(input.SetReflectedValue("test10", "    5.97dis    "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test10, 5.97f);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "5.97");

  TEST_EQUAL(input.SetReflectedValue("test10", "    dis8.05    "),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test10, 5.97f);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "5.97");

  TEST_EQUAL(input.SetReflectedValue("test10", "NaN"),
             Reflector::ErrorType::None);
  TEST_CHECK(std::isnan(input.test10));
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "nan");

  TEST_EQUAL(input.SetReflectedValue("test10", "-NaN"),
             Reflector::ErrorType::None);
  TEST_CHECK(std::isnan(input.test10));
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_CHECK((cPair.value == "nan" || cPair.value == "-nan"));

  TEST_EQUAL(input.SetReflectedValue("test10", "inf"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test10, INFINITY);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "inf");

  TEST_EQUAL(input.SetReflectedValue("test10", "-inf"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test10, -INFINITY);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "-inf");

  TEST_EQUAL(input.SetReflectedValue("test10", "     inf\t"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test10, INFINITY);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "inf");

  TEST_EQUAL(input.SetReflectedValue("test10", "     infinity&beyond"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test10, INFINITY);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "inf");

  TEST_EQUAL(input.SetReflectedValue("test10", "it's a NaN"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test10, INFINITY);
  cPair = input.GetReflectedPair(9);
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "inf");

  TEST_EQUAL(input.SetReflectedValueFloat("test10", 1.56),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test10, 1.56f);

  TEST_EQUAL(input.SetReflectedValueUInt("test10", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test10, 1.56f);

  TEST_EQUAL(input.SetReflectedValueInt("test10", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test10, 1.56f);

  return 0;
}

int test_reflector_double(reflClass &input) {
  TEST_EQUAL(input.test11, 0);
  TEST_EQUAL(input.SetReflectedValue("test11", "1.567513"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test11, 1.567513);
  Reflector::KVPair cPair = input.GetReflectedPair(10);
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "1.567513");

  TEST_EQUAL(input.SetReflectedValue("test11", "3.41282347e+308F"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test11, DBL_MAX);
  cPair = input.GetReflectedPair(10);
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "1.797693134862e+308");

  TEST_EQUAL(input.SetReflectedValue("test11", "-3.41282347e+308F"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test11, -DBL_MAX);
  cPair = input.GetReflectedPair(10);
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "-1.797693134862e+308");

  TEST_EQUAL(input.SetReflectedValue("test11", "1.10549435e-308F"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test11, DBL_MIN);
  cPair = input.GetReflectedPair(10);
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "2.225073858507e-308");

  TEST_EQUAL(input.SetReflectedValue("test11", "-1.10549435e-308F"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test11, -DBL_MIN);
  cPair = input.GetReflectedPair(10);
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "-2.225073858507e-308");

  TEST_EQUAL(input.SetReflectedValueFloat("test11", 1.567513),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test11, 1.567513);

  TEST_EQUAL(input.SetReflectedValueUInt("test11", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test11, 1.567513);

  TEST_EQUAL(input.SetReflectedValueInt("test11", 100),
             Reflector::ErrorType::InvalidDestination);
  TEST_EQUAL(input.test11, 1.567513);

  return 0;
}

int test_reflector_enum00(reflClass &input) {
  TEST_EQUAL(input.test14, EnumWrap00::E1);
  TEST_EQUAL(input.SetReflectedValue("test14", "E2"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test14, EnumWrap00::E2);
  Reflector::KVPair cPair = input.GetReflectedPair(13);
  TEST_EQUAL(cPair.name, "test14");
  TEST_EQUAL(cPair.value, "E2");

  TEST_EQUAL(input.SetReflectedValue("test14", "  \t E3   "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test14, EnumWrap00::E3);
  cPair = input.GetReflectedPair(13);
  TEST_EQUAL(cPair.name, "test14");
  TEST_EQUAL(cPair.value, "E3");

  TEST_EQUAL(input.SetReflectedValue("test14", "pE2"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test14, EnumWrap00::E3);
  cPair = input.GetReflectedPair(13);
  TEST_EQUAL(cPair.name, "test14");
  TEST_EQUAL(cPair.value, "E3");

  TEST_EQUAL(input.SetReflectedValue("test14", "E25"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test14, EnumWrap00::E3);
  cPair = input.GetReflectedPair(13);
  TEST_EQUAL(cPair.name, "test14");
  TEST_EQUAL(cPair.value, "E3");

  return 0;
}

int test_reflector_enum01(reflClass &input) {
  TEST_EQUAL(input.test15, EnumWrap02::E4);
  TEST_EQUAL(input.SetReflectedValue("test15", "E6"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test15, EnumWrap02::E6);
  Reflector::KVPair cPair = input.GetReflectedPair(14);
  TEST_EQUAL(cPair.name, "test15");
  TEST_EQUAL(cPair.value, "E6");

  TEST_EQUAL(input.SetReflectedValue("test15", "  \t E5   "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test15, EnumWrap02::E5);
  cPair = input.GetReflectedPair(14);
  TEST_EQUAL(cPair.name, "test15");
  TEST_EQUAL(cPair.value, "E5");

  TEST_EQUAL(input.SetReflectedValue("test15", "pE5"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test15, EnumWrap02::E5);
  cPair = input.GetReflectedPair(14);
  TEST_EQUAL(cPair.name, "test15");
  TEST_EQUAL(cPair.value, "E5");

  TEST_EQUAL(input.SetReflectedValue("test15", "E52"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test15, EnumWrap02::E5);
  cPair = input.GetReflectedPair(14);
  TEST_EQUAL(cPair.name, "test15");
  TEST_EQUAL(cPair.value, "E5");

  return 0;
}

int test_reflector_enum02(reflClass &input) {
  TEST_EQUAL(static_cast<int>(input.test16), 0);
  TEST_EQUAL(input.SetReflectedValue("test16", "E7"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test16, EnumWrap03::E7);
  Reflector::KVPair cPair = input.GetReflectedPair(15);
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E7");

  TEST_EQUAL(input.SetReflectedValue("test16", "  \t E8   "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test16, EnumWrap03::E8);
  cPair = input.GetReflectedPair(15);
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E8");

  TEST_EQUAL(input.SetReflectedValue("test16", "pE9"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test16, EnumWrap03::E8);
  cPair = input.GetReflectedPair(15);
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E8");

  TEST_EQUAL(input.SetReflectedValue("test16", "E92"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test16, EnumWrap03::E8);
  cPair = input.GetReflectedPair(15);
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E8");

  TEST_EQUAL(input.SetReflectedValue("test16", "E9"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test16, EnumWrap03::E9);
  cPair = input.GetReflectedPair(15);
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E9");

  return 0;
}

int test_reflector_enum03(reflClass &input) {
  TEST_EQUAL(input.test17, EnumWrap04::E10);
  TEST_EQUAL(input.SetReflectedValue("test17", "E11"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test17, EnumWrap04::E11);
  Reflector::KVPair cPair = input.GetReflectedPair(16);
  TEST_EQUAL(cPair.name, "test17");
  TEST_EQUAL(cPair.value, "E11");

  TEST_EQUAL(input.SetReflectedValue("test17", "  \t E12   "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test17, EnumWrap04::E12);
  cPair = input.GetReflectedPair(16);
  TEST_EQUAL(cPair.name, "test17");
  TEST_EQUAL(cPair.value, "E12");

  TEST_EQUAL(input.SetReflectedValue("test17", "pE129"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test17, EnumWrap04::E12);
  cPair = input.GetReflectedPair(16);
  TEST_EQUAL(cPair.name, "test17");
  TEST_EQUAL(cPair.value, "E12");

  TEST_EQUAL(input.SetReflectedValue("test17", "E102"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test17, EnumWrap04::E12);
  cPair = input.GetReflectedPair(16);
  TEST_EQUAL(cPair.name, "test17");
  TEST_EQUAL(cPair.value, "E12");

  return 0;
}

int test_reflector_enumflags00(reflClass &input) {
  TEST_CHECK(!input.test12[EnumWrap00::E1]);
  TEST_CHECK(!input.test12[EnumWrap00::E2]);
  TEST_CHECK(!input.test12[EnumWrap00::E3]);
  TEST_EQUAL(input.SetReflectedValue("test12", "E1 | E2 | E3"),
             Reflector::ErrorType::None);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  Reflector::KVPair cPair = input.GetReflectedPair(11);
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E2 | E3");

  TEST_EQUAL(input.SetReflectedValue("test12", "\tE1   |   E2 \t|  E3\t   "),
             Reflector::ErrorType::None);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = input.GetReflectedPair(11);
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E2 | E3");

  TEST_EQUAL(input.SetReflectedValue("test12", "E1|E2|E3"),
             Reflector::ErrorType::None);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = input.GetReflectedPair(11);
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E2 | E3");

  TEST_EQUAL(input.SetReflectedValue("test12", "E1 | E2 | "),
             Reflector::ErrorType::EmptyInput);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(input.test12[EnumWrap00::E2]);
  TEST_CHECK(!input.test12[EnumWrap00::E3]);
  cPair = input.GetReflectedPair(11);
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E2");

  TEST_EQUAL(input.SetReflectedValue("test12", "E1 |  | E3"),
             Reflector::ErrorType::EmptyInput);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(!input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = input.GetReflectedPair(11);
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E3");

  TEST_EQUAL(input.SetReflectedValue("test12", " |  | E3"),
             Reflector::ErrorType::EmptyInput);
  TEST_CHECK(!input.test12[EnumWrap00::E1]);
  TEST_CHECK(!input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = input.GetReflectedPair(11);
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E3");

  TEST_EQUAL(input.SetReflectedValue("test12", "E1 | sirius | E3"),
             Reflector::ErrorType::InvalidFormat);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(!input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = input.GetReflectedPair(11);
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E3");

  return 0;
}

int test_reflector_enumflags01(reflClass &input) {
  TEST_CHECK(!input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(!input.test13[EnumWrap01_E3]);
  TEST_EQUAL(input.SetReflectedValue(
                 "test13", "EnumWrap01_E1 | EnumWrap01_E2 | EnumWrap01_E3"),
             Reflector::ErrorType::None);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  Reflector::KVPair cPair = input.GetReflectedPair(12);
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E2 | EnumWrap01_E3");

  TEST_EQUAL(input.SetReflectedValue(
                 "test13",
                 "\tEnumWrap01_E1   |   EnumWrap01_E2 \t|  EnumWrap01_E3\t   "),
             Reflector::ErrorType::None);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = input.GetReflectedPair(12);
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E2 | EnumWrap01_E3");

  TEST_EQUAL(input.SetReflectedValue(
                 "test13", "EnumWrap01_E1|EnumWrap01_E2|EnumWrap01_E3"),
             Reflector::ErrorType::None);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = input.GetReflectedPair(12);
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E2 | EnumWrap01_E3");

  TEST_EQUAL(
      input.SetReflectedValue("test13", "EnumWrap01_E1 | EnumWrap01_E2 | "),
      Reflector::ErrorType::EmptyInput);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(input.test13[EnumWrap01_E2]);
  TEST_CHECK(!input.test13[EnumWrap01_E3]);
  cPair = input.GetReflectedPair(12);
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E2");

  TEST_EQUAL(
      input.SetReflectedValue("test13", "EnumWrap01_E1 |  | EnumWrap01_E3"),
      Reflector::ErrorType::EmptyInput);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = input.GetReflectedPair(12);
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E3");

  TEST_EQUAL(input.SetReflectedValue("test13", " |  | EnumWrap01_E3"),
             Reflector::ErrorType::EmptyInput);
  TEST_CHECK(!input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = input.GetReflectedPair(12);
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E3");

  TEST_EQUAL(input.SetReflectedValue(
                 "test13", "EnumWrap01_E1 | EnumWrap01_ | EnumWrap01_E3"),
             Reflector::ErrorType::InvalidFormat);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = input.GetReflectedPair(12);
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E3");

  TEST_EQUAL(
      input.SetReflectedValue("test13", "EnumWrap01_E1 || EnumWrap01_E3"),
      Reflector::ErrorType::EmptyInput);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = input.GetReflectedPair(12);
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E3");

  return 0;
}

int test_reflector_subclass(reflClass &input) {
  Reflector::KVPair cPair = input.GetReflectedPair(21);
  TEST_EQUAL(cPair.name, "test22");
  TEST_EQUAL(cPair.value, "SUBCLASS_TYPE");
  TEST_CHECK(input.IsReflectedSubClass(21));

  auto rClass = input.GetReflectedSubClass("test22");
  ReflectorSubClass sClass(rClass);
  subrefl &lClass = input.test22;

  TEST_EQUAL(lClass.data0, 0);
  TEST_EQUAL(sClass.SetReflectedValue("data0", "-17845"),
             Reflector::ErrorType::None);
  TEST_EQUAL(lClass.data0, -17845);
  cPair = sClass.GetReflectedPair(0);
  TEST_EQUAL(cPair.name, "data0");
  TEST_EQUAL(cPair.value, "-17845");

  TEST_EQUAL(lClass.data1, 0);
  TEST_EQUAL(sClass.SetReflectedValue("data1", "-1.5975"),
             Reflector::ErrorType::None);
  TEST_EQUAL(lClass.data1, -1.5975f);
  cPair = sClass.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "data1");
  TEST_EQUAL(cPair.value, "-1.5975");

  return 0;
}

int test_reflector_vector(reflClass &input) {
  TEST_EQUAL(input.test18, Vector());
  TEST_EQUAL(input.SetReflectedValue("test18", "[1.5, 2.8, 5.4]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));
  Reflector::KVPair cPair = input.GetReflectedPair(17);
  TEST_EQUAL(cPair.name, "test18");
  TEST_EQUAL(cPair.value, "[1.5, 2.8, 5.4]");

  TEST_EQUAL(input.SetReflectedValue("test18", "[3.5, 2.1, 1.4,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(3.5f, 2.1f, 1.4f));

  TEST_EQUAL(input.SetReflectedValue("test18", "[1.2, 2.1, 5.9, 1.0]"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test18, Vector(1.2f, 2.1f, 5.9f));

  TEST_EQUAL(input.SetReflectedValue("test18", "[1.6, 2.5,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(1.6f, 2.5f, 5.9f));

  TEST_EQUAL(input.SetReflectedValue("test18", "[3.6, 8.5]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(3.6f, 8.5f, 5.9f));

  TEST_EQUAL(
      input.SetReflectedValue(
          "test18", "   [      1.5      ,      2.8      ,      5.4     ]   "),
      Reflector::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));

  TEST_EQUAL(input.SetReflectedValue("test18", "3.5, 2.1, 1.4,]"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));

  TEST_EQUAL(input.SetReflectedValue("test18", "[3.5, 2.1, 1.4,"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));

  TEST_EQUAL(input.SetReflectedValue("test18", "3.5, 2.1, 1.4,"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));

  TEST_EQUAL(input.SetReflectedValue("test18", "[3.5 2.1, 1.4,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(3.5f, 1.4f, 5.4f));

  TEST_EQUAL(input.SetReflectedValue("test18", "[1.2, , 2.1,]"),
             Reflector::ErrorType::ShortInput);
  TEST_EQUAL(input.test18, Vector(1.2f, 1.4f, 5.4f));

  return 0;
}

int test_reflector_vector2(reflClass &input) {
  TEST_EQUAL(input.test19, Vector2());
  TEST_EQUAL(input.SetReflectedValue("test19", "[1.5, 5.4]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(1.5f, 5.4f));
  Reflector::KVPair cPair = input.GetReflectedPair(18);
  TEST_EQUAL(cPair.name, "test19");
  TEST_EQUAL(cPair.value, "[1.5, 5.4]");

  TEST_EQUAL(input.SetReflectedValue("test19", "[3.5, 2.1,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(3.5f, 2.1f));

  TEST_EQUAL(input.SetReflectedValue("test19", "[1.2, 2.1, 5.9]"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test19, Vector2(1.2f, 2.1f));

  TEST_EQUAL(input.SetReflectedValue("test19", "[1.6, 2.5,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(1.6f, 2.5f));

  TEST_EQUAL(input.SetReflectedValue("test19", "[3.6, 8.5]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(3.6f, 8.5f));

  TEST_EQUAL(input.SetReflectedValue("test19",
                                     "   [      1.5      ,      2.8      ]   "),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(1.5f, 2.8f));

  TEST_EQUAL(input.SetReflectedValue("test19", "3.5, 2.1,]"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test19, Vector2(1.5f, 2.8f));

  TEST_EQUAL(input.SetReflectedValue("test19", "[3.5, 2.1,"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test19, Vector2(1.5f, 2.8f));

  TEST_EQUAL(input.SetReflectedValue("test19", "3.5, 2.1,"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test19, Vector2(1.5f, 2.8f));

  TEST_EQUAL(input.SetReflectedValue("test19", "[3.5 2.1,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(3.5f, 2.8f));

  TEST_EQUAL(input.SetReflectedValue("test19", "[1.2, , 2.1,]"),
             Reflector::ErrorType::ShortInput);
  TEST_EQUAL(input.test19, Vector2(1.2f, 2.8f));

  return 0;
}

int test_reflector_vector4(reflClass &input) {
  TEST_EQUAL(input.test20, Vector4());
  TEST_EQUAL(input.SetReflectedValue("test20", "[1.5, 2.8, 5.4, 7.2]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 7.2f));
  Reflector::KVPair cPair = input.GetReflectedPair(19);
  TEST_EQUAL(cPair.name, "test20");
  TEST_EQUAL(cPair.value, "[1.5, 2.8, 5.4, 7.2]");

  TEST_EQUAL(input.SetReflectedValue("test20", "[3.5, 2.1, 1.4, 6.1,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(3.5f, 2.1f, 1.4f, 6.1f));

  TEST_EQUAL(input.SetReflectedValue("test20", "[1.2, 2.1, 5.9, 3.2, 1.0]"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test20, Vector4(1.2f, 2.1f, 5.9f, 3.2f));

  TEST_EQUAL(input.SetReflectedValue("test20", "[1.6, 2.5, 7.3]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(1.6f, 2.5f, 7.3f, 3.2f));

  TEST_EQUAL(input.SetReflectedValue("test20", "[3.6, 8.5, 2.1]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(3.6f, 8.5f, 2.1f, 3.2f));

  TEST_EQUAL(
      input.SetReflectedValue(
          "test20",
          "   [      1.5      ,      2.8      ,      5.4  ,    6.7   ]   "),
      Reflector::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test20", "3.5, 2.1, 1.4,]"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test20", "[3.5, 2.1, 1.4,"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test20", "3.5, 2.1, 1.4,"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test20", "[3.5 2.1, 1.4,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(3.5f, 1.4f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test20", "[1.2, , 2.1,]"),
             Reflector::ErrorType::ShortInput);
  TEST_EQUAL(input.test20, Vector4(1.2f, 1.4f, 5.4f, 6.7f));

  return 0;
}

int test_reflector_vector4A16(reflClass &input) {
  TEST_EQUAL(input.test21, Vector4A16());
  TEST_EQUAL(input.SetReflectedValue("test21", "[1.5, 2.8, 5.4, 7.2]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 7.2f));
  Reflector::KVPair cPair = input.GetReflectedPair(20);
  TEST_EQUAL(cPair.name, "test21");
  TEST_EQUAL(cPair.value, "[1.5, 2.8, 5.4, 7.2]");

  TEST_EQUAL(input.SetReflectedValue("test21", "[3.5, 2.1, 1.4, 6.1,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(3.5f, 2.1f, 1.4f, 6.1f));

  TEST_EQUAL(input.SetReflectedValue("test21", "[1.2, 2.1, 5.9, 3.2, 1.0]"),
             Reflector::ErrorType::OutOfRange);
  TEST_EQUAL(input.test21, Vector4A16(1.2f, 2.1f, 5.9f, 3.2f));

  TEST_EQUAL(input.SetReflectedValue("test21", "[1.6, 2.5, 7.3]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(1.6f, 2.5f, 7.3f, 3.2f));

  TEST_EQUAL(input.SetReflectedValue("test21", "[3.6, 8.5, 2.1]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(3.6f, 8.5f, 2.1f, 3.2f));

  TEST_EQUAL(
      input.SetReflectedValue(
          "test21",
          "   [      1.5      ,      2.8      ,      5.4  ,    6.7   ]   "),
      Reflector::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test21", "3.5, 2.1, 1.4,]"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test21", "[3.5, 2.1, 1.4,"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test21", "3.5, 2.1, 1.4,"),
             Reflector::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test21", "[3.5 2.1, 1.4,]"),
             Reflector::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(3.5f, 1.4f, 5.4f, 6.7f));

  TEST_EQUAL(input.SetReflectedValue("test21", "[1.2, , 2.1,]"),
             Reflector::ErrorType::ShortInput);
  TEST_EQUAL(input.test21, Vector4A16(1.2f, 1.4f, 5.4f, 6.7f));

  return 0;
}

int test_reflector_arrays(reflClass &input) {
  TEST_EQUAL(input.test40[0], false);
  TEST_EQUAL(input.test40[1], false);
  TEST_EQUAL(input.test40[2], false);
  TEST_EQUAL(input.test40[3], false);
  input.SetReflectedValue("test40", "{false, true, true, false}");
  TEST_EQUAL(input.test40[0], false);
  TEST_EQUAL(input.test40[1], true);
  TEST_EQUAL(input.test40[2], true);
  TEST_EQUAL(input.test40[3], false);
  Reflector::KVPair cPair = input.GetReflectedPair(22);
  TEST_EQUAL(cPair.name, "test40");
  TEST_EQUAL(cPair.value, "{false, true, true, false}");

  TEST_EQUAL(input.test41[0], 0);
  TEST_EQUAL(input.test41[1], 0);
  input.SetReflectedValue("test41", "{12, -69}");
  TEST_EQUAL(input.test41[0], 12);
  TEST_EQUAL(input.test41[1], -69);
  cPair = input.GetReflectedPair(23);
  TEST_EQUAL(cPair.name, "test41");
  TEST_EQUAL(cPair.value, "{12, -69}");

  TEST_EQUAL(input.test42[0], 0);
  TEST_EQUAL(input.test42[1], 0);
  TEST_EQUAL(input.test42[2], 0);
  TEST_EQUAL(input.test42[3], 0);
  input.SetReflectedValue("test42", "{128, 57, 251, -75}");
  TEST_EQUAL(input.test42[0], 128);
  TEST_EQUAL(input.test42[1], 57);
  TEST_EQUAL(input.test42[2], 251);
  TEST_EQUAL(input.test42[3], 181);
  cPair = input.GetReflectedPair(24);
  TEST_EQUAL(cPair.name, "test42");
  TEST_EQUAL(cPair.value, "{128, 57, 251, 181}");

  TEST_EQUAL(input.test43[0], 0);
  TEST_EQUAL(input.test43[1], 0);
  input.SetReflectedValue("test43", "{1200, -6956}");
  TEST_EQUAL(input.test43[0], 1200);
  TEST_EQUAL(input.test43[1], -6956);
  cPair = input.GetReflectedPair(25);
  TEST_EQUAL(cPair.name, "test43");
  TEST_EQUAL(cPair.value, "{1200, -6956}");

  TEST_EQUAL(input.test44[0], 0);
  TEST_EQUAL(input.test44[1], 0);
  TEST_EQUAL(input.test44[2], 0);
  TEST_EQUAL(input.test44[3], 0);
  input.SetReflectedValue("test44", "{1286, 572, 2515, -755}");
  TEST_EQUAL(input.test44[0], 1286);
  TEST_EQUAL(input.test44[1], 572);
  TEST_EQUAL(input.test44[2], 2515);
  TEST_EQUAL(input.test44[3], 64781);
  cPair = input.GetReflectedPair(26);
  TEST_EQUAL(cPair.name, "test44");
  TEST_EQUAL(cPair.value, "{1286, 572, 2515, 64781}");

  TEST_EQUAL(input.test45[0], 0);
  TEST_EQUAL(input.test45[1], 0);
  input.SetReflectedValue("test45", "{120053, -695641}");
  TEST_EQUAL(input.test45[0], 120053);
  TEST_EQUAL(input.test45[1], -695641);
  cPair = input.GetReflectedPair(27);
  TEST_EQUAL(cPair.name, "test45");
  TEST_EQUAL(cPair.value, "{120053, -695641}");

  TEST_EQUAL(input.test46[0], 0);
  TEST_EQUAL(input.test46[1], 0);
  TEST_EQUAL(input.test46[2], 0);
  TEST_EQUAL(input.test46[3], 0);
  input.SetReflectedValue("test46", "{128612, 572573, 2515513, -755613}");
  TEST_EQUAL(input.test46[0], 128612);
  TEST_EQUAL(input.test46[1], 572573);
  TEST_EQUAL(input.test46[2], 2515513);
  TEST_EQUAL(input.test46[3], 4294211683);
  cPair = input.GetReflectedPair(28);
  TEST_EQUAL(cPair.name, "test46");
  TEST_EQUAL(cPair.value, "{128612, 572573, 2515513, 4294211683}");

  TEST_EQUAL(input.test47[0], 0);
  TEST_EQUAL(input.test47[1], 0);
  input.SetReflectedValue("test47", "{120053613654541, -6956415461654}");
  TEST_EQUAL(input.test47[0], 120053613654541);
  TEST_EQUAL(input.test47[1], -6956415461654);
  cPair = input.GetReflectedPair(29);
  TEST_EQUAL(cPair.name, "test47");
  TEST_EQUAL(cPair.value, "{120053613654541, -6956415461654}");

  TEST_EQUAL(input.test48[0], 0);
  TEST_EQUAL(input.test48[1], 0);
  TEST_EQUAL(input.test48[2], 0);
  TEST_EQUAL(input.test48[3], 0);
  input.SetReflectedValue(
      "test48",
      "{128612465345, 5725735436343, 2515513435453, -75561345345321}");
  TEST_EQUAL(input.test48[0], 128612465345);
  TEST_EQUAL(input.test48[1], 5725735436343);
  TEST_EQUAL(input.test48[2], 2515513435453);
  TEST_EQUAL(input.test48[3], 18446668512364206295ULL);
  cPair = input.GetReflectedPair(30);
  TEST_EQUAL(cPair.name, "test48");
  TEST_EQUAL(
      cPair.value,
      "{128612465345, 5725735436343, 2515513435453, 18446668512364206295}");

  TEST_EQUAL(input.test49[0], 0);
  TEST_EQUAL(input.test49[1], 0);
  input.SetReflectedValue("test49", "{1.5, -3.6}");
  TEST_EQUAL(input.test49[0], 1.5f);
  TEST_EQUAL(input.test49[1], -3.6f);
  cPair = input.GetReflectedPair(31);
  TEST_EQUAL(cPair.name, "test49");
  TEST_EQUAL(cPair.value, "{1.5, -3.6}");

  TEST_EQUAL(input.test50[0], 0);
  TEST_EQUAL(input.test50[1], 0);
  TEST_EQUAL(input.test50[2], 0);
  TEST_EQUAL(input.test50[3], 0);
  input.SetReflectedValue("test50", "{1.536, 9.861, 6.45521, -7.32123}");
  TEST_EQUAL(input.test50[0], 1.536);
  TEST_EQUAL(input.test50[1], 9.861);
  TEST_EQUAL(input.test50[2], 6.45521);
  TEST_EQUAL(input.test50[3], -7.32123);
  cPair = input.GetReflectedPair(32);
  TEST_EQUAL(cPair.name, "test50");
  TEST_EQUAL(cPair.value, "{1.536, 9.861, 6.45521, -7.32123}");

  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E1);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E3);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E1);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E3);
  input.SetReflectedValue("test51", "{E1 | E3, E2}");
  TEST_EQUAL(input.test51[0], EnumWrap00::E1);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E2);
  TEST_EQUAL(input.test51[0], EnumWrap00::E3);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E1);
  TEST_EQUAL(input.test51[1], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E3);
  cPair = input.GetReflectedPair(33);
  TEST_EQUAL(cPair.name, "test51");
  TEST_EQUAL(cPair.value, "{E1 | E3, E2}");

  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E3);
  input.SetReflectedValue("test52",
                          "{EnumWrap01_E1 | EnumWrap01_E3, EnumWrap01_E2, "
                          "EnumWrap01_E2 | EnumWrap01_E3}");
  TEST_EQUAL(input.test52[0], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E2);
  TEST_EQUAL(input.test52[0], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E1);
  TEST_EQUAL(input.test52[1], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E1);
  TEST_EQUAL(input.test52[2], EnumWrap01_E2);
  TEST_EQUAL(input.test52[2], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E3);
  cPair = input.GetReflectedPair(34);
  TEST_EQUAL(cPair.name, "test52");
  TEST_EQUAL(cPair.value, "{EnumWrap01_E1 | EnumWrap01_E3, EnumWrap01_E2, "
                          "EnumWrap01_E2 | EnumWrap01_E3, NULL}");

  TEST_EQUAL(input.test53[0], EnumWrap00::E1);
  TEST_EQUAL(input.test53[1], EnumWrap00::E1);
  input.SetReflectedValue("test53", "{E2, E1}");
  TEST_EQUAL(input.test53[0], EnumWrap00::E2);
  TEST_EQUAL(input.test53[1], EnumWrap00::E1);
  cPair = input.GetReflectedPair(35);
  TEST_EQUAL(cPair.name, "test53");
  TEST_EQUAL(cPair.value, "{E2, E1}");

  TEST_EQUAL(input.test54[0], EnumWrap02::E4);
  TEST_EQUAL(input.test54[1], EnumWrap02::E4);
  TEST_EQUAL(input.test54[2], EnumWrap02::E4);
  TEST_EQUAL(input.test54[3], EnumWrap02::E4);
  input.SetReflectedValue("test54", "{E5, E6, E4, E5}");
  TEST_EQUAL(input.test54[0], EnumWrap02::E5);
  TEST_EQUAL(input.test54[1], EnumWrap02::E6);
  TEST_EQUAL(input.test54[2], EnumWrap02::E4);
  TEST_EQUAL(input.test54[3], EnumWrap02::E5);
  cPair = input.GetReflectedPair(36);
  TEST_EQUAL(cPair.name, "test54");
  TEST_EQUAL(cPair.value, "{E5, E6, E4, E5}");

  TEST_EQUAL(int(input.test55[0]), 0);
  TEST_EQUAL(int(input.test55[1]), 0);
  input.SetReflectedValue("test55", "{E9, E8}");
  TEST_EQUAL(input.test55[0], EnumWrap03::E9);
  TEST_EQUAL(input.test55[1], EnumWrap03::E8);
  cPair = input.GetReflectedPair(37);
  TEST_EQUAL(cPair.name, "test55");
  TEST_EQUAL(cPair.value, "{E9, E8}");

  TEST_EQUAL(input.test56[0], EnumWrap04::E10);
  TEST_EQUAL(input.test56[1], EnumWrap04::E10);
  TEST_EQUAL(input.test56[2], EnumWrap04::E10);
  TEST_EQUAL(input.test56[3], EnumWrap04::E10);
  input.SetReflectedValue("test56", "{E11, E12, E10, E12}");
  TEST_EQUAL(input.test56[0], EnumWrap04::E11);
  TEST_EQUAL(input.test56[1], EnumWrap04::E12);
  TEST_EQUAL(input.test56[2], EnumWrap04::E10);
  TEST_EQUAL(input.test56[3], EnumWrap04::E12);
  cPair = input.GetReflectedPair(38);
  TEST_EQUAL(cPair.name, "test56");
  TEST_EQUAL(cPair.value, "{E11, E12, E10, E12}");

  TEST_EQUAL(input.test57[0], Vector());
  TEST_EQUAL(input.test57[1], Vector());
  input.SetReflectedValue("test57", "{[5.6, 51.6, 31.7], [18.1, 3.5, 6.1]}");
  TEST_EQUAL(input.test57[0], Vector(5.6f, 51.6f, 31.7f));
  TEST_EQUAL(input.test57[1], Vector(18.1f, 3.5f, 6.1f));
  cPair = input.GetReflectedPair(39);
  TEST_EQUAL(cPair.name, "test57");
  TEST_EQUAL(cPair.value, "{[5.6, 51.6, 31.7], [18.1, 3.5, 6.1]}");

  TEST_EQUAL(input.test58[0], Vector2());
  TEST_EQUAL(input.test58[1], Vector2());
  TEST_EQUAL(input.test58[2], Vector2());
  TEST_EQUAL(input.test58[3], Vector2());
  input.SetReflectedValue(
      "test58", "{[5.6, 51.6], [31.7, 18.1], [3.5, 6.1], [7.5, 62.1]}");
  TEST_EQUAL(input.test58[0], Vector2(5.6f, 51.6f));
  TEST_EQUAL(input.test58[1], Vector2(31.7f, 18.1f));
  TEST_EQUAL(input.test58[2], Vector2(3.5f, 6.1f));
  TEST_EQUAL(input.test58[3], Vector2(7.5f, 62.1f));
  cPair = input.GetReflectedPair(40);
  TEST_EQUAL(cPair.name, "test58");
  TEST_EQUAL(cPair.value,
             "{[5.6, 51.6], [31.7, 18.1], [3.5, 6.1], [7.5, 62.1]}");

  TEST_EQUAL(input.test59[0], Vector4());
  TEST_EQUAL(input.test59[1], Vector4());
  input.SetReflectedValue("test59",
                          "{[5.6, 51.6, 31.7, 18.1], [3.5, 6.1, 7.5, 62.1]}");
  TEST_EQUAL(input.test59[0], Vector4(5.6f, 51.6f, 31.7f, 18.1f));
  TEST_EQUAL(input.test59[1], Vector4(3.5f, 6.1f, 7.5f, 62.1f));
  cPair = input.GetReflectedPair(41);
  TEST_EQUAL(cPair.name, "test59");
  TEST_EQUAL(cPair.value, "{[5.6, 51.6, 31.7, 18.1], [3.5, 6.1, 7.5, 62.1]}");

  TEST_EQUAL(input.test60[0], Vector4A16());
  TEST_EQUAL(input.test60[1], Vector4A16());
  TEST_EQUAL(input.test60[2], Vector4A16());
  TEST_EQUAL(input.test60[3], Vector4A16());
  input.SetReflectedValue("test60",
                          "{[5.6, 51.6, 31.7, 18.1], [3.5, 6.1, 7.5, 62.1], "
                          "[1.8, 57.5, 36.9, 5.78], [7.41, 5.8, 41.8, 6.12]}");
  TEST_EQUAL(input.test60[0], Vector4A16(5.6f, 51.6f, 31.7f, 18.1f));
  TEST_EQUAL(input.test60[1], Vector4A16(3.5f, 6.1f, 7.5f, 62.1f));
  TEST_EQUAL(input.test60[2], Vector4A16(1.8f, 57.5f, 36.9f, 5.78f));
  TEST_EQUAL(input.test60[3], Vector4A16(7.41f, 5.8f, 41.8f, 6.12f));
  cPair = input.GetReflectedPair(42);
  TEST_EQUAL(cPair.name, "test60");
  TEST_EQUAL(cPair.value, "{[5.6, 51.6, 31.7, 18.1], [3.5, 6.1, 7.5, 62.1], "
                          "[1.8, 57.5, 36.9, 5.78], [7.41, 5.8, 41.8, 6.12]}");

  return 0;
}

int test_reflector_array_subclass(reflClass &input) {
  Reflector::KVPair cPair = input.GetReflectedPair(43);
  TEST_EQUAL(cPair.name, "test61");
  TEST_EQUAL(cPair.value, "{SUBCLASS_TYPE, SUBCLASS_TYPE}");
  TEST_CHECK(input.IsReflectedSubClass("test61"));
  TEST_CHECK(input.IsArray("test61"));

  auto rClass = input.GetReflectedSubClass("test61");
  ReflectorSubClass sClass(rClass);
  subrefl &lClass = input.test61[0];

  TEST_EQUAL(lClass.data0, 0);
  TEST_EQUAL(sClass.SetReflectedValue("data0", "-1451"),
             Reflector::ErrorType::None);
  TEST_EQUAL(lClass.data0, -1451);
  cPair = sClass.GetReflectedPair(0);
  TEST_EQUAL(cPair.name, "data0");
  TEST_EQUAL(cPair.value, "-1451");

  TEST_EQUAL(lClass.data1, 0);
  TEST_EQUAL(sClass.SetReflectedValue("data1", "81.65"),
             Reflector::ErrorType::None);
  TEST_EQUAL(lClass.data1, 81.65f);
  cPair = sClass.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "data1");
  TEST_EQUAL(cPair.value, "81.65");

  auto rClass2 = input.GetReflectedSubClass("test61", 1);
  ReflectorSubClass sClass2(rClass2);
  subrefl &lClass2 = input.test61[1];

  TEST_EQUAL(lClass2.data0, 0);
  TEST_EQUAL(sClass2.SetReflectedValue("data0", "9841"),
             Reflector::ErrorType::None);
  TEST_EQUAL(lClass2.data0, 9841);
  cPair = sClass2.GetReflectedPair(0);
  TEST_EQUAL(cPair.name, "data0");
  TEST_EQUAL(cPair.value, "9841");

  TEST_EQUAL(lClass2.data1, 0);
  TEST_EQUAL(sClass2.SetReflectedValue("data1", "5.874"),
             Reflector::ErrorType::None);
  TEST_EQUAL(lClass2.data1, 5.874f);
  cPair = sClass2.GetReflectedPair(1);
  TEST_EQUAL(cPair.name, "data1");
  TEST_EQUAL(cPair.value, "5.874");

  return 0;
}

int test_reflector_string(reflClass &input) {
  TEST_CHECK(input.test80.empty());
  TEST_EQUAL(input.SetReflectedValue("test80", "This is a test string"), Reflector::ErrorType::None);
  TEST_EQUAL(input.test80, "This is a test string");
  Reflector::KVPair cPair = input.GetReflectedPair(44);
  TEST_EQUAL(cPair.name, "test80");
  TEST_EQUAL(cPair.value, "This is a test string");

  return 0;
}

int test_reflector(reflClass &input) {
  TEST_CHECK(input.UseNames());
  TEST_EQUAL(input.GetClassName(), es::string_view("reflClass"));
  TEST_EQUAL(input.SetReflectedValue("pest", ""),
             Reflector::ErrorType::InvalidDestination);

  return 0;
}

reflClass ReflectorTest() {
  reflClass test = {};
  test.SetReflectedValue("test22", "test string");
  return test;
}