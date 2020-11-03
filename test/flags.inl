#include "../datas/flags.hpp"
#include "../datas/unit_testing.hpp"

constexpr auto flgtest00 = es::Flags<uint8>(1, 2);

ES_STATIC_ASSERT(static_cast<uint8>(flgtest00) == 0b110);

int test_flags_00() {
  es::Flags<uint8> flags;
  uint8 &flagsRaw = reinterpret_cast<uint8 &>(flags);

  flags += 0;

  TEST_EQUAL(flagsRaw, 1);
  TEST_EQUAL(flags, 0);
  TEST_NOT_EQUAL(flags, 1);
  TEST_NOT_EQUAL(flags, 2);

  flags += 0;

  TEST_EQUAL(flagsRaw, 1);
  TEST_EQUAL(flags, 0);
  TEST_NOT_EQUAL(flags, 1);
  TEST_NOT_EQUAL(flags, 2);

  flags += 1;

  TEST_EQUAL(flagsRaw, 3);
  TEST_EQUAL(flags, 0);
  TEST_EQUAL(flags, 1);
  TEST_NOT_EQUAL(flags, 2);

  flags -= 0;

  TEST_EQUAL(flagsRaw, 2);
  TEST_NOT_EQUAL(flags, 0);
  TEST_EQUAL(flags, 1);
  TEST_NOT_EQUAL(flags, 2);

  flags -= 0;

  TEST_EQUAL(flagsRaw, 2);
  TEST_NOT_EQUAL(flags, 0);
  TEST_EQUAL(flags, 1);
  TEST_NOT_EQUAL(flags, 2);

  flags += 9; // integer overflow

  TEST_EQUAL(flagsRaw, 2);
  TEST_NOT_EQUAL(flags, 0);
  TEST_EQUAL(flags, 1);
  TEST_NOT_EQUAL(flags, 2);

  return 0;
}

int test_flags_01() {
  enum testEnum : uint8 { flag0, flag1, flag2 };

  es::Flags<testEnum> flags;
  uint8 &flagsRaw = reinterpret_cast<uint8 &>(flags);

  flags += flag0;

  TEST_EQUAL(flagsRaw, 1);
  TEST_EQUAL(flags, flag0);
  TEST_NOT_EQUAL(flags, flag1);
  TEST_NOT_EQUAL(flags, flag2);

  flags += flag0;

  TEST_EQUAL(flagsRaw, 1);
  TEST_EQUAL(flags, flag0);
  TEST_NOT_EQUAL(flags, flag1);
  TEST_NOT_EQUAL(flags, flag2);

  flags += flag1;

  TEST_EQUAL(flagsRaw, 3);
  TEST_EQUAL(flags, flag0);
  TEST_EQUAL(flags, flag1);
  TEST_NOT_EQUAL(flags, flag2);

  flags -= flag0;

  TEST_EQUAL(flagsRaw, 2);
  TEST_NOT_EQUAL(flags, flag0);
  TEST_EQUAL(flags, flag1);
  TEST_NOT_EQUAL(flags, flag2);

  flags -= flag0;

  TEST_EQUAL(flagsRaw, 2);
  TEST_NOT_EQUAL(flags, flag0);
  TEST_EQUAL(flags, flag1);
  TEST_NOT_EQUAL(flags, flag2);

  return 0;
}

int test_flags_02() {
  es::Flags<uint8> flags(1, 3, 5, 7);

  TEST_NOT_EQUAL(flags, 0);
  TEST_EQUAL(flags, 1);
  TEST_NOT_EQUAL(flags, 2);
  TEST_EQUAL(flags, 3);
  TEST_NOT_EQUAL(flags, 4);
  TEST_EQUAL(flags, 5);
  TEST_NOT_EQUAL(flags, 6);
  TEST_EQUAL(flags, 7);

  return 0;
}
