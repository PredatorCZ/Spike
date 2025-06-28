#pragma once
#include "reflector_def.inl"
#include "spike/util/unit_testing.hpp"

using namespace es::jenhash_literals;
using namespace std::string_view_literals;

static_assert(sizeof(EnumWrap02) == 1);
static_assert(sizeof(EnumWrap03) == 2);
static_assert(sizeof(EnumWrap04) == 4);

int test_reflector_enum00() {
  auto rEnum = GetReflectedEnum<EnumWrap00>();

  TEST_EQUAL(rEnum->enumHash, "EnumWrap00"_jh);
  TEST_EQUAL(rEnum->numMembers, 3);
  TEST_EQUAL(rEnum->enumName, "EnumWrap00"sv);
  TEST_EQUAL(rEnum->descriptions, nullptr);

  static const char *names[] = {"E1", "E2", "E3"};
  static const uint64 ids[] = {0, 1, 7};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum->names[t], names[t]);
    TEST_EQUAL(rEnum->values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum01() {
  auto rEnum = GetReflectedEnum<EnumWrap01>();

  TEST_EQUAL(rEnum->enumHash, "EnumWrap01"_jh);
  TEST_EQUAL(rEnum->numMembers, 3);
  TEST_EQUAL(rEnum->enumName, "EnumWrap01"sv);
  TEST_EQUAL(rEnum->descriptions, nullptr);

  static const char *names[] = {"EnumWrap01_E1", "EnumWrap01_E2",
                                "EnumWrap01_E3"};
  static const uint64 ids[] = {0, 1, 2};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum->names[t], names[t]);
    TEST_EQUAL(rEnum->values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum02() {
  auto rEnum = GetReflectedEnum<EnumWrap02>();

  TEST_EQUAL(rEnum->enumHash, "EnumWrap02"_jh);
  TEST_EQUAL(rEnum->numMembers, 3);
  TEST_EQUAL(rEnum->enumName, "EnumWrap02"sv);
  TEST_EQUAL(rEnum->descriptions, nullptr);

  static const char *names[] = {"E4", "E5", "E6"};
  static const uint64 ids[] = {0, 1, 2};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum->names[t], names[t]);
    TEST_EQUAL(rEnum->values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum03() {
  auto rEnum = GetReflectedEnum<EnumWrap03>();

  TEST_EQUAL(rEnum->enumHash, "EnumWrap03"_jh);
  TEST_EQUAL(rEnum->numMembers, 3);
  TEST_EQUAL(rEnum->enumName, "EnumWrap03"sv);
  TEST_EQUAL(rEnum->descriptions, nullptr);

  static const char *names[] = {"E7", "E8", "E9"};
  static const uint64 ids[] = {7, 16586, 0x8bcd};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum->names[t], names[t]);
    TEST_EQUAL(rEnum->values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum04() {
  auto rEnum = GetReflectedEnum<EnumWrap04>();

  TEST_EQUAL(rEnum->enumHash, "EnumWrap04"_jh);
  TEST_EQUAL(rEnum->numMembers, 3);
  TEST_EQUAL(rEnum->enumName, "EnumWrap04"sv);
  TEST_EQUAL(rEnum->descriptions, nullptr);

  static const char *names[] = {"E10", "E11", "E12"};
  static const uint64 ids[] = {0, 1, 2};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum->names[t], names[t]);
    TEST_EQUAL(rEnum->values[t], ids[t]);
  }

  return 0;
}

int test_reflector_enum05() {
  auto rEnum = GetReflectedEnum<EnumType>();

  TEST_EQUAL(rEnum->enumHash, "EnumType"_jh);
  TEST_EQUAL(rEnum->numMembers, 3);
  TEST_EQUAL(rEnum->enumName, "EnumType"sv);
  TEST_NOT_EQUAL(rEnum->descriptions, nullptr);

  static const char *names[] = {"Type1", "Type2", "Type3"};
  static const uint64 ids[] = {0, 1, 0};
  static const char *descs[] = {"Type 1 of EnumType", "Type 2 of EnumType",
                                "Type 3 is same as Type 1"};

  for (int t = 0; t < 3; t++) {
    TEST_EQUAL(rEnum->names[t], names[t]);
    TEST_EQUAL(rEnum->values[t], ids[t]);
    TEST_EQUAL(rEnum->descriptions[t], descs[t]);
  }

  return 0;
}

int test_reflector_bool(reflClass &input) {
  TEST_EQUAL(input.test1, false);
  ReflectorMember member = input["test1"];
  TEST_EQUAL(member.ReflectValue("true"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test1, true);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "true");

  TEST_EQUAL(member.ReflectValue("False"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test1, false);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "false");

  TEST_EQUAL(member.ReflectValue("TRUE"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test1, true);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "true");

  TEST_EQUAL(member.ReflectValue("tralse"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test1, false);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "false");

  TEST_EQUAL(member.ReflectValue("   TrUE   "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test1, true);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test1");
  TEST_EQUAL(cPair.value, "true");

  return 0;
}

int test_reflector_int8(reflClass &input) {
  TEST_EQUAL(input.test2, 0);
  ReflectorMember member = input["test2"];
  TEST_EQUAL(member.ReflectValue("-107"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, -107);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "-107");

  TEST_EQUAL(member.ReflectValue("-1070"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test2, -0x80);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "-128");

  TEST_EQUAL(member.ReflectValue("1070"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test2, 0x7f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "127");

  TEST_EQUAL(member.ReflectValue("\t   107 \t  "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, 107);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "107");

  TEST_EQUAL(member.ReflectValue("    107dis    "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, 107);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "107");

  TEST_EQUAL(member.ReflectValue("    dis121    "),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test2, 107);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "107");

  TEST_EQUAL(member.ReflectValue("0x7f"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, 0x7f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "127");

  TEST_EQUAL(member.ReflectValue("0x80"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test2, 0x7f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "127");

  TEST_EQUAL(member.ReflectValue("-0x80"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, -0x80);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test2");
  TEST_EQUAL(cPair.value, "-128");

  TEST_EQUAL(member.ReflectValue(56), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, 56);

  TEST_EQUAL(member.ReflectValue(-107), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, -107);

  TEST_EQUAL(member.ReflectValue(100ul), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, 100);

  TEST_EQUAL(member.ReflectValue(101ul), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test2, 101);

  return 0;
}

int test_reflector_uint8(reflClass &input) {
  TEST_EQUAL(input.test3, 0);
  ReflectorMember member = input["test3"];
  TEST_EQUAL(member.ReflectValue("157"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test3, 157);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "157");

  TEST_EQUAL(member.ReflectValue("1258"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test3, 0xff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "255");

  TEST_EQUAL(member.ReflectValue("-10"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(input.test3, 246);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "246");

  TEST_EQUAL(member.ReflectValue("0x80"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test3, 0x80);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "128");

  TEST_EQUAL(member.ReflectValue("-0x80"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(input.test3, 0x80);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test3");
  TEST_EQUAL(cPair.value, "128");

  TEST_EQUAL(member.ReflectValue(uint64(-107)),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test3, uint8(-107));

  TEST_EQUAL(member.ReflectValue(157), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test3, 157);

  TEST_EQUAL(member.ReflectValue(100ul), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test3, 100);

  TEST_EQUAL(member.ReflectValue(101ul), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test3, 101);

  return 0;
}

int test_reflector_int16(reflClass &input) {
  TEST_EQUAL(input.test4, 0);
  ReflectorMember member = input["test4"];
  TEST_EQUAL(member.ReflectValue("-1070"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test4, -1070);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "-1070");

  TEST_EQUAL(member.ReflectValue("-54987"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test4, -0x8000);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "-32768");

  TEST_EQUAL(member.ReflectValue("39641"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test4, 0x7fff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "32767");

  TEST_EQUAL(member.ReflectValue("0x7fc"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test4, 0x7fc);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "2044");

  TEST_EQUAL(member.ReflectValue("0x80f2"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test4, 0x7fff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "32767");

  TEST_EQUAL(member.ReflectValue("-0x4048"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test4, -0x4048);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test4");
  TEST_EQUAL(cPair.value, "-16456");

  TEST_EQUAL(member.ReflectValue(56), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test4, 56);

  TEST_EQUAL(member.ReflectValue(-1070), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test4, -1070);

  TEST_EQUAL(member.ReflectValue(-100ul), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test4, int16(65436));

  TEST_EQUAL(member.ReflectValue(-101.58f), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test4, -101);

  return 0;
}

int test_reflector_uint16(reflClass &input) {
  TEST_EQUAL(input.test5, 0);
  ReflectorMember member = input["test5"];
  TEST_EQUAL(member.ReflectValue("1570"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test5, 1570);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "1570");

  TEST_EQUAL(member.ReflectValue("105896"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test5, 0xffff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "65535");

  TEST_EQUAL(member.ReflectValue("-10"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(input.test5, 65526);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "65526");

  TEST_EQUAL(member.ReflectValue("0x8bca"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test5, 35786);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "35786");

  TEST_EQUAL(member.ReflectValue("-0x80"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(input.test5, 65408);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test5");
  TEST_EQUAL(cPair.value, "65408");

  TEST_EQUAL(member.ReflectValue(uint64(-107)),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test5, uint16(-107));

  TEST_EQUAL(member.ReflectValue(1570), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test5, 1570);

  TEST_EQUAL(member.ReflectValue(-100), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test5, uint16(-100));

  TEST_EQUAL(member.ReflectValue(-105.56), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test5, uint16(-105));

  return 0;
}

int test_reflector_int32(reflClass &input) {
  TEST_EQUAL(input.test6, 0);
  ReflectorMember member = input["test6"];
  TEST_EQUAL(member.ReflectValue("-1586954"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test6, -1586954);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "-1586954");

  TEST_EQUAL(member.ReflectValue("-5896321478"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test6, std::numeric_limits<int32>::min());
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "-2147483648");

  TEST_EQUAL(member.ReflectValue("5269874106"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test6, 0x7fffffff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "2147483647");

  TEST_EQUAL(member.ReflectValue("0x5b6214a9"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test6, 0x5b6214a9);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "1533154473");

  TEST_EQUAL(member.ReflectValue("0xab6214a9"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test6, 0x7fffffff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "2147483647");

  TEST_EQUAL(member.ReflectValue("-0x5b6214a9"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test6, -0x5b6214a9);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test6");
  TEST_EQUAL(cPair.value, "-1533154473");

  TEST_EQUAL(member.ReflectValue(56), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test6, 56);

  TEST_EQUAL(member.ReflectValue(-1586954), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test6, -1586954);

  TEST_EQUAL(member.ReflectValue(-1586955ul), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test6, -1586955);

  TEST_EQUAL(member.ReflectValue(100.5), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test6, 100);

  return 0;
}

int test_reflector_uint32(reflClass &input) {
  TEST_EQUAL(input.test7, 0);
  ReflectorMember member = input["test7"];
  TEST_EQUAL(member.ReflectValue("3896542158"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test7, 3896542158);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "3896542158");

  TEST_EQUAL(member.ReflectValue("126987541236"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test7, 0xffffffff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "4294967295");

  TEST_EQUAL(member.ReflectValue("-10"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(input.test7, 4294967286);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "4294967286");

  TEST_EQUAL(member.ReflectValue("0x8bca7854"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test7, 0x8bca7854);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "2345302100");

  TEST_EQUAL(member.ReflectValue("-0x80"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(input.test7, 4294967168);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test7");
  TEST_EQUAL(cPair.value, "4294967168");

  TEST_EQUAL(member.ReflectValue(uint64(-107)),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test7, uint32(-107));

  TEST_EQUAL(member.ReflectValue(3896542158), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test7, 3896542158);

  TEST_EQUAL(member.ReflectValue(-100ul), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test7, uint32(-100));

  TEST_EQUAL(member.ReflectValue(-115.58), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test7, uint32(-115));

  return 0;
}

int test_reflector_int64(reflClass &input) {
  TEST_EQUAL(input.test8, 0);
  ReflectorMember member = input["test8"];
  TEST_EQUAL(member.ReflectValue("-125896354410"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test8, -125896354410);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "-125896354410");

  TEST_EQUAL(member.ReflectValue("-58963214786424582492542623145"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test8, std::numeric_limits<int64>::min());
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "-9223372036854775808");

  TEST_EQUAL(member.ReflectValue("984165254656562566174615456"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test8, 0x7fffffffffffffff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "9223372036854775807");

  TEST_EQUAL(member.ReflectValue("0x59f5772c64b1a785"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test8, 0x59f5772c64b1a785);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "6482218271221327749");

  TEST_EQUAL(member.ReflectValue("0xd9f5772c64b1a785"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test8, 0x7fffffffffffffff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "9223372036854775807");

  TEST_EQUAL(member.ReflectValue("-0x59f5772c64b1a785"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test8, -0x59f5772c64b1a785);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test8");
  TEST_EQUAL(cPair.value, "-6482218271221327749");

  TEST_EQUAL(member.ReflectValue(56), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test8, 56);

  TEST_EQUAL(member.ReflectValue(-125896354410),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test8, -125896354410);

  TEST_EQUAL(member.ReflectValue(-100ull), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test8, -100);

  TEST_EQUAL(member.ReflectValue(-108.6f), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test8, -108);

  return 0;
}

int test_reflector_uint64(reflClass &input) {
  TEST_EQUAL(input.test9, 0);
  ReflectorMember member = input["test9"];
  TEST_EQUAL(member.ReflectValue("86125863479851"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test9, 86125863479851);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "86125863479851");

  TEST_EQUAL(member.ReflectValue("81529745125936574564614465245525426"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test9, 0xffffffffffffffff);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "18446744073709551615");

  TEST_EQUAL(member.ReflectValue("-10"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(input.test9, 18446744073709551606ULL);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "18446744073709551606");

  TEST_EQUAL(member.ReflectValue("0x8bca78548514c6a9"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test9, 0x8bca78548514c6a9);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "10072995820972852905");

  TEST_EQUAL(member.ReflectValue("-0x80"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(input.test9, 18446744073709551488ULL);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test9");
  TEST_EQUAL(cPair.value, "18446744073709551488");

  TEST_EQUAL(member.ReflectValue(uint64(-107)),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test9, uint64(-107));

  TEST_EQUAL(member.ReflectValue(86125863479851),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test9, 86125863479851);

  TEST_EQUAL(member.ReflectValue(-120), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test9, -120ull);

  TEST_EQUAL(member.ReflectValue(100058.654), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test9, 100058);

  return 0;
}

int test_reflector_float(reflClass &input) {
  TEST_EQUAL(input.test10, 0);
  ReflectorMember member = input["test10"];
  TEST_EQUAL(member.ReflectValue("1.56"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, 1.56f);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "1.56");

  TEST_EQUAL(member.ReflectValue("3.41282347e+38"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test10, FLT_MAX);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  if (cPair.value != "3.40282e+38") {
    TEST_EQUAL(cPair.value, "3.40282e+038");
  }
  TEST_EQUAL(member.ReflectValue("-3.41282347e+38"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test10, -FLT_MAX);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  if (cPair.value != "-3.40282e+38") {
    TEST_EQUAL(cPair.value, "-3.40282e+038");
  }

  TEST_EQUAL(member.ReflectValue("1.10549435e-38F"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test10, FLT_MIN);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  if (cPair.value != "1.17549e-38") {
    TEST_EQUAL(cPair.value, "1.17549e-038");
  }

  TEST_EQUAL(member.ReflectValue("-1.10549435e-38F"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test10, -FLT_MIN);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  if (cPair.value != "-1.17549e-38") {
    TEST_EQUAL(cPair.value, "-1.17549e-038");
  }

  TEST_EQUAL(member.ReflectValue("\t   1.48 \t  "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, 1.48f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "1.48");

  TEST_EQUAL(member.ReflectValue("    5.97dis    "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, 5.97f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "5.97");

  TEST_EQUAL(member.ReflectValue("    dis8.05    "),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test10, 5.97f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "5.97");

  TEST_EQUAL(member.ReflectValue("NaN"), ReflectorMember::ErrorType::None);
  TEST_CHECK(std::isnan(input.test10));
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "nan");

  TEST_EQUAL(member.ReflectValue("-NaN"), ReflectorMember::ErrorType::None);
  TEST_CHECK(std::isnan(input.test10));
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_CHECK((cPair.value == "nan" || cPair.value == "-nan"));

  TEST_EQUAL(member.ReflectValue("inf"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, INFINITY);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "inf");

  TEST_EQUAL(member.ReflectValue("-inf"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, -INFINITY);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "-inf");

  TEST_EQUAL(member.ReflectValue("     inf\t"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, INFINITY);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "inf");

  TEST_EQUAL(member.ReflectValue("     infinity&beyond"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, INFINITY);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "inf");

  TEST_EQUAL(member.ReflectValue("it's a NaN"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test10, INFINITY);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test10");
  TEST_EQUAL(cPair.value, "inf");

  TEST_EQUAL(member.ReflectValue(1.56), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, 1.56f);

  TEST_EQUAL(member.ReflectValue(1528), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test10, 1528.f);

  TEST_EQUAL(member.ReflectValue(-8745ull), ReflectorMember::ErrorType::None);
  TEST_GT(input.test10, 1.84467e+19);

  input.test10 = 1578.5f;

  return 0;
}

int test_reflector_double(reflClass &input) {
  TEST_EQUAL(input.test11, 0);
  ReflectorMember member = input["test11"];
  TEST_EQUAL(member.ReflectValue("1.567513"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test11, 1.567513);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "1.567513");

  TEST_EQUAL(member.ReflectValue("3.41282347e+308F"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test11, DBL_MAX);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "1.797693134862e+308");

  TEST_EQUAL(member.ReflectValue("-3.41282347e+308F"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test11, -DBL_MAX);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "-1.797693134862e+308");

  TEST_EQUAL(member.ReflectValue("1.10549435e-308F"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test11, DBL_MIN);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "2.225073858507e-308");

  TEST_EQUAL(member.ReflectValue("-1.10549435e-308F"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test11, -DBL_MIN);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test11");
  TEST_EQUAL(cPair.value, "-2.225073858507e-308");

  TEST_EQUAL(member.ReflectValue(1.567513), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test11, 1.567513);

  TEST_EQUAL(member.ReflectValue(-589ull), ReflectorMember::ErrorType::None);
  TEST_GT(input.test11, 1.84467e+19);

  TEST_EQUAL(member.ReflectValue(567), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test11, 567);

  return 0;
}

int test_reflector_enum00(reflClass &input) {
  TEST_EQUAL(input.test14, EnumWrap00::E1);
  ReflectorMember member = input["test14"];
  TEST_EQUAL(member.ReflectValue("E2"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test14, EnumWrap00::E2);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test14");
  TEST_EQUAL(cPair.value, "E2");

  TEST_EQUAL(member.ReflectValue("  \t E3   "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test14, EnumWrap00::E3);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test14");
  TEST_EQUAL(cPair.value, "E3");

  TEST_EQUAL(member.ReflectValue("pE2"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test14, EnumWrap00::E3);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test14");
  TEST_EQUAL(cPair.value, "E3");

  TEST_EQUAL(member.ReflectValue("E25"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test14, EnumWrap00::E3);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test14");
  TEST_EQUAL(cPair.value, "E3");

  return 0;
}

int test_reflector_enum01(reflClass &input) {
  TEST_EQUAL(input.test15, EnumWrap02::E4);
  ReflectorMember member = input["test15"];
  TEST_EQUAL(member.ReflectValue("E6"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test15, EnumWrap02::E6);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test15");
  TEST_EQUAL(cPair.value, "E6");

  TEST_EQUAL(member.ReflectValue("  \t E5   "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test15, EnumWrap02::E5);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test15");
  TEST_EQUAL(cPair.value, "E5");

  TEST_EQUAL(member.ReflectValue("pE5"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test15, EnumWrap02::E5);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test15");
  TEST_EQUAL(cPair.value, "E5");

  TEST_EQUAL(member.ReflectValue("E52"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test15, EnumWrap02::E5);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test15");
  TEST_EQUAL(cPair.value, "E5");

  return 0;
}

int test_reflector_enum02(reflClass &input) {
  TEST_EQUAL(static_cast<int>(input.test16), 0);
  ReflectorMember member = input["test16"];
  TEST_EQUAL(member.ReflectValue("E7"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test16, EnumWrap03::E7);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E7");

  TEST_EQUAL(member.ReflectValue("  \t E8   "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test16, EnumWrap03::E8);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E8");

  TEST_EQUAL(member.ReflectValue("pE9"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test16, EnumWrap03::E8);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E8");

  TEST_EQUAL(member.ReflectValue("E92"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test16, EnumWrap03::E8);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E8");

  TEST_EQUAL(member.ReflectValue("E9"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test16, EnumWrap03::E9);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test16");
  TEST_EQUAL(cPair.value, "E9");

  return 0;
}

int test_reflector_enum03(reflClass &input) {
  TEST_EQUAL(input.test17, EnumWrap04::E10);
  ReflectorMember member = input["test17"];
  TEST_EQUAL(member.ReflectValue("E11"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test17, EnumWrap04::E11);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test17");
  TEST_EQUAL(cPair.value, "E11");

  TEST_EQUAL(member.ReflectValue("  \t E12   "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test17, EnumWrap04::E12);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test17");
  TEST_EQUAL(cPair.value, "E12");

  TEST_EQUAL(member.ReflectValue("pE129"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test17, EnumWrap04::E12);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test17");
  TEST_EQUAL(cPair.value, "E12");

  TEST_EQUAL(member.ReflectValue("E102"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test17, EnumWrap04::E12);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test17");
  TEST_EQUAL(cPair.value, "E12");

  return 0;
}

int test_reflector_enumflags00(reflClass &input) {
  TEST_CHECK(!input.test12[EnumWrap00::E1]);
  TEST_CHECK(!input.test12[EnumWrap00::E2]);
  TEST_CHECK(!input.test12[EnumWrap00::E3]);
  ReflectorMember member = input["test12"];
  TEST_EQUAL(member.ReflectValue("E1 | E2 | E3"),
             ReflectorMember::ErrorType::None);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E2 | E3");

  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E1), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E2), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E3), "true");

  TEST_EQUAL(member.ReflectValue("\tE1   |   E2 \t|  E3\t   "),
             ReflectorMember::ErrorType::None);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E2 | E3");

  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E1), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E2), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E3), "true");

  TEST_EQUAL(member.ReflectValue("E1|E2|E3"), ReflectorMember::ErrorType::None);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E2 | E3");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E1), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E2), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E3), "true");

  TEST_EQUAL(member.ReflectValue("E1 | E2 | "),
             ReflectorMember::ErrorType::EmptyInput);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(input.test12[EnumWrap00::E2]);
  TEST_CHECK(!input.test12[EnumWrap00::E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E2");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E1), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E2), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E3), "false");

  TEST_EQUAL(member.ReflectValue("E1 |  | E3"),
             ReflectorMember::ErrorType::EmptyInput);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(!input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E3");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E1), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E2), "false");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E3), "true");

  TEST_EQUAL(member.ReflectValue(" |  | E3"),
             ReflectorMember::ErrorType::EmptyInput);
  TEST_CHECK(!input.test12[EnumWrap00::E1]);
  TEST_CHECK(!input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E3");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E1), "false");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E2), "false");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E3), "true");

  TEST_EQUAL(member.ReflectValue("E1 | sirius | E3"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_CHECK(input.test12[EnumWrap00::E1]);
  TEST_CHECK(!input.test12[EnumWrap00::E2]);
  TEST_CHECK(input.test12[EnumWrap00::E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test12");
  TEST_EQUAL(cPair.value, "E1 | E3");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E1), "true");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E2), "false");
  TEST_EQUAL(member.ReflectedValue((size_t)EnumWrap00::E3), "true");

  return 0;
}

int test_reflector_enumflags01(reflClass &input) {
  TEST_CHECK(!input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(!input.test13[EnumWrap01_E3]);
  ReflectorMember member = input["test13"];
  TEST_EQUAL(
      member.ReflectValue("EnumWrap01_E1 | EnumWrap01_E2 | EnumWrap01_E3"),
      ReflectorMember::ErrorType::None);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E2 | EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E1), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E2), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E3), "true");

  TEST_EQUAL(member.ReflectValue(
                 "\tEnumWrap01_E1   |   EnumWrap01_E2 \t|  EnumWrap01_E3\t   "),
             ReflectorMember::ErrorType::None);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E2 | EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E1), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E2), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E3), "true");

  TEST_EQUAL(member.ReflectValue("EnumWrap01_E1|EnumWrap01_E2|EnumWrap01_E3"),
             ReflectorMember::ErrorType::None);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E2 | EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E1), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E2), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E3), "true");

  TEST_EQUAL(member.ReflectValue("EnumWrap01_E1 | EnumWrap01_E2 | "),
             ReflectorMember::ErrorType::EmptyInput);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(input.test13[EnumWrap01_E2]);
  TEST_CHECK(!input.test13[EnumWrap01_E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E2");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E1), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E2), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E3), "false");

  TEST_EQUAL(member.ReflectValue("EnumWrap01_E1 |  | EnumWrap01_E3"),
             ReflectorMember::ErrorType::EmptyInput);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E1), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E2), "false");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E3), "true");

  TEST_EQUAL(member.ReflectValue(" |  | EnumWrap01_E3"),
             ReflectorMember::ErrorType::EmptyInput);
  TEST_CHECK(!input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E1), "false");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E2), "false");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E3), "true");

  TEST_EQUAL(member.ReflectValue("EnumWrap01_E1 | EnumWrap01_ | EnumWrap01_E3"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E1), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E2), "false");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E3), "true");

  TEST_EQUAL(member.ReflectValue("EnumWrap01_E1 || EnumWrap01_E3"),
             ReflectorMember::ErrorType::EmptyInput);
  TEST_CHECK(input.test13[EnumWrap01_E1]);
  TEST_CHECK(!input.test13[EnumWrap01_E2]);
  TEST_CHECK(input.test13[EnumWrap01_E3]);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test13");
  TEST_EQUAL(cPair.value, "EnumWrap01_E1 | EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E1), "true");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E2), "false");
  TEST_EQUAL(member.ReflectedValue(EnumWrap01_E3), "true");

  return 0;
}

int test_reflector_subclass(reflClass &input) {
  ReflectorMember member = input["test22"];
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test22");
  TEST_EQUAL(cPair.value, "SUBCLASS_TYPE");
  TEST_CHECK(member.IsReflectedSubClass());

  auto sClass = member.ReflectedSubClass();
  subrefl &lClass = input.test22;
  member = sClass["data0"];

  TEST_EQUAL(lClass.data0, 0);
  TEST_EQUAL(member.ReflectValue("-17845"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass.data0, -17845);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "data0");
  TEST_EQUAL(cPair.value, "-17845");

  member = sClass["data1"];

  TEST_EQUAL(lClass.data1, 0);
  TEST_EQUAL(member.ReflectValue("-1.5975"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass.data1, -1.5975f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "data1");
  TEST_EQUAL(cPair.value, "-1.5975");

  return 0;
}

int test_reflector_vector(reflClass &input) {
  TEST_EQUAL(input.test18, Vector());
  ReflectorMember member = input["test18"];
  TEST_EQUAL(member.ReflectValue("[1.5, 2.8, 5.4]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test18");
  TEST_EQUAL(cPair.value, "[1.5, 2.8, 5.4]");
  TEST_EQUAL(member.ReflectedValue(0), "1.5");
  TEST_EQUAL(member.ReflectedValue(1), "2.8");
  TEST_EQUAL(member.ReflectedValue(2), "5.4");

  TEST_EQUAL(member.ReflectValue("[3.5, 2.1, 1.4,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(3.5f, 2.1f, 1.4f));

  TEST_EQUAL(member.ReflectValue("[1.2, 2.1, 5.9, 1.0]"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test18, Vector(1.2f, 2.1f, 5.9f));

  TEST_EQUAL(member.ReflectValue("[1.6, 2.5,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(1.6f, 2.5f, 5.9f));

  TEST_EQUAL(member.ReflectValue("[3.6, 8.5]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(3.6f, 8.5f, 5.9f));

  TEST_EQUAL(member.ReflectValue(
                 "   [      1.5      ,      2.8      ,      5.4     ]   "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));

  TEST_EQUAL(member.ReflectValue("3.5, 2.1, 1.4,]"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));

  TEST_EQUAL(member.ReflectValue("[3.5, 2.1, 1.4,"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));

  TEST_EQUAL(member.ReflectValue("3.5, 2.1, 1.4,"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test18, Vector(1.5f, 2.8f, 5.4f));

  TEST_EQUAL(member.ReflectValue("[3.5 2.1, 1.4,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test18, Vector(3.5f, 1.4f, 5.4f));

  TEST_EQUAL(member.ReflectValue("[1.2, , 2.1,]"),
             ReflectorMember::ErrorType::ShortInput);
  TEST_EQUAL(input.test18, Vector(1.2f, 1.4f, 5.4f));

  return 0;
}

int test_reflector_vector2(reflClass &input) {
  TEST_EQUAL(input.test19, Vector2());
  ReflectorMember member = input["test19"];
  TEST_EQUAL(member.ReflectValue("[1.5, 5.4]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(1.5f, 5.4f));
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test19");
  TEST_EQUAL(cPair.value, "[1.5, 5.4]");
  TEST_EQUAL(member.ReflectedValue(0), "1.5");
  TEST_EQUAL(member.ReflectedValue(1), "5.4");

  TEST_EQUAL(member.ReflectValue("[3.5, 2.1,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(3.5f, 2.1f));

  TEST_EQUAL(member.ReflectValue("[1.2, 2.1, 5.9]"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test19, Vector2(1.2f, 2.1f));

  TEST_EQUAL(member.ReflectValue("[1.6, 2.5,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(1.6f, 2.5f));

  TEST_EQUAL(member.ReflectValue("[3.6, 8.5]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(3.6f, 8.5f));

  TEST_EQUAL(member.ReflectValue("   [      1.5      ,      2.8      ]   "),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(1.5f, 2.8f));

  TEST_EQUAL(member.ReflectValue("3.5, 2.1,]"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test19, Vector2(1.5f, 2.8f));

  TEST_EQUAL(member.ReflectValue("[3.5, 2.1,"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test19, Vector2(1.5f, 2.8f));

  TEST_EQUAL(member.ReflectValue("3.5, 2.1,"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test19, Vector2(1.5f, 2.8f));

  TEST_EQUAL(member.ReflectValue("[3.5 2.1,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test19, Vector2(3.5f, 2.8f));

  TEST_EQUAL(member.ReflectValue("[1.2, , 2.1,]"),
             ReflectorMember::ErrorType::ShortInput);
  TEST_EQUAL(input.test19, Vector2(1.2f, 2.8f));

  return 0;
}

int test_reflector_vector4(reflClass &input) {
  TEST_EQUAL(input.test20, Vector4());
  ReflectorMember member = input["test20"];
  TEST_EQUAL(member.ReflectValue("[1.5, 2.8, 5.4, 7.2]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 7.2f));
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test20");
  TEST_EQUAL(cPair.value, "[1.5, 2.8, 5.4, 7.2]");
  TEST_EQUAL(member.ReflectedValue(0), "1.5");
  TEST_EQUAL(member.ReflectedValue(1), "2.8");
  TEST_EQUAL(member.ReflectedValue(2), "5.4");
  TEST_EQUAL(member.ReflectedValue(3), "7.2");

  TEST_EQUAL(member.ReflectValue("[3.5, 2.1, 1.4, 6.1,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(3.5f, 2.1f, 1.4f, 6.1f));

  TEST_EQUAL(member.ReflectValue("[1.2, 2.1, 5.9, 3.2, 1.0]"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test20, Vector4(1.2f, 2.1f, 5.9f, 3.2f));

  TEST_EQUAL(member.ReflectValue("[1.6, 2.5, 7.3]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(1.6f, 2.5f, 7.3f, 3.2f));

  TEST_EQUAL(member.ReflectValue("[3.6, 8.5, 2.1]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(3.6f, 8.5f, 2.1f, 3.2f));

  TEST_EQUAL(
      member.ReflectValue(
          "   [      1.5      ,      2.8      ,      5.4  ,    6.7   ]   "),
      ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("3.5, 2.1, 1.4,]"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("[3.5, 2.1, 1.4,"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("3.5, 2.1, 1.4,"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test20, Vector4(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("[3.5 2.1, 1.4,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test20, Vector4(3.5f, 1.4f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("[1.2, , 2.1,]"),
             ReflectorMember::ErrorType::ShortInput);
  TEST_EQUAL(input.test20, Vector4(1.2f, 1.4f, 5.4f, 6.7f));

  return 0;
}

int test_reflector_vector4A16(reflClass &input) {
  TEST_EQUAL(input.test21, Vector4A16());
  ReflectorMember member = input["test21"];
  TEST_EQUAL(member.ReflectValue("[1.5, 2.8, 5.4, 7.2]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 7.2f));
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test21");
  TEST_EQUAL(cPair.value, "[1.5, 2.8, 5.4, 7.2]");
  TEST_EQUAL(member.ReflectedValue(0), "1.5");
  TEST_EQUAL(member.ReflectedValue(1), "2.8");
  TEST_EQUAL(member.ReflectedValue(2), "5.4");
  TEST_EQUAL(member.ReflectedValue(3), "7.2");

  TEST_EQUAL(member.ReflectValue("[3.5, 2.1, 1.4, 6.1,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(3.5f, 2.1f, 1.4f, 6.1f));

  TEST_EQUAL(member.ReflectValue("[1.2, 2.1, 5.9, 3.2, 1.0]"),
             ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(input.test21, Vector4A16(1.2f, 2.1f, 5.9f, 3.2f));

  TEST_EQUAL(member.ReflectValue("[1.6, 2.5, 7.3]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(1.6f, 2.5f, 7.3f, 3.2f));

  TEST_EQUAL(member.ReflectValue("[3.6, 8.5, 2.1]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(3.6f, 8.5f, 2.1f, 3.2f));

  TEST_EQUAL(
      member.ReflectValue(
          "   [      1.5      ,      2.8      ,      5.4  ,    6.7   ]   "),
      ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("3.5, 2.1, 1.4,]"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("[3.5, 2.1, 1.4,"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("3.5, 2.1, 1.4,"),
             ReflectorMember::ErrorType::InvalidFormat);
  TEST_EQUAL(input.test21, Vector4A16(1.5f, 2.8f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("[3.5 2.1, 1.4,]"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test21, Vector4A16(3.5f, 1.4f, 5.4f, 6.7f));

  TEST_EQUAL(member.ReflectValue("[1.2, , 2.1,]"),
             ReflectorMember::ErrorType::ShortInput);
  TEST_EQUAL(input.test21, Vector4A16(1.2f, 1.4f, 5.4f, 6.7f));

  return 0;
}

int test_reflector_arrays(reflClass &input) {
  TEST_EQUAL(input.test40[0], false);
  TEST_EQUAL(input.test40[1], false);
  TEST_EQUAL(input.test40[2], false);
  TEST_EQUAL(input.test40[3], false);
  ReflectorMember member = input["test40"];
  member = "{false, true, true, false}";
  TEST_EQUAL(input.test40[0], false);
  TEST_EQUAL(input.test40[1], true);
  TEST_EQUAL(input.test40[2], true);
  TEST_EQUAL(input.test40[3], false);
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test40");
  TEST_EQUAL(cPair.value, "{false, true, true, false}");
  TEST_EQUAL(member.ReflectedValue(0), "false");
  TEST_EQUAL(member.ReflectedValue(1), "true");
  TEST_EQUAL(member.ReflectedValue(2), "true");
  TEST_EQUAL(member.ReflectedValue(3), "false");
  member.ReflectValue("true", 0);
  TEST_EQUAL(input.test40[0], true);
  TEST_EQUAL(input.test40[1], true);
  TEST_EQUAL(input.test40[2], true);
  TEST_EQUAL(input.test40[3], false);
  member.ReflectValue("false", 1);
  TEST_EQUAL(input.test40[0], true);
  TEST_EQUAL(input.test40[1], false);
  TEST_EQUAL(input.test40[2], true);
  TEST_EQUAL(input.test40[3], false);
  member.ReflectValue("false", 2);
  TEST_EQUAL(input.test40[0], true);
  TEST_EQUAL(input.test40[1], false);
  TEST_EQUAL(input.test40[2], false);
  TEST_EQUAL(input.test40[3], false);
  member.ReflectValue("true", 3);
  TEST_EQUAL(input.test40[0], true);
  TEST_EQUAL(input.test40[1], false);
  TEST_EQUAL(input.test40[2], false);
  TEST_EQUAL(input.test40[3], true);

  TEST_EQUAL(input.test41[0], 0);
  TEST_EQUAL(input.test41[1], 0);
  member = input["test41"];
  member = "{12, -69}";
  TEST_EQUAL(input.test41[0], 12);
  TEST_EQUAL(input.test41[1], -69);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test41");
  TEST_EQUAL(cPair.value, "{12, -69}");
  TEST_EQUAL(member.ReflectedValue(0), "12");
  TEST_EQUAL(member.ReflectedValue(1), "-69");
  member.ReflectValue("13", 0);
  TEST_EQUAL(input.test41[0], 13);
  TEST_EQUAL(input.test41[1], -69);
  member.ReflectValue("-66", 1);
  TEST_EQUAL(input.test41[0], 13);
  TEST_EQUAL(input.test41[1], -66);

  TEST_EQUAL(input.test42[0], 0);
  TEST_EQUAL(input.test42[1], 0);
  TEST_EQUAL(input.test42[2], 0);
  TEST_EQUAL(input.test42[3], 0);
  member = input["test42"];
  member = "{128, 57, 251, -75}";
  TEST_EQUAL(input.test42[0], 128);
  TEST_EQUAL(input.test42[1], 57);
  TEST_EQUAL(input.test42[2], 251);
  TEST_EQUAL(input.test42[3], 181);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test42");
  TEST_EQUAL(cPair.value, "{128, 57, 251, 181}");
  TEST_EQUAL(member.ReflectedValue(0), "128");
  TEST_EQUAL(member.ReflectedValue(1), "57");
  TEST_EQUAL(member.ReflectedValue(2), "251");
  TEST_EQUAL(member.ReflectedValue(3), "181");
  member.ReflectValue("120", 0);
  TEST_EQUAL(input.test42[0], 120);
  TEST_EQUAL(input.test42[1], 57);
  TEST_EQUAL(input.test42[2], 251);
  TEST_EQUAL(input.test42[3], 181);
  member.ReflectValue("50", 1);
  TEST_EQUAL(input.test42[0], 120);
  TEST_EQUAL(input.test42[1], 50);
  TEST_EQUAL(input.test42[2], 251);
  TEST_EQUAL(input.test42[3], 181);
  member.ReflectValue("252", 2);
  TEST_EQUAL(input.test42[0], 120);
  TEST_EQUAL(input.test42[1], 50);
  TEST_EQUAL(input.test42[2], 252);
  TEST_EQUAL(input.test42[3], 181);
  member.ReflectValue("172", 3);
  TEST_EQUAL(input.test42[0], 120);
  TEST_EQUAL(input.test42[1], 50);
  TEST_EQUAL(input.test42[2], 252);
  TEST_EQUAL(input.test42[3], 172);

  TEST_EQUAL(input.test43[0], 0);
  TEST_EQUAL(input.test43[1], 0);
  member = input["test43"];
  member = "{1200, -6956}";
  TEST_EQUAL(input.test43[0], 1200);
  TEST_EQUAL(input.test43[1], -6956);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test43");
  TEST_EQUAL(cPair.value, "{1200, -6956}");
  TEST_EQUAL(member.ReflectedValue(0), "1200");
  TEST_EQUAL(member.ReflectedValue(1), "-6956");
  member.ReflectValue("1208", 0);
  TEST_EQUAL(input.test43[0], 1208);
  TEST_EQUAL(input.test43[1], -6956);
  member.ReflectValue("-5669", 1);
  TEST_EQUAL(input.test43[0], 1208);
  TEST_EQUAL(input.test43[1], -5669);

  TEST_EQUAL(input.test44[0], 0);
  TEST_EQUAL(input.test44[1], 0);
  TEST_EQUAL(input.test44[2], 0);
  TEST_EQUAL(input.test44[3], 0);
  member = input["test44"];
  member = "{1286, 572, 2515, -755}";
  TEST_EQUAL(input.test44[0], 1286);
  TEST_EQUAL(input.test44[1], 572);
  TEST_EQUAL(input.test44[2], 2515);
  TEST_EQUAL(input.test44[3], 64781);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test44");
  TEST_EQUAL(cPair.value, "{1286, 572, 2515, 64781}");
  TEST_EQUAL(member.ReflectedValue(0), "1286");
  TEST_EQUAL(member.ReflectedValue(1), "572");
  TEST_EQUAL(member.ReflectedValue(2), "2515");
  TEST_EQUAL(member.ReflectedValue(3), "64781");
  member.ReflectValue("1862", 0);
  TEST_EQUAL(input.test44[0], 1862);
  TEST_EQUAL(input.test44[1], 572);
  TEST_EQUAL(input.test44[2], 2515);
  TEST_EQUAL(input.test44[3], 64781);
  member.ReflectValue("678", 1);
  TEST_EQUAL(input.test44[0], 1862);
  TEST_EQUAL(input.test44[1], 678);
  TEST_EQUAL(input.test44[2], 2515);
  TEST_EQUAL(input.test44[3], 64781);
  member.ReflectValue("4879", 2);
  TEST_EQUAL(input.test44[0], 1862);
  TEST_EQUAL(input.test44[1], 678);
  TEST_EQUAL(input.test44[2], 4879);
  TEST_EQUAL(input.test44[3], 64781);
  member.ReflectValue("62158", 3);
  TEST_EQUAL(input.test44[0], 1862);
  TEST_EQUAL(input.test44[1], 678);
  TEST_EQUAL(input.test44[2], 4879);
  TEST_EQUAL(input.test44[3], 62158);

  TEST_EQUAL(input.test45[0], 0);
  TEST_EQUAL(input.test45[1], 0);
  member = input["test45"];
  member = "{120053, -695641}";
  TEST_EQUAL(input.test45[0], 120053);
  TEST_EQUAL(input.test45[1], -695641);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test45");
  TEST_EQUAL(cPair.value, "{120053, -695641}");
  TEST_EQUAL(member.ReflectedValue(0), "120053");
  TEST_EQUAL(member.ReflectedValue(1), "-695641");
  member.ReflectValue("350021", 0);
  TEST_EQUAL(input.test45[0], 350021);
  TEST_EQUAL(input.test45[1], -695641);
  member.ReflectValue("-415669", 1);
  TEST_EQUAL(input.test45[0], 350021);
  TEST_EQUAL(input.test45[1], -415669);

  TEST_EQUAL(input.test46[0], 0);
  TEST_EQUAL(input.test46[1], 0);
  TEST_EQUAL(input.test46[2], 0);
  TEST_EQUAL(input.test46[3], 0);
  member = input["test46"];
  member = "{128612, 572573, 2515513, -755613}";
  TEST_EQUAL(input.test46[0], 128612);
  TEST_EQUAL(input.test46[1], 572573);
  TEST_EQUAL(input.test46[2], 2515513);
  TEST_EQUAL(input.test46[3], 4294211683);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test46");
  TEST_EQUAL(cPair.value, "{128612, 572573, 2515513, 4294211683}");
  TEST_EQUAL(member.ReflectedValue(0), "128612");
  TEST_EQUAL(member.ReflectedValue(1), "572573");
  TEST_EQUAL(member.ReflectedValue(2), "2515513");
  TEST_EQUAL(member.ReflectedValue(3), "4294211683");

  TEST_EQUAL(input.test47[0], 0);
  TEST_EQUAL(input.test47[1], 0);
  member = input["test47"];
  member = "{120053613654541, -6956415461654}";
  TEST_EQUAL(input.test47[0], 120053613654541);
  TEST_EQUAL(input.test47[1], -6956415461654);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test47");
  TEST_EQUAL(cPair.value, "{120053613654541, -6956415461654}");
  TEST_EQUAL(member.ReflectedValue(0), "120053613654541");
  TEST_EQUAL(member.ReflectedValue(1), "-6956415461654");

  TEST_EQUAL(input.test48[0], 0);
  TEST_EQUAL(input.test48[1], 0);
  TEST_EQUAL(input.test48[2], 0);
  TEST_EQUAL(input.test48[3], 0);
  member = input["test48"];
  member = "{128612465345, 5725735436343, 2515513435453, -75561345345321}";
  TEST_EQUAL(input.test48[0], 128612465345);
  TEST_EQUAL(input.test48[1], 5725735436343);
  TEST_EQUAL(input.test48[2], 2515513435453);
  TEST_EQUAL(input.test48[3], 18446668512364206295ULL);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test48");
  TEST_EQUAL(
      cPair.value,
      "{128612465345, 5725735436343, 2515513435453, 18446668512364206295}");
  TEST_EQUAL(member.ReflectedValue(0), "128612465345");
  TEST_EQUAL(member.ReflectedValue(1), "5725735436343");
  TEST_EQUAL(member.ReflectedValue(2), "2515513435453");
  TEST_EQUAL(member.ReflectedValue(3), "18446668512364206295");

  TEST_EQUAL(input.test49[0], 0);
  TEST_EQUAL(input.test49[1], 0);
  member = input["test49"];
  member = "{1.5, -3.6}";
  TEST_EQUAL(input.test49[0], 1.5f);
  TEST_EQUAL(input.test49[1], -3.6f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test49");
  TEST_EQUAL(cPair.value, "{1.5, -3.6}");
  TEST_EQUAL(member.ReflectedValue(0), "1.5");
  TEST_EQUAL(member.ReflectedValue(1), "-3.6");
  member.ReflectValue("1.22", 0);
  TEST_EQUAL(input.test49[0], 1.22f);
  TEST_EQUAL(input.test49[1], -3.6f);
  member.ReflectValue("-3.72", 1);
  TEST_EQUAL(input.test49[0], 1.22f);
  TEST_EQUAL(input.test49[1], -3.72f);

  TEST_EQUAL(input.test50[0], 0);
  TEST_EQUAL(input.test50[1], 0);
  TEST_EQUAL(input.test50[2], 0);
  TEST_EQUAL(input.test50[3], 0);
  member = input["test50"];
  member = "{1.536, 9.861, 6.45521, -7.32123}";
  TEST_EQUAL(input.test50[0], 1.536);
  TEST_EQUAL(input.test50[1], 9.861);
  TEST_EQUAL(input.test50[2], 6.45521);
  TEST_EQUAL(input.test50[3], -7.32123);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test50");
  TEST_EQUAL(cPair.value, "{1.536, 9.861, 6.45521, -7.32123}");
  TEST_EQUAL(member.ReflectedValue(0), "1.536");
  TEST_EQUAL(member.ReflectedValue(1), "9.861");
  TEST_EQUAL(member.ReflectedValue(2), "6.45521");
  TEST_EQUAL(member.ReflectedValue(3), "-7.32123");

  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E1);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E3);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E1);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E3);
  member = input["test51"];
  member = "{E1 | E3, E2}";
  TEST_EQUAL(input.test51[0], EnumWrap00::E1);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E2);
  TEST_EQUAL(input.test51[0], EnumWrap00::E3);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E1);
  TEST_EQUAL(input.test51[1], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E3);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test51");
  TEST_EQUAL(cPair.value, "{E1 | E3, E2}");
  TEST_EQUAL(member.ReflectedValue(0), "E1 | E3");
  TEST_EQUAL(member.ReflectedValue(1), "E2");
  auto sClass =  member.ReflectedSubClass(0);
  TEST_EQUAL(sClass[0].ReflectedValue(), "true");
  TEST_EQUAL(sClass[1].ReflectedValue(), "false");
  TEST_EQUAL(sClass[2].ReflectedValue(), "true");
  sClass =  member.ReflectedSubClass(1);
  TEST_EQUAL(sClass[0].ReflectedValue(), "false");
  TEST_EQUAL(sClass[1].ReflectedValue(), "true");
  TEST_EQUAL(sClass[2].ReflectedValue(), "false");
  member.ReflectValue("E1", 0);
  TEST_EQUAL(input.test51[0], EnumWrap00::E1);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E3);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E1);
  TEST_EQUAL(input.test51[1], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E3);
  member.ReflectValue("E2 | E3", 1);
  TEST_EQUAL(input.test51[0], EnumWrap00::E1);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E2);
  TEST_NOT_EQUAL(input.test51[0], EnumWrap00::E3);
  TEST_NOT_EQUAL(input.test51[1], EnumWrap00::E1);
  TEST_EQUAL(input.test51[1], EnumWrap00::E2);
  TEST_EQUAL(input.test51[1], EnumWrap00::E3);

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
  member = input["test52"];
  member.ReflectValue("{EnumWrap01_E1 | EnumWrap01_E3, EnumWrap01_E2, "
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
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test52");
  TEST_EQUAL(cPair.value, "{EnumWrap01_E1 | EnumWrap01_E3, EnumWrap01_E2, "
                          "EnumWrap01_E2 | EnumWrap01_E3, NULL}");
  TEST_EQUAL(member.ReflectedValue(0), "EnumWrap01_E1 | EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(1), "EnumWrap01_E2");
  TEST_EQUAL(member.ReflectedValue(2), "EnumWrap01_E2 | EnumWrap01_E3");
  TEST_EQUAL(member.ReflectedValue(3), "NULL");
  sClass =  member.ReflectedSubClass(0);
  TEST_EQUAL(sClass[0].ReflectedValue(), "true");
  TEST_EQUAL(sClass[1].ReflectedValue(), "false");
  TEST_EQUAL(sClass[2].ReflectedValue(), "true");
  sClass =  member.ReflectedSubClass(1);
  TEST_EQUAL(sClass[0].ReflectedValue(), "false");
  TEST_EQUAL(sClass[1].ReflectedValue(), "true");
  TEST_EQUAL(sClass[2].ReflectedValue(), "false");
  sClass =  member.ReflectedSubClass(2);
  TEST_EQUAL(sClass[0].ReflectedValue(), "false");
  TEST_EQUAL(sClass[1].ReflectedValue(), "true");
  TEST_EQUAL(sClass[2].ReflectedValue(), "true");
  sClass =  member.ReflectedSubClass(3);
  TEST_EQUAL(sClass[0].ReflectedValue(), "false");
  TEST_EQUAL(sClass[1].ReflectedValue(), "false");
  TEST_EQUAL(sClass[2].ReflectedValue(), "false");
  member.ReflectValue("NULL", 0);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E1);
  TEST_EQUAL(input.test52[1], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E1);
  TEST_EQUAL(input.test52[2], EnumWrap01_E2);
  TEST_EQUAL(input.test52[2], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E3);
  member.ReflectValue("EnumWrap01_E1 | EnumWrap01_E3", 1);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E3);
  TEST_EQUAL(input.test52[1], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E2);
  TEST_EQUAL(input.test52[1], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E1);
  TEST_EQUAL(input.test52[2], EnumWrap01_E2);
  TEST_EQUAL(input.test52[2], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E3);
  member.ReflectValue("EnumWrap01_E2", 2);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E3);
  TEST_EQUAL(input.test52[1], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E2);
  TEST_EQUAL(input.test52[1], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E1);
  TEST_EQUAL(input.test52[2], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E3);
  member.ReflectValue("EnumWrap01_E2 | EnumWrap01_E3", 3);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[0], EnumWrap01_E3);
  TEST_EQUAL(input.test52[1], EnumWrap01_E1);
  TEST_NOT_EQUAL(input.test52[1], EnumWrap01_E2);
  TEST_EQUAL(input.test52[1], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E1);
  TEST_EQUAL(input.test52[2], EnumWrap01_E2);
  TEST_NOT_EQUAL(input.test52[2], EnumWrap01_E3);
  TEST_NOT_EQUAL(input.test52[3], EnumWrap01_E1);
  TEST_EQUAL(input.test52[3], EnumWrap01_E2);
  TEST_EQUAL(input.test52[3], EnumWrap01_E3);

  TEST_EQUAL(input.test53[0], EnumWrap00::E1);
  TEST_EQUAL(input.test53[1], EnumWrap00::E1);
  member = input["test53"];
  member.ReflectValue("{E2, E1}");
  TEST_EQUAL(input.test53[0], EnumWrap00::E2);
  TEST_EQUAL(input.test53[1], EnumWrap00::E1);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test53");
  TEST_EQUAL(cPair.value, "{E2, E1}");
  TEST_EQUAL(member.ReflectedValue(0), "E2");
  TEST_EQUAL(member.ReflectedValue(1), "E1");
  member.ReflectValue("E3", 0);
  TEST_EQUAL(input.test53[0], EnumWrap00::E3);
  TEST_EQUAL(input.test53[1], EnumWrap00::E1);
  member.ReflectValue("E2", 1);
  TEST_EQUAL(input.test53[0], EnumWrap00::E3);
  TEST_EQUAL(input.test53[1], EnumWrap00::E2);

  TEST_EQUAL(input.test54[0], EnumWrap02::E4);
  TEST_EQUAL(input.test54[1], EnumWrap02::E4);
  TEST_EQUAL(input.test54[2], EnumWrap02::E4);
  TEST_EQUAL(input.test54[3], EnumWrap02::E4);
  member = input["test54"];
  member.ReflectValue("{E5, E6, E4, E5}");
  TEST_EQUAL(input.test54[0], EnumWrap02::E5);
  TEST_EQUAL(input.test54[1], EnumWrap02::E6);
  TEST_EQUAL(input.test54[2], EnumWrap02::E4);
  TEST_EQUAL(input.test54[3], EnumWrap02::E5);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test54");
  TEST_EQUAL(cPair.value, "{E5, E6, E4, E5}");
  TEST_EQUAL(member.ReflectedValue(0), "E5");
  TEST_EQUAL(member.ReflectedValue(1), "E6");
  TEST_EQUAL(member.ReflectedValue(2), "E4");
  TEST_EQUAL(member.ReflectedValue(3), "E5");

  TEST_EQUAL(int(input.test55[0]), 0);
  TEST_EQUAL(int(input.test55[1]), 0);
  member = input["test55"];
  member = "{E9, E8}";
  TEST_EQUAL(input.test55[0], EnumWrap03::E9);
  TEST_EQUAL(input.test55[1], EnumWrap03::E8);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test55");
  TEST_EQUAL(cPair.value, "{E9, E8}");
  TEST_EQUAL(member.ReflectedValue(0), "E9");
  TEST_EQUAL(member.ReflectedValue(1), "E8");

  TEST_EQUAL(input.test56[0], EnumWrap04::E10);
  TEST_EQUAL(input.test56[1], EnumWrap04::E10);
  TEST_EQUAL(input.test56[2], EnumWrap04::E10);
  TEST_EQUAL(input.test56[3], EnumWrap04::E10);
  member = input["test56"];
  member.ReflectValue("{E11, E12, E10, E12}");
  TEST_EQUAL(input.test56[0], EnumWrap04::E11);
  TEST_EQUAL(input.test56[1], EnumWrap04::E12);
  TEST_EQUAL(input.test56[2], EnumWrap04::E10);
  TEST_EQUAL(input.test56[3], EnumWrap04::E12);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test56");
  TEST_EQUAL(cPair.value, "{E11, E12, E10, E12}");
  TEST_EQUAL(member.ReflectedValue(0), "E11");
  TEST_EQUAL(member.ReflectedValue(1), "E12");
  TEST_EQUAL(member.ReflectedValue(2), "E10");
  TEST_EQUAL(member.ReflectedValue(3), "E12");

  TEST_EQUAL(input.test57[0], Vector());
  TEST_EQUAL(input.test57[1], Vector());
  member = input["test57"];
  member = "{[5.6, 51.6, 31.7], [18.1, 3.5, 6.1]}";
  TEST_EQUAL(input.test57[0], Vector(5.6f, 51.6f, 31.7f));
  TEST_EQUAL(input.test57[1], Vector(18.1f, 3.5f, 6.1f));
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test57");
  TEST_EQUAL(cPair.value, "{[5.6, 51.6, 31.7], [18.1, 3.5, 6.1]}");
  TEST_EQUAL(member.ReflectedValue(0), "[5.6, 51.6, 31.7]");
  TEST_EQUAL(member.ReflectedValue(1), "[18.1, 3.5, 6.1]");
  sClass = member.ReflectedSubClass();
  TEST_EQUAL(sClass[0].ReflectedValue(), "5.6");
  TEST_EQUAL(sClass[1].ReflectedValue(), "51.6");
  TEST_EQUAL(sClass[2].ReflectedValue(), "31.7");
  sClass = member.ReflectedSubClass(1);
  TEST_EQUAL(sClass[0].ReflectedValue(), "18.1");
  TEST_EQUAL(sClass[1].ReflectedValue(), "3.5");
  TEST_EQUAL(sClass[2].ReflectedValue(), "6.1");
  member.ReflectValue("[7.1, 11.8, 105.1]", 0);
  TEST_EQUAL(input.test57[0], Vector(7.1f, 11.8f, 105.1f));
  TEST_EQUAL(input.test57[1], Vector(18.1f, 3.5f, 6.1f));
  member.ReflectValue("[12.5, 78.1, 478.85]", 1);
  TEST_EQUAL(input.test57[0], Vector(7.1f, 11.8f, 105.1f));
  TEST_EQUAL(input.test57[1], Vector(12.5f, 78.1f, 478.85f));
  sClass = member.ReflectedSubClass();
  sClass["y"] = "8.11";
  TEST_EQUAL(input.test57[0], Vector(7.1f, 8.11f, 105.1f));
  TEST_EQUAL(input.test57[1], Vector(12.5f, 78.1f, 478.85f));
  sClass = member.ReflectedSubClass(1);
  sClass["z"] = "54.62";
  TEST_EQUAL(input.test57[0], Vector(7.1f, 8.11f, 105.1f));
  TEST_EQUAL(input.test57[1], Vector(12.5f, 78.1f, 54.62f));

  TEST_EQUAL(input.test58[0], Vector2());
  TEST_EQUAL(input.test58[1], Vector2());
  TEST_EQUAL(input.test58[2], Vector2());
  TEST_EQUAL(input.test58[3], Vector2());
  member = input["test58"];
  member = "{[5.6, 51.6], [31.7, 18.1], [3.5, 6.1], [7.5, 62.1]}";
  TEST_EQUAL(input.test58[0], Vector2(5.6f, 51.6f));
  TEST_EQUAL(input.test58[1], Vector2(31.7f, 18.1f));
  TEST_EQUAL(input.test58[2], Vector2(3.5f, 6.1f));
  TEST_EQUAL(input.test58[3], Vector2(7.5f, 62.1f));
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test58");
  TEST_EQUAL(cPair.value,
             "{[5.6, 51.6], [31.7, 18.1], [3.5, 6.1], [7.5, 62.1]}");
  TEST_EQUAL(member.ReflectedValue(0), "[5.6, 51.6]");
  TEST_EQUAL(member.ReflectedValue(1), "[31.7, 18.1]");
  TEST_EQUAL(member.ReflectedValue(2), "[3.5, 6.1]");
  TEST_EQUAL(member.ReflectedValue(3), "[7.5, 62.1]");
  sClass = member.ReflectedSubClass();
  TEST_EQUAL(sClass[0].ReflectedValue(), "5.6");
  TEST_EQUAL(sClass[1].ReflectedValue(), "51.6");
  sClass = member.ReflectedSubClass(1);
  TEST_EQUAL(sClass[0].ReflectedValue(), "31.7");
  TEST_EQUAL(sClass[1].ReflectedValue(), "18.1");
  sClass = member.ReflectedSubClass(2);
  TEST_EQUAL(sClass[0].ReflectedValue(), "3.5");
  TEST_EQUAL(sClass[1].ReflectedValue(), "6.1");
  sClass = member.ReflectedSubClass(3);
  TEST_EQUAL(sClass[0].ReflectedValue(), "7.5");
  TEST_EQUAL(sClass[1].ReflectedValue(), "62.1");

  TEST_EQUAL(input.test59[0], Vector4());
  TEST_EQUAL(input.test59[1], Vector4());
  member = input["test59"];
  member = "{[5.6, 51.6, 31.7, 18.1], [3.5, 6.1, 7.5, 62.1]}";
  TEST_EQUAL(input.test59[0], Vector4(5.6f, 51.6f, 31.7f, 18.1f));
  TEST_EQUAL(input.test59[1], Vector4(3.5f, 6.1f, 7.5f, 62.1f));
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test59");
  TEST_EQUAL(cPair.value, "{[5.6, 51.6, 31.7, 18.1], [3.5, 6.1, 7.5, 62.1]}");
  TEST_EQUAL(member.ReflectedValue(0), "[5.6, 51.6, 31.7, 18.1]");
  TEST_EQUAL(member.ReflectedValue(1), "[3.5, 6.1, 7.5, 62.1]");

  TEST_EQUAL(input.test60[0], Vector4A16());
  TEST_EQUAL(input.test60[1], Vector4A16());
  TEST_EQUAL(input.test60[2], Vector4A16());
  TEST_EQUAL(input.test60[3], Vector4A16());
  member = input["test60"];
  member = "{[5.6, 51.6, 31.7, 18.1], [3.5, 6.1, 7.5, 62.1], "
           "[1.8, 57.5, 36.9, 5.78], [7.41, 5.8, 41.8, 6.12]}";
  TEST_EQUAL(input.test60[0], Vector4A16(5.6f, 51.6f, 31.7f, 18.1f));
  TEST_EQUAL(input.test60[1], Vector4A16(3.5f, 6.1f, 7.5f, 62.1f));
  TEST_EQUAL(input.test60[2], Vector4A16(1.8f, 57.5f, 36.9f, 5.78f));
  TEST_EQUAL(input.test60[3], Vector4A16(7.41f, 5.8f, 41.8f, 6.12f));
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test60");
  TEST_EQUAL(cPair.value, "{[5.6, 51.6, 31.7, 18.1], [3.5, 6.1, 7.5, 62.1], "
                          "[1.8, 57.5, 36.9, 5.78], [7.41, 5.8, 41.8, 6.12]}");
  TEST_EQUAL(member.ReflectedValue(0), "[5.6, 51.6, 31.7, 18.1]");
  TEST_EQUAL(member.ReflectedValue(1), "[3.5, 6.1, 7.5, 62.1]");
  TEST_EQUAL(member.ReflectedValue(2), "[1.8, 57.5, 36.9, 5.78]");
  TEST_EQUAL(member.ReflectedValue(3), "[7.41, 5.8, 41.8, 6.12]");
  sClass = member.ReflectedSubClass();
  TEST_EQUAL(sClass[0].ReflectedValue(), "5.6");
  TEST_EQUAL(sClass[1].ReflectedValue(), "51.6");
  TEST_EQUAL(sClass[2].ReflectedValue(), "31.7");
  TEST_EQUAL(sClass[3].ReflectedValue(), "18.1");
  sClass = member.ReflectedSubClass(1);
  TEST_EQUAL(sClass[0].ReflectedValue(), "3.5");
  TEST_EQUAL(sClass[1].ReflectedValue(), "6.1");
  TEST_EQUAL(sClass[2].ReflectedValue(), "7.5");
  TEST_EQUAL(sClass[3].ReflectedValue(), "62.1");
  sClass = member.ReflectedSubClass(2);
  TEST_EQUAL(sClass[0].ReflectedValue(), "1.8");
  TEST_EQUAL(sClass[1].ReflectedValue(), "57.5");
  TEST_EQUAL(sClass[2].ReflectedValue(), "36.9");
  TEST_EQUAL(sClass[3].ReflectedValue(), "5.78");
  sClass = member.ReflectedSubClass(3);
  TEST_EQUAL(sClass[0].ReflectedValue(), "7.41");
  TEST_EQUAL(sClass[1].ReflectedValue(), "5.8");
  TEST_EQUAL(sClass[2].ReflectedValue(), "41.8");
  TEST_EQUAL(sClass[3].ReflectedValue(), "6.12");

  return 0;
}

int test_reflector_array_subclass(reflClass &input) {
  ReflectorMember member = input["test61"];
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test61");
  TEST_EQUAL(cPair.value, "{SUBCLASS_TYPE, SUBCLASS_TYPE}");
  TEST_CHECK(member.IsReflectedSubClass());
  TEST_CHECK(member.IsArray());

  auto sClass = member.ReflectedSubClass();
  subrefl &lClass = input.test61[0];

  member = sClass["data0"];
  TEST_EQUAL(lClass.data0, 0);
  TEST_EQUAL(member.ReflectValue("-1451"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass.data0, -1451);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "data0");
  TEST_EQUAL(cPair.value, "-1451");

  member = sClass["data1"];
  TEST_EQUAL(lClass.data1, 0);
  TEST_EQUAL(member.ReflectValue("81.65"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass.data1, 81.65f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "data1");
  TEST_EQUAL(cPair.value, "81.65");

  auto sClass2 = input["test61"].ReflectedSubClass(1);
  subrefl &lClass2 = input.test61[1];

  member = sClass2["data0"];
  TEST_EQUAL(lClass2.data0, 0);
  TEST_EQUAL(member.ReflectValue("9841"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass2.data0, 9841);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "data0");
  TEST_EQUAL(cPair.value, "9841");

  member = sClass2["data1"];
  TEST_EQUAL(lClass2.data1, 0);
  TEST_EQUAL(member.ReflectValue("5.874"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass2.data1, 5.874f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "data1");
  TEST_EQUAL(cPair.value, "5.874");

  return 0;
}

int test_reflector_string(reflClass &input) {
  ReflectorMember member = input["test80"];
  TEST_CHECK(input.test80.empty());
  TEST_EQUAL(member.ReflectValue("This is a test string"),
             ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test80, "This is a test string");
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test80");
  TEST_EQUAL(cPair.value, "This is a test string");

  return 0;
}

int test_reflector_bitfield(reflClass &input) {
  ReflectorMember member = input["test23"];
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test23");
  TEST_EQUAL(cPair.value, "SUBCLASS_TYPE");
  TEST_CHECK(member.IsReflectedSubClass());

  auto sClass = member.ReflectedSubClass();
  auto &lClass = input.test23;

  member = sClass["member0"];
  TEST_EQUAL(lClass.Get<member0>(), 0);
  TEST_EQUAL(member.ReflectValue("2"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass.Get<member0>(), 2);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "member0");
  TEST_EQUAL(cPair.value, "2");

  TEST_EQUAL(member.ReflectValue("4"), ReflectorMember::ErrorType::OutOfRange);
  TEST_EQUAL(lClass.Get<member0>(), 3);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "member0");
  TEST_EQUAL(cPair.value, "3");

  TEST_EQUAL(member.ReflectValue("1"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass.Get<member0>(), 1);

  member = sClass["member1"];
  TEST_EQUAL(lClass.Get<member1>(), 0);
  TEST_EQUAL(member.ReflectValue("25"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(lClass.Get<member1>(), 25);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "member1");
  TEST_EQUAL(cPair.value, "25");

  return 0;
}

int test_custom_float(reflClass &input) {
  ReflectorMember member = input["test24"];
  TEST_EQUAL(member.ReflectValue("851.5"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(input.test24, 851.5f);
  input.test24 = -851.5f;
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test24");
  TEST_EQUAL(cPair.value, "-851.5");

  return 0;
}

int test_reflector_bitfield_custom_float(reflClass &input) {
  ReflectorMember member = input["test25"];
  ReflectorMember::KVPair cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "test25");
  TEST_EQUAL(cPair.value, "SUBCLASS_TYPE");
  TEST_CHECK(member.IsReflectedSubClass());

  auto rClass = member.ReflectedSubClass();
  ReflectorPureWrap sClass(rClass);
  auto &lClass = input.test25;
  using X = BFVectorR10G11B10::x;
  using Y = BFVectorR10G11B10::y;
  using Z = BFVectorR10G11B10::z;

  member = sClass["x"];
  TEST_EQUAL(lClass.Get<X>(), 0);
  TEST_EQUAL(member.ReflectValue("12.75"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(X::value_type::ToFloat(lClass.Get<X>()), 12.75f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "x");
  TEST_EQUAL(cPair.value, "12.75");

  TEST_EQUAL(member.ReflectValue("-104"),
             ReflectorMember::ErrorType::SignMismatch);
  TEST_EQUAL(X::value_type::ToFloat(lClass.Get<X>()), 104.f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "x");
  TEST_EQUAL(cPair.value, "104");

  member = sClass["y"];
  TEST_EQUAL(member.ReflectValue("5.125"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(Y::value_type::ToFloat(lClass.Get<Y>()), 5.125f);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "y");
  TEST_EQUAL(cPair.value, "5.125");

  member = sClass["z"];
  TEST_EQUAL(lClass.Get<Z>(), 0);
  TEST_EQUAL(member.ReflectValue("25"), ReflectorMember::ErrorType::None);
  TEST_EQUAL(Z::value_type::ToFloat(lClass.Get<Z>()), 25);
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "z");
  TEST_EQUAL(cPair.value, "25");

  member = sClass["x"];
  cPair = member.ReflectedPair();
  TEST_EQUAL(cPair.name, "x");
  TEST_EQUAL(cPair.value, "104");

  return 0;
}

struct ReflectedInstanceFriend : ReflectedInstance {
  void *Instance() { return instance; }
  const void *Instance() const { return constInstance; }
  const reflectorStatic *Refl() const { return rfStatic; }
};

int test_reflector(reflClass &input) {
  TEST_EQUAL(input.ClassName(), std::string_view("reflClass"));
  ReflectorMember member = input["pest"];

  TEST_EQUAL(member.ReflectValue(""),
             ReflectorMember::ErrorType::InvalidDestination);

  TEST_THROW(std::out_of_range, member = input[200];);
  member = input["non existant"];

  TEST_EQUAL(member.ReflectValue(0.0),
             ReflectorMember::ErrorType::InvalidDestination);
  TEST_EQUAL(member.ReflectValue(0),
             ReflectorMember::ErrorType::InvalidDestination);
  TEST_EQUAL(member.ReflectValue(uint64(0)),
             ReflectorMember::ErrorType::InvalidDestination);
  auto noClass = member.ReflectedSubClass();
  TEST_NOT_CHECK(noClass.data);

  auto noPair = member.ReflectedPair();

  TEST_CHECK(noPair.name.empty());
  TEST_CHECK(noPair.value.empty());

  TEST_NOT_CHECK(member.IsReflectedSubClass());
  TEST_NOT_CHECK(member.IsArray());

  return 0;
}
