#pragma once
#include "reflector_decl_io.inl"

int test_reflector_io(reflClass &rClass) {
  {
    BinWritter mwr("reflected.bin");

    TEST_CHECK(mwr.IsValid());
    TEST_NOT_CHECK(ReflectorBinUtil::Save(rClass, mwr));
  }

  BinReader mrd("reflected.bin");
  reflClass rClass2 = {};

  TEST_CHECK(mrd.IsValid());
  TEST_NOT_CHECK(ReflectorBinUtil::Load(rClass2, mrd));

  return compare_classes(rClass, rClass2);
}
