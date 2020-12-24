#pragma once
#include "reflector_decl_io.inl"

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
