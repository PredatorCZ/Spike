#include "../datas/flags.hpp"
#include "../datas/unit_testing.hpp"

int test_flags_00() {
  esFlags<uint8> flags;
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
  enum testEnum { flag0, flag1, flag2 };

  esFlags<uint8, testEnum> flags;
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
  esFlags<uint8> flags(1, 3, 5, 7);

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
